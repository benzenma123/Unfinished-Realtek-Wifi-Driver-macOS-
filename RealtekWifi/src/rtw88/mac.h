#ifndef __RTW_MAC_H__
#define __RTW_MAC_H__

int rtw_mac_init(struct rtw_dev *dev);
int rtw_mac_postinit(struct rtw_dev *dev);
void rtw_set_channel_mac(struct rtw_dev *dev, u8 channel, u8 bw,
			 u8 primary_chan_idx);

#endif
