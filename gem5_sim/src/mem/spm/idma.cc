#include "idma.hh"
#include "mem/packet_access.hh"
#include "debug/IDMA.hh"
#include "sim/full_system.hh"
#include "sim/process.hh"

namespace gem5 {

IDMA::IDMA(const Params &p) 
    : DmaVirtDevice(p),
      regBase(0x80060000),  // 寄存器基地址，可以从参数中获取
      srcAddrReg(0),
      dstAddrReg(0),
      sizeReg(0),
      commandReg(0),
      statusReg(0)
{
    dmaBuffer = new uint8_t[1024 * 1024];
}

IDMA::~IDMA() {
    if (dmaBuffer) {
        delete[] dmaBuffer;
    }
}

TranslationGenPtr
IDMA::translate(Addr vaddr, Addr size)
{
    if (!FullSystem) {
        // Syscall-emulation 模式下，使用进程页表做虚实地址转换
        auto process = sys->threads[0]->getProcessPtr();
        return process->pTable->translateRange(vaddr, size);
    }

    // FullSystem 模式目前未实现 IDMA 的地址翻译
    panic("IDMA::translate not implemented for full-system mode.\n");
}

AddrRangeList IDMA::getAddrRanges() const {
    AddrRangeList ranges;
    // 返回寄存器地址范围（例如：4KB 的寄存器空间）
    ranges.push_back(RangeSize(regBase, 0x1000));
    return ranges;
}

// CPU读IDMA寄存器
// IDMA继承自DmaDevice，DmaDevice继承自PioDevice，read是PioDevice的纯虚函数
Tick IDMA::read(PacketPtr pkt) {
    // 1. 获取访问的地址
    Addr addr = pkt->getAddr();
    
    // DPRINTF(IDMA, "Current Regs - Src: %#x, Dst: %#x, Size: %#x, Command: %#x, Status: %#x\n", srcAddrReg, dstAddrReg, sizeReg, commandReg, statusReg);
    // 2. 计算寄存器偏移
    Addr offset = addr - regBase;
    DPRINTF(IDMA,"XWY, I've received ReadRequest from 0x%x, offset: %x\n", pkt->getAddr(), offset);

    // 3. 根据偏移地址读取相应的寄存器
    switch (offset) {
      case REG_STATUS_OFFSET:
        if (pkt->getSize() == sizeof(uint32_t)) {
            pkt->setLE<uint32_t>(statusReg);
        } else {
            panic("Invalid access size for STATUS register: %d\n", pkt->getSize());
        }
        break;
        
      case REG_SRC_ADDR_OFFSET:
        if (pkt->getSize() == sizeof(uint64_t)) {
            pkt->setLE<uint64_t>(srcAddrReg);
        } else if (pkt->getSize() == sizeof(uint32_t)) {
            // 读取低 32 位
            pkt->setLE<uint32_t>(static_cast<uint32_t>(srcAddrReg));
        } else {
            panic("Invalid access size for SRC_ADDR register: %d\n", pkt->getSize());
        }
        break;
        
      case REG_DST_ADDR_OFFSET:
        if (pkt->getSize() == sizeof(uint64_t)) {
            pkt->setLE<uint64_t>(dstAddrReg);
        } else if (pkt->getSize() == sizeof(uint32_t)) {
            pkt->setLE<uint32_t>(static_cast<uint32_t>(dstAddrReg));
        } else {
            panic("Invalid access size for DST_ADDR register: %d\n", pkt->getSize());
        }
        break;
        
      case REG_SIZE_OFFSET:
        if (pkt->getSize() == sizeof(uint32_t)) {
            pkt->setLE<uint32_t>(sizeReg);
        } else {
            panic("Invalid access size for LENGTH register: %d\n", pkt->getSize());
        }
        break;
        
      case REG_COMMAND_OFFSET:
        if (pkt->getSize() == sizeof(uint32_t)) {
            pkt->setLE<uint32_t>(commandReg);
        } else {
            panic("Invalid access size for COMMAND register: %d\n", pkt->getSize());
        }
        break;
        
      default:
        warn("Read from unknown register offset: 0x%x\n", offset);
        // 返回 0
        if (pkt->getSize() == sizeof(uint32_t)) {
            pkt->setLE<uint32_t>(0);
        } else if (pkt->getSize() == sizeof(uint64_t)) {
            pkt->setLE<uint64_t>(0);
        }
        break;
    }
    
    // 4. 标记 packet 为响应
    pkt->makeResponse();
    
    // 5. 返回访问延迟（单位：Tick）
    return 10;  // 你可以从参数中获取 pioDelay
}

// CPU写IDMA寄存器
Tick IDMA::write(PacketPtr pkt) {
    // 1. 获取访问的地址
    Addr addr = pkt->getAddr();

    // 2. 计算寄存器偏移
    Addr offset = addr - regBase;
    DPRINTF(IDMA, "Writing to IDMA regs, Address: %#x, Offset: %#x\n", addr, offset);

    // 3. 根据偏移地址写入相应的寄存器
    switch (offset) {
      case REG_STATUS_OFFSET:
        // 状态寄存器通常是只读的，但这里允许写入用于测试
        if (pkt->getSize() == sizeof(uint32_t)) {
            statusReg = pkt->getLE<uint32_t>();
            DPRINTF(IDMA, "Writing to STATUS register, Value: %#x\n", statusReg);
        } else {
            panic("Invalid access size for STATUS register: %d\n", pkt->getSize());
        }
        break;
        
      case REG_SRC_ADDR_OFFSET:
        if (pkt->getSize() == sizeof(uint64_t)) {
            srcAddrReg = pkt->getLE<uint64_t>();
        } else if (pkt->getSize() == sizeof(uint32_t)) {
            // 写入低 32 位（假设是低地址部分）
            srcAddrReg = (srcAddrReg & 0xFFFFFFFF00000000ULL) | 
                         pkt->getLE<uint32_t>();
            DPRINTF(IDMA, "Writing to SRC_ADDR register, Value: %#x\n", srcAddrReg);
        } else {
            panic("Invalid access size for SRC_ADDR register: %d\n", pkt->getSize());
        }
        break;
        
      case REG_DST_ADDR_OFFSET:
        if (pkt->getSize() == sizeof(uint64_t)) {
            dstAddrReg = pkt->getLE<uint64_t>();
        } else if (pkt->getSize() == sizeof(uint32_t)) {
            dstAddrReg = (dstAddrReg & 0xFFFFFFFF00000000ULL) | 
                         pkt->getLE<uint32_t>();
            DPRINTF(IDMA, "Writing to DST_ADDR register, Value: %#x\n", dstAddrReg);
        } else {
            panic("Invalid access size for DST_ADDR register: %d\n", pkt->getSize());
        }
        break;
        
      case REG_SIZE_OFFSET:
        if (pkt->getSize() == sizeof(uint32_t)) {
            sizeReg = pkt->getLE<uint32_t>();
            DPRINTF(IDMA, "Writing to SIZE register, Value: %#x\n", sizeReg);
        } else {
            panic("Invalid access size for LENGTH register: %d\n", pkt->getSize());
        }
        break;
        
      case REG_COMMAND_OFFSET:
        if (pkt->getSize() == sizeof(uint32_t)) {
            commandReg = pkt->getLE<uint32_t>();
            DPRINTF(IDMA, "Writing to COMMAND register, Value: %#x\n", commandReg);
            // 根据命令寄存器启动 DMA 传输
            if (commandReg & 0x1) {
                idmaTransfer();
            }
        } else {
            panic("Invalid access size for COMMAND register: %d\n", pkt->getSize());
        }
        break;
        
      default:
        warn("Write to unknown register offset: 0x%x\n", offset);
        break;
    }
    
    // 4. 标记 packet 为响应
    pkt->makeResponse();
    
    // 5. 返回访问延迟
    return 10;
}
void IDMA::idmaTransfer() {
    uint32_t size = sizeReg;
    uint32_t srcAddr = srcAddrReg;
    uint32_t dstAddr = dstAddrReg;
    uint32_t command = commandReg;
    uint32_t status = statusReg;

    // command = 1使能IDMA传输
    if (command & 0x1) {
        // 读操作

        if(dmaPending()) {
            warn("DMA transfer already in progress!");
            return;
        }
        // 切换到busy状态
        statusReg = IDMA_BUSY;

        // 为本次读传输创建回调对象，完成后调用 idmaReadDone
        auto *readCb = new DmaVirtCallback<int>(
            [this](const int &) { idmaReadDone(); });
        dmaReadVirt(srcAddr, size, readCb, dmaBuffer);

    }
}
void IDMA::idmaReadDone() {
    // 将数据从缓冲区写入目标地址
    DPRINTF(IDMA, "idmaReadDone!calling dmaWrite(), Src: %#x, Dst: %#x, Size: %#x, Command: %#x, Status: %#x\n", srcAddrReg, dstAddrReg, sizeReg, commandReg, statusReg);

    // 为写传输创建回调对象，完成后调用 idmaWriteDone
    auto *writeCb = new DmaVirtCallback<int>(
        [this](const int &) { idmaWriteDone(); });
    dmaWriteVirt(dstAddrReg, sizeReg, writeCb, dmaBuffer);
}

void IDMA::idmaWriteDone() {
    // 传输完成！
    DPRINTF(IDMA, "idmaWriteDone! Src: %#x, Dst: %#x, Size: %#x, Command: %#x, Status: %#x\n", srcAddrReg, dstAddrReg, sizeReg, commandReg, statusReg);
    statusReg = IDMA_COMPLETE;
    // 可以触发中断或通知 CPU
}


}