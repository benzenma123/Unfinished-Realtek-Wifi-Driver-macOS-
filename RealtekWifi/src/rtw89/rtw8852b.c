#include "rtw89.h"
#include "rtw8852b.h"

/* XTAL SI address constants */
#define XTAL_SI_ANAPAR_WL       0x00
#define XTAL_SI_GND_SHDN_WL     0x02
#define XTAL_SI_SHDN_WL         0x01
#define XTAL_SI_OFF_WEI         0x08
#define XTAL_SI_OFF_EI          0x04
#define XTAL_SI_RFC2RF          0x10
#define XTAL_SI_PON_WEI         0x20
#define XTAL_SI_PON_EI          0x10
#define XTAL_SI_SRAM2RFC        0x40
#define XTAL_SI_SRAM_CTRL       0x01
#define XTAL_SI_SRAM_DIS        0x02
#define XTAL_SI_XTAL_XMD_2      0x02
#define XTAL_SI_LDO_LPS         0x01
#define XTAL_SI_XTAL_XMD_4      0x04
#define XTAL_SI_LPS_CAP         0x01
#define XTAL_SI_WL_RFC_S0       0x20
#define XTAL_SI_RF00            0x01
#define XTAL_SI_WL_RFC_S1       0x21
#define XTAL_SI_RF10            0x01

/* Register not in reg.h */
#define R_AX_SYS_SDIO_CTRL      0x0060
#define B_AX_PCIE_CALIB_EN_V1   BIT(5)
#define R_AX_SYS_ADIE_PAD_PWR_CTRL 0x0038
#define B_AX_SYM_PADPDN_WL_PTA_1P3 BIT(12)
#define B_AX_SYM_PADPDN_WL_RFC_1P3 BIT(11)
#define R_AX_PMC_DBG_CTRL2      0x2008
#define B_AX_SYSON_DIS_PMCR_AX_WRMSK BIT(15)
#define R_AX_WLRF_CTRL          0x0028
#define B_AX_AFC_AFEDIG         BIT(25)
#define R_AX_SYS_SWR_CTRL1      0x0014
#define B_AX_SYM_CTRL_SPS_PWMFREQ BIT(8)
#define R_AX_EECS_EESK_FUNC_SEL 0x0300
#define B_AX_PINMUX_EESK_FUNC_SEL_MASK GENMASK(21, 20)
#define PINMUX_EESK_FUNC_SEL_BT_LOG 0x1
#define R_AX_SYS_STATUS1        0x0018
#define B_AX_AUTO_WLPON         BIT(12)
#define R_AX_RSV_CTRL           0x001C
#define B_AX_R_DIS_PRST         BIT(4)
#define R_AX_GPIO_MUXCFG        0x0024
#define B_AX_BOOT_MODE          BIT(0)

#define R_AX_SPS_DIG_ON_CTRL0   0x2010
#define B_AX_OCP_L1_MASK        GENMASK(3, 0)
#define B_AX_VOL_L1_MASK        GENMASK(7, 4)
#define B_AX_VREFPFM_L_MASK     GENMASK(11, 8)
#define B_AX_REG_ZCDC_H_MASK    GENMASK(31, 28)

#define R_AX_SPS_DIG_OFF_CTRL0  0x2014
#define B_AX_C1_L1_MASK         GENMASK(3, 0)
#define B_AX_C2_L1_MASK         GENMASK(7, 4)
#define B_AX_C3_L1_MASK         GENMASK(11, 8)
#define B_AX_R1_L1_MASK         GENMASK(15, 12)

#define R_AX_SPS_ANA_ON_CTRL2   0x2028

#define R_AX_HCI_LDO_CTRL       0x0064
#define B_AX_R_AX_VADJ_MASK     GENMASK(7, 4)

/* SW_LPS_OPTION for pwr_off */
#define SW_LPS_OPTION           0x00000003

/* Custom RFE type SPS analog value */
#define RTL8852B_RFE_05_SPS_ANA 0x0222

static void rtw8852b_pwr_sps_ana(struct rtw89_dev *rtwdev)
{
    if (rtwdev->rfe_type == 0x5)
        rtw89_write16(rtwdev, R_AX_SPS_ANA_ON_CTRL2, RTL8852B_RFE_05_SPS_ANA);
}

static void rtw8852b_pwr_sps_dig_off(struct rtw89_dev *rtwdev)
{
    if (rtwdev->rfe_type == 0x5) {
        rtw89_write32_mask(rtwdev, R_AX_SPS_DIG_OFF_CTRL0,
                            B_AX_C1_L1_MASK, 0x1);
        rtw89_write32_mask(rtwdev, R_AX_SPS_DIG_OFF_CTRL0,
                            B_AX_C2_L1_MASK, 0x1);
        rtw89_write32_mask(rtwdev, R_AX_SPS_DIG_OFF_CTRL0,
                            B_AX_C3_L1_MASK, 0x2);
        rtw89_write32_mask(rtwdev, R_AX_SPS_DIG_OFF_CTRL0,
                            B_AX_R1_L1_MASK, 0x1);
    } else {
        rtw89_write32_mask(rtwdev, R_AX_SPS_DIG_OFF_CTRL0,
                            B_AX_C1_L1_MASK, 0x1);
        rtw89_write32_mask(rtwdev, R_AX_SPS_DIG_OFF_CTRL0,
                            B_AX_C3_L1_MASK, 0x3);
    }
}

int rtw8852b_pwr_on_func(struct rtw89_dev *rtwdev)
{
    u32 val32;
    int ret;

    rtw8852b_pwr_sps_ana(rtwdev);

    rtw89_write32_clr(rtwdev, R_AX_SYS_PW_CTRL,
                       B_AX_AFSM_WLSUS_EN | B_AX_AFSM_PCIE_SUS_EN);
    rtw89_write32_set(rtwdev, R_AX_SYS_PW_CTRL, B_AX_DIS_WLBT_PDNSUSEN_SOPC);
    rtw89_write32_set(rtwdev, R_AX_WLLPS_CTRL, B_AX_DIS_WLBT_LPSEN_LOPC);
    rtw89_write32_clr(rtwdev, R_AX_SYS_PW_CTRL, B_AX_APDM_HPDN);
    rtw89_write32_clr(rtwdev, R_AX_SYS_PW_CTRL, B_AX_APFM_SWLPS);

    ret = read_poll_timeout(rtw89_read32, val32, val32 & B_AX_RDY_SYSPWR,
                            1000, 20000, rtwdev, R_AX_SYS_PW_CTRL);
    if (ret) return ret;

    rtw89_write32_set(rtwdev, R_AX_AFE_LDO_CTRL, B_AX_AON_OFF_PC_EN);
    ret = read_poll_timeout(rtw89_read32, val32, val32 & B_AX_AON_OFF_PC_EN,
                            1000, 20000, rtwdev, R_AX_AFE_LDO_CTRL);
    if (ret) return ret;

    rtw8852b_pwr_sps_dig_off(rtwdev);
    rtw89_write32_set(rtwdev, R_AX_SYS_PW_CTRL, B_AX_EN_WLON);
    rtw89_write32_set(rtwdev, R_AX_SYS_PW_CTRL, B_AX_APFN_ONMAC);

    ret = read_poll_timeout(rtw89_read32, val32, !(val32 & B_AX_APFN_ONMAC),
                            1000, 20000, rtwdev, R_AX_SYS_PW_CTRL);
    if (ret) return ret;

    /* Toggle platform enable (32-bit RMW — avoids 8-bit write hang) */
    rtw89_write32_set(rtwdev, R_AX_PLATFORM_ENABLE, B_AX_PLATFORM_EN);
    rtw89_write32_clr(rtwdev, R_AX_PLATFORM_ENABLE, B_AX_PLATFORM_EN);
    rtw89_write32_set(rtwdev, R_AX_PLATFORM_ENABLE, B_AX_PLATFORM_EN);
    rtw89_write32_clr(rtwdev, R_AX_PLATFORM_ENABLE, B_AX_PLATFORM_EN);

    rtw89_write32_set(rtwdev, R_AX_PLATFORM_ENABLE, B_AX_PLATFORM_EN);
    rtw89_write32_clr(rtwdev, R_AX_SYS_SDIO_CTRL, B_AX_PCIE_CALIB_EN_V1);

    rtw89_write32_set(rtwdev, R_AX_SYS_ADIE_PAD_PWR_CTRL,
                       B_AX_SYM_PADPDN_WL_PTA_1P3);

    ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_ANAPAR_WL,
                                   XTAL_SI_GND_SHDN_WL, XTAL_SI_GND_SHDN_WL);
    if (ret) return ret;

    rtw89_write32_set(rtwdev, R_AX_SYS_ADIE_PAD_PWR_CTRL,
                       B_AX_SYM_PADPDN_WL_RFC_1P3);

    ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_ANAPAR_WL,
                                   XTAL_SI_SHDN_WL, XTAL_SI_SHDN_WL);
    if (ret) return ret;
    ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_ANAPAR_WL,
                                   XTAL_SI_OFF_WEI, XTAL_SI_OFF_WEI);
    if (ret) return ret;
    ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_ANAPAR_WL,
                                   XTAL_SI_OFF_EI, XTAL_SI_OFF_EI);
    if (ret) return ret;
    ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_ANAPAR_WL, 0, XTAL_SI_RFC2RF);
    if (ret) return ret;
    ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_ANAPAR_WL,
                                   XTAL_SI_PON_WEI, XTAL_SI_PON_WEI);
    if (ret) return ret;
    ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_ANAPAR_WL,
                                   XTAL_SI_PON_EI, XTAL_SI_PON_EI);
    if (ret) return ret;
    ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_ANAPAR_WL, 0, XTAL_SI_SRAM2RFC);
    if (ret) return ret;
    ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_SRAM_CTRL, 0, XTAL_SI_SRAM_DIS);
    if (ret) return ret;
    ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_XTAL_XMD_2, 0, XTAL_SI_LDO_LPS);
    if (ret) return ret;
    ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_XTAL_XMD_4, 0, XTAL_SI_LPS_CAP);
    if (ret) return ret;

    rtw89_write32_set(rtwdev, R_AX_PMC_DBG_CTRL2,
                       B_AX_SYSON_DIS_PMCR_AX_WRMSK);
    rtw89_write32_set(rtwdev, R_AX_SYS_ISO_CTRL, B_AX_ISO_EB2CORE);
    rtw89_write32_clr(rtwdev, R_AX_SYS_ISO_CTRL, B_AX_PWC_EV2EF_B15);

    fsleep(1000);

    rtw89_write32_clr(rtwdev, R_AX_SYS_ISO_CTRL, B_AX_PWC_EV2EF_B14);
    rtw89_write32_clr(rtwdev, R_AX_PMC_DBG_CTRL2,
                       B_AX_SYSON_DIS_PMCR_AX_WRMSK);

    if (!rtwdev->efuse_valid || rtwdev->power_k_valid)
        goto func_en;

    rtw89_write32_mask(rtwdev, R_AX_SPS_DIG_ON_CTRL0,
                        B_AX_VOL_L1_MASK, 0x9);
    rtw89_write32_mask(rtwdev, R_AX_SPS_DIG_ON_CTRL0,
                        B_AX_VREFPFM_L_MASK, 0xA);

    if (rtwdev->cv == 0 && 1) {
        rtw89_write32_set(rtwdev, R_AX_PMC_DBG_CTRL2,
                           B_AX_SYSON_DIS_PMCR_AX_WRMSK);
        rtw89_write16_mask(rtwdev, R_AX_HCI_LDO_CTRL,
                            B_AX_R_AX_VADJ_MASK, 0xA);
        rtw89_write32_clr(rtwdev, R_AX_PMC_DBG_CTRL2,
                           B_AX_SYSON_DIS_PMCR_AX_WRMSK);
    }

func_en:
    rtw89_write32_set(rtwdev, R_AX_DMAC_FUNC_EN,
                      B_AX_MAC_FUNC_EN | B_AX_DMAC_FUNC_EN |
                      B_AX_MPDU_PROC_EN | B_AX_WD_RLS_EN |
                      B_AX_DLE_WDE_EN | B_AX_TXPKT_CTRL_EN |
                      B_AX_STA_SCH_EN | B_AX_DLE_PLE_EN |
                      B_AX_PKT_BUF_EN | B_AX_DMAC_TBL_EN |
                      B_AX_PKT_IN_EN | B_AX_DLE_CPUIO_EN |
                      B_AX_DISPATCHER_EN | B_AX_BBRPT_EN |
                      B_AX_MAC_SEC_EN | B_AX_DMACREG_GCKEN);
    rtw89_write32_set(rtwdev, R_AX_CMAC_FUNC_EN,
                      B_AX_CMAC_EN | B_AX_CMAC_TXEN |
                      B_AX_CMAC_RXEN | B_AX_FORCE_CMACREG_GCKEN |
                      B_AX_PHYINTF_EN | B_AX_CMAC_DMA_EN |
                      B_AX_PTCLTOP_EN | B_AX_SCHEDULER_EN |
                      B_AX_TMAC_EN | B_AX_RMAC_EN);

    rtw89_write32_mask(rtwdev, R_AX_EECS_EESK_FUNC_SEL,
                        B_AX_PINMUX_EESK_FUNC_SEL_MASK,
                        PINMUX_EESK_FUNC_SEL_BT_LOG);

    return 0;
}

int rtw8852b_pwr_off_func(struct rtw89_dev *rtwdev)
{
    u32 val32;
    int ret;

    rtw8852b_pwr_sps_ana(rtwdev);

    ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_ANAPAR_WL,
                                   XTAL_SI_RFC2RF, XTAL_SI_RFC2RF);
    if (ret) return ret;
    ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_ANAPAR_WL, 0, XTAL_SI_OFF_EI);
    if (ret) return ret;
    ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_ANAPAR_WL, 0, XTAL_SI_OFF_WEI);
    if (ret) return ret;
    ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_WL_RFC_S0, 0, XTAL_SI_RF00);
    if (ret) return ret;
    ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_WL_RFC_S1, 0, XTAL_SI_RF10);
    if (ret) return ret;
    ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_ANAPAR_WL,
                                   XTAL_SI_SRAM2RFC, XTAL_SI_SRAM2RFC);
    if (ret) return ret;
    ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_ANAPAR_WL, 0, XTAL_SI_PON_EI);
    if (ret) return ret;
    ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_ANAPAR_WL, 0, XTAL_SI_PON_WEI);
    if (ret) return ret;

    rtw89_write32_set(rtwdev, R_AX_SYS_PW_CTRL, B_AX_EN_WLON);
    rtw89_write32_clr(rtwdev, R_AX_WLRF_CTRL, B_AX_AFC_AFEDIG);
    rtw89_write32_clr(rtwdev, R_AX_SYS_FUNC_EN,
                      (u32)(B_AX_FEN_BB_GLB_RSTN | B_AX_FEN_BBRSTB));
    rtw89_write32_clr(rtwdev, R_AX_SYS_ADIE_PAD_PWR_CTRL,
                       B_AX_SYM_PADPDN_WL_RFC_1P3);

    ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_ANAPAR_WL, 0, XTAL_SI_SHDN_WL);
    if (ret) return ret;

    rtw89_write32_clr(rtwdev, R_AX_SYS_ADIE_PAD_PWR_CTRL,
                       B_AX_SYM_PADPDN_WL_PTA_1P3);

    ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_ANAPAR_WL, 0,
                                   XTAL_SI_GND_SHDN_WL);
    if (ret) return ret;

    rtw89_write32_set(rtwdev, R_AX_SYS_PW_CTRL, B_AX_APFM_OFFMAC);

    ret = read_poll_timeout(rtw89_read32, val32, !(val32 & B_AX_APFM_OFFMAC),
                            1000, 20000, rtwdev, R_AX_SYS_PW_CTRL);
    if (ret) return ret;

    rtw89_write32(rtwdev, R_AX_WLLPS_CTRL, SW_LPS_OPTION);

    rtw89_write32_set(rtwdev, R_AX_SYS_SWR_CTRL1,
                       B_AX_SYM_CTRL_SPS_PWMFREQ);
    rtw89_write32_mask(rtwdev, R_AX_SPS_DIG_ON_CTRL0,
                        B_AX_REG_ZCDC_H_MASK, 0x3);

    rtw89_write32_set(rtwdev, R_AX_SYS_PW_CTRL, B_AX_APFM_SWLPS);

    return 0;
}

static struct rtw89_chip_ops rtw8852b_chip_ops = {
    .pwr_on_func  = rtw8852b_pwr_on_func,
    .pwr_off_func = rtw8852b_pwr_off_func,
};

struct rtw89_chip_info rtw8852b_chip_info = {
    .chip_id              = RTL8852B,
    .chip_gen             = RTW89_CHIP_AX,
    .ops                  = &rtw8852b_chip_ops,
    .fw_basename          = (const u8 *)"rtw89/rtw8852b_fw.bin",
    .fifo_size            = 196608,
    .dle_scc_rsvd_size    = 98304,
    .hci_func_en_addr     = R_AX_HCI_FUNC_EN,
    .h2c_ctrl_reg         = R_AX_H2CREG_CTRL,
    .c2h_ctrl_reg         = R_AX_C2HREG_CTRL,
    .h2c_desc_size        = sizeof(struct rtw89_txwd_body),
    .txwd_body_size       = sizeof(struct rtw89_txwd_body),
    .txwd_info_size       = sizeof(struct rtw89_txwd_info),
};

int rtw8852b_register_chip(struct rtw89_dev *rtwdev)
{
    if (!rtwdev) return -1;
    rtwdev->chip = &rtw8852b_chip_info;
    return 0;
}
