#include <libkern/libkern.h>
#include "rtw88.h"
#include "mac.h"
#include "fw.h"
#include "pci.h"
#include "tx.h"
#include "rx.h"

static struct rtw_vif_port rtw_vif_port[RTW_PORT_NUM] = {
	[0] = {
		.mac_addr  = { .addr = 0x0610 },
		.bssid     = { .addr = 0x0618 },
		.net_type  = { .addr = 0x0100, .mask = 0x30000 },
		.aid       = { .addr = 0x06a8, .mask = 0x7ff },
		.bcn_ctrl  = { .addr = 0x0550, .mask = 0xff },
	},
	[1] = {
		.mac_addr  = { .addr = 0x0700 },
		.bssid     = { .addr = 0x0708 },
		.net_type  = { .addr = 0x0100, .mask = 0xc0000 },
		.aid       = { .addr = 0x0710, .mask = 0x7ff },
		.bcn_ctrl  = { .addr = 0x0551, .mask = 0xff },
	},
	[2] = {
		.mac_addr  = { .addr = 0x1620 },
		.bssid     = { .addr = 0x1628 },
		.net_type  = { .addr = 0x1100, .mask = 0x3 },
		.aid       = { .addr = 0x1600, .mask = 0x7ff },
		.bcn_ctrl  = { .addr = 0x0578, .mask = 0xff },
	},
	[3] = {
		.mac_addr  = { .addr = 0x1630 },
		.bssid     = { .addr = 0x1638 },
		.net_type  = { .addr = 0x1100, .mask = 0xc },
		.aid       = { .addr = 0x1604, .mask = 0x7ff },
		.bcn_ctrl  = { .addr = 0x0579, .mask = 0xff },
	},
	[4] = {
		.mac_addr  = { .addr = 0x1640 },
		.bssid     = { .addr = 0x1648 },
		.net_type  = { .addr = 0x1100, .mask = 0x30 },
		.aid       = { .addr = 0x1608, .mask = 0x7ff },
		.bcn_ctrl  = { .addr = 0x057a, .mask = 0xff },
	},
};

struct rtw_dev *rtw_alloc_dev(void)
{
	struct rtw_dev *dev = kzalloc(sizeof(*dev), 0);
	if (!dev)
		return NULL;
	mutex_init(&dev->mutex);
	spin_lock_init(&dev->lock);
	dev->flags = 0;
	bitmap_zero(dev->flags_map, NUM_OF_RTW_FLAGS);
	return dev;
}

void rtw_free_dev(struct rtw_dev *dev)
{
	if (!dev) return;
	rtw_pci_teardown(dev);
	mutex_destroy(&dev->mutex);
	kfree(dev);
}

int rtw_core_init(struct rtw_dev *dev)
{
	int ret;

	if (!dev->chip) {
		pr_err("No chip info\n");
		return -1;
	}

	ret = rtw_pci_setup(dev, NULL);
	if (ret) return ret;

	pr_info("Core init done\n");
	return 0;
}

void rtw_core_deinit(struct rtw_dev *dev)
{
	rtw_pci_teardown(dev);
}

int rtw_power_on(struct rtw_dev *dev)
{
	if (!dev->chip || !dev->chip->ops || !dev->chip->ops->power_on)
		return -1;
	return dev->chip->ops->power_on(dev);
}

void rtw_power_off(struct rtw_dev *dev)
{
	if (dev->chip && dev->chip->ops && dev->chip->ops->power_off)
		dev->chip->ops->power_off(dev);
}

void rtw_phy_set_param(struct rtw_dev *dev)
{
	if (dev->chip && dev->chip->ops && dev->chip->ops->phy_set_param)
		dev->chip->ops->phy_set_param(dev);
}

int rtw_efuse_read(struct rtw_dev *dev, u8 *map)
{
	if (dev->chip && dev->chip->ops && dev->chip->ops->read_efuse)
		return dev->chip->ops->read_efuse(dev, map);
	return -1;
}

void rtw_set_tx_power_index(struct rtw_dev *dev)
{
	if (dev->chip && dev->chip->ops && dev->chip->ops->set_tx_power_index)
		dev->chip->ops->set_tx_power_index(dev);
}
