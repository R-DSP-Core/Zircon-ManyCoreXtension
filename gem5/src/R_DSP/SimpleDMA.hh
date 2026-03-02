#ifndef __DEV_SIMPLE_DMA_HH__
#define __DEV_SIMPLE_DMA_HH__

#include "dev/dma_device.hh"
#include "params/SimpleDMA.hh"

namespace gem5
{
class SimpleDMA : public DmaDevice
{
  private:

    // Registers
    Addr srcReg;
    Addr dstReg;
    uint32_t lenReg;
    uint32_t ctrlReg;
    uint32_t statusReg;

    bool busy;

    static const uint32_t CTRL_START = 0x1;
    static const uint32_t STATUS_BUSY = 0x1;
    static const uint32_t STATUS_DONE = 0x2;

    EventFunctionWrapper dmaReadDoneEvent;
    EventFunctionWrapper dmaWriteDoneEvent;
    EventFunctionWrapper dmaEvent;
    uint8_t* dmaBuffer;

    void startDma();
    void doDma();

    void dmaReadDone();
    void dmaWriteDone();

  public:
    SimpleDMA(const SimpleDMAParams &p);

    Tick read(PacketPtr pkt) override;
    Tick write(PacketPtr pkt) override;

    AddrRangeList getAddrRanges() const override;
};
}
#endif