#include "rtw8723d.h"

static const struct rtw_pwr_seq_cmd trans_carddis_to_cardemu_8723d[] = {
	{0x0005, RTW_PWR_CUT_ALL_MSK, RTW_PWR_INTF_ALL_MSK,
	 RTW_PWR_ADDR_MAC, RTW_PWR_CMD_WRITE, BIT(3) | BIT(4) | BIT(7), 0},
	{0x0300, RTW_PWR_CUT_ALL_MSK, RTW_PWR_INTF_PCI_MSK,
	 RTW_PWR_ADDR_MAC, RTW_PWR_CMD_WRITE, 0xFF, 0},
	{0x0301, RTW_PWR_CUT_ALL_MSK, RTW_PWR_INTF_PCI_MSK,
	 RTW_PWR_ADDR_MAC, RTW_PWR_CMD_WRITE, 0xFF, 0},
	{0xFFFF, RTW_PWR_CUT_ALL_MSK, RTW_PWR_INTF_ALL_MSK,
	 0, RTW_PWR_CMD_END, 0, 0},
};

static const struct rtw_pwr_seq_cmd trans_cardemu_to_act_8723d[] = {
	{0x0005, RTW_PWR_CUT_ALL_MSK, RTW_PWR_INTF_PCI_MSK,
	 RTW_PWR_ADDR_MAC, RTW_PWR_CMD_WRITE,
	 BIT(3) | BIT(2) | BIT(1), BIT(3) | BIT(2) | BIT(1)},
	{0x0005, RTW_PWR_CUT_ALL_MSK, RTW_PWR_INTF_PCI_MSK,
	 RTW_PWR_ADDR_MAC, RTW_PWR_CMD_POLLING,
	 BIT(3) | BIT(2) | BIT(1), BIT(3) | BIT(2) | BIT(1)},
	{0x0001, RTW_PWR_CUT_ALL_MSK, RTW_PWR_INTF_PCI_MSK,
	 RTW_PWR_ADDR_MAC, RTW_PWR_CMD_WRITE, BIT(0), BIT(0)},
	{0xFFFF, RTW_PWR_CUT_ALL_MSK, RTW_PWR_INTF_ALL_MSK,
	 0, RTW_PWR_CMD_END, 0, 0},
};

static const struct rtw_pwr_seq_cmd trans_act_to_cardemu_8723d[] = {
	{0x0001, RTW_PWR_CUT_ALL_MSK, RTW_PWR_INTF_PCI_MSK,
	 RTW_PWR_ADDR_MAC, RTW_PWR_CMD_WRITE, BIT(0), 0},
	{0xFFFF, RTW_PWR_CUT_ALL_MSK, RTW_PWR_INTF_ALL_MSK,
	 0, RTW_PWR_CMD_END, 0, 0},
};

static const struct rtw_pwr_seq_cmd trans_cardemu_to_carddis_8723d[] = {
	{0x0006, RTW_PWR_CUT_ALL_MSK, RTW_PWR_INTF_PCI_MSK,
	 RTW_PWR_ADDR_MAC, RTW_PWR_CMD_WRITE, BIT(0), BIT(0)},
	{0x0020, RTW_PWR_CUT_ALL_MSK, RTW_PWR_INTF_PCI_MSK,
	 RTW_PWR_ADDR_MAC, RTW_PWR_CMD_WRITE, 0xFF, 0xFF},
	{0x0005, RTW_PWR_CUT_ALL_MSK, RTW_PWR_INTF_PCI_MSK,
	 RTW_PWR_ADDR_MAC, RTW_PWR_CMD_WRITE,
	 BIT(3) | BIT(2) | BIT(1), BIT(3) | BIT(2) | BIT(1)},
	{0xFFFF, RTW_PWR_CUT_ALL_MSK, RTW_PWR_INTF_ALL_MSK,
	 0, RTW_PWR_CMD_END, 0, 0},
};

static const struct rtw_pwr_seq_cmd * const card_enable_flow_8723d[] = {
	trans_carddis_to_cardemu_8723d,
	trans_cardemu_to_act_8723d,
	NULL
};

static const struct rtw_pwr_seq_cmd * const card_disable_flow_8723d[] = {
	trans_act_to_cardemu_8723d,
	trans_cardemu_to_carddis_8723d,
	NULL
};

static int rtw8723d_power_on(struct rtw_dev *rtwdev)
{
	return rtw_power_on(rtwdev);
}

static void rtw8723d_power_off(struct rtw_dev *rtwdev)
{
	rtw_power_off(rtwdev);
}

static int rtw8723d_mac_init(struct rtw_dev *rtwdev)
{
	(void)rtwdev;
	return 0;
}

static int rtw8723d_read_efuse(struct rtw_dev *rtwdev, u8 *map)
{
	(void)rtwdev;
	(void)map;
	return 0;
}

static void rtw8723d_phy_set_param(struct rtw_dev *rtwdev)
{
	(void)rtwdev;
}

static void rtw8723d_set_channel(struct rtw_dev *rtwdev, u8 channel,
				 u8 bw, u8 primary_chan_idx)
{
	rtw_set_channel_mac(rtwdev, channel, bw, primary_chan_idx);
}

static u32 rtw8723d_read_rf(struct rtw_dev *rtwdev, enum rtw_rf_path rf_path,
			    u32 addr, u32 mask)
{
	(void)rtwdev;
	(void)rf_path;
	(void)addr;
	(void)mask;
	return 0;
}

static bool rtw8723d_write_rf(struct rtw_dev *rtwdev, enum rtw_rf_path rf_path,
			      u32 addr, u32 mask, u32 data)
{
	(void)rtwdev;
	(void)rf_path;
	(void)addr;
	(void)mask;
	(void)data;
	return true;
}

static void rtw8723d_set_tx_power_index(struct rtw_dev *rtwdev)
{
	(void)rtwdev;
}

static void rtw8723d_cfg_ldo25(struct rtw_dev *rtwdev, bool enable)
{
	(void)rtwdev;
	(void)enable;
}

static void rtw8723d_efuse_grant(struct rtw_dev *rtwdev, bool enable)
{
	(void)rtwdev;
	(void)enable;
}

static const struct rtw_chip_ops rtw8723d_ops = {
	.power_on			= rtw8723d_power_on,
	.power_off			= rtw8723d_power_off,
	.mac_init			= rtw8723d_mac_init,
	.read_efuse			= rtw8723d_read_efuse,
	.phy_set_param			= rtw8723d_phy_set_param,
	.set_channel			= rtw8723d_set_channel,
	.read_rf			= rtw8723d_read_rf,
	.write_rf			= rtw8723d_write_rf,
	.set_tx_power_index		= rtw8723d_set_tx_power_index,
	.cfg_ldo25			= rtw8723d_cfg_ldo25,
	.efuse_grant			= rtw8723d_efuse_grant,
	.query_phy_status		= NULL,
	.false_alarm_statistics		= NULL,
	.phy_calibration		= NULL,
	.dpk_track			= NULL,
	.pwr_track			= NULL,
	.cfo_init			= NULL,
	.cfo_track			= NULL,
};

const struct rtw_chip_info rtw8723d_hw_spec = {
	.ops			= &rtw8723d_ops,
	.id			= 0,
	.fw_name		= "rtw88/rtw8723d_fw.bin",
	.tx_pkt_desc_sz		= 40,
	.rx_pkt_desc_sz		= 24,
	.txff_size		= 0x9600,
	.rxff_size		= 0x1E00,
	.fw_rxff_size		= 0x1000,
	.rsvd_drv_pg_num	= 8,
	.band			= 1,
	.page_size		= 128,
	.dig_max		= 0x3e,
	.dig_min		= 0x1c,
	.max_power_index	= 0x3f,
	.ampdu_density		= 5,
	.rx_ldpc		= true,
	.tx_stbc		= true,
	.ht_supported		= true,
	.vht_supported		= false,
	.lps_deep_mode_supported = 0,
	.sys_func_en		= 0,
	.pwr_on_seq		= card_enable_flow_8723d,
	.pwr_off_seq		= card_disable_flow_8723d,
	.dig			= &(const struct rtw_hw_reg){ .addr = 0xc50, .mask = 0x7f },
	.dig_cck		= NULL,
	.rf_base_addr		= {0},
	.rf_sipi_addr		= {0},
	.fw_fifo_addr		= {0},
	.c2h_ra_report_size	= 0,
	.bfer_su_max_num	= 0,
	.bfer_mu_max_num	= 0,
};
