#include "R_DSP/SimpleDMA.hh"
#include "base/trace.hh"
#include "mem/packet.hh"
#include "mem/packet_access.hh"

namespace gem5
{

SimpleDMA::SimpleDMA(const SimpleDMAParams &p)
    : DmaDevice(p),
      srcReg(0),
      dstReg(0),
      lenReg(0),
      ctrlReg(0),
      statusReg(0),
      busy(false),
      dmaEvent([this]{ doDma(); }, name()),
      dmaReadDoneEvent([this]{ dmaReadDone(); }, name()+".readDone"),
      dmaWriteDoneEvent([this]{ dmaWriteDone(); }, name()+".writeDone"),
      dmaBuffer(nullptr)
{
}

Tick
SimpleDMA::write(PacketPtr pkt)
{
    const auto &p = static_cast<const SimpleDMAParams&>(params());
    Addr offset = pkt->getAddr() - p.pio_addr;
    uint32_t val = pkt->getLE<uint32_t>();

    switch (offset) {

        case 0x00:  // SRC
            srcReg = val;
            break;

        case 0x08:  // DST
            dstReg = val;
            break;

        case 0x10:  // LEN
            lenReg = val;
            break;

        case 0x18:  // CTRL
            ctrlReg = val;

            if (val & CTRL_START && !busy) {
                startDma();
            }
            break;

        default:
            panic("Invalid write offset");
    }
    pkt->makeResponse();
    return p.pio_delay;
}

Tick
SimpleDMA::read(PacketPtr pkt)
{
    const auto &p = static_cast<const SimpleDMAParams&>(params());
    Addr offset = pkt->getAddr() - p.pio_addr;
    uint32_t val = 0;

    switch (offset) {

        case 0x00:
            val = srcReg;
            break;

        case 0x08:
            val = dstReg;
            break;

        case 0x10:
            val = lenReg;
            break;

        case 0x18:
            val = ctrlReg;
            break;

        case 0x20:
            val = statusReg;
            break;

        default:
            panic("Invalid read offset");
    }

    pkt->setLE<uint32_t>(val);
    pkt->makeResponse();
    return p.pio_delay;
}

void
SimpleDMA::startDma()
{
    busy = true;
    statusReg = STATUS_BUSY;

    schedule(dmaEvent, curTick());
}

void
SimpleDMA::doDma()
{
    if (lenReg == 0) {
        busy = false;
        statusReg = STATUS_DONE;
        return;
    }

    const int chunk = std::min(64U, lenReg);

    uint8_t *buffer = new uint8_t[chunk];

    dmaRead(
        srcReg,
        chunk,
        &dmaReadDoneEvent,
        dmaBuffer
    );
}

void 
SimpleDMA::dmaReadDone(){
    const int chunk = std::min(64U, lenReg);
    dmaWrite(
        dstReg,
        chunk,
        &dmaWriteDoneEvent,
        dmaBuffer
    );
}

void 
SimpleDMA::dmaWriteDone(){
    const int chunk = std::min(64U, lenReg);
    delete[] dmaBuffer;
    srcReg += chunk;
    dstReg += chunk;
    lenReg += chunk;
    schedule(dmaEvent, curTick() + cyclesToTicks(Cycles(1)));
}

AddrRangeList 
SimpleDMA::getAddrRanges() const {
    const auto &p = static_cast<const SimpleDMAParams&>(params());
    return { RangeSize( p.pio_addr, p.pio_size) } ;
}

}