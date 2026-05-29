#ifndef RTW89_H
#define RTW89_H

#ifdef __cplusplus
extern "C" {
#endif

#include "platform/osdep.h"
#include "reg.h"

#define RTW89_TXCH_NUM          12
#define RTW89_RXCH_NUM          2
#define RTW89_PCI_TXBD_NUM_MAX  1024
#define RTW89_PCI_RXBD_NUM_MAX  1024
#define RTW89_PCI_TXWD_NUM_MAX  2048

/* Power states */
#define RTW89_PS_MODE_RFOFF       BIT(0)
#define RTW89_PS_MODE_CLK_GATED   BIT(1)
#define RTW89_PS_MODE_PWR_GATED   BIT(2)
#define RPWM_SEQ_NUM_MAX         3
#define CPWM_SEQ_NUM_MAX         3

/* FW types */
enum rtw89_fw_type {
    RTW89_FW_NORMAL = 0,
    RTW89_FW_WOWLAN,
    RTW89_FW_BBMCU0,
    RTW89_FW_BBMCU1,
};

/* FW download status */
#define FWDL_SECTION_MAX_NUM     16
#define FWDL_CHK_RDY_CNT         30000
#define FWDL_CHK_RDY_DELAY       500
#define FWDL_PKT_LEN             2020

#define RTW89_DMA_ACH4           BIT(4)
#define RTW89_DMA_ACH5           BIT(5)
#define RTW89_DMA_ACH6           BIT(6)
#define RTW89_DMA_ACH7           BIT(7)
#define RTW89_DMA_B1MG           BIT(8)
#define RTW89_DMA_B1HI           BIT(9)
#define RTW89_DMA_H2C            12

enum rtw89_core_chip_id {
    RTL8852A,
    RTL8852B,
    RTL8852C,
    RTL8852BT,
};

enum rtw89_core_chip_gen {
    RTW89_CHIP_AX,
};

enum rtw89_mac_idx {
    RTW89_MAC_0,
    RTW89_MAC_1,
};

/* H2C header structure (4 bytes, matches Linux struct rtw89_h2c_header) */
struct rtw89_h2c_header {
    u16 hdr;   /* [1:0] cat, [7:2] class, [15:8] func */
    u16 len;   /* [13:0] total_len, [14] req_ack, [15] done_ack */
} __packed;

#define H2C_HEADER_LEN              4

/* H2C hdr bit fields (u16) */
#define H2C_HDR_CAT                 (0x3 << 0)
#define H2C_HDR_CLASS               (0xFC << 0)
#define H2C_HDR_FUNC                (0xFF00 << 0)

/* H2C len bit fields (u16) */
#define H2C_HDR_TOTAL_LEN           (0x3FFF << 0)
#define H2C_HDR_REC_ACK             (0x4000 << 0)
#define H2C_HDR_DONE_ACK            (0x8000 << 0)

/* H2C constants */
#define FWCMD_TYPE_H2C              0
#define H2C_CAT_MAC                 0x1
#define H2C_CL_MAC_FWDL             0x3
#define H2C_FUNC_MAC_FWHDR_DL       0x0
#define H2C_FUNC_MAC_FWDATA_DL      0x1

/* Firmware header structures (simplified) */
struct rtw89_fw_hdr_section {
    u32 w0;
    u32 w1;
    u32 w2;
    u32 w3;
};

struct rtw89_fw_hdr {
    u32 w0;
    u32 w1;
    u32 w2;
    u32 w3;
    u32 w4;
    u32 w5;
    u32 w6;
    u32 w7;
    struct rtw89_fw_hdr_section sections[];
};

struct rtw89_fw_hdr_section_v1 {
    u32 w0;
    u32 w1;
    u32 w2;
    u32 w3;
};

struct rtw89_fw_hdr_v1 {
    u32 w0;
    u32 w1;
    u32 w2;
    u32 w3;
    u32 w4;
    u32 w5;
    u32 w6;
    u32 w7;
    u32 w8;
    u32 w9;
    u32 w10;
    u32 w11;
    struct rtw89_fw_hdr_section_v1 sections[];
};

/* Forward declarations */
struct rtw89_dev;
struct rtw89_chip_info;

/* Callback operations for chip-specific behavior */
struct rtw89_chip_ops {
    int (*pwr_on_func)(struct rtw89_dev *rtwdev);
    int (*pwr_off_func)(struct rtw89_dev *rtwdev);
};

/* Chip info - minimal for power-on */
struct rtw89_chip_info {
    enum rtw89_core_chip_id chip_id;
    enum rtw89_core_chip_gen chip_gen;
    struct rtw89_chip_ops *ops;
    const u8 *fw_basename;
    u32 fifo_size;
    u32 dle_scc_rsvd_size;
    u32 hci_func_en_addr;
    u32 h2c_ctrl_reg;
    u32 c2h_ctrl_reg;
    u32 h2c_desc_size;
    u32 txwd_body_size;
    u32 txwd_info_size;
};

/* TX BD entry format for AX chips (8 bytes)
 * dword0: DMA[33:26] in bits 13:6, LS in bit 14
 * dword1: DMA[25:0] in bits 31:6, length in bits 5:0 */
struct rtw89_pci_tx_bd_32 {
    u32 dword0;
    u32 dword1;
} __packed;

/* TX descriptor body for AX chips (24 bytes / 6 dwords) */
struct rtw89_txwd_body {
    u32 dword0;
    u32 dword1;
    u32 dword2;
    u32 dword3;
    u32 dword4;
    u32 dword5;
} __packed;

/* TX descriptor info extension for AX chips (24 bytes / 6 dwords) */
struct rtw89_txwd_info {
    u32 dword0;
    u32 dword1;
    u32 dword2;
    u32 dword3;
    u32 dword4;
    u32 dword5;
} __packed;

/* TXWD dword0 bit positions (for H2C firmware download) */
#define TXWD_BODY0_FW_DL        BIT(20)
#define TXWD_BODY0_CH_DMA(v)    (((v) & 0xF) << 16)
#define TXWD_BODY0_WP_OFFSET(v) (((v) & 0xFF) << 24)

/* TXWD dword2 bit positions */
#define TXWD_BODY2_TXPKT_SIZE(v) ((v) & 0x3FFF)

/* CH12 TX ring state */
#define RTW89_CH12_NUM_BDS      4
#define RTW89_CH12_BUF_SIZE     (24 + 8 + 2048)  /* h2c_desc_size + max_h2c_header + max_payload */

struct rtw89_pci_ch12_ring {
    struct rtw89_pci_tx_bd_32 *bd_vaddr;
    dma_addr_t bd_phys;
    u8 *buf_vaddr;
    dma_addr_t buf_phys;
    u32 wp;
    u32 num_bds;
    /* Debug fields populated during send_h2c */
    u32 dbg_bd_dword0;
    u32 dbg_bd_dword1;
    u32 dbg_txbd_idx_write;
    u32 dbg_txbd_idx_poll;
    u16 dbg_total_size;
    int  dbg_send_ret;
};

/* Core device struct */
struct rtw89_dev {
    void *mmio;
    struct rtw89_chip_info *chip;
    u8 mac_addr[6];
    u8 h2c_seq;
    u8 rec_seq;
    u32 flags;
#define RTW89_FLAG_POWERON       BIT(0)
#define RTW89_FLAG_FW_RDY        BIT(1)
#define RTW89_FLAG_DMAC_FUNC     BIT(2)
#define RTW89_FLAG_CMAC0_FUNC    BIT(3)
#define RTW89_FLAG_CMAC1_FUNC    BIT(4)
#define RTW89_FLAG_CMAC0_PWR     BIT(5)
#define RTW89_FLAG_CMAC1_PWR     BIT(6)
#define RTW89_FLAG_PROBE_DONE    BIT(7)
#define RTW89_FLAG_UNPLUGGED     BIT(8)
#define RTW89_FLAG_CH12_INIT     BIT(9)
    u32 rpwm_seq_num;
    u32 cpwm_seq_num;
    u8 cv;
    u8 rfe_type;
    bool efuse_valid;
    bool power_k_valid;

    /* CH12 H2C/FW download DMA ring state */
    struct rtw89_pci_ch12_ring ch12_ring;
};

/* Register access macros */
static inline u32 rtw89_read32(struct rtw89_dev *rtwdev, u32 addr)
{
    if (!rtwdev || !rtwdev->mmio) return 0;
    return *(volatile u32 *)((uintptr_t)rtwdev->mmio + addr);
}

static inline u16 rtw89_read16(struct rtw89_dev *rtwdev, u32 addr)
{
    if (!rtwdev || !rtwdev->mmio) return 0;
    return *(volatile u16 *)((uintptr_t)rtwdev->mmio + addr);
}

static inline u8 rtw89_read8(struct rtw89_dev *rtwdev, u32 addr)
{
    if (!rtwdev || !rtwdev->mmio) return 0;
    return *(volatile u8 *)((uintptr_t)rtwdev->mmio + addr);
}

static inline void rtw89_write32(struct rtw89_dev *rtwdev, u32 addr, u32 val)
{
    if (!rtwdev || !rtwdev->mmio) return;
    *(volatile u32 *)((uintptr_t)rtwdev->mmio + addr) = val;
}

static inline void rtw89_write16(struct rtw89_dev *rtwdev, u32 addr, u16 val)
{
    if (!rtwdev || !rtwdev->mmio) return;
    *(volatile u16 *)((uintptr_t)rtwdev->mmio + addr) = val;
}

static inline void rtw89_write8(struct rtw89_dev *rtwdev, u32 addr, u8 val)
{
    if (!rtwdev || !rtwdev->mmio) return;
    *(volatile u8 *)((uintptr_t)rtwdev->mmio + addr) = val;
}

static inline void rtw89_write32_set(struct rtw89_dev *rtwdev, u32 addr, u32 bits)
{
    u32 v = rtw89_read32(rtwdev, addr);
    rtw89_write32(rtwdev, addr, v | bits);
}

static inline void rtw89_write32_clr(struct rtw89_dev *rtwdev, u32 addr, u32 bits)
{
    u32 v = rtw89_read32(rtwdev, addr);
    rtw89_write32(rtwdev, addr, v & ~bits);
}

static inline void rtw89_write8_set(struct rtw89_dev *rtwdev, u32 addr, u8 bits)
{
    u8 v = rtw89_read8(rtwdev, addr);
    rtw89_write8(rtwdev, addr, v | bits);
}

static inline void rtw89_write8_clr(struct rtw89_dev *rtwdev, u32 addr, u8 bits)
{
    u8 v = rtw89_read8(rtwdev, addr);
    rtw89_write8(rtwdev, addr, v & ~bits);
}

static inline u32 rtw89_read32_mask(struct rtw89_dev *rtwdev, u32 addr, u32 mask)
{
    return rtw89_read32(rtwdev, addr) & mask;
}

static inline void rtw89_write32_mask(struct rtw89_dev *rtwdev, u32 addr,
                                       u32 mask, u32 val)
{
    u32 v = rtw89_read32(rtwdev, addr);
    rtw89_write32(rtwdev, addr, (v & ~mask) | (val & mask));
}

static inline void rtw89_write16_mask(struct rtw89_dev *rtwdev, u32 addr,
                                       u32 mask, u16 val)
{
    u16 v = rtw89_read16(rtwdev, addr);
    rtw89_write16(rtwdev, addr, (v & ~mask) | (val & mask));
}

/* Function declarations */
int rtw89_core_init(struct rtw89_dev *rtwdev);
void rtw89_core_deinit(struct rtw89_dev *rtwdev);

int rtw89_mac_pwr_on(struct rtw89_dev *rtwdev);
int rtw89_mac_pwr_off(struct rtw89_dev *rtwdev);
int rtw89_mac_init(struct rtw89_dev *rtwdev);
int rtw89_mac_write_xtal_si(struct rtw89_dev *rtwdev, u8 addr, u8 mask, u8 data);
void rtw89_mac_hci_func_en_ax(struct rtw89_dev *rtwdev);
void rtw89_mac_ctrl_hci_dma_trx(struct rtw89_dev *rtwdev, bool enable);
void rtw89_mac_dmac_pre_init(struct rtw89_dev *rtwdev);

int rtw89_fw_download(struct rtw89_dev *rtwdev, const u8 *fw_data, u32 fw_size);
int rtw89_fw_check_rdy(struct rtw89_dev *rtwdev);
int rtw89_fwdl_enable_wcpu(struct rtw89_dev *rtwdev, bool en);

int rtw89_pci_ch12_ring_init(struct rtw89_dev *rtwdev);
void rtw89_pci_ch12_ring_deinit(struct rtw89_dev *rtwdev);
int rtw89_pci_ch12_send_h2c(struct rtw89_dev *rtwdev, const u8 *data, u32 len,
                            bool fw_dl);

int rtw8852b_register_chip(struct rtw89_dev *rtwdev);

/* poll timeout helper */
#define read_poll_timeout(read_fn, val, cond, sleep_us, timeout_us, rtwdev, addr) \
    ({ \
        int __t = (timeout_us) / (sleep_us ? sleep_us : 1); \
        int __ret = 0; \
        do { \
            (val) = read_fn(rtwdev, addr); \
            if (cond) break; \
            if (sleep_us) fsleep(sleep_us); \
        } while (--__t); \
        if (!__t) __ret = -1; \
        __ret; \
    })

#define fsleep(us) IODelay(us)

#ifdef __cplusplus
}
#endif

#endif
