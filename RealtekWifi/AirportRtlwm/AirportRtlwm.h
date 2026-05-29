#ifndef AirportRtlwm_h
#define AirportRtlwm_h

#define __IO80211_TARGET __MAC_13_0

#include <IOKit/IOService.h>
#include <IOKit/network/IOEthernetController.h>
#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/IOWorkLoop.h>
#include <IOKit/IOInterruptEventSource.h>
#include <IOKit/IOTimerEventSource.h>

#include <IOKit/80211/IO80211Controller.h>
#include <IOKit/80211/IO80211Interface.h>
#include <IOKit/80211/IO80211VirtualInterface.h>
#include <IOKit/80211/apple80211_ioctl.h>
#include <IOKit/80211/apple80211_var.h>

#include "rtw88/rtw88.h"
#include "rtw89/rtw89.h"
#include "platform/osdep.h"

#define RTW_SCAN_CACHE_SIZE      32

struct rtw_scan_entry {
    uint8_t     bssid[6];
    uint8_t     ssid[32];
    uint8_t     ssid_len;
    uint16_t    channel;
    uint32_t    channel_flags;
    int16_t     rssi;
    uint16_t    beacon_int;
    uint16_t    cap_info;
    int16_t     noise;
    uint32_t    age;
};

struct rtw_conn_info {
    uint8_t     bssid[6];
    uint8_t     ssid[32];
    uint8_t     ssid_len;
    uint16_t    channel;
    uint32_t    channel_flags;
    int16_t     rssi;
    int16_t     noise;
    uint8_t     rate;       /* current TX rate (MCS index or legacy) */
    uint8_t     mcs;        /* MCS index */
    uint32_t    state;      /* apple80211_state */
};

enum ChipGen {
    kGenUnknown = 0,
    kGen8821CE, kGen8822BE, kGen8822CE, kGen8723DE,
    kGen8852AE, kGen8852BE, kGen8852CE, kGen8922AE,
};

class com_realtek_driver_AirportRtlwm : public IO80211Controller
{
    OSDeclareDefaultStructors(com_realtek_driver_AirportRtlwm)

public:
    bool init(OSDictionary *dict) APPLE_KEXT_OVERRIDE;
    bool start(IOService *provider) APPLE_KEXT_OVERRIDE;
    void stop(IOService *provider) APPLE_KEXT_OVERRIDE;
    void free() APPLE_KEXT_OVERRIDE;

    bool createWorkLoop() APPLE_KEXT_OVERRIDE;
    IOWorkLoop *getWorkLoop() const APPLE_KEXT_OVERRIDE;

    IOReturn enable(IONetworkInterface *interface) APPLE_KEXT_OVERRIDE;
    IOReturn disable(IONetworkInterface *interface) APPLE_KEXT_OVERRIDE;
    IOReturn getHardwareAddress(IOEthernetAddress *addr) APPLE_KEXT_OVERRIDE;
    IOService *getProvider() const APPLE_KEXT_OVERRIDE;
    IOOutputQueue *getOutputQueue() const APPLE_KEXT_OVERRIDE;
    bool configureInterface(IONetworkInterface *interface) APPLE_KEXT_OVERRIDE;

    SInt32 apple80211Request(UInt32 req, int type, IO80211Interface *intf, void *data) APPLE_KEXT_OVERRIDE;
    SInt32 stopDMA() APPLE_KEXT_OVERRIDE;
    UInt32 hardwareOutputQueueDepth(IO80211Interface *interface) APPLE_KEXT_OVERRIDE;
    SInt32 performCountryCodeOperation(IO80211Interface *interface, IO80211CountryCodeOp op) APPLE_KEXT_OVERRIDE;
    SInt32 enableFeature(IO80211FeatureCode feature, void *refcon) APPLE_KEXT_OVERRIDE;
    void inputMonitorPacket(mbuf_t m, UInt32 dlt, void *hdr, unsigned long hdrLen) APPLE_KEXT_OVERRIDE;
    IO80211Interface *getNetworkInterface() APPLE_KEXT_OVERRIDE;

    static void interruptOccurred(OSObject *target, IOInterruptEventSource *source, int count);
    static void timerFired(OSObject *target, IOTimerEventSource *source);
    UInt32 outputPacket(mbuf_t m, void *param) APPLE_KEXT_OVERRIDE;

private:
    IOPCIDevice            *fProvider;
    IOWorkLoop             *fWorkLoop;
    IOInterruptEventSource *fInterruptSrc;
    IOTimerEventSource     *fTimerSrc;
    IO80211Interface       *fNetInterface;
    rtw_dev                *fRtwDev;
    rtw89_dev              *fRtw89Dev;
    enum ChipGen            fChipGen;
    bool                    fEnabled;
    bool                    fAssociated;
    uint8_t                 fMacAddr[6];
    volatile void          *fBar0Map;
    volatile void          *fBar1Map;

    struct rtw_scan_entry  fScanCache[RTW_SCAN_CACHE_SIZE];
    int                     fScanCount;
    int                     fScanResultCursor;
    bool                    fScanInProgress;
    struct rtw_conn_info    fConn;
    uint32_t                fPowerState; /* 0=off, 1=on */

    IOReturn handleGet(apple80211req *req);
    IOReturn handleSet(apple80211req *req);
    void scanDone();

    void connUpdateState(uint32_t newState);
    void connSetBSSID(const uint8_t *bssid);
    void connSetSSID(const char *ssid, uint8_t len);
    void connSetChannel(uint16_t ch, uint32_t flags);
    void connUpdateRSSI(int16_t rssi, int16_t noise);
    int  scanAddResult(const uint8_t *bssid, const char *ssid, uint8_t ssid_len,
                       uint16_t ch, uint32_t ch_flags, int16_t rssi,
                       uint16_t beacon_int, uint16_t cap_info);
    void scanFlush();

    bool initPCIConfig();
    bool mapBars();
    void unmapBars();
    bool initHardware();
    void startHardware();
    void stopHardware();
    void handleInterrupt();
    void handleTimer();

    u32 r32(u32 a);
    u16 r16(u32 a);
    u8  r8(u32 a);
    void w32(u32 a, u32 v);
    void w16(u32 a, u16 v);
    void w8(u32 a, u8 v);
};

#endif
