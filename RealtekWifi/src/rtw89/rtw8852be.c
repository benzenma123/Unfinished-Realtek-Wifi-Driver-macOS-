#include "rtw89.h"
#include "rtw8852b.h"

/* PCIe information for RTL8852BE */
struct rtw89_pci_ch_dma_addr {
    u32 num;
    u32 idx;
    u32 bdram;
    u32 desa_l;
    u32 desa_h;
};

struct rtw89_pci_isr_def {
    u32 isr_rdu;
    u32 isr_halt_c2h;
    u32 isr_wdt_timeout;
    u32 isr_sps_ocp;
};

struct rtw89_pci_info {
    u32 init_cfg_reg;
    u32 txhci_en_bit;
    u32 rxhci_en_bit;
    u32 rxbd_mode_bit;
    u32 exp_ctrl_reg;
    u32 max_tag_num_mask;
    u32 rxbd_rwptr_clr_reg;
    u32 rxbd_mode;
    u32 tx_burst;
    u32 rx_burst;
    u32 tx_dma_ch_mask;
    u32 mit_addr;
};

/* RTL8852BE PCI info */
struct rtw89_pci_info rtw8852be_pci_info = {
    .init_cfg_reg       = R_AX_PCIE_INIT_CFG1,
    .txhci_en_bit       = B_AX_TXHCI_EN,
    .rxhci_en_bit       = B_AX_RXHCI_EN,
    .rxbd_mode_bit      = B_AX_RXBD_MODE,
    .exp_ctrl_reg       = R_AX_PCIE_EXP_CTRL,
    .max_tag_num_mask   = GENMASK(7, 0),
    .rxbd_rwptr_clr_reg = R_AX_RXBD_RWPTR_CLR,
    .rxbd_mode          = 0,
    .tx_burst           = 3,
    .rx_burst           = 0,
    .tx_dma_ch_mask     = BIT(4) | BIT(5) | BIT(6) | BIT(7) |
                           BIT(10) | BIT(11),
    .mit_addr           = R_AX_INT_MIT_RX,
};

/* Configure PCIe init registers for RTL8852BE */
int rtw8852be_pci_init(struct rtw89_dev *rtwdev)
{
    u32 val;

    /* Stop DMA first */
    rtw89_write32_set(rtwdev, R_AX_PCIE_DMA_STOP1, B_AX_STOP_WPDMA);
    IODelay(1000);

    /* Reset BDRAM */
    val = rtw89_read32(rtwdev, R_AX_PCIE_INIT_CFG1);
    val |= B_AX_RST_BDRAM;
    rtw89_write32(rtwdev, R_AX_PCIE_INIT_CFG1, val);
    IODelay(100);
    val &= ~B_AX_RST_BDRAM;
    rtw89_write32(rtwdev, R_AX_PCIE_INIT_CFG1, val);

    /* Clear RX BD read/write pointer */
    rtw89_write32(rtwdev, R_AX_RXBD_RWPTR_CLR, 0);

    /* Enable TX and RX HCI */
    val = rtw89_read32(rtwdev, R_AX_PCIE_INIT_CFG1);
    val |= B_AX_TXHCI_EN | B_AX_RXHCI_EN;
    rtw89_write32(rtwdev, R_AX_PCIE_INIT_CFG1, val);

    return 0;
}

/* Enable RTL8852BE interrupts */
void rtw8852be_enable_interrupts(struct rtw89_dev *rtwdev)
{
    /* Enable host interrupts: C2H + watchdog */
    rtw89_write32(rtwdev, R_AX_HD0IMR,
                  B_AX_C2H_INT_EN | B_AX_WDT_PTFM_INT_EN);

    /* Enable PCIe interrupts */
    rtw89_write32(rtwdev, R_AX_PCIE_HIMR00, 0xFFFFFFFF);
    rtw89_write32(rtwdev, R_AX_PCIE_HIMR10, 0xFFFFFFFF);
}

/* Disable RTL8852BE interrupts */
void rtw8852be_disable_interrupts(struct rtw89_dev *rtwdev)
{
    rtw89_write32(rtwdev, R_AX_HD0IMR, 0);
    rtw89_write32(rtwdev, R_AX_PCIE_HIMR00, 0);
    rtw89_write32(rtwdev, R_AX_PCIE_HIMR10, 0);
}
