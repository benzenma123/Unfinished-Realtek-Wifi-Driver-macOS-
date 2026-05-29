#ifndef RTW88_H
#define RTW88_H

#ifdef __cplusplus
extern "C" {
#endif

#include "platform/osdep.h"
#include "util/list.h"
#include "util/bitmap.h"
#include "rtw88/reg.h"

#define RTW_MAX_MAC_ID_NUM		32
#define RTW_MAX_SEC_CAM_NUM		32
#define RTW_MAX_CHANNEL_NUM_2G		14
#define RTW_MAX_CHANNEL_NUM_5G		49
#define RTW_SCAN_MAX_SSIDS		4
#define RTW_MAX_PATTERN_NUM		12

#define RFREG_MASK			0xfffff
#define TX_PAGE_SIZE_SHIFT		7
#define TX_PAGE_SIZE			(1 << TX_PAGE_SIZE_SHIFT)
#define RTW_CHANNEL_WIDTH_MAX		3
#define RTW_RF_PATH_MAX			4
#define RTW_PORT_NUM			5

#define MAC_CLK_SPEED		80
#define MAC_CLK_HW_DEF_80M	0
#define BIT_SHIFT_MAC_CLK_SEL	20

#define RTW_SC_20_UPPER		1
#define RTW_SC_20_UPMOST	3
#define RTW_SC_40_UPPER		2
#define RTW_SC_40_LOWER		1

#define IS_CH_2G_BAND(ch)		((ch) <= 14)
#define IS_CH_5G_BAND_1(ch)		((ch) >= 36 && (ch) <= 48)
#define IS_CH_5G_BAND_2(ch)		((ch) >= 52 && (ch) <= 64)
#define IS_CH_5G_BAND_3(ch)		((ch) >= 100 && (ch) <= 144)
#define IS_CH_5G_BAND_4(ch)		((ch) >= 149 && (ch) <= 177)
#define IS_CH_5G_BAND(ch)		(IS_CH_5G_BAND_1(ch) || IS_CH_5G_BAND_2(ch) || \
					 IS_CH_5G_BAND_3(ch) || IS_CH_5G_BAND_4(ch))

enum rtw_chip_type {
	RTW_CHIP_TYPE_8822B,
	RTW_CHIP_TYPE_8822C,
	RTW_CHIP_TYPE_8723D,
	RTW_CHIP_TYPE_8821C,
	RTW_CHIP_TYPE_8703B,
	RTW_CHIP_TYPE_8821A,
	RTW_CHIP_TYPE_8812A,
	RTW_CHIP_TYPE_8814A,
};

enum rtw_hci_type {
	RTW_HCI_TYPE_PCIE,
	RTW_HCI_TYPE_USB,
	RTW_HCI_TYPE_SDIO,
	RTW_HCI_TYPE_UNDEFINE,
};

enum rtw_bandwidth {
	RTW_CHANNEL_WIDTH_20	= 0,
	RTW_CHANNEL_WIDTH_40	= 1,
	RTW_CHANNEL_WIDTH_80	= 2,
	RTW_CHANNEL_WIDTH_160	= 3,
	RTW_CHANNEL_WIDTH_80_80	= 4,
};

enum rtw_net_type {
	RTW_NET_NO_LINK		= 0,
	RTW_NET_AD_HOC		= 1,
	RTW_NET_MGD_LINKED	= 2,
	RTW_NET_AP_MODE		= 3,
};

enum rtw_rf_path {
	RF_PATH_A = 0,
	RF_PATH_B = 1,
	RF_PATH_C = 2,
	RF_PATH_D = 3,
};

enum rtw_rf_type {
	RF_1T1R = 0,
	RF_1T2R = 1,
	RF_2T2R = 2,
	RF_2T3R = 3,
	RF_2T4R = 4,
	RF_3T3R = 5,
	RF_3T4R = 6,
	RF_4T4R = 7,
	RF_TYPE_MAX,
};

enum rtw_tx_queue_type {
	RTW_TX_QUEUE_BK = 0x0,
	RTW_TX_QUEUE_BE = 0x1,
	RTW_TX_QUEUE_VI = 0x2,
	RTW_TX_QUEUE_VO = 0x3,
	RTW_TX_QUEUE_BCN = 0x4,
	RTW_TX_QUEUE_MGMT = 0x5,
	RTW_TX_QUEUE_HI0 = 0x6,
	RTW_TX_QUEUE_H2C = 0x7,
	RTK_MAX_TX_QUEUE_NUM
};

enum rtw_rx_queue_type {
	RTW_RX_QUEUE_MPDU = 0x0,
	RTW_RX_QUEUE_C2H = 0x1,
	RTK_MAX_RX_QUEUE_NUM
};

enum rtw_rate_section {
	RTW_RATE_SECTION_CCK = 0,
	RTW_RATE_SECTION_OFDM,
	RTW_RATE_SECTION_HT_1S,
	RTW_RATE_SECTION_HT_2S,
	RTW_RATE_SECTION_VHT_1S,
	RTW_RATE_SECTION_VHT_2S,
	RTW_RATE_SECTION_NUM,
};

enum rtw_trx_desc_rate {
	DESC_RATE1M	= 0x00,
	DESC_RATE2M	= 0x01,
	DESC_RATE5_5M	= 0x02,
	DESC_RATE11M	= 0x03,
	DESC_RATE6M	= 0x04,
	DESC_RATE9M	= 0x05,
	DESC_RATE12M	= 0x06,
	DESC_RATE18M	= 0x07,
	DESC_RATE24M	= 0x08,
	DESC_RATE36M	= 0x09,
	DESC_RATE48M	= 0x0a,
	DESC_RATE54M	= 0x0b,
	DESC_RATEMCS0	= 0x0c,
	DESC_RATEMCS1	= 0x0d,
	DESC_RATEMCS2	= 0x0e,
	DESC_RATEMCS3	= 0x0f,
	DESC_RATEMCS4	= 0x10,
	DESC_RATEMCS5	= 0x11,
	DESC_RATEMCS6	= 0x12,
	DESC_RATEMCS7	= 0x13,
	DESC_RATEMCS8	= 0x14,
	DESC_RATEMCS9	= 0x15,
	DESC_RATEMCS10	= 0x16,
	DESC_RATEMCS11	= 0x17,
	DESC_RATEMCS12	= 0x18,
	DESC_RATEMCS13	= 0x19,
	DESC_RATEMCS14	= 0x1a,
	DESC_RATEMCS15	= 0x1b,
	DESC_RATEMCS31	= 0x2b,
	DESC_RATEVHT1SS_MCS0 = 0x2c,
	DESC_RATEVHT2SS_MCS0 = 0x36,
	DESC_RATE_MAX,
};

enum rtw_flags {
	RTW_FLAG_RUNNING,
	RTW_FLAG_FW_RUNNING,
	RTW_FLAG_SCANNING,
	RTW_FLAG_POWERON,
	NUM_OF_RTW_FLAGS,
};

enum rtw_lps_mode {
	RTW_MODE_ACTIVE = 0,
	RTW_MODE_LPS = 1,
};

enum rtw_pwr_state {
	RTW_RF_OFF	= 0x0,
	RTW_RF_ON	= 0x4,
	RTW_ALL_ON	= 0xc,
};

// Power sequence commands
#define RTW_PWR_CMD_READ	0x00
#define RTW_PWR_CMD_WRITE	0x01
#define RTW_PWR_CMD_POLLING	0x02
#define RTW_PWR_CMD_DELAY	0x03
#define RTW_PWR_CMD_END		0x04

#define RTW_PWR_ADDR_MAC	0x00
#define RTW_PWR_ADDR_USB	0x01
#define RTW_PWR_ADDR_PCIE	0x02
#define RTW_PWR_ADDR_SDIO	0x03

#define RTW_PWR_INTF_PCI_MSK	BIT(2)
#define RTW_PWR_INTF_ALL_MSK	(BIT(0)|BIT(1)|BIT(2)|BIT(3))
#define RTW_PWR_CUT_ALL_MSK	0xFF

struct rtw_pwr_seq_cmd {
	u16 offset;
	u8 cut_mask;
	u8 intf_mask;
	u8 base:4;
	u8 cmd:4;
	u8 mask;
	u8 value;
};

struct rtw_hw_reg {
	u32 addr;
	u32 mask;
};

struct rtw_vif_port {
	struct rtw_hw_reg mac_addr;
	struct rtw_hw_reg bssid;
	struct rtw_hw_reg net_type;
	struct rtw_hw_reg aid;
	struct rtw_hw_reg bcn_ctrl;
};

struct rtw_tx_pkt_info {
	u32 tx_pkt_size;
	u8 offset;
	u8 pkt_offset;
	u8 mac_id;
	u8 rate_id;
	u8 rate;
	u8 qsel;
	u8 bw;
	u8 sec_type;
	bool ampdu_en;
	u8 ampdu_factor;
	u16 seq;
	bool stbc;
	bool ldpc;
	bool dis_rate_fallback;
	bool bmc;
	bool use_rate;
	bool ls;
	bool fs;
	bool short_gi;
	bool report;
	bool rts;
	bool en_hwseq;
	u8 hw_ssn_sel;
};

struct rtw_rx_pkt_stat {
	bool phy_status;
	bool icv_err;
	bool crc_err;
	bool decrypted;
	bool is_c2h;
	s32 signal_power;
	u16 pkt_len;
	u8 bw;
	u8 drv_info_sz;
	u8 shift;
	u8 rate;
	u8 mac_id;
	s8 rx_power[RTW_RF_PATH_MAX];
	u8 rssi;
	s8 rx_snr[RTW_RF_PATH_MAX];
	u16 freq;
	u8 band;
};

DECLARE_EWMA(tp, 10, 2);

struct rtw_traffic_stats {
	u64 tx_unicast;
	u64 rx_unicast;
	u64 tx_cnt;
	u64 rx_cnt;
	u32 tx_throughput;
	u32 rx_throughput;
	struct ewma_tp tx_ewma_tp;
	struct ewma_tp rx_ewma_tp;
};

struct rtw_sec_desc {
	bool default_key_search;
	u32 total_cam_num;
};

struct rtw_sta_info {
	struct rtw_dev *rtwdev;
	u8 mac_id;
	u8 rate_id;
	enum rtw_bandwidth bw_mode;
	bool sgi_enable;
	bool vht_enable;
	u64 ra_mask;
};

struct rtw_vif {
	enum rtw_net_type net_type;
	u16 aid;
	u8 mac_id;
	u8 mac_addr[ETH_ALEN];
	u8 bssid[ETH_ALEN];
	u8 port;
	u8 bcn_ctrl;
	struct list_head rsvd_page_list;
	struct rtw_traffic_stats stats;
};

struct rtw_chip_ops;

struct rtw_chip_info {
	const struct rtw_chip_ops *ops;
	u8 id;
	const char *fw_name;
	u8 tx_pkt_desc_sz;
	u8 rx_pkt_desc_sz;
	u32 txff_size;
	u32 rxff_size;
	u32 fw_rxff_size;
	u16 rsvd_drv_pg_num;
	u8 band;
	u16 page_size;
	u8 dig_max;
	u8 dig_min;
	u8 max_power_index;
	u8 ampdu_density;
	bool rx_ldpc;
	bool tx_stbc;
	bool ht_supported;
	bool vht_supported;
	u8 lps_deep_mode_supported;
	u8 sys_func_en;

	const struct rtw_pwr_seq_cmd * const *pwr_on_seq;
	const struct rtw_pwr_seq_cmd * const *pwr_off_seq;

	const struct rtw_hw_reg *dig;
	const struct rtw_hw_reg *dig_cck;
	u32 rf_base_addr[RTW_RF_PATH_MAX];
	u32 rf_sipi_addr[RTW_RF_PATH_MAX];

	const struct rtw_table *mac_tbl;
	const struct rtw_table *agc_tbl;
	const struct rtw_table *bb_tbl;
	const struct rtw_table *rf_tbl[RTW_RF_PATH_MAX];

	u16 fw_fifo_addr[8];
	u8 c2h_ra_report_size;
	u8 bfer_su_max_num;
	u8 bfer_mu_max_num;
	u32 coex_para_ver;
};

struct rtw_chip_ops {
	int (*power_on)(struct rtw_dev *rtwdev);
	void (*power_off)(struct rtw_dev *rtwdev);
	int (*mac_init)(struct rtw_dev *rtwdev);
	int (*read_efuse)(struct rtw_dev *rtwdev, u8 *map);
	void (*phy_set_param)(struct rtw_dev *rtwdev);
	void (*set_channel)(struct rtw_dev *rtwdev, u8 channel,
			    u8 bandwidth, u8 primary_chan_idx);
	void (*query_phy_status)(struct rtw_dev *rtwdev, u8 *phy_status,
				 struct rtw_rx_pkt_stat *pkt_stat);
	u32 (*read_rf)(struct rtw_dev *rtwdev, enum rtw_rf_path rf_path,
		       u32 addr, u32 mask);
	bool (*write_rf)(struct rtw_dev *rtwdev, enum rtw_rf_path rf_path,
			 u32 addr, u32 mask, u32 data);
	void (*set_tx_power_index)(struct rtw_dev *rtwdev);
	void (*cfg_ldo25)(struct rtw_dev *rtwdev, bool enable);
	void (*efuse_grant)(struct rtw_dev *rtwdev, bool enable);
	void (*false_alarm_statistics)(struct rtw_dev *rtwdev);
	void (*phy_calibration)(struct rtw_dev *rtwdev);
	void (*dpk_track)(struct rtw_dev *rtwdev);
	void (*pwr_track)(struct rtw_dev *rtwdev);
	void (*cfo_init)(struct rtw_dev *rtwdev);
	void (*cfo_track)(struct rtw_dev *rtwdev);
};

struct rtw_table {
	const void *data;
	const u32 size;
	void (*parse)(struct rtw_dev *rtwdev, const struct rtw_table *tbl);
	void (*do_cfg)(struct rtw_dev *rtwdev, const struct rtw_table *tbl,
		       u32 addr, u32 data);
	enum rtw_rf_path rf_path;
};

struct rtw_efuse {
	u32 size;
	u32 physical_size;
	u32 logical_size;
	u8 addr[ETH_ALEN];
	u8 channel_plan;
	u8 country_code[2];
	u8 rf_board_option;
	u8 rfe_option;
	u8 thermal_meter[RTW_RF_PATH_MAX];
	u8 thermal_meter_k;
	u8 crystal_cap;
	u8 ant_div_cfg;
	u8 regd;
	bool btcoex;
	bool share_ant;
	struct {
		u8 hci;
		u8 bw;
		u8 ptcl;
		u8 nss;
		u8 ant_num;
	} hw_cap;
};

struct rtw_fifo_conf {
	u16 rsvd_boundary;
	u16 rsvd_pg_num;
	u16 rsvd_drv_pg_num;
	u16 txff_pg_num;
	u16 acq_pg_num;
	u16 rsvd_drv_addr;
};

struct rtw_fw_state {
	const struct firmware *firmware;
	struct rtw_dev *rtwdev;
	struct completion completion;
	u16 version;
	u8 sub_version;
	u32 feature;
	u32 feature_ext;
};

struct rtw_hal {
	u32 rcr;
	u32 chip_version;
	u8 cut_version;
	u8 mp_chip;
	u8 oem_id;
	u8 pkg_type;
	u8 ps_mode;
	u8 current_channel;
	u8 current_band_width;
	u8 current_band_type;
	u8 rf_type;
	u8 rf_path_num;
	u32 antenna_tx;
	u32 antenna_rx;
	u8 cch_by_bw[RTW_CHANNEL_WIDTH_MAX + 1];
};

struct rtw_dm_info {
	u32 cck_fa_cnt;
	u32 ofdm_fa_cnt;
	u32 total_fa_cnt;
	u8 min_rssi;
	u8 igi_history[4];
	u8 thermal_avg[RTW_RF_PATH_MAX];
	u8 thermal_meter_k;
};

struct rtw_pci_rx_buffer {
	struct rtw_pci_rx_ring *ring;
	dma_addr_t dma;
	u32 dma_len;
	void *data;
	u32 offset;
};

struct rtw_pci_tx_buffer {
	dma_addr_t dma;
	u32 dma_len;
	void *data;
};

struct rtw_pci_rx_ring {
	u8 *desc;
	dma_addr_t dma;
	u32 len;
	u32 idx;
	u32 desc_size;
	u32 buf_size;
	u32 total_desc;
	struct rtw_pci_rx_buffer *buffer;
};

struct rtw_pci_tx_ring {
	u8 *desc;
	dma_addr_t dma;
	u32 len;
	u32 idx;
	u32 cur_head;
	u32 cur_tail;
	u32 q_len;
	u32 desc_size;
	u32 total_desc;
	struct rtw_pci_tx_buffer *buffer;
};

struct rtw_pci {
	struct rtw_pci_tx_ring tx_rings[RTK_MAX_TX_QUEUE_NUM];
	struct rtw_pci_rx_ring rx_rings[RTK_MAX_RX_QUEUE_NUM];
	u32 rx_tag;
	u8 irq_mask[4];
};

struct rtw_dev {
	struct rtw_hal hal;
	struct rtw_efuse efuse;
	struct rtw_fw_state fw;
	struct rtw_fifo_conf fifo;
	struct rtw_sec_desc sec;
	struct rtw_dm_info dm;
	struct rtw_chip_info *chip;
	struct rtw_pci pci;
	struct rtw_vif vifs[RTW_PORT_NUM];
	u8 mac_addr[ETH_ALEN];
	u32 flags;
	DECLARE_BITMAP(flags_map, NUM_OF_RTW_FLAGS);
	mutex_t mutex;
	spinlock_t lock;

	void (*write32)(struct rtw_dev *dev, u32 addr, u32 val);
	u32  (*read32)(struct rtw_dev *dev, u32 addr);
	void (*write16)(struct rtw_dev *dev, u32 addr, u16 val);
	u16  (*read16)(struct rtw_dev *dev, u32 addr);
	void (*write8)(struct rtw_dev *dev, u32 addr, u8 val);
	u8   (*read8)(struct rtw_dev *dev, u32 addr);
};

// MMIO accessors (will be overridden by driver)
static inline u32 rtw_read32(struct rtw_dev *dev, u32 a) { return dev->read32(dev, a); }
static inline u16 rtw_read16(struct rtw_dev *dev, u32 a) { return dev->read16(dev, a); }
static inline u8  rtw_read8(struct rtw_dev *dev, u32 a)  { return dev->read8(dev, a); }
static inline void rtw_write32(struct rtw_dev *dev, u32 a, u32 v) { dev->write32(dev, a, v); }
static inline void rtw_write16(struct rtw_dev *dev, u32 a, u16 v) { dev->write16(dev, a, v); }
static inline void rtw_write8(struct rtw_dev *dev, u32 a, u8 v)   { dev->write8(dev, a, v); }

static inline void rtw_write32_set(struct rtw_dev *d, u32 a, u32 v) { rtw_write32(d, a, rtw_read32(d, a) | v); }
static inline void rtw_write32_clr(struct rtw_dev *d, u32 a, u32 v) { rtw_write32(d, a, rtw_read32(d, a) & ~v); }
static inline void rtw_write16_set(struct rtw_dev *d, u32 a, u16 v) { rtw_write16(d, a, rtw_read16(d, a) | v); }
static inline void rtw_write8_set(struct rtw_dev *d, u32 a, u8 v)   { rtw_write8(d, a, rtw_read8(d, a) | v); }
static inline void rtw_write8_clr(struct rtw_dev *d, u32 a, u8 v)   { rtw_write8(d, a, rtw_read8(d, a) & ~v); }

// RF register access
static inline u32 rtw_read_rf(struct rtw_dev *d, enum rtw_rf_path p, u32 a, u32 m) { return d->chip->ops->read_rf(d, p, a, m); }
static inline bool rtw_write_rf(struct rtw_dev *d, enum rtw_rf_path p, u32 a, u32 m, u32 v) { return d->chip->ops->write_rf(d, p, a, m, v); }

// Core API
struct rtw_dev *rtw_alloc_dev(void);
void rtw_free_dev(struct rtw_dev *dev);
int rtw_core_init(struct rtw_dev *dev);
void rtw_core_deinit(struct rtw_dev *dev);
int rtw_power_on(struct rtw_dev *dev);
void rtw_power_off(struct rtw_dev *dev);
int rtw_pci_setup(struct rtw_dev *dev, void *provider);
void rtw_pci_teardown(struct rtw_dev *dev);
void rtw_pci_irq_handler(struct rtw_dev *dev);
int rtw_mac_init(struct rtw_dev *dev);
int rtw_mac_postinit(struct rtw_dev *dev);
void rtw_set_channel_mac(struct rtw_dev *dev, u8 channel, u8 bw,
			 u8 primary_chan_idx);
int rtw_fw_download(struct rtw_dev *dev);
int rtw_set_channel(struct rtw_dev *dev, u8 channel, u8 bandwidth, u8 primary_chan);
void rtw_phy_set_param(struct rtw_dev *dev);
int rtw_efuse_read(struct rtw_dev *dev, u8 *map);
void rtw_set_tx_power_index(struct rtw_dev *dev);

// Chip-specific init functions (to be implemented per chip)
int rtw_8821ce_init(struct rtw_dev *dev);
int rtw_8822be_init(struct rtw_dev *dev);
int rtw_8822ce_init(struct rtw_dev *dev);
int rtw_8723de_init(struct rtw_dev *dev);

#ifdef __cplusplus
}
#endif

#endif
