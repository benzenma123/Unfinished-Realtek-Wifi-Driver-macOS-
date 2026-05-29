#include "rtw89.h"

/* Write to XTAL SI (serial interface) */
int rtw89_mac_write_xtal_si(struct rtw89_dev *rtwdev, u8 addr, u8 mask, u8 data)
{
    u32 val;
    int ret;

    val = rtw89_read32(rtwdev, R_AX_WLAN_XTAL_SI_CTRL);
    val &= ~B_AX_WL_XTAL_GNT;
    val |= B_AX_WL_XTAL_GNT;
    rtw89_write32(rtwdev, R_AX_WLAN_XTAL_SI_CTRL, val);

    val = ((u32)XTAL_SI_NORMAL_WRITE << 24) |
          (((u32)mask & 0xFF) << 16) |
          (((u32)data & 0xFF) << 8) |
          ((u32)addr & 0xFF);
    rtw89_write32(rtwdev, R_AX_WLAN_XTAL_SI_CTRL, val);

    val |= B_AX_WL_XTAL_SI_CMD_POLL;
    rtw89_write32(rtwdev, R_AX_WLAN_XTAL_SI_CTRL, val);

    ret = read_poll_timeout(rtw89_read32, val,
                            !(val & B_AX_WL_XTAL_SI_CMD_POLL),
                            100, 10000, rtwdev, R_AX_WLAN_XTAL_SI_CTRL);
    if (ret)
        return ret;

    val = rtw89_read32(rtwdev, R_AX_WLAN_XTAL_SI_CTRL);
    val &= ~B_AX_WL_XTAL_GNT;
    rtw89_write32(rtwdev, R_AX_WLAN_XTAL_SI_CTRL, val);

    return 0;
}

/* Enable DMAC function for AX generation */
int dmac_func_en_ax(struct rtw89_dev *rtwdev)
{
    u32 func_en, clk_en;

    func_en = B_AX_MAC_FUNC_EN | B_AX_DMAC_FUNC_EN |
              B_AX_MAC_SEC_EN | B_AX_DISPATCHER_EN |
              B_AX_DLE_CPUIO_EN | B_AX_PKT_IN_EN |
              B_AX_DMAC_TBL_EN | B_AX_PKT_BUF_EN |
              B_AX_STA_SCH_EN | B_AX_TXPKT_CTRL_EN |
              B_AX_WD_RLS_EN | B_AX_MPDU_PROC_EN |
              B_AX_DMAC_CRPRT;
    rtw89_write32(rtwdev, R_AX_DMAC_FUNC_EN, func_en);

    clk_en = B_AX_MAC_SEC_CLK_EN | B_AX_DISPATCHER_CLK_EN |
             B_AX_DLE_CPUIO_CLK_EN | B_AX_PKT_IN_CLK_EN |
             B_AX_STA_SCH_CLK_EN | B_AX_TXPKT_CTRL_CLK_EN |
             B_AX_WD_RLS_CLK_EN | B_AX_BBRPT_CLK_EN;
    rtw89_write32(rtwdev, R_AX_DMAC_CLK_EN, clk_en);

    return 0;
}

/* Enable CMAC function for AX generation */
int cmac_func_en_ax(struct rtw89_dev *rtwdev, u8 mac_idx, bool en)
{
    u32 func_en, ck_en;

    func_en = B_AX_CMAC_EN | B_AX_CMAC_TXEN | B_AX_CMAC_RXEN |
              B_AX_PHYINTF_EN | B_AX_CMAC_DMA_EN | B_AX_PTCLTOP_EN |
              B_AX_SCHEDULER_EN | B_AX_TMAC_EN | B_AX_RMAC_EN |
              B_AX_CMAC_CRPRT;
    ck_en = BIT(31) | BIT(30) | BIT(29) | BIT(28) |
            BIT(27) | BIT(26) | BIT(25) | BIT(24);

    if (en) {
        rtw89_write32_set(rtwdev, R_AX_CMAC_FUNC_EN, func_en);
    } else {
        rtw89_write32_clr(rtwdev, R_AX_CMAC_FUNC_EN, func_en);
    }

    return 0;
}

/* System init for AX: enable DMAC + CMAC */
int sys_init_ax(struct rtw89_dev *rtwdev)
{
    int ret;

    ret = dmac_func_en_ax(rtwdev);
    if (ret) return ret;

    ret = cmac_func_en_ax(rtwdev, RTW89_MAC_0, true);
    if (ret) return ret;

    return 0;
}

/* MAC power switch - main power on/off controller */
int rtw89_mac_power_switch(struct rtw89_dev *rtwdev, bool on)
{
    int ret;

    if (on) {
        if (rtwdev->chip && rtwdev->chip->ops && rtwdev->chip->ops->pwr_on_func) {
            ret = rtwdev->chip->ops->pwr_on_func(rtwdev);
            if (ret) return ret;
        }

        rtwdev->flags |= RTW89_FLAG_POWERON;
        rtwdev->flags |= RTW89_FLAG_DMAC_FUNC;
        rtwdev->flags |= RTW89_FLAG_CMAC0_FUNC;
    } else {
        if (rtwdev->chip && rtwdev->chip->ops && rtwdev->chip->ops->pwr_off_func) {
            ret = rtwdev->chip->ops->pwr_off_func(rtwdev);
            if (ret) return ret;
        }

        rtwdev->flags &= ~RTW89_FLAG_POWERON;
        rtwdev->flags &= ~RTW89_FLAG_DMAC_FUNC;
        rtwdev->flags &= ~RTW89_FLAG_CMAC0_FUNC;
        rtwdev->flags &= ~RTW89_FLAG_FW_RDY;
    }

    return 0;
}

/* Power on the MAC */
int rtw89_mac_pwr_on(struct rtw89_dev *rtwdev)
{
    int ret;

    ret = rtw89_mac_power_switch(rtwdev, true);
    if (ret) {
        rtw89_mac_power_switch(rtwdev, false);
        ret = rtw89_mac_power_switch(rtwdev, true);
        if (ret) return ret;
    }

    return 0;
}

/* Power off the MAC */
int rtw89_mac_pwr_off(struct rtw89_dev *rtwdev)
{
    return rtw89_mac_power_switch(rtwdev, false);
}

/* Enable HCI function and DMA (Linux rtw89_mac_hci_func_en_ax) */
void rtw89_mac_hci_func_en_ax(struct rtw89_dev *rtwdev)
{
    rtw89_write32_set(rtwdev, R_AX_HCI_FUNC_EN, B_AX_HCI_FUNC_EN);
    rtw89_write32_set(rtwdev, R_AX_HCI_FUNC_EN,
                      B_AX_HCI_TXDMA_EN | B_AX_HCI_RXDMA_EN);
}

/* Control HCI DMA TRX (Linux rtw89_mac_ctrl_hci_dma_trx) */
void rtw89_mac_ctrl_hci_dma_trx(struct rtw89_dev *rtwdev, bool enable)
{
    if (enable) {
        rtw89_write32_set(rtwdev, R_AX_HCI_FUNC_EN,
                          B_AX_HCI_TXDMA_EN | B_AX_HCI_RXDMA_EN);
    } else {
        rtw89_write32_clr(rtwdev, R_AX_HCI_FUNC_EN,
                          B_AX_HCI_TXDMA_EN | B_AX_HCI_RXDMA_EN);
    }
}

/* Pre-init for firmware download (Linux rtw89_mac_dmac_func_pre_en_ax + cmac) */
void rtw89_mac_dmac_pre_init(struct rtw89_dev *rtwdev)
{
    sys_init_ax(rtwdev);
}

/* MAC init - placeholder for now, will be expanded */
int rtw89_mac_init(struct rtw89_dev *rtwdev)
{
    return 0;
}
