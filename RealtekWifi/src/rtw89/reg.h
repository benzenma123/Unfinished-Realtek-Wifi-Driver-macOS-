#ifndef RTW89_REG_H
#define RTW89_REG_H

#include "platform/osdep.h"

#ifndef BIT
#define BIT(n) (1ULL << (n))
#endif
#ifndef GENMASK
#define GENMASK(h, l) (((~0ULL) << (l)) & (~0ULL >> (63 - (h))))
#endif

/* Power Control */
#define R_AX_SYS_PW_CTRL        0x0004
#define B_AX_RDY_SYSPWR         BIT(17)
#define B_AX_EN_WLON            BIT(16)
#define B_AX_APDM_HPDN          BIT(15)
#define B_AX_PSUS_OFF_CAPC_EN   BIT(14)
#define B_AX_AFSM_PCIE_SUS_EN   BIT(12)
#define B_AX_AFSM_WLSUS_EN      BIT(11)
#define B_AX_APFM_SWLPS         BIT(10)
#define B_AX_APFM_OFFMAC        BIT(9)
#define B_AX_APFN_ONMAC         BIT(8)
#define B_AX_DIS_WLBT_PDNSUSEN_SOPC BIT(18)

#define R_AX_SYS_SDIO_CTRL      0x0070
#define B_AX_PCIE_DIS_L2_CTRL_LDO_HCI BIT(15)
#define B_AX_PCIE_DIS_WLSUS_AFT_PDN BIT(14)

#define R_AX_HCI_OPT_CTRL       0x0074
#define BIT_WAKE_CTRL           BIT(5)

#define R_AX_SYS_ISO_CTRL       0x0000
#define B_AX_ISO_EB2CORE        BIT(8)
#define B_AX_PWC_EV2EF_B15      BIT(15)
#define B_AX_PWC_EV2EF_B14      BIT(14)

#define R_AX_SYS_CLK_CTRL       0x0008
#define B_AX_CPU_CLK_EN         BIT(14)

#define R_AX_SYS_FUNC_EN        0x0002
#define B_AX_FEN_BB_GLB_RSTN    BIT(1)
#define B_AX_FEN_BBRSTB         BIT(0)

#define R_AX_AFE_LDO_CTRL       0x0020
#define B_AX_AON_OFF_PC_EN      BIT(23)

#define R_AX_PLATFORM_ENABLE    0x0088
#define B_AX_PLATFORM_EN        BIT(0)
#define B_AX_WCPU_EN            BIT(1)
#define B_AX_APB_WRAP_EN        BIT(2)
#define B_AX_AXIDMA_EN          BIT(3)

#define R_AX_WLLPS_CTRL         0x0090
#define B_AX_DIS_WLBT_LPSEN_LOPC BIT(1)

/* XTAL SI */
#define R_AX_WLAN_XTAL_SI_CTRL  0x0270
#define B_AX_WL_XTAL_SI_CMD_POLL BIT(31)
#define B_AX_WL_XTAL_GNT        BIT(29)
#define B_AX_WL_XTAL_SI_MODE_MASK GENMASK(25, 24)
#define XTAL_SI_NORMAL_WRITE    0x00
#define B_AX_WL_XTAL_SI_BITMASK_MASK GENMASK(23, 16)
#define B_AX_WL_XTAL_SI_DATA_MASK GENMASK(15, 8)
#define B_AX_WL_XTAL_SI_ADDR_MASK GENMASK(7, 0)

/* Security */
#define R_AX_SEC_CTRL           0x0C00
#define B_AX_SEC_IDMEM_SIZE_CONFIG_MASK GENMASK(17, 16)

/* Firmware Download */
#define R_AX_WCPU_FW_CTRL       0x01E0
#define B_AX_WCPU_FWDL_STS_MASK GENMASK(7, 5)
#define B_AX_FWDL_PATH_RDY      BIT(2)
#define B_AX_H2C_PATH_RDY       BIT(1)
#define B_AX_WCPU_FWDL_EN       BIT(0)

#define R_AX_BOOT_REASON        0x01E6
#define B_AX_BOOT_REASON_MASK   GENMASK(2, 0)

#define R_AX_HALT_H2C_CTRL      0x0160
#define R_AX_HALT_H2C           0x0168
#define B_AX_HALT_H2C_TRIGGER   BIT(0)
#define R_AX_HALT_C2H_CTRL      0x0164
#define R_AX_HALT_C2H           0x016C

#define R_AX_UDM1               0x01F4
#define R_AX_UDM2               0x01F8

/* H2C/C2H Mailbox */
#define R_AX_H2CREG_DATA0       0x8140
#define R_AX_H2CREG_DATA1       0x8144
#define R_AX_H2CREG_DATA2       0x8148
#define R_AX_H2CREG_DATA3       0x814C
#define R_AX_C2HREG_DATA0       0x8150
#define R_AX_C2HREG_DATA1       0x8154
#define R_AX_C2HREG_DATA2       0x8158
#define R_AX_C2HREG_DATA3       0x815C
#define R_AX_H2CREG_CTRL        0x8160
#define B_AX_H2CREG_TRIGGER     BIT(0)
#define R_AX_C2HREG_CTRL        0x8164
#define B_AX_C2HREG_TRIGGER     BIT(0)

/* Host Interrupts */
#define R_AX_HD0IMR             0x8110
#define B_AX_C2H_INT_EN         BIT(0)
#define B_AX_WDT_PTFM_INT_EN    BIT(5)
#define R_AX_HD0ISR             0x8114
#define B_AX_C2H_INT            BIT(0)

/* HCI Function Enable */
#define R_AX_HCI_FUNC_EN        0x8380
#define B_AX_HCI_FUNC_EN        BIT(2)
#define B_AX_HCI_RXDMA_EN       BIT(1)
#define B_AX_HCI_TXDMA_EN       BIT(0)

/* HCI Flow Control */
#define R_AX_HCI_FC_CTRL        0x8A00
#define B_AX_HCI_FC_CH12_FULL_COND_MASK GENMASK(11, 10)
#define B_AX_HCI_FC_WP_CH811_FULL_COND_MASK GENMASK(9, 8)
#define B_AX_HCI_FC_WP_CH07_FULL_COND_MASK GENMASK(7, 6)
#define B_AX_HCI_FC_WD_FULL_COND_MASK GENMASK(5, 4)
#define B_AX_HCI_FC_CH12_EN     BIT(3)
#define B_AX_HCI_FC_MODE_MASK   GENMASK(2, 1)
#define B_AX_HCI_FC_EN          BIT(0)

/* DMAC Function */
#define R_AX_DMAC_FUNC_EN       0x8400
#define B_AX_DMAC_CRPRT         BIT(31)
#define B_AX_MAC_FUNC_EN        BIT(30)
#define B_AX_DMAC_FUNC_EN       BIT(29)
#define B_AX_MPDU_PROC_EN       BIT(28)
#define B_AX_WD_RLS_EN          BIT(27)
#define B_AX_DLE_WDE_EN         BIT(26)
#define B_AX_TXPKT_CTRL_EN      BIT(25)
#define B_AX_STA_SCH_EN         BIT(24)
#define B_AX_DLE_PLE_EN         BIT(23)
#define B_AX_PKT_BUF_EN         BIT(22)
#define B_AX_DMAC_TBL_EN        BIT(21)
#define B_AX_PKT_IN_EN          BIT(20)
#define B_AX_DLE_CPUIO_EN       BIT(19)
#define B_AX_DISPATCHER_EN      BIT(18)
#define B_AX_BBRPT_EN           BIT(17)
#define B_AX_MAC_SEC_EN         BIT(16)
#define B_AX_DMACREG_GCKEN      BIT(15)

#define R_AX_DMAC_CLK_EN        0x8404
#define B_AX_WD_RLS_CLK_EN      BIT(27)
#define B_AX_DLE_WDE_CLK_EN     BIT(26)
#define B_AX_TXPKT_CTRL_CLK_EN  BIT(25)
#define B_AX_STA_SCH_CLK_EN     BIT(24)
#define B_AX_DLE_PLE_CLK_EN     BIT(23)
#define B_AX_PKT_IN_CLK_EN      BIT(20)
#define B_AX_DLE_CPUIO_CLK_EN   BIT(19)
#define B_AX_DISPATCHER_CLK_EN  BIT(18)
#define B_AX_BBRPT_CLK_EN       BIT(17)
#define B_AX_MAC_SEC_CLK_EN     BIT(16)
#define B_AX_AXIDMA_CLK_EN      BIT(9)

/* DMAC Error Interrupts */
#define R_AX_DMAC_ERR_IMR       0x8520
#define R_AX_DMAC_ERR_ISR       0x8524

/* CMAC Function */
#define R_AX_CMAC_FUNC_EN       0xC000
#define B_AX_CMAC_EN            BIT(31)
#define B_AX_CMAC_TXEN          BIT(30)
#define B_AX_CMAC_RXEN          BIT(29)
#define B_AX_PHYINTF_EN         BIT(28)
#define B_AX_CMAC_DMA_EN        BIT(27)
#define B_AX_PTCLTOP_EN         BIT(26)
#define B_AX_SCHEDULER_EN       BIT(25)
#define B_AX_TMAC_EN            BIT(24)
#define B_AX_RMAC_EN            BIT(23)
#define B_AX_CMAC_CRPRT         BIT(22)
#define B_AX_FORCE_CMACREG_GCKEN BIT(21)

#define R_AX_CMAC_ERR_IMR       0xC160
#define R_AX_CMAC_ERR_ISR       0xC164

/* PCIe HCI */
#define R_AX_PCIE_INIT_CFG1     0x1000
#define B_AX_RXBD_MODE          BIT(18)
#define B_AX_PCIE_MAX_RXDMA_MASK GENMASK(16, 14)
#define B_AX_RXHCI_EN           BIT(13)
#define B_AX_LATENCY_CONTROL    BIT(12)
#define B_AX_TXHCI_EN           BIT(11)
#define B_AX_PCIE_MAX_TXDMA_MASK GENMASK(10, 8)
#define B_AX_TXHCI_EN_V1        BIT(7)
#define B_AX_RXHCI_EN_V1        BIT(15)
#define B_AX_TX_TRUNC_MODE      BIT(5)
#define B_AX_RX_TRUNC_MODE      BIT(4)
#define B_AX_RST_BDRAM          BIT(3)
#define B_AX_DIS_RXDMA_PRE      BIT(2)
#define B_AX_STOP_AXI_MST       BIT(17)
#define B_AX_HAXI_RST_KEEP_REG  BIT(16)
#define B_AX_PCIE_TXRST_KEEP_REG BIT(22)
#define B_AX_PCIE_RXRST_KEEP_REG BIT(23)

#define R_AX_PCIE_INIT_CFG2     0x1004
#define R_AX_PCIE_INIT_CFG3     0x1008

#define R_AX_PCIE_DMA_STOP1     0x1010
#define B_AX_STOP_WPDMA         BIT(19)
#define B_AX_STOP_CH12          BIT(18)
#define B_AX_STOP_CH9           BIT(17)
#define B_AX_STOP_CH8           BIT(16)
#define B_AX_STOP_ACH7          BIT(15)
#define B_AX_STOP_ACH6          BIT(14)
#define B_AX_STOP_ACH5          BIT(13)
#define B_AX_STOP_ACH4          BIT(12)
#define B_AX_STOP_ACH3          BIT(11)
#define B_AX_STOP_ACH2          BIT(10)
#define B_AX_STOP_ACH1          BIT(9)
#define B_AX_STOP_ACH0          BIT(8)
#define B_AX_STOP_PCIEIO        BIT(19)

#define R_AX_PCIE_DMA_MONITOR   0x101C
#define B_AX_PCIE_DMA_IDLE      BIT(0)
#define R_AX_PCIE_DMA_BUSY1     0x101C
#define B_AX_ACH7_BUSY          BIT(15)
#define B_AX_ACH6_BUSY          BIT(14)
#define B_AX_ACH5_BUSY          BIT(13)
#define B_AX_ACH4_BUSY          BIT(12)
#define B_AX_ACH3_BUSY          BIT(11)
#define B_AX_ACH2_BUSY          BIT(10)
#define B_AX_ACH1_BUSY          BIT(9)
#define B_AX_ACH0_BUSY          BIT(8)
#define B_AX_WPDMA_BUSY         BIT(19)
#define B_AX_HAXIIO_BUSY        BIT(20)

#define R_AX_TXWD_RWPTR_CLR     0x101C

#define R_AX_PCIE_DMA_BUSY3     0x1208
#define B_AX_RPQ_BUSY           BIT(1)
#define B_AX_RXQ_BUSY           BIT(0)

#define R_AX_PCIE_DMA_STOP2     0x1310
#define B_AX_STOP_CH11          BIT(1)
#define B_AX_STOP_CH10          BIT(0)

#define R_AX_TXBD_RWPTR_CLR1    0x1014
#define B_AX_CLR_CH12_IDX       BIT(10)
#define B_AX_CLR_CH9_IDX        BIT(9)
#define B_AX_CLR_CH8_IDX        BIT(8)
#define B_AX_CLR_ACH7_IDX       BIT(7)
#define B_AX_CLR_ACH6_IDX       BIT(6)
#define B_AX_CLR_ACH5_IDX       BIT(5)
#define B_AX_CLR_ACH4_IDX       BIT(4)
#define B_AX_CLR_ACH3_IDX       BIT(3)
#define B_AX_CLR_ACH2_IDX       BIT(2)
#define B_AX_CLR_ACH1_IDX       BIT(1)
#define B_AX_CLR_ACH0_IDX       BIT(0)

#define B_AX_CLR_RXQ_IDX        BIT(0)
#define B_AX_CLR_RPQ_IDX        BIT(1)

#define R_AX_RXBD_RWPTR_CLR     0x1018

#define R_AX_PCIE_INIT_CFG2     0x1004
#define R_AX_PCIE_PS_CTRL       0x1008
#define B_AX_L1OFF_PWR_OFF_EN   BIT(5)

#define R_AX_PCIE_DBG_CTRL      0x11C0
#define B_AX_ASFF_FULL_NO_STK   BIT(1)
#define B_AX_EN_STUCK_DBG       BIT(0)

#define R_AX_TX_ADDRESS_INFO_MODE_SETTING 0x8810
#define B_AX_HOST_ADDR_INFO_8B_SEL BIT(0)

#define R_AX_PKTIN_SETTING      0x9A00
#define B_AX_WD_ADDR_INFO_LENGTH BIT(1)

#define R_AX_INT_MIT_RX         0x10D4
#define R_AX_PCIE_HRPWM         0x10C0
#define R_AX_PCIE_EXP_CTRL      0x13F0
#define B_AX_SIC_EN_FORCE_CLKREQ BIT(4)
#define R_AX_HAXI_EXP_CTRL      0x1204

/* PCIe Interrupts */
#define R_AX_PCIE_HIMR00        0x10B0
#define B_AX_RDU_INT            BIT(31)
#define B_AX_RXP1DMA_INT        BIT(12)
#define B_AX_RPQDMA_INT         BIT(11)
#define B_AX_RXBD_FULL_INT      BIT(10)
#define B_AX_RXDMA_INT          BIT(9)
#define B_AX_RPQBD_FULL_INT     BIT(8)

#define R_AX_PCIE_HISR00        0x10B4
#define R_AX_PCIE_HIMR10        0x13B0
#define R_AX_PCIE_HISR10        0x13B4

#define R_AX_HAXI_DMA_BUSY2     0x11C8
#define B_AX_CH11_BUSY          BIT(1)
#define B_AX_CH10_BUSY          BIT(0)

/* TX DMA Channels */
#define R_AX_ACH0_TXBD_NUM      0x1000
#define R_AX_ACH0_TXBD_IDX      0x1008
#define R_AX_ACH0_TXBD_DESA_L   0x1050
#define R_AX_ACH0_TXBD_DESA_H   0x1054
#define R_AX_ACH0_BDRAM_CTRL    0x1060

#define R_AX_ACH1_TXBD_NUM      0x1100
#define R_AX_ACH1_TXBD_IDX      0x1108
#define R_AX_ACH1_TXBD_DESA_L   0x1150
#define R_AX_ACH1_TXBD_DESA_H   0x1154
#define R_AX_ACH1_BDRAM_CTRL    0x1160

#define R_AX_ACH4_TXBD_NUM      0x1400
#define R_AX_ACH4_TXBD_IDX      0x1408
#define R_AX_ACH4_TXBD_DESA_L   0x1450
#define R_AX_ACH4_TXBD_DESA_H   0x1454
#define R_AX_ACH4_BDRAM_CTRL    0x1460

#define R_AX_ACH5_TXBD_NUM      0x1500
#define R_AX_ACH5_TXBD_IDX      0x1508
#define R_AX_ACH5_TXBD_DESA_L   0x1550
#define R_AX_ACH5_TXBD_DESA_H   0x1554
#define R_AX_ACH5_BDRAM_CTRL    0x1560

#define R_AX_ACH6_TXBD_NUM      0x1600
#define R_AX_ACH6_TXBD_IDX      0x1608
#define R_AX_ACH6_TXBD_DESA_L   0x1650
#define R_AX_ACH6_TXBD_DESA_H   0x1654
#define R_AX_ACH6_BDRAM_CTRL    0x1660

#define R_AX_ACH7_TXBD_NUM      0x1700
#define R_AX_ACH7_TXBD_IDX      0x1708
#define R_AX_ACH7_TXBD_DESA_L   0x1750
#define R_AX_ACH7_TXBD_DESA_H   0x1754
#define R_AX_ACH7_BDRAM_CTRL    0x1760

#define R_AX_CH10_TXBD_NUM      0x1800
#define R_AX_CH10_TXBD_IDX      0x1808
#define R_AX_CH10_TXBD_DESA_L   0x1850
#define R_AX_CH10_TXBD_DESA_H   0x1854
#define R_AX_CH10_BDRAM_CTRL    0x1860

#define R_AX_CH11_TXBD_NUM      0x1900
#define R_AX_CH11_TXBD_IDX      0x1908
#define R_AX_CH11_TXBD_DESA_L   0x1950
#define R_AX_CH11_TXBD_DESA_H   0x1954
#define R_AX_CH11_BDRAM_CTRL    0x1960

/* CH12 (H2C/FW download) TX BD — special channel, different offset layout */
#define R_AX_CH12_TXBD_NUM      0x1038
#define R_AX_CH12_TXBD_IDX      0x1080
#define R_AX_CH12_TXBD_DESA_L   0x1160
#define R_AX_CH12_TXBD_DESA_H   0x1164
#define R_AX_CH12_BDRAM_CTRL    0x1228

/* TX BD (8-byte descriptor) fields */
#define B_AX_TXBD_LS            BIT(14)
#define B_AX_TXBD_DMA_HI_MASK   GENMASK(13, 6)
#define B_AX_TXBD_LEN           GENMASK(5, 0)

/* BDRAM_CTRL fields */
#define B_AX_BDRAM_SIDX_MASK    GENMASK(7, 0)
#define B_AX_BDRAM_MAX_MASK     GENMASK(15, 8)
#define B_AX_BDRAM_MIN_MASK     GENMASK(23, 16)

/* Doorbell write-pointer mask (host WP in bits 11:0, HW RP in bits 27:16) */
#define B_AX_TXBD_IDX_MASK      GENMASK(11, 0)
#define B_AX_TXBD_HW_IDX_MASK   GENMASK(27, 16)

/* DMA error / debug registers */
#define R_AX_HAXI_IDCT          0x10BC
#define B_AX_TXBD_LEN0_ERR_IDCT  BIT(3)
#define B_AX_TXBD_4KBOUND_ERR_IDCT BIT(2)

/* TX doorbell valid bit */
#define B_AX_TXWP_VALID         BIT(15)

/* RX BD */
#define R_AX_RX0_RXBD_NUM       0x1020
#define R_AX_RX0_RXBD_IDX       0x1024
#define R_AX_RX0_RXBD_DESA_L    0x1028
#define R_AX_RX0_RXBD_DESA_H    0x102C
#define R_AX_RX0_BDRAM_CTRL     0x1030

#define R_AX_RPQ_RXBD_NUM       0x1020
#define R_AX_RPQ_RXBD_IDX       0x1024
#define R_AX_RPQ_RXBD_DESA_L    0x1028
#define R_AX_RPQ_RXBD_DESA_H    0x102C
#define R_AX_RPQ_BDRAM_CTRL     0x1030

#define R_AX_RX1_RXBD_NUM       0x1120
#define R_AX_RX1_RXBD_IDX       0x1124
#define R_AX_RX1_RXBD_DESA_L    0x1128
#define R_AX_RX1_RXBD_DESA_H    0x112C
#define R_AX_RX1_BDRAM_CTRL     0x1130

/* MAC AX constants */
#define FWDL_WAIT_CNT           400000
#define FWDL_SECTION_PER_PKT_LEN 2020

enum rtw89_fw_dl_status {
    RTW89_FWDL_INITIAL_STATE    = 0,
    RTW89_FWDL_FWDL_ONGOING     = 1,
    RTW89_FWDL_CHECKSUM_FAIL    = 2,
    RTW89_FWDL_SECURITY_FAIL    = 3,
    RTW89_FWDL_CV_NOT_MATCH     = 4,
    RTW89_FWDL_WCPU_FWDL_RDY    = 6,
    RTW89_FWDL_WCPU_FW_INIT_RDY = 7,
};

#endif
