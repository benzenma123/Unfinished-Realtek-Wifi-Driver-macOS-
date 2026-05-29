#include "rtw89.h"

static void rtw89_pci_ch12_write_bd(struct rtw89_dev *rtwdev, u32 idx,
                                     dma_addr_t dma, u16 length, bool last_seg)
{
    struct rtw89_pci_tx_bd_32 *bd = &rtwdev->ch12_ring.bd_vaddr[idx];
    u32 dma_lo = (u32)(dma & 0xFFFFFFFF);
    u32 dma_hi = (u32)((dma >> 32) & 0xFF);
    u16 opt = 0;
    if (last_seg)
        opt |= B_AX_TXBD_LS; /* BIT(14) = LS flag in option field */
    opt |= (dma_hi << 6) & GENMASK(13, 6); /* DMA_HI in bits 13:6 */
    bd->dword0 = length | ((u32)opt << 16);
    bd->dword1 = dma_lo;
}

static void rtw89_pci_ch12_fill_txwd_body(struct rtw89_dev *rtwdev,
                                           struct rtw89_txwd_body *wd,
                                           u16 pkt_size, bool fw_dl)
{
    wd->dword0 = TXWD_BODY0_CH_DMA(RTW89_DMA_H2C);
    if (fw_dl)
        wd->dword0 |= TXWD_BODY0_FW_DL;
    wd->dword1 = 0;
    wd->dword2 = TXWD_BODY2_TXPKT_SIZE(pkt_size);
    wd->dword3 = 0;
    wd->dword4 = 0;
    wd->dword5 = 0;
}

static void rtw89_pci_ch12_fill_h2c_hdr(struct rtw89_h2c_header *hdr,
                                         u8 cat, u8 cls, u8 func, u16 len,
                                         bool req_ack)
{
    hdr->hdr = (u16)((cat & 0x3) |
                     ((cls & 0x3F) << 2) |
                     ((func & 0xFF) << 8));
    hdr->len = (u16)(len & 0x3FFF);
    if (req_ack)
        hdr->len |= (u16)H2C_HDR_REC_ACK;
}

int rtw89_pci_ch12_ring_init(struct rtw89_dev *rtwdev)
{
    struct rtw89_pci_ch12_ring *ring = &rtwdev->ch12_ring;
    u32 num_bds = ring->num_bds;
    u32 ctrl;

    if (!ring->bd_vaddr || !ring->buf_vaddr || !num_bds) {
        IOLog("rtw89: CH12 ring not allocated\n");
        return -1;
    }

    memset(ring->bd_vaddr, 0, num_bds * sizeof(struct rtw89_pci_tx_bd_32));
    __sync_synchronize();

    rtw89_write32(rtwdev, R_AX_CH12_TXBD_NUM, num_bds);

    rtw89_write32(rtwdev, R_AX_CH12_TXBD_DESA_L,
                  (u32)(ring->bd_phys & 0xFFFFFFFF));
    rtw89_write32(rtwdev, R_AX_CH12_TXBD_DESA_H,
                  (u32)((ring->bd_phys >> 32) & 0xFFFFFFFF));

    ctrl = (30 & B_AX_BDRAM_SIDX_MASK) |
           ((4 << 8) & B_AX_BDRAM_MAX_MASK) |
           ((1 << 16) & B_AX_BDRAM_MIN_MASK);
    rtw89_write32(rtwdev, R_AX_CH12_BDRAM_CTRL, ctrl);

    ring->wp = 0;
    rtwdev->flags |= RTW89_FLAG_CH12_INIT;

    IOLog("rtw89: CH12 ring init done (num=%d, bd_phys=0x%llx, buf_phys=0x%llx)\n",
          num_bds, ring->bd_phys, ring->buf_phys);
    return 0;
}

void rtw89_pci_ch12_ring_deinit(struct rtw89_dev *rtwdev)
{
    rtwdev->flags &= ~RTW89_FLAG_CH12_INIT;
    rtwdev->ch12_ring.bd_vaddr = NULL;
    rtwdev->ch12_ring.buf_vaddr = NULL;
    rtwdev->ch12_ring.bd_phys = 0;
    rtwdev->ch12_ring.buf_phys = 0;
    rtwdev->ch12_ring.num_bds = 0;
    rtwdev->ch12_ring.wp = 0;
    IOLog("rtw89: CH12 ring deinit\n");
}

int rtw89_pci_ch12_send_h2c(struct rtw89_dev *rtwdev, const u8 *data,
                             u32 len, bool fw_dl)
{
    struct rtw89_pci_ch12_ring *ring = &rtwdev->ch12_ring;
    u32 idx = 0;
    u32 wp;
    u8 *buf = ring->buf_vaddr;
    struct rtw89_txwd_body *wd;
    struct rtw89_h2c_header *hdr;
    u16 pkt_size = (u16)(H2C_HEADER_LEN + len);
    u16 total_size = 24 + pkt_size;
    u32 val;
    int ret;

    if (!ring->buf_vaddr || !ring->bd_vaddr) {
        IOLog("rtw89: CH12 ring not initialized\n");
        return -1;
    }

    memset(buf, 0, total_size);

    wd = (struct rtw89_txwd_body *)buf;
    rtw89_pci_ch12_fill_txwd_body(rtwdev, wd, pkt_size, fw_dl);

    hdr = (struct rtw89_h2c_header *)(buf + 24);
    rtw89_pci_ch12_fill_h2c_hdr(hdr, H2C_CAT_MAC, H2C_CL_MAC_FWDL, 0,
                                 (u16)len, true);

    if (len > 0)
        memcpy(buf + 24 + H2C_HEADER_LEN, data, len);

    rtw89_pci_ch12_write_bd(rtwdev, idx, ring->buf_phys,
                             total_size, true);
    ring->dbg_bd_dword0 = ring->bd_vaddr[idx].dword0;
    ring->dbg_total_size = total_size;
    ring->dbg_bd_dword1 = ring->bd_vaddr[idx].dword1;

    __sync_synchronize(); /* ensure BD content globally visible before doorbell */
    wp = (idx + 1) | B_AX_TXWP_VALID;
    rtw89_write32(rtwdev, R_AX_CH12_TXBD_IDX, wp);
    ring->dbg_txbd_idx_write = rtw89_read32(rtwdev, R_AX_CH12_TXBD_IDX);

    ret = read_poll_timeout(rtw89_read32, val,
                            ((val & B_AX_TXBD_HW_IDX_MASK) >> 16) == (idx + 1),
                            10, 100000, rtwdev, R_AX_CH12_TXBD_IDX);
    ring->dbg_txbd_idx_poll = val;

    if (ret) {
        IOLog("rtw89: CH12 TX timeout (wp=%d, hw_idx=0x%08x)\n", wp, val);
        ring->dbg_send_ret = -1;
        return -1;
    }

    ring->wp = wp;
    ring->dbg_send_ret = 0;
    return 0;
}
