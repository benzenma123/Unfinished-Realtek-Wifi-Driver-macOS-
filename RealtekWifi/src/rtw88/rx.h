#ifndef __RTW_RX_H__
#define __RTW_RX_H__

#include "platform/osdep.h"

#define RX_DESC_SIZE            24        // 6 dwords

// RX descriptor word 0 (offset 0)
#define RX_DESC_W0_RXPKTSIZE    0x00003fff
#define RX_DESC_W0_OFFSET       0x001f0000
#define RX_DESC_W0_OWN          BIT(31)
#define RX_DESC_W0_EOP          BIT(30)
#define RX_DESC_W0_FS           BIT(29)
#define RX_DESC_W0_PHYSTS       BIT(28)
#define RX_DESC_W0_SW_DEC       BIT(27)
#define RX_DESC_W0_DRV_INFO_SZ  0x0f000000

// RX descriptor word 1 (offset 4)
#define RX_DESC_W1_MACID        0x0000001f
#define RX_DESC_W1_RX_RATE      0x0000007f
#define RX_DESC_W1_BW           0x00000080
#define RX_DESC_W1_HT           0x00000100
#define RX_DESC_W1_EOSP         BIT(30)
#define RX_DESC_W1_CRC_ERR      BIT(31)

// RX descriptor word 2 (offset 8)
#define RX_DESC_W2_ICV_ERR      BIT(15)
#define RX_DESC_W2_SEC_TYPE     0x000c0000

// RX descriptor word 4 (offset 16)
#define RX_DESC_W4_PPDU_CNT     0x000000ff
#define RX_DESC_W4_RSSI         0x0000ff00
#define RX_DESC_W4_RXSC         0x01ff0000

#define GET_RX_DESC_W0(desc)    le32_to_cpu(*(volatile u32 *)(desc))
#define GET_RX_DESC_W1(desc)    le32_to_cpu(*(volatile u32 *)((u8 *)(desc) + 4))
#define GET_RX_DESC_W2(desc)    le32_to_cpu(*(volatile u32 *)((u8 *)(desc) + 8))
#define GET_RX_DESC_W4(desc)    le32_to_cpu(*(volatile u32 *)((u8 *)(desc) + 16))

#define GET_RX_DESC_RXPKTSIZE(desc)  (GET_RX_DESC_W0(desc) & RX_DESC_W0_RXPKTSIZE)
#define GET_RX_DESC_OFFSET(desc)     ((GET_RX_DESC_W0(desc) >> 16) & 0x1f)
#define GET_RX_DESC_OWN(desc)        ((GET_RX_DESC_W0(desc) >> 31) & 1)
#define GET_RX_DESC_EOP(desc)        ((GET_RX_DESC_W0(desc) >> 30) & 1)
#define GET_RX_DESC_FS(desc)         ((GET_RX_DESC_W0(desc) >> 29) & 1)
#define GET_RX_DESC_PHYSTS(desc)     ((GET_RX_DESC_W0(desc) >> 28) & 1)
#define GET_RX_DESC_DRV_INFO_SZ(desc) ((GET_RX_DESC_W0(desc) >> 24) & 0x0f)
#define GET_RX_DESC_MACID(desc)      (GET_RX_DESC_W1(desc) & RX_DESC_W1_MACID)
#define GET_RX_DESC_RATE(desc)       (GET_RX_DESC_W1(desc) & RX_DESC_W1_RX_RATE)
#define GET_RX_DESC_CRC_ERR(desc)    ((GET_RX_DESC_W1(desc) >> 31) & 1)
#define GET_RX_DESC_ICV_ERR(desc)    ((GET_RX_DESC_W2(desc) >> 15) & 1)
#define GET_RX_DESC_RSSI(desc)       ((GET_RX_DESC_W4(desc) >> 8) & 0xff)

static inline void rtw_query_rx_desc(struct rtw_rx_pkt_stat *pkt_stat,
				     u8 *rx_desc)
{
	u32 w0 = GET_RX_DESC_W0(rx_desc);
	u32 w1 = GET_RX_DESC_W1(rx_desc);

	pkt_stat->pkt_len = w0 & 0x3fff;
	pkt_stat->drv_info_sz = (w0 >> 24) & 0x0f;
	pkt_stat->shift = (w0 >> 16) & 0x1f;
	pkt_stat->phy_status = (w0 >> 28) & 1;
	pkt_stat->crc_err = (w1 >> 31) & 1;
	pkt_stat->icv_err = (GET_RX_DESC_W2(rx_desc) >> 15) & 1;
	pkt_stat->rate = w1 & 0x7f;
	pkt_stat->bw = (w1 >> 7) & 1;
	pkt_stat->decrypted = (w0 >> 27) & 1;
	pkt_stat->is_c2h = false;
	pkt_stat->rssi = (GET_RX_DESC_W4(rx_desc) >> 8) & 0xff;
}

#endif
