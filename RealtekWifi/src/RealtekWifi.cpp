#include "RealtekWifi.h"
#include "rtw89/rtw89.h"
#include "rtw89/rtw8852b.h"
#include "fw/rtw8852b_fw.h"
#include <net/ethernet.h>
#include <sys/kpi_mbuf.h>
#include <sys/errno.h>
#include <mach/kmod.h>

#define super IOEthernetController

OSDefineMetaClassAndStructors(com_realtek_driver_RealtekWifi, IOEthernetController)

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

KMOD_EXPLICIT_DECL(com.realtek.driver.RealtekWifi, "1.0.0", _start, _stop)

bool com_realtek_driver_RealtekWifi::init(OSDictionary *dict)
{
    if (!super::init(dict)) return false;
    fProvider = NULL;
    fWorkLoop = NULL;
    fInterruptSrc = NULL;
    fTimerSrc = NULL;
    fNetInterface = NULL;
    fBar0Map = NULL;
    fRtw89Dev = NULL;
    fCh12BdDesc = NULL;
    fCh12BufDesc = NULL;
    return true;
}

bool com_realtek_driver_RealtekWifi::start(IOService *provider)
{
    if (!super::start(provider)) return false;

    fProvider = OSDynamicCast(IOPCIDevice, provider);
    if (!fProvider) return false;
    fProvider->retain();

    fProvider->setBusMasterEnable(true);
    fProvider->setMemoryEnable(true);

    /* Map the correct BAR: Linux uses BIT(2) = BAR2 (0x18),
     * which is mapDeviceMemoryWithIndex(1) in IOKit.
     * BAR index 0 is 256 bytes — too small.
     * BAR index 1 is 1MB — the full register space. */
    {
        IOMemoryMap *map = fProvider->mapDeviceMemoryWithIndex(1);
        if (!map) return false;
        fBar0Map = reinterpret_cast<volatile void *>(map->getVirtualAddress());
        setProperty("Step6_BAR_size", OSNumber::withNumber(map->getLength(), 64));
        setProperty("Step6_BAR_addr", OSNumber::withNumber(map->getPhysicalAddress(), 64));
    }

    /* Allocate rtw89_dev, set mmio, register chip */
    fRtw89Dev = new rtw89_dev();
    if (!fRtw89Dev) return false;
    memset(fRtw89Dev, 0, sizeof(rtw89_dev));
    fRtw89Dev->mmio = (void *)fBar0Map;
    rtw8852b_register_chip(fRtw89Dev);
    rtw89_core_init(fRtw89Dev);

    fWorkLoop = IOWorkLoop::workLoop();
    if (!fWorkLoop) return false;

    for (int i = 0; i < 4; i++) {
        int intrType = 0;
        if (kIOReturnSuccess == fProvider->getInterruptType(i, &intrType)) {
            fInterruptSrc = IOInterruptEventSource::interruptEventSource(
                this, interruptOccurred, fProvider, i);
            if (fInterruptSrc) break;
        }
    }
    if (!fInterruptSrc) return false;
    fWorkLoop->addEventSource(fInterruptSrc);

    fTimerSrc = IOTimerEventSource::timerEventSource(this, timerFired);
    if (!fTimerSrc) return false;
    fWorkLoop->addEventSource(fTimerSrc);
    fTimerSrc->setTimeoutMS(1000);

    if (!attachInterface((IONetworkInterface **)&fNetInterface, true)) return false;

    setProperty("Step3_DevAllocated", true);
    registerService();
    return true;
}

/* Helper to safely call IOBufferMemoryDescriptor::complete() despite
 * osdep.h's `complete` macro conflicting with the method name. */
static void safe_bmd_complete(IOBufferMemoryDescriptor *bmd)
{
#pragma push_macro("complete")
#undef complete
    bmd->complete();
#pragma pop_macro("complete")
}

void com_realtek_driver_RealtekWifi::stop(IOService *provider)
{
    if (fTimerSrc && fWorkLoop) {
        fTimerSrc->cancelTimeout();
        fWorkLoop->removeEventSource(fTimerSrc);
    }
    if (fInterruptSrc && fWorkLoop)
        fWorkLoop->removeEventSource(fInterruptSrc);
    if (fInterruptSrc) { fInterruptSrc->release(); fInterruptSrc = NULL; }
    if (fTimerSrc) { fTimerSrc->release(); fTimerSrc = NULL; }
    if (fWorkLoop) { fWorkLoop->release(); fWorkLoop = NULL; }
    if (fNetInterface) { fNetInterface->release(); fNetInterface = NULL; }
    fBar0Map = NULL;
    if (fRtw89Dev) {
        rtw89_pci_ch12_ring_deinit(fRtw89Dev);
        delete fRtw89Dev; fRtw89Dev = NULL;
    }
    if (fCh12BdDesc) { safe_bmd_complete(fCh12BdDesc); fCh12BdDesc->release(); fCh12BdDesc = NULL; }
    if (fCh12BufDesc) { safe_bmd_complete(fCh12BufDesc); fCh12BufDesc->release(); fCh12BufDesc = NULL; }
    if (fProvider) { fProvider->release(); fProvider = NULL; }
    super::stop(provider);
}

bool com_realtek_driver_RealtekWifi::configureInterface(IONetworkInterface *interface)
{
    return super::configureInterface(interface);
}

IOReturn com_realtek_driver_RealtekWifi::enable(IONetworkInterface *interface)
{
    fInterruptSrc->enable();
    return kIOReturnSuccess;
}

IOReturn com_realtek_driver_RealtekWifi::disable(IONetworkInterface *interface)
{
    fInterruptSrc->disable();
    return kIOReturnSuccess;
}

const OSString *com_realtek_driver_RealtekWifi::newVendorString() const
{
    return OSString::withCString("Realtek");
}

const OSString *com_realtek_driver_RealtekWifi::newModelString() const
{
    return OSString::withCString("802.11 Wireless LAN");
}

IOReturn com_realtek_driver_RealtekWifi::getHardwareAddress(IOEthernetAddress *addr)
{
    memset(addr, 0, sizeof(IOEthernetAddress));
    return kIOReturnSuccess;
}

IOReturn com_realtek_driver_RealtekWifi::getMaxPacketSize(UInt32 *maxSize) const
{
    *maxSize = 16000;
    return kIOReturnSuccess;
}

IOReturn com_realtek_driver_RealtekWifi::getMinPacketSize(UInt32 *minSize) const
{
    *minSize = 32;
    return kIOReturnSuccess;
}

UInt32 com_realtek_driver_RealtekWifi::outputPacket(mbuf_t packet, void *param)
{
    #ifndef kIOReturnOutputSuccess
    #define kIOReturnOutputSuccess 0
    #define kIOReturnOutputDropped 1
    #endif
    freePacket(packet);
    return kIOReturnOutputDropped;
}

void com_realtek_driver_RealtekWifi::interruptOccurred(
    OSObject *target, IOInterruptEventSource *source, int count)
{
}

void com_realtek_driver_RealtekWifi::timerFired(
    OSObject *target, IOTimerEventSource *source)
{
    auto drv = OSDynamicCast(com_realtek_driver_RealtekWifi, target);
    if (drv) drv->handleTimer();
}

void com_realtek_driver_RealtekWifi::handleTimer()
{
    setProperty("StepC_TimerFired", true);
    if (!fRtw89Dev || !fBar0Map) return;

    /* D0 wake via PMCSR */
    {
        UInt8 cap2 = fProvider->configRead8(0x34);
        while (cap2) {
            UInt8 id = fProvider->configRead8(cap2);
            if (id == 0x01) {
                fProvider->configWrite8(cap2 + 4, 0x00);
                IODelay(10000);
                break;
            }
            cap2 = fProvider->configRead8(cap2 + 1);
        }
    }
    setProperty("StepC_D0", true);

    /* Full power-on */
    int ret = rtw89_mac_pwr_on(fRtw89Dev);
    setProperty("StepE_PwrOn_ret", OSNumber::withNumber((unsigned long long)ret, 64));
    if (ret) { setProperty("StepE_SkipFW", true); return; }

    /* Debug: read initial register state before touching anything */
    u32 platInit = rtw89_read32(fRtw89Dev, R_AX_PLATFORM_ENABLE);
    u32 wcpuInit = rtw89_read32(fRtw89Dev, R_AX_WCPU_FW_CTRL);
    u32 clkInit = rtw89_read32(fRtw89Dev, R_AX_SYS_CLK_CTRL);
    setProperty("StepE_PLAT_init", OSNumber::withNumber(platInit, 64));
    setProperty("StepE_WCPU_init", OSNumber::withNumber(wcpuInit, 64));
    setProperty("StepE_CLK_init", OSNumber::withNumber(clkInit, 64));

    /* Ensure WCPU is disabled (clean state) */
    rtw89_write32_clr(fRtw89Dev, R_AX_PLATFORM_ENABLE, B_AX_WCPU_EN);
    rtw89_write32_clr(fRtw89Dev, R_AX_WCPU_FW_CTRL,
                      B_AX_WCPU_FWDL_EN | B_AX_H2C_PATH_RDY | B_AX_FWDL_PATH_RDY);
    rtw89_write32_clr(fRtw89Dev, R_AX_SYS_CLK_CTRL, B_AX_CPU_CLK_EN);
    u32 wcpuDis = rtw89_read32(fRtw89Dev, R_AX_WCPU_FW_CTRL);
    setProperty("StepE_WCPU_dis", OSNumber::withNumber(wcpuDis, 64));

    IODelay(1000);

    /* DMAC + CMAC system init (enables full DMA function blocks) */
    rtw89_mac_dmac_pre_init(fRtw89Dev);
    rtw89_write32_set(fRtw89Dev, R_AX_PLATFORM_ENABLE, B_AX_AXIDMA_EN);

    /* Enable CPU in firmware download mode (Linux rtw89_mac_enable_cpu_ax) */
    rtw89_write32_set(fRtw89Dev, R_AX_SYS_CLK_CTRL, B_AX_CPU_CLK_EN);

    rtw89_write32(fRtw89Dev, R_AX_UDM1, 0);
    rtw89_write32(fRtw89Dev, R_AX_UDM2, 0);
    rtw89_write32(fRtw89Dev, R_AX_HALT_H2C_CTRL, 0);
    rtw89_write32(fRtw89Dev, R_AX_HALT_C2H_CTRL, 0);
    rtw89_write32(fRtw89Dev, R_AX_HALT_H2C, 0);
    rtw89_write32(fRtw89Dev, R_AX_HALT_C2H, 0);

    u32 val = rtw89_read32(fRtw89Dev, R_AX_WCPU_FW_CTRL);
    val &= ~(B_AX_WCPU_FWDL_EN | B_AX_H2C_PATH_RDY | B_AX_FWDL_PATH_RDY);
    val &= ~B_AX_WCPU_FWDL_STS_MASK;
    val |= B_AX_WCPU_FWDL_EN;
    rtw89_write32(fRtw89Dev, R_AX_WCPU_FW_CTRL, val);

    /* For RTL8852B: configure IDMEM size (Linux rtw89_mac_enable_cpu_ax) */
    {
        u32 tmp = rtw89_read32(fRtw89Dev, R_AX_SEC_CTRL);
        tmp &= ~B_AX_SEC_IDMEM_SIZE_CONFIG_MASK;
        tmp |= (0x2 << 16);
        rtw89_write32(fRtw89Dev, R_AX_SEC_CTRL, tmp);
    }

    /* Set boot reason to 0 (FW download mode) */
    rtw89_write32_clr(fRtw89Dev, R_AX_BOOT_REASON, B_AX_BOOT_REASON_MASK);

    /* Enable WCPU */
    rtw89_write32_set(fRtw89Dev, R_AX_PLATFORM_ENABLE, B_AX_WCPU_EN);

    u32 wcpuPost = rtw89_read32(fRtw89Dev, R_AX_WCPU_FW_CTRL);
    u32 platPost = rtw89_read32(fRtw89Dev, R_AX_PLATFORM_ENABLE);
    setProperty("StepE_WCPU_post", OSNumber::withNumber(wcpuPost, 64));
    setProperty("StepE_PLATFORM_post", OSNumber::withNumber(platPost, 64));

    IODelay(2000);

    /* Read state before firmware download */
    u32 wcpuPreFw = rtw89_read32(fRtw89Dev, R_AX_WCPU_FW_CTRL);
    setProperty("StepE_WCPU_preFW", OSNumber::withNumber(wcpuPreFw, 64));
    u32 clkPreFw = rtw89_read32(fRtw89Dev, R_AX_SYS_CLK_CTRL);
    setProperty("StepE_CLK_preFW", OSNumber::withNumber(clkPreFw, 64));

    /* Enable HCI function (HCI_TXDMA_EN + HCI_RXDMA_EN) */
    rtw89_mac_hci_func_en_ax(fRtw89Dev);
    setProperty("StepE_HCI_FUNC_EN", true);

    /* Allocate CH12 DMA ring */
    IOBufferMemoryDescriptor *bdDesc = IOBufferMemoryDescriptor::withOptions(
        kIODirectionOutIn, sizeof(struct rtw89_pci_tx_bd_32), 4096);
    IOBufferMemoryDescriptor *bufDesc = IOBufferMemoryDescriptor::withOptions(
        kIODirectionOutIn, 24 + 8 + FWDL_SECTION_PER_PKT_LEN, 4096);

    if (!bdDesc || !bufDesc) {
        setProperty("StepE_CH12_ALLOC_FAIL", true);
        if (bdDesc) { bdDesc->release(); bdDesc = NULL; }
        if (bufDesc) { bufDesc->release(); bufDesc = NULL; }
        return;
    }

    bdDesc->prepare();
    bufDesc->prepare();

    fCh12BdDesc = bdDesc;
    fCh12BufDesc = bufDesc;

    fRtw89Dev->ch12_ring.bd_vaddr = (struct rtw89_pci_tx_bd_32 *)bdDesc->getBytesNoCopy();
    fRtw89Dev->ch12_ring.bd_phys = bdDesc->getPhysicalSegment(0, NULL);
    fRtw89Dev->ch12_ring.buf_vaddr = (u8 *)bufDesc->getBytesNoCopy();
    fRtw89Dev->ch12_ring.buf_phys = bufDesc->getPhysicalSegment(0, NULL);
    fRtw89Dev->ch12_ring.num_bds = 1;
    fRtw89Dev->ch12_ring.wp = 0;

    setProperty("StepE_CH12_bd_phys", OSNumber::withNumber(
        fRtw89Dev->ch12_ring.bd_phys, 64));
    setProperty("StepE_CH12_buf_phys", OSNumber::withNumber(
        fRtw89Dev->ch12_ring.buf_phys, 64));

    /* === PCIe PHY/Power init (Linux rtw89_pci_ops_mac_pre_init_ax steps 1-22) === */
    /* l1off_pwroff: disable L1 power-off (active for 8852B) */
    rtw89_write32_clr(fRtw89Dev, R_AX_PCIE_PS_CTRL, B_AX_L1OFF_PWR_OFF_EN);
    /* aphy_pwrcut: disable analog PHY power cut (active for 8852B) */
    rtw89_write32_clr(fRtw89Dev, R_AX_SYS_PW_CTRL, B_AX_PSUS_OFF_CAPC_EN);
    /* hci_ldo: configure HCI LDO (active for 8852B) */
    rtw89_write32_set(fRtw89Dev, R_AX_SYS_SDIO_CTRL,
                      B_AX_PCIE_DIS_L2_CTRL_LDO_HCI);
    rtw89_write32_clr(fRtw89Dev, R_AX_SYS_SDIO_CTRL,
                      B_AX_PCIE_DIS_WLSUS_AFT_PDN);
    /* power_wake: wake HCI controller */
    rtw89_write32_set(fRtw89Dev, R_AX_HCI_OPT_CTRL, BIT_WAKE_CTRL);
    /* set_sic: clear force CLKREQ (active for 8852B) */
    rtw89_write32_clr(fRtw89Dev, R_AX_PCIE_EXP_CTRL,
                      B_AX_SIC_EN_FORCE_CLKREQ);
    /* set_dbg: enable debug (active for 8852B) */
    rtw89_write32_set(fRtw89Dev, R_AX_PCIE_DBG_CTRL,
                      B_AX_ASFF_FULL_NO_STK | B_AX_EN_STUCK_DBG);
    /* set_keep_reg: prevent register reset across TX/RX reset events */
    rtw89_write32_set(fRtw89Dev, R_AX_PCIE_INIT_CFG1,
                      B_AX_PCIE_TXRST_KEEP_REG | B_AX_PCIE_RXRST_KEEP_REG);

    /* Toggle HCI_FUNC_EN to reset PCIe DMA interface */
    rtw89_write16(fRtw89Dev, R_AX_HCI_FUNC_EN, 0);
    IODelay(10);
    rtw89_write16(fRtw89Dev, R_AX_HCI_FUNC_EN, 3);
    IODelay(10);

    /* Configure NORM mode address info: 0x8810 clear 8B_SEL, 0x9A00 set WD_ADDR_INFO_LENGTH */
    rtw89_write32_clr(fRtw89Dev, R_AX_TX_ADDRESS_INFO_MODE_SETTING,
                      B_AX_HOST_ADDR_INFO_8B_SEL);
    rtw89_write32_set(fRtw89Dev, R_AX_PKTIN_SETTING,
                      B_AX_WD_ADDR_INFO_LENGTH);

    /* === PCIe DMA pre-init (Linux rtw89_pci_ops_mac_pre_init_ax steps 23-32) === */

    /* Step 1: Stop DMA engines (WPDMA + PCIEIO + disable TXHCI/RXHCI) */
    rtw89_write32_set(fRtw89Dev, R_AX_PCIE_DMA_STOP1, B_AX_STOP_WPDMA);
    rtw89_write32_clr(fRtw89Dev, R_AX_PCIE_INIT_CFG1,
                      B_AX_TXHCI_EN | B_AX_RXHCI_EN);

    /* Step 2: Poll until DMA idle */
    {
        u32 dma_poll;
        int dma_ret = read_poll_timeout(rtw89_read32, dma_poll,
                                        !(dma_poll & B_AX_PCIE_DMA_IDLE),
                                        100, 20000, fRtw89Dev,
                                        R_AX_PCIE_DMA_MONITOR);
        setProperty("StepE_DMA_idle_ret",
                    OSNumber::withNumber((unsigned long long)dma_ret, 64));
    }

    /* Step 3: Clear BD/WD indexes (Linux rtw89_pci_clr_idx_all_ax for 8852B) */
    rtw89_write32_set(fRtw89Dev, R_AX_TXBD_RWPTR_CLR1,
                      B_AX_CLR_ACH0_IDX | B_AX_CLR_ACH1_IDX |
                      B_AX_CLR_ACH2_IDX | B_AX_CLR_ACH3_IDX |
                      B_AX_CLR_CH8_IDX | B_AX_CLR_CH9_IDX |
                      B_AX_CLR_CH12_IDX);
    rtw89_write32_set(fRtw89Dev, R_AX_RXBD_RWPTR_CLR,
                      B_AX_CLR_RXQ_IDX | B_AX_CLR_RPQ_IDX);

    /* Step 4: Configure PCIe mode (Linux rtw89_pci_mode_op for 8852B) */
    {
        u32 init_cfg1 = rtw89_read32(fRtw89Dev, R_AX_PCIE_INIT_CFG1);
        init_cfg1 &= ~B_AX_TX_TRUNC_MODE;
        init_cfg1 &= ~B_AX_RX_TRUNC_MODE;
        init_cfg1 &= ~B_AX_RXBD_MODE;
        init_cfg1 &= ~B_AX_PCIE_MAX_TXDMA_MASK;
        init_cfg1 &= ~B_AX_PCIE_MAX_RXDMA_MASK;
        init_cfg1 |= (7 << 8) & B_AX_PCIE_MAX_TXDMA_MASK;  /* 2048B burst */
        init_cfg1 |= (5 << 14) & B_AX_PCIE_MAX_RXDMA_MASK; /* 128B burst */
        init_cfg1 |= B_AX_LATENCY_CONTROL;                  /* multi-tag mode */
        rtw89_write32(fRtw89Dev, R_AX_PCIE_INIT_CFG1, init_cfg1);

        rtw89_write32(fRtw89Dev, R_AX_PCIE_INIT_CFG2, 0x19010190);
        rtw89_write32(fRtw89Dev, R_AX_PCIE_INIT_CFG3, 0x10000);
        rtw89_write32(fRtw89Dev, 0x1060, 0x30303030);
        rtw89_write32(fRtw89Dev, 0x1064, 0);
        rtw89_write32(fRtw89Dev, 0x1068, 0xCC);
        rtw89_write32(fRtw89Dev, 0x106C, 0);
    }

    /* Step 5: Init CH12 ring registers and BDRAM (Linux rtw89_pci_ops_reset) */
    ret = rtw89_pci_ch12_ring_init(fRtw89Dev);
    setProperty("StepE_CH12_init_ret",
                OSNumber::withNumber((unsigned long long)ret, 64));
    if (ret) return;

    /* Step 6: Reset BDRAM after ring config (Linux order: reset after init) */
    rtw89_write32_set(fRtw89Dev, R_AX_PCIE_INIT_CFG1, B_AX_RST_BDRAM);
    {
        u32 poll_val;
        int poll_ret = read_poll_timeout(rtw89_read32, poll_val,
                                         !(poll_val & B_AX_RST_BDRAM),
                                         1, 1000, fRtw89Dev,
                                         R_AX_PCIE_INIT_CFG1);
        if (poll_ret)
            setProperty("StepE_BDRAM_RST_FAIL", true);
    }

    /* Step 7: Disable all TX channels except CH12, then enable CH12 */
    rtw89_write32_set(fRtw89Dev, R_AX_PCIE_DMA_STOP1,
                      B_AX_STOP_ACH0 | B_AX_STOP_ACH1 | B_AX_STOP_ACH2 |
                      B_AX_STOP_ACH3 | B_AX_STOP_ACH4 | B_AX_STOP_ACH5 |
                      B_AX_STOP_ACH6 | B_AX_STOP_ACH7 |
                      B_AX_STOP_CH8 | B_AX_STOP_CH9);
    rtw89_write32_clr(fRtw89Dev, R_AX_PCIE_DMA_STOP1, B_AX_STOP_CH12);

    /* Step 8: Re-enable TXHCI/RXHCI + clear PCIEIO stop */
    rtw89_write32_set(fRtw89Dev, R_AX_PCIE_INIT_CFG1,
                      B_AX_TXHCI_EN | B_AX_RXHCI_EN);
    rtw89_write32_clr(fRtw89Dev, R_AX_PCIE_DMA_STOP1, B_AX_STOP_WPDMA);

    /* Debug: dump register state before firmware download */
    setProperty("StepE_DMA_STOP1", OSNumber::withNumber(
        rtw89_read32(fRtw89Dev, R_AX_PCIE_DMA_STOP1), 64));
    setProperty("StepE_INIT_CFG1", OSNumber::withNumber(
        rtw89_read32(fRtw89Dev, R_AX_PCIE_INIT_CFG1), 64));

    /* CH12 ring HW register state */
    setProperty("StepE_CH12_NUM", OSNumber::withNumber(
        rtw89_read32(fRtw89Dev, R_AX_CH12_TXBD_NUM), 64));
    setProperty("StepE_CH12_DESA_L", OSNumber::withNumber(
        rtw89_read32(fRtw89Dev, R_AX_CH12_TXBD_DESA_L), 64));
    setProperty("StepE_CH12_DESA_H", OSNumber::withNumber(
        rtw89_read32(fRtw89Dev, R_AX_CH12_TXBD_DESA_H), 64));
    setProperty("StepE_CH12_BDRAM", OSNumber::withNumber(
        rtw89_read32(fRtw89Dev, R_AX_CH12_BDRAM_CTRL), 64));

    /* Key function/clock enable registers */
    setProperty("StepE_HCI_FUNC_REG", OSNumber::withNumber(
        rtw89_read32(fRtw89Dev, R_AX_HCI_FUNC_EN), 64));
    setProperty("StepE_DMAC_FUNC_REG", OSNumber::withNumber(
        rtw89_read32(fRtw89Dev, R_AX_DMAC_FUNC_EN), 64));
    setProperty("StepE_DMAC_CLK_REG", OSNumber::withNumber(
        rtw89_read32(fRtw89Dev, R_AX_DMAC_CLK_EN), 64));
    setProperty("StepE_HCI_FC_REG", OSNumber::withNumber(
        rtw89_read32(fRtw89Dev, R_AX_HCI_FC_CTRL), 64));

    /* DMA error / status registers */
    setProperty("StepE_HAXI_IDCT", OSNumber::withNumber(
        rtw89_read32(fRtw89Dev, R_AX_HAXI_IDCT), 64));
    setProperty("StepE_DMA_MONITOR", OSNumber::withNumber(
        rtw89_read32(fRtw89Dev, R_AX_PCIE_DMA_MONITOR), 64));
    setProperty("StepE_BUSY2", OSNumber::withNumber(
        rtw89_read32(fRtw89Dev, R_AX_HAXI_DMA_BUSY2), 64));
    setProperty("StepE_ADDR_MODE", OSNumber::withNumber(
        rtw89_read32(fRtw89Dev, R_AX_TX_ADDRESS_INFO_MODE_SETTING), 64));
    setProperty("StepE_PKTIN", OSNumber::withNumber(
        rtw89_read32(fRtw89Dev, R_AX_PKTIN_SETTING), 64));
    setProperty("StepE_HCI_OPT", OSNumber::withNumber(
        rtw89_read32(fRtw89Dev, R_AX_HCI_OPT_CTRL), 64));

    /* DMA-based firmware download */
    ret = rtw89_fw_download(fRtw89Dev, rtw8852b_fw_bin, rtw8852b_fw_bin_len);
    setProperty("StepE_FW_ret", OSNumber::withNumber((unsigned long long)ret, 64));

    /* CH12 debug from last send_h2c call */
    setProperty("StepE_BD_dword0", OSNumber::withNumber(
        fRtw89Dev->ch12_ring.dbg_bd_dword0, 64));
    setProperty("StepE_BD_dword1", OSNumber::withNumber(
        fRtw89Dev->ch12_ring.dbg_bd_dword1, 64));
    setProperty("StepE_TXBD_write", OSNumber::withNumber(
        fRtw89Dev->ch12_ring.dbg_txbd_idx_write, 64));
    setProperty("StepE_TXBD_poll", OSNumber::withNumber(
        fRtw89Dev->ch12_ring.dbg_txbd_idx_poll, 64));
    setProperty("StepE_total_size", OSNumber::withNumber(
        fRtw89Dev->ch12_ring.dbg_total_size, 64));
    setProperty("StepE_send_ret", OSNumber::withNumber(
        (unsigned long long)fRtw89Dev->ch12_ring.dbg_send_ret, 64));

    u32 wcpuFinal = rtw89_read32(fRtw89Dev, R_AX_WCPU_FW_CTRL);
    setProperty("StepE_WCPU_final", OSNumber::withNumber(wcpuFinal, 64));

    setProperty("StepE_Done", true);
}
