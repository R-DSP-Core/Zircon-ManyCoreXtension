/*
 * Copyright (c) 2015. Markos Horro
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Markos Horro
 *
 */

 #include "base/random.hh"
 #include "mem/spm/spm.hh"
 #include "debug/Drain.hh"
 #include "debug/ScratchpadMemory.hh"
#include "spm.hh"
#include <iostream>
 
namespace gem5 {
namespace memory {

    // ScratchpadMemory下的DmaPort构造函数
    // _name 在脚本里对应为字符串“system.lxx_spm.dma_port”
    ScratchpadMemory::DmaPort::DmaPort(const std::string& _name, ScratchpadMemory& _memory)
        : ResponsePort(_name), owner(_memory),
        retryResp(false),
        dmaDequeueEvent([this]{ dmaDequeue(); }, name() + ".dmaDequeue")
    { 
        std::cout<<_name << " DmaPort constructor" << std::endl;
    }

    // 构造函数
    ScratchpadMemory::ScratchpadMemory(const Params &p) 
        : SimpleMemory(p),
          dmaPort(name() + ".dma_port", *this)
    {
    }

    void
    ScratchpadMemory::init()
    {
        SimpleMemory::init();
        
        // Notify crossbar about dma_port address ranges
        if (dmaPort.isConnected()) {
            dmaPort.sendRangeChange();
        }
    }

    Port &
    ScratchpadMemory::getPort(const std::string &if_name, PortID idx)
    {
        if (if_name == "dma_port") {
            return dmaPort;
        } else {
            return SimpleMemory::getPort(if_name, idx);
        }
    }

    void
    ScratchpadMemory::regStats()
    {
        // using namespace Stats;

        SimpleMemory::regStats();
    }

    // Public method for DmaPort to process requests
    // 这里的this是ScratchpadMemory，而非ScratchpadMemory::DmaPort，没有重写recvAtomic方法，这里调用的是继承自SimpleMemory的recvAtomic方法
    Tick
    ScratchpadMemory::processDmaRequest(PacketPtr pkt)
    {
        // Use the parent class's recvAtomic to process the packet
        // This will access memory and create response
        return recvAtomic(pkt);
    }

    // Public method for DmaPort to schedule events
    void
    ScratchpadMemory::scheduleDmaEvent(Event &event, Tick when)
    {
        schedule(event, when);
    }
    

    void
    ScratchpadMemory::DmaPort::dmaDequeue()
    {
        if (dmaPacketQueue.empty()) {
            DPRINTF(ScratchpadMemory, "In dmaDequeue, dmaPacketQueue is empty, returning\n");
            return;
        }

        // Get the first packet from the queue
        auto [pkt, ready_time] = dmaPacketQueue.front();

        // Try to send the response
        if (sendTimingResp(pkt)) {
            // Success - remove from queue
            dmaPacketQueue.pop_front();
            retryResp = false;

            // If there are more packets, schedule next dequeue
            if (!dmaPacketQueue.empty()) {
                auto [next_pkt, next_time] = dmaPacketQueue.front();
                if (!dmaDequeueEvent.scheduled()) {
                    // Make sure we don't schedule in the past
                    Tick schedule_time = std::max(next_time, curTick() + 1);
                    owner.scheduleDmaEvent(dmaDequeueEvent, schedule_time);
                }
            }
        } else {
            // Failed - wait for retry
            retryResp = true;
            DPRINTF(ScratchpadMemory, "In dmaDequeue, failed to send timing response, setting retryResp to %d\n", retryResp);

        }
    }

    // 实现虚函数
    AddrRangeList
    ScratchpadMemory::DmaPort::getAddrRanges() const
    {
        return { owner.getAddrRange() };
    
    }

    // ScratchpadMemory::DmaPort下的recvAtomic方法实现
    // 这里的this是ScratchpadMemory::DmaPort，没有重写recvAtomic方法，这里调用的是继承自ResponsePort的recvAtomic方法
    Tick
    ScratchpadMemory::DmaPort::recvAtomic(PacketPtr pkt)
    {
        return owner.recvAtomic(pkt); // 转发给 owner 处理
    }

    void
    ScratchpadMemory::DmaPort::recvFunctional(PacketPtr pkt)
    {
        owner.recvFunctional(pkt); // 转发给 owner 处理
    }

    bool
    ScratchpadMemory::DmaPort::recvTimingReq(PacketPtr pkt)
    {
        // Check if we're currently blocked waiting for retry
        DPRINTF(ScratchpadMemory, "Receiving cmd:%s towards %#x, size: %d\n", pkt->cmdString(),  pkt->getAddr(), pkt->getSize());

        // Note: We should NOT reject new requests just because retryResp is true.
        // retryResp only affects response sending, not request receiving.
        // Rejecting requests here causes deadlock when the same master has both
        // pending responses and new requests (e.g., DMA with multiple transfers).

        // Process the packet similar to SimpleMemory
        bool needs_response = pkt->needsResponse();

        // Use owner's public method to process the packet
        // This will access memory and create response
        Tick latency = owner.processDmaRequest(pkt);

        if (needs_response) {
            // Calculate when to send response
            Tick when_to_send = curTick() + latency;

            // Add to response queue
            dmaPacketQueue.push_back(std::make_pair(pkt, when_to_send));

            // Schedule dequeue event if not already scheduled
            if (!retryResp && !dmaDequeueEvent.scheduled()) {
                owner.scheduleDmaEvent(dmaDequeueEvent, when_to_send);
            }
        }

        return true;
    }

    void
    ScratchpadMemory::DmaPort::recvRespRetry()
    {
        // When peer is ready to receive responses again
        assert(retryResp);
        retryResp = false;
        dmaDequeue(); // Retry sending queued packets
    }

}
}
