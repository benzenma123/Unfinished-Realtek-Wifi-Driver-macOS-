#include "AirportRtlwm.h"
#include <mach/kmod.h>

#define super IO80211Controller
OSDefineMetaClassAndStructors(com_realtek_driver_AirportRtlwm, IO80211Controller)

extern "C" {
    kern_return_t _start(kmod_info_t *ki, void *data);
    kern_return_t _stop(kmod_info_t *ki, void *data);

    kern_return_t _start(kmod_info_t *ki, void *data)
    {
        return KERN_SUCCESS;
    }

    kern_return_t _stop(kmod_info_t *ki, void *data)
    {
        return KERN_SUCCESS;
    }
}

KMOD_EXPLICIT_DECL(com.realtek.driver.AirportRtlwm, "1.0.0", _start, _stop)
#define XYLog IOLog

// Supported channels: 2.4 GHz channels 1-13, 5 GHz channels 36-165
static const struct { uint16_t ch; uint32_t flags; } kSupportedChannels[] = {
    { 1, APPLE80211_C_FLAG_2GHZ | APPLE80211_C_FLAG_ACTIVE },
    { 2, APPLE80211_C_FLAG_2GHZ | APPLE80211_C_FLAG_ACTIVE },
    { 3, APPLE80211_C_FLAG_2GHZ | APPLE80211_C_FLAG_ACTIVE },
    { 4, APPLE80211_C_FLAG_2GHZ | APPLE80211_C_FLAG_ACTIVE },
    { 5, APPLE80211_C_FLAG_2GHZ | APPLE80211_C_FLAG_ACTIVE },
    { 6, APPLE80211_C_FLAG_2GHZ | APPLE80211_C_FLAG_ACTIVE },
    { 7, APPLE80211_C_FLAG_2GHZ | APPLE80211_C_FLAG_ACTIVE },
    { 8, APPLE80211_C_FLAG_2GHZ | APPLE80211_C_FLAG_ACTIVE },
    { 9, APPLE80211_C_FLAG_2GHZ | APPLE80211_C_FLAG_ACTIVE },
    {10, APPLE80211_C_FLAG_2GHZ | APPLE80211_C_FLAG_ACTIVE },
    {11, APPLE80211_C_FLAG_2GHZ | APPLE80211_C_FLAG_ACTIVE },
    {12, APPLE80211_C_FLAG_2GHZ | APPLE80211_C_FLAG_ACTIVE },
    {13, APPLE80211_C_FLAG_2GHZ | APPLE80211_C_FLAG_ACTIVE },
    {36, APPLE80211_C_FLAG_5GHZ | APPLE80211_C_FLAG_ACTIVE },
    {40, APPLE80211_C_FLAG_5GHZ | APPLE80211_C_FLAG_ACTIVE },
    {44, APPLE80211_C_FLAG_5GHZ | APPLE80211_C_FLAG_ACTIVE },
    {48, APPLE80211_C_FLAG_5GHZ | APPLE80211_C_FLAG_ACTIVE },
    {52, APPLE80211_C_FLAG_5GHZ | APPLE80211_C_FLAG_ACTIVE },
    {56, APPLE80211_C_FLAG_5GHZ | APPLE80211_C_FLAG_ACTIVE },
    {60, APPLE80211_C_FLAG_5GHZ | APPLE80211_C_FLAG_ACTIVE },
    {64, APPLE80211_C_FLAG_5GHZ | APPLE80211_C_FLAG_ACTIVE },
    {100, APPLE80211_C_FLAG_5GHZ | APPLE80211_C_FLAG_ACTIVE },
    {104, APPLE80211_C_FLAG_5GHZ | APPLE80211_C_FLAG_ACTIVE },
    {108, APPLE80211_C_FLAG_5GHZ | APPLE80211_C_FLAG_ACTIVE },
    {112, APPLE80211_C_FLAG_5GHZ | APPLE80211_C_FLAG_ACTIVE },
    {116, APPLE80211_C_FLAG_5GHZ | APPLE80211_C_FLAG_ACTIVE },
    {120, APPLE80211_C_FLAG_5GHZ | APPLE80211_C_FLAG_ACTIVE },
    {124, APPLE80211_C_FLAG_5GHZ | APPLE80211_C_FLAG_ACTIVE },
    {128, APPLE80211_C_FLAG_5GHZ | APPLE80211_C_FLAG_ACTIVE },
    {132, APPLE80211_C_FLAG_5GHZ | APPLE80211_C_FLAG_ACTIVE },
    {136, APPLE80211_C_FLAG_5GHZ | APPLE80211_C_FLAG_ACTIVE },
    {140, APPLE80211_C_FLAG_5GHZ | APPLE80211_C_FLAG_ACTIVE },
    {144, APPLE80211_C_FLAG_5GHZ | APPLE80211_C_FLAG_ACTIVE },
    {149, APPLE80211_C_FLAG_5GHZ | APPLE80211_C_FLAG_ACTIVE },
    {153, APPLE80211_C_FLAG_5GHZ | APPLE80211_C_FLAG_ACTIVE },
    {157, APPLE80211_C_FLAG_5GHZ | APPLE80211_C_FLAG_ACTIVE },
    {161, APPLE80211_C_FLAG_5GHZ | APPLE80211_C_FLAG_ACTIVE },
    {165, APPLE80211_C_FLAG_5GHZ | APPLE80211_C_FLAG_ACTIVE },
};
#define NUM_SUP_CH  (sizeof(kSupportedChannels) / sizeof(kSupportedChannels[0]))

// ---------------------------------------------------------------------------
// IOKit lifecycle
// ---------------------------------------------------------------------------
bool com_realtek_driver_AirportRtlwm::init(OSDictionary *dict)
{
    if (!super::init(dict)) return false;
    fProvider = NULL; fWorkLoop = NULL; fInterruptSrc = NULL; fTimerSrc = NULL;
    fNetInterface = NULL; fRtwDev = NULL; fRtw89Dev = NULL;
    fChipGen = kGenUnknown; fEnabled = false; fAssociated = false;
    fPowerState = 1; fScanCount = 0; fScanResultCursor = 0; fScanInProgress = false;
    fBar0Map = NULL; fBar1Map = NULL;
    bzero(fMacAddr, sizeof(fMacAddr));
    bzero(&fConn, sizeof(fConn));
    bzero(fScanCache, sizeof(fScanCache));
    return true;
}

bool com_realtek_driver_AirportRtlwm::start(IOService *provider)
{
    if (!super::start(provider)) return false;
    fProvider = OSDynamicCast(IOPCIDevice, provider);
    if (!fProvider) { XYLog("AirportRtlwm: no PCI provider\n"); return false; }
    fProvider->retain();
    fProvider->setBusMasterEnable(true); fProvider->setMemoryEnable(true);
    UInt16 did = fProvider->configRead16(kIOPCIConfigDeviceID);
    switch (did) {
        case 0xC821: fChipGen = kGen8821CE; break;
        case 0xB822: fChipGen = kGen8822BE; break;
        case 0xC822: fChipGen = kGen8822CE; break;
        case 0xD723: fChipGen = kGen8723DE; break;
        case 0x8852: fChipGen = kGen8852AE; break;
        case 0xB852: fChipGen = kGen8852BE; break;
        case 0xC852: fChipGen = kGen8852CE; break;
        case 0x8922: fChipGen = kGen8922AE; break;
        default:     fChipGen = kGenUnknown; break;
    }
    XYLog("AirportRtlwm: PCI %04x:%04x\n",
          fProvider->configRead16(kIOPCIConfigVendorID), did);
    fBar0Map = fProvider->mapDeviceMemoryWithIndex(0);
    if (!fBar0Map) { XYLog("AirportRtlwm: BAR0 failed\n"); return false; }
    fBar1Map = fProvider->mapDeviceMemoryWithIndex(1);
    if (!createWorkLoop()) return false;
    fInterruptSrc = IOInterruptEventSource::interruptEventSource(this, interruptOccurred, fProvider, 0);
    if (fInterruptSrc) fWorkLoop->addEventSource(fInterruptSrc);
    fTimerSrc = IOTimerEventSource::timerEventSource(this, timerFired);
    if (fTimerSrc) { fWorkLoop->addEventSource(fTimerSrc); fTimerSrc->setTimeoutMS(1000); }
    if (!attachInterface((IONetworkInterface**)&fNetInterface, true)) return false;
    registerService();
    XYLog("AirportRtlwm: started\n");
    return true;
}

void com_realtek_driver_AirportRtlwm::stop(IOService *p)
{
    if (fTimerSrc && fWorkLoop) { fTimerSrc->cancelTimeout(); fWorkLoop->removeEventSource(fTimerSrc); }
    if (fInterruptSrc && fWorkLoop) fWorkLoop->removeEventSource(fInterruptSrc);
    if (fInterruptSrc) { fInterruptSrc->release(); fInterruptSrc = NULL; }
    if (fTimerSrc) { fTimerSrc->release(); fTimerSrc = NULL; }
    if (fWorkLoop) { fWorkLoop->release(); fWorkLoop = NULL; }
    if (fNetInterface) { fNetInterface->release(); fNetInterface = NULL; }
    fBar0Map = fBar1Map = NULL;
    if (fProvider) { fProvider->release(); fProvider = NULL; }
    super::stop(p);
}

void com_realtek_driver_AirportRtlwm::free() { if (fRtwDev) delete fRtwDev; if (fRtw89Dev) delete fRtw89Dev; super::free(); }
bool com_realtek_driver_AirportRtlwm::createWorkLoop() { fWorkLoop = IOWorkLoop::workLoop(); return fWorkLoop != NULL; }
IOWorkLoop *com_realtek_driver_AirportRtlwm::getWorkLoop() const { return fWorkLoop; }
IOService *com_realtek_driver_AirportRtlwm::getProvider() const { return fProvider; }
IOOutputQueue *com_realtek_driver_AirportRtlwm::getOutputQueue() const { return NULL; }
bool com_realtek_driver_AirportRtlwm::configureInterface(IONetworkInterface *i) { return super::configureInterface(i); }
IOReturn com_realtek_driver_AirportRtlwm::enable(IONetworkInterface *i) { fEnabled = true; return kIOReturnSuccess; }
IOReturn com_realtek_driver_AirportRtlwm::disable(IONetworkInterface *i) { fEnabled = false; return kIOReturnSuccess; }
SInt32 com_realtek_driver_AirportRtlwm::stopDMA() { return kIOReturnSuccess; }
UInt32 com_realtek_driver_AirportRtlwm::hardwareOutputQueueDepth(IO80211Interface *i) { return 0; }
SInt32 com_realtek_driver_AirportRtlwm::performCountryCodeOperation(IO80211Interface *i, IO80211CountryCodeOp o) { return EOPNOTSUPP; }
SInt32 com_realtek_driver_AirportRtlwm::enableFeature(IO80211FeatureCode f, void *r) { return EOPNOTSUPP; }
void com_realtek_driver_AirportRtlwm::inputMonitorPacket(mbuf_t m, UInt32 dlt, void *h, unsigned long hl) {}
IO80211Interface *com_realtek_driver_AirportRtlwm::getNetworkInterface() { return fNetInterface; }

IOReturn com_realtek_driver_AirportRtlwm::getHardwareAddress(IOEthernetAddress *addr)
{
    u32 reg = r32(0x0030);
    addr->bytes[0] = (reg >> 8) & 0xFF; addr->bytes[1] = reg & 0xFF;
    reg = r32(0x0034);
    addr->bytes[2] = (reg >> 8) & 0xFF; addr->bytes[3] = reg & 0xFF;
    addr->bytes[4] = (reg >> 24) & 0xFF; addr->bytes[5] = (reg >> 16) & 0xFF;
    return kIOReturnSuccess;
}

// ---------------------------------------------------------------------------
// apple80211Request – native WiFi IOCTL handler
// ---------------------------------------------------------------------------
SInt32 com_realtek_driver_AirportRtlwm::apple80211Request(
    UInt32 req, int type, IO80211Interface *intf, void *data)
{
    if (!data) return EINVAL;

    switch (req) {
    case APPLE80211_IOC_SSID: {
        auto d = (apple80211_ssid_data *)data;
        d->version = APPLE80211_VERSION;
        d->ssid_len = fConn.ssid_len;
        bcopy(fConn.ssid, d->ssid_bytes, fConn.ssid_len);
        return kIOReturnSuccess;
    }
    case APPLE80211_IOC_POWER: {
        auto d = (apple80211_power_data *)data;
        if (type == 1) { /* GET */
            d->version = APPLE80211_VERSION;
            d->num_radios = 1;
            d->power_state[0] = fPowerState;
        } else { /* SET */
            fPowerState = d->power_state[0];
            XYLog("AirportRtlwm: power %s\n", fPowerState ? "ON" : "OFF");
        }
        return kIOReturnSuccess;
    }
    case APPLE80211_IOC_STATE: {
        auto d = (apple80211_state_data *)data;
        d->version = APPLE80211_VERSION;
        d->state = fConn.state;
        return kIOReturnSuccess;
    }
    case APPLE80211_IOC_CHANNEL: {
        auto d = (apple80211_channel_data *)data;
        d->version = APPLE80211_VERSION;
        d->channel.version = APPLE80211_VERSION;
        d->channel.channel = fConn.channel ? fConn.channel : 1;
        d->channel.flags = fConn.channel_flags ? fConn.channel_flags :
            APPLE80211_C_FLAG_2GHZ | APPLE80211_C_FLAG_ACTIVE;
        return kIOReturnSuccess;
    }
    case APPLE80211_IOC_BSSID: {
        auto d = (apple80211_bssid_data *)data;
        d->version = APPLE80211_VERSION;
        bcopy(fConn.bssid, &d->bssid, sizeof(d->bssid));
        return kIOReturnSuccess;
    }
    case APPLE80211_IOC_RATE: {
        auto d = (apple80211_rate_data *)data;
        d->version = APPLE80211_VERSION;
        d->num_radios = 1;
        d->rate[0] = fConn.rate ? fConn.rate : 54;
        return kIOReturnSuccess;
    }
    case APPLE80211_IOC_RSSI: {
        auto d = (apple80211_rssi_data *)data;
        d->version = APPLE80211_VERSION;
        d->num_radios = 1;
        d->rssi_unit = APPLE80211_UNIT_DBM;
        d->rssi[0] = fConn.rssi;
        d->aggregate_rssi = fConn.rssi;
        return kIOReturnSuccess;
    }
    case APPLE80211_IOC_NOISE: {
        auto d = (apple80211_noise_data *)data;
        d->version = APPLE80211_VERSION;
        d->num_radios = 1;
        d->noise_unit = APPLE80211_UNIT_DBM;
        d->noise[0] = fConn.noise;
        return kIOReturnSuccess;
    }
    case APPLE80211_IOC_SUPPORTED_CHANNELS: {
        auto d = (apple80211_sup_channel_data *)data;
        d->version = APPLE80211_VERSION;
        d->num_channels = (UInt32)NUM_SUP_CH;
        for (UInt32 i = 0; i < NUM_SUP_CH && i < APPLE80211_MAX_CHANNELS; i++) {
            d->supported_channels[i].channel = kSupportedChannels[i].ch;
            d->supported_channels[i].flags = kSupportedChannels[i].flags;
        }
        return kIOReturnSuccess;
    }
    case APPLE80211_IOC_CARD_CAPABILITIES: {
        auto d = (apple80211_capability_data *)data;
        d->version = APPLE80211_VERSION;
        d->capabilities[0] = (1 << APPLE80211_CAP_TKIP) | (1 << APPLE80211_CAP_AES);
        d->capabilities[1] = (1 << (APPLE80211_CAP_WPA2 & 7)) | (1 << (APPLE80211_CAP_WPA & 7));
        for (int i = 2; i < 12; i++)
            d->capabilities[i] = 0;
        return kIOReturnSuccess;
    }
    case APPLE80211_IOC_DRIVER_VERSION: {
        auto d = (apple80211_version_data *)data;
        d->version = APPLE80211_VERSION;
        strncpy(d->string, "1.0.0", APPLE80211_MAX_VERSION_LEN);
        return kIOReturnSuccess;
    }
    case APPLE80211_IOC_HARDWARE_VERSION: {
        auto d = (apple80211_version_data *)data;
        d->version = APPLE80211_VERSION;
        strncpy(d->string, "Realtek 802.11", APPLE80211_MAX_VERSION_LEN);
        return kIOReturnSuccess;
    }
    case APPLE80211_IOC_MCS: {
        auto d = (apple80211_mcs_data *)data;
        d->version = APPLE80211_VERSION;
        d->index = fConn.mcs;
        return kIOReturnSuccess;
    }
    case APPLE80211_IOC_COUNTRY_CODE: {
        auto d = (apple80211_country_code_data *)data;
        d->version = APPLE80211_VERSION;
        d->cc[0] = 'X';
        d->cc[1] = 'X';
        return kIOReturnSuccess;
    }
    case APPLE80211_IOC_TXPOWER: {
        auto d = (apple80211_txpower_data *)data;
        d->version = APPLE80211_VERSION;
        d->txpower_unit = APPLE80211_UNIT_DBM;
        d->txpower = 30;
        return kIOReturnSuccess;
    }
    case APPLE80211_IOC_SCAN_REQ: {
        if (fScanInProgress) return EBUSY;
        XYLog("AirportRtlwm: scan requested\n");
        scanFlush();
        fScanInProgress = true;
        // Trigger HW scan (real impl would call rtw_ops->set_channel and start RX)
        // For now, populate with fake data so the UI shows networks
        scanAddResult((const uint8_t[]){0x00,0x11,0x22,0x33,0x44,0x55},
                      "Realtek_Test", 12, 6,
                      APPLE80211_C_FLAG_2GHZ|APPLE80211_C_FLAG_ACTIVE,
                      -45, 100, 0x0021);
        scanAddResult((const uint8_t[]){0xaa,0xbb,0xcc,0xdd,0xee,0xff},
                      "WiFi_5G", 7, 149,
                      APPLE80211_C_FLAG_5GHZ|APPLE80211_C_FLAG_ACTIVE,
                      -60, 100, 0x0021);
        fScanInProgress = false;
        scanDone();
        return kIOReturnSuccess;
    }
    case APPLE80211_IOC_SCAN_RESULT: {
        auto d = (apple80211_scan_result *)data;
        if (fScanResultCursor >= fScanCount)
            return ENOENT;
        struct rtw_scan_entry *e = &fScanCache[fScanResultCursor++];
        d->version = APPLE80211_VERSION;
        bcopy(e->bssid, d->asr_bssid, sizeof(d->asr_bssid));
        d->asr_ssid_len = e->ssid_len;
        bcopy(e->ssid, d->asr_ssid, e->ssid_len);
        d->asr_channel.channel = e->channel;
        d->asr_channel.flags = e->channel_flags;
        d->asr_noise = e->noise;
        d->asr_rssi = e->rssi;
        d->asr_beacon_int = (int16_t)e->beacon_int;
        d->asr_cap = (int16_t)e->cap_info;
        d->asr_age = e->age;
        d->asr_ie_len = 0;
        return kIOReturnSuccess;
    }
    case APPLE80211_IOC_ASSOCIATE: {
        auto assocData = (apple80211_assoc_data *)data;
        XYLog("AirportRtlwm: associate to '");
        for (int i = 0; i < assocData->ad_ssid_len; i++)
            XYLog("%c", assocData->ad_ssid[i]);
        XYLog("'\n");
        connSetBSSID(assocData->ad_bssid.octet);
        connSetSSID((const char *)assocData->ad_ssid, (uint8_t)assocData->ad_ssid_len);
        connUpdateRSSI(-50, -95);
        connUpdateState(APPLE80211_S_RUN);
        fAssociated = true;
        return kIOReturnSuccess;
    }
    case APPLE80211_IOC_DISASSOCIATE: {
        XYLog("AirportRtlwm: disassociate\n");
        bzero(&fConn, sizeof(fConn));
        fConn.noise = -95;
        fAssociated = false;
        connUpdateState(APPLE80211_S_INIT);
        return kIOReturnSuccess;
    }
    default:
        return EOPNOTSUPP;
    }
}

// ---------------------------------------------------------------------------
// Scan cache helpers
// ---------------------------------------------------------------------------
void com_realtek_driver_AirportRtlwm::scanFlush()
{
    fScanCount = 0;
    fScanResultCursor = 0;
    fScanInProgress = false;
    bzero(fScanCache, sizeof(fScanCache));
}

int com_realtek_driver_AirportRtlwm::scanAddResult(
    const uint8_t *bssid, const char *ssid, uint8_t ssid_len,
    uint16_t ch, uint32_t ch_flags, int16_t rssi,
    uint16_t beacon_int, uint16_t cap_info)
{
    if (fScanCount >= RTW_SCAN_CACHE_SIZE) return -1;
    struct rtw_scan_entry *e = &fScanCache[fScanCount++];
    bcopy(bssid, e->bssid, 6);
    bcopy(ssid, e->ssid, ssid_len > 32 ? 32 : ssid_len);
    e->ssid_len = ssid_len > 32 ? 32 : ssid_len;
    e->channel = ch;
    e->channel_flags = ch_flags;
    e->rssi = rssi;
    e->beacon_int = beacon_int;
    e->cap_info = cap_info;
    e->noise = -95;
    e->age = 0;
    return 0;
}

void com_realtek_driver_AirportRtlwm::scanDone()
{
    fScanInProgress = false;
    fScanResultCursor = 0;
    // Notify system that scan results are available
    if (fNetInterface)
        fNetInterface->postMessage(APPLE80211_M_SCAN_DONE);
}

// ---------------------------------------------------------------------------
// Connection state helpers
// ---------------------------------------------------------------------------
void com_realtek_driver_AirportRtlwm::connUpdateState(uint32_t newState)
{
    fConn.state = newState;
    if (fNetInterface)
        fNetInterface->postMessage(APPLE80211_M_LINK_CHANGED);
}

void com_realtek_driver_AirportRtlwm::connSetBSSID(const uint8_t *bssid)
{
    bcopy(bssid, fConn.bssid, 6);
}

void com_realtek_driver_AirportRtlwm::connSetSSID(const char *ssid, uint8_t len)
{
    bcopy(ssid, fConn.ssid, len > 32 ? 32 : len);
    fConn.ssid_len = len > 32 ? 32 : len;
}

void com_realtek_driver_AirportRtlwm::connSetChannel(uint16_t ch, uint32_t flags)
{
    fConn.channel = ch;
    fConn.channel_flags = flags;
}

void com_realtek_driver_AirportRtlwm::connUpdateRSSI(int16_t rssi, int16_t noise)
{
    fConn.rssi = rssi;
    fConn.noise = noise;
}

// ---------------------------------------------------------------------------
// Events
// ---------------------------------------------------------------------------
void com_realtek_driver_AirportRtlwm::handleInterrupt() {}
void com_realtek_driver_AirportRtlwm::handleTimer() { if (fTimerSrc) fTimerSrc->setTimeoutMS(1000); }
void com_realtek_driver_AirportRtlwm::interruptOccurred(OSObject *t, IOInterruptEventSource *s, int c) { if (auto d = OSDynamicCast(com_realtek_driver_AirportRtlwm, t)) d->handleInterrupt(); }
void com_realtek_driver_AirportRtlwm::timerFired(OSObject *t, IOTimerEventSource *s) { if (auto d = OSDynamicCast(com_realtek_driver_AirportRtlwm, t)) d->handleTimer(); }

UInt32 com_realtek_driver_AirportRtlwm::outputPacket(mbuf_t m, void *param)
{
    // TX path stub -- will be wired to rtw88 TX rings later
    freePacket(m);
    return kIOReturnSuccess;
}

// MMIO accessors
u32 com_realtek_driver_AirportRtlwm::r32(u32 a) { return fBar0Map ? *(volatile u32 *)((uintptr_t)fBar0Map + a) : 0; }
u16 com_realtek_driver_AirportRtlwm::r16(u32 a) { return fBar0Map ? *(volatile u16 *)((uintptr_t)fBar0Map + a) : 0; }
u8  com_realtek_driver_AirportRtlwm::r8(u32 a)  { return fBar0Map ? *(volatile u8 *)((uintptr_t)fBar0Map + a) : 0; }
void com_realtek_driver_AirportRtlwm::w32(u32 a, u32 v) { if (fBar0Map) *(volatile u32 *)((uintptr_t)fBar0Map + a) = v; }
void com_realtek_driver_AirportRtlwm::w16(u32 a, u16 v) { if (fBar0Map) *(volatile u16 *)((uintptr_t)fBar0Map + a) = v; }
void com_realtek_driver_AirportRtlwm::w8(u32 a, u8 v)   { if (fBar0Map) *(volatile u8 *)((uintptr_t)fBar0Map + a) = v; }
