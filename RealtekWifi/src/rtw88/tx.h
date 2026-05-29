#ifndef __RTW_TX_H__
#define __RTW_TX_H__

#include "platform/osdep.h"

#define TX_DESC_SIZE            40        // 10 dwords for 8822B/8822C/8723D/8821C

// TX descriptor word 0 (offset 0)
#define TX_DESC_W0_TXPKTSIZE    0x0000ffff
#define TX_DESC_W0_QSEL         0x1f000000
#define TX_DESC_W0_OWN          BIT(31)

// TX descriptor word 1 (offset 4)
#define TX_DESC_W1_MACID        0x0000001f
#define TX_DESC_W1_RAID         0x00003f00
#define TX_DESC_W1_DATASC       0x03000000
#define TX_DESC_W1_BMC          BIT(28)
#define TX_DESC_W1_LS           BIT(30)
#define TX_DESC_W1_FS           BIT(31)

// TX descriptor word 2 (offset 8)
#define TX_DESC_W2_AGGEN        BIT(12)
#define TX_DESC_W2_AMPDU_DEN    0x00700000
#define TX_DESC_W2_BW           0x03000000

// TX descriptor word 3 (offset 12)
#define TX_DESC_W3_RATE_ID      0x0000003f
#define TX_DESC_W3_DISDATAFB    BIT(6)
#define TX_DESC_W3_DISRTSFB     BIT(7)
#define TX_DESC_W3_SECTYPE      0x00000300
#define TX_DESC_W3_PKT_OFFSET   0x00ff0000

// TX descriptor word 4 (offset 16)
#define TX_DESC_W4_RTSRATE      0x000003e0
#define TX_DESC_W4_RTSEN        BIT(31)

// TX descriptor word 5 (offset 20)
#define TX_DESC_W5_DATARATE     0x0000003f
#define TX_DESC_W5_SGI          BIT(6)
#define TX_DESC_W5_STBC         BIT(7)

// TX descriptor word 6 (offset 24)
#define TX_DESC_W6_TXANT        0x00000070
#define TX_DESC_W6_NAVUSEHDR    BIT(15)
#define TX_DESC_W6_MAXAGG       0xff000000

// TX descriptor word 7 (offset 28)
#define TX_DESC_W7_HWSSNSEL     0x00000003
#define TX_DESC_W7_HWSEQEN      BIT(4)

static inline void rtw_fill_tx_desc(struct rtw_dev *dev,
				    struct rtw_tx_pkt_info *pkt_info,
				    u8 *txdesc)
{
	u32 w0, w1, w2, w3, w5;

	w0 = (pkt_info->tx_pkt_size & TX_DESC_W0_TXPKTSIZE) |
	     ((pkt_info->qsel << 24) & TX_DESC_W0_QSEL) |
	     TX_DESC_W0_OWN;
	*(volatile u32 *)txdesc = cpu_to_le32(w0);

	w1 = (pkt_info->mac_id & TX_DESC_W1_MACID) |
	     ((pkt_info->rate_id << 8) & TX_DESC_W1_RAID) |
	     (pkt_info->bmc ? TX_DESC_W1_BMC : 0) |
	     (pkt_info->ls ? TX_DESC_W1_LS : 0) |
	     (pkt_info->fs ? TX_DESC_W1_FS : 0);
	*(volatile u32 *)(txdesc + 4) = cpu_to_le32(w1);

	w2 = (pkt_info->ampdu_en ? TX_DESC_W2_AGGEN : 0) |
	     ((pkt_info->ampdu_factor << 20) & TX_DESC_W2_AMPDU_DEN) |
	     ((pkt_info->bw << 24) & TX_DESC_W2_BW);
	*(volatile u32 *)(txdesc + 8) = cpu_to_le32(w2);

	w3 = ((pkt_info->rate_id) & TX_DESC_W3_RATE_ID) |
	     (pkt_info->dis_rate_fallback ? TX_DESC_W3_DISDATAFB : 0) |
	     ((pkt_info->sec_type << 8) & TX_DESC_W3_SECTYPE) |
	     ((pkt_info->pkt_offset << 16) & TX_DESC_W3_PKT_OFFSET);
	*(volatile u32 *)(txdesc + 12) = cpu_to_le32(w3);

	w5 = (pkt_info->rate & TX_DESC_W5_DATARATE) |
	     (pkt_info->short_gi ? TX_DESC_W5_SGI : 0) |
	     (pkt_info->stbc ? TX_DESC_W5_STBC : 0);
	*(volatile u32 *)(txdesc + 20) = cpu_to_le32(w5);

	if (pkt_info->rts)
		*(volatile u32 *)(txdesc + 16) |= cpu_to_le32(TX_DESC_W4_RTSEN);
}

#endif
