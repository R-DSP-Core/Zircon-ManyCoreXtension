/*
 * Copyright (c) 2015. Markos Horro
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
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

/**
 * @file
 * ScratchpadMemory declaration, based on SimpleMemory
 */

 #ifndef __SCRATCHPAD_MEMORY_HH__
 #define __SCRATCHPAD_MEMORY_HH__
 
 #include <deque>
 
 #include "base/statistics.hh"
 #include "mem/simple_mem.hh"
 #include "mem/port.hh"
 #include "params/ScratchpadMemory.hh"
 
 namespace gem5
{

namespace memory
{

 /**
  * This definition of scratchpad just adds stats to output
  *
  * @sa  \ref gem5MemorySystem "gem5 Memory System"
  */
 class ScratchpadMemory : public SimpleMemory
 {
   protected:
     /** DMA port for accessing next-level memory */
     class DmaPort : public ResponsePort
     {
       private:
         ScratchpadMemory &owner;
         
         // Queue for outgoing responses
         std::deque<std::pair<PacketPtr, Tick>> dmaPacketQueue;
         
         // Whether we are waiting for a retry from the peer
         bool retryResp;
         
         // Event to dequeue packets
         EventFunctionWrapper dmaDequeueEvent;

         void dmaDequeue(); 

       public:
         DmaPort(const std::string &name, ScratchpadMemory &_owner);

         AddrRangeList getAddrRanges() const override;
         Tick recvAtomic(PacketPtr pkt) override;
         bool recvTimingReq(PacketPtr pkt) override;
         void recvFunctional(PacketPtr pkt) override;
         void recvRespRetry() override;
     };
 
     DmaPort dmaPort;

   public:
     PARAMS(ScratchpadMemory);
     void regStats();
     ScratchpadMemory(const Params &p);
     void init() override;

     Port &getPort(const std::string &if_name,
                   PortID idx=InvalidPortID) override;

     // Public methods for DmaPort to access
     Tick processDmaRequest(PacketPtr pkt);
     void scheduleDmaEvent(Event &event, Tick when);
 };
}
}
 #endif //__SCRATCHPAD_MEMORY_HH__
