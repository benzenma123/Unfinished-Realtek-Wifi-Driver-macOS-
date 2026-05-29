#include "rtw88.h"

void rtw_set_channel_mac(struct rtw_dev *dev, u8 channel, u8 bw,
			 u8 primary_ch_idx)
{
	u32 val32;
	u8 txsc40 = 0;
	u8 txsc20 = primary_ch_idx;
	if (bw == RTW_CHANNEL_WIDTH_80) {
		if (txsc20 == RTW_SC_20_UPPER || txsc20 == RTW_SC_20_UPMOST)
			txsc40 = RTW_SC_40_UPPER;
		else
			txsc40 = RTW_SC_40_LOWER;
	}
	rtw_write8(dev, REG_DATA_SC,
		   (u8)(BIT_TXSC_20M(txsc20) | BIT_TXSC_40M(txsc40)));

	val32 = rtw_read32(dev, REG_WMAC_TRXPTCL_CTL) & ~BIT_RFMOD;
	switch (bw) {
	case RTW_CHANNEL_WIDTH_80:
		val32 |= BIT_RFMOD_80M;
		break;
	case RTW_CHANNEL_WIDTH_40:
		val32 |= BIT_RFMOD_40M;
		break;
	default:
		break;
	}
	rtw_write32(dev, REG_WMAC_TRXPTCL_CTL, val32);

	val32 = rtw_read32(dev, REG_AFE_CTRL1) & ~(BIT_MAC_CLK_SEL);
	val32 |= (MAC_CLK_HW_DEF_80M << BIT_SHIFT_MAC_CLK_SEL);
	rtw_write32(dev, REG_AFE_CTRL1, val32);

	rtw_write8(dev, REG_USTIME_TSF, MAC_CLK_SPEED);
	rtw_write8(dev, REG_USTIME_EDCA, MAC_CLK_SPEED);

	if (channel > 14) {
		val32 = rtw_read8(dev, REG_CCK_CHECK) | BIT_CHECK_CCK_EN;
		rtw_write8(dev, REG_CCK_CHECK, (u8)val32);
	} else {
		val32 = rtw_read8(dev, REG_CCK_CHECK) & ~BIT_CHECK_CCK_EN;
		rtw_write8(dev, REG_CCK_CHECK, (u8)val32);
	}
}

/* MAC layer initialization
 * Power on -> poll MAC enable -> configure PBP -> configure DMA ->
 * configure MAC registers -> set up LLT
 */

static int rtw_mac_power_on(struct rtw_dev *dev)
{
	u32 val;
	int retries = 100;

	rtw_write16_set(dev, REG_SYS_FUNC_EN,
			BIT_FEN_CPUEN | BIT_FEN_BB_GLB_RST | BIT_FEN_BB_RSTB);

	rtw_write16_set(dev, REG_SYS_FUNC_EN, BIT_FEN_ELDR);

	rtw_write16_set(dev, REG_SYS_FUNC_EN, BIT_FEN_EN_25_1);

	rtw_write32_set(dev, REG_APS_FSMCO, APS_FSMCO_MAC_ENABLE);

	IODelay(1000);

	while (retries--) {
		val = rtw_read32(dev, REG_CR);
		if (val & (BIT_MAC_SEC_EN | BIT_SCHEDULE_EN))
			return 0;
		IODelay(1000);
	}
	pr_err("MAC power on timeout\n");
	return -1;
}

static int rtw_init_mac(struct rtw_dev *dev)
{
	u32 val;
	int ret;

	ret = rtw_mac_power_on(dev);
	if (ret)
		return ret;

	rtw_write8(dev, REG_CR, 0x00);

	IODelay(1000);

	rtw_write8(dev, REG_CR, MAC_TRX_ENABLE);

	IODelay(1000);

	rtw_write32(dev, REG_PBP, 0x22);

	val = rtw_read32(dev, REG_CR);
	if (!(val & MAC_TRX_ENABLE)) {
		pr_err("MAC enable failed, val=0x%08x\n", val);
		return -1;
	}

	return 0;
}

/* Initialize the LLT (Local Loopback Test) table that maps
 * TX FIFO pages to queue types
 */
static int rtw_llt_init(struct rtw_dev *dev)
{
	u16 pg_sz = dev->chip->page_size;
	u32 tot_pg = dev->chip->txff_size / pg_sz;
	u16 rsvd_pg = dev->chip->rsvd_drv_pg_num;
	u16 boundary = (u16)(tot_pg - rsvd_pg);
	u32 val;
	int i;

	if (!pg_sz || !tot_pg)
		return -1;

	rtw_write32(dev, REG_AUTO_LLT, BIT_AUTO_INIT_LLT);

	for (i = 0; i < 100; i++) {
		val = rtw_read32(dev, REG_AUTO_LLT);
		if (!(val & BIT_AUTO_INIT_LLT))
			break;
		IODelay(100);
	}

	if (i == 100) {
		pr_err("LLT auto-init timeout\n");
		return -1;
	}

	rtw_write16(dev, REG_TRXFF_BNDY, boundary & 0xfff);
	rtw_write16(dev, REG_BCNQ_BDNY, boundary & 0xfff);
	rtw_write8(dev, REG_MGQ_BDNY, boundary & 0xff);

	dev->fifo.rsvd_boundary = boundary;
	dev->fifo.txff_pg_num = (u16)tot_pg;
	dev->fifo.acq_pg_num = boundary;

	pr_info("LLT init: page_size=%d total=%d rsvd=%d boundary=%d\n",
		pg_sz, tot_pg, rsvd_pg, boundary);

	return 0;
}

int rtw_mac_init(struct rtw_dev *dev)
{
	int ret;

	ret = rtw_init_mac(dev);
	if (ret) {
		pr_err("Failed to init MAC\n");
		return ret;
	}

	ret = rtw_llt_init(dev);
	if (ret) {
		pr_err("Failed to init LLT\n");
		return ret;
	}

	pr_info("MAC initialized\n");
	return 0;
}

int rtw_mac_postinit(struct rtw_dev *dev)
{
	u32 val;

	val = rtw_read32(dev, REG_RCR);
	val |= BIT_APP_PHYSTS | BIT_APP_FCS | BIT_AM | BIT_AB | BIT_AAP;
	rtw_write32(dev, REG_RCR, val);

	rtw_write8(dev, REG_RX_DRVINFO_SZ, 0);

	rtw_write32(dev, REG_RXDMA_AGG_PG_TH, 0x0A0A);

	rtw_write32(dev, REG_EARLY_MODE_CONTROL, 0x44444444);

	return 0;
}

int rtw_set_channel(struct rtw_dev *dev, u8 channel,
		    u8 bandwidth, u8 primary_chan_idx)
{
	if (!dev->chip->ops->set_channel)
		return -1;

	dev->hal.current_channel = channel;
	dev->hal.current_band_width = bandwidth;
	dev->chip->ops->set_channel(dev, channel, bandwidth, primary_chan_idx);

	return 0;
}
