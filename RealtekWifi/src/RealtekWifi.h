#ifndef RealtekWifi_h
#define RealtekWifi_h

#include <IOKit/IOService.h>
#include <IOKit/network/IOEthernetController.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/IOWorkLoop.h>
#include <IOKit/IOInterruptEventSource.h>
#include <IOKit/IOTimerEventSource.h>
#include <IOKit/IOBufferMemoryDescriptor.h>

#include "platform/osdep.h"

struct rtw89_dev;

class com_realtek_driver_RealtekWifi : public IOEthernetController
{
    OSDeclareDefaultStructors(com_realtek_driver_RealtekWifi)

public:
    bool init(OSDictionary *dict) APPLE_KEXT_OVERRIDE;
    bool start(IOService *provider) APPLE_KEXT_OVERRIDE;
    void stop(IOService *provider) APPLE_KEXT_OVERRIDE;

    bool configureInterface(IONetworkInterface *interface) APPLE_KEXT_OVERRIDE;
    IOReturn enable(IONetworkInterface *interface) APPLE_KEXT_OVERRIDE;
    IOReturn disable(IONetworkInterface *interface) APPLE_KEXT_OVERRIDE;

    const OSString *newVendorString() const APPLE_KEXT_OVERRIDE;
    const OSString *newModelString() const APPLE_KEXT_OVERRIDE;

    IOReturn getHardwareAddress(IOEthernetAddress *addr) APPLE_KEXT_OVERRIDE;
    IOReturn getMaxPacketSize(UInt32 *maxSize) const APPLE_KEXT_OVERRIDE;
    IOReturn getMinPacketSize(UInt32 *minSize) const APPLE_KEXT_OVERRIDE;
    UInt32 outputPacket(mbuf_t m, void *param) APPLE_KEXT_OVERRIDE;

    static void interruptOccurred(OSObject *target, IOInterruptEventSource *source, int count);
    static void timerFired(OSObject *target, IOTimerEventSource *source);

private:
    IOPCIDevice            *fProvider;
    IOWorkLoop             *fWorkLoop;
    IOInterruptEventSource *fInterruptSrc;
    IOTimerEventSource     *fTimerSrc;
    IOEthernetInterface    *fNetInterface;
    volatile void          *fBar0Map;

    rtw89_dev              *fRtw89Dev;

    IOBufferMemoryDescriptor *fCh12BdDesc;
    IOBufferMemoryDescriptor *fCh12BufDesc;

    void handleTimer();
};

#endif
