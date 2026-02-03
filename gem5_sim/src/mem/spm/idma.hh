#ifndef __IDMA_HH__ 
#define __IDMA_HH__
#define IDMA_BUSY 1
#define IDMA_COMPLETE 2

#include "dev/dma_virt_device.hh"
#include "params/IDMA.hh"

namespace gem5 {

class IDMA : public DmaVirtDevice 
{
  protected:
    // 寄存器基地址（从配置中获取，或使用固定值）
    Addr regBase;
    
    // DMA 控制寄存器
    uint32_t srcAddrReg;     // 源地址寄存器
    uint32_t dstAddrReg;     // 目标地址寄存器
    uint32_t sizeReg;        // 传输长度寄存器
    uint32_t commandReg;     // 命令寄存器
    uint32_t statusReg;      // 状态寄存器

    // DMA 传输缓冲区
    uint8_t *dmaBuffer;

    // 寄存器偏移地址（相对于基地址）
    static const Addr REG_SRC_ADDR_OFFSET = 0x00;
    static const Addr REG_DST_ADDR_OFFSET = 0x04;
    static const Addr REG_SIZE_OFFSET = 0x08;
    static const Addr REG_COMMAND_OFFSET = 0x0C;
    static const Addr REG_STATUS_OFFSET = 0x10;

  public:
    PARAMS(IDMA);
    IDMA(const Params &p);
    ~IDMA();

    AddrRangeList getAddrRanges() const override;
    Tick read(PacketPtr pkt) override;
    Tick write(PacketPtr pkt) override;
    TranslationGenPtr translate(Addr vaddr, Addr size) override;
    void idmaTransfer();
    void idmaReadDone();
    void idmaWriteDone();
};

};
#endif