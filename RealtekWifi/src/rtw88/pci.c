#include "rtw88.h"

#define PCI_TX_RING_ADDR        0x10020000
#define PCI_TX_RING_SIZE        0x4000
#define PCI_RX_RING_ADDR        0x10040000
#define PCI_RX_RING_SIZE        0x2000

#define PCI_TX_DESC_NUM         128
#define PCI_RX_DESC_NUM         64
#define PCI_TX_BUF_SZ           2048
#define PCI_RX_BUF_SZ           4096

static int rtw_pci_init_tx_ring(struct rtw_dev *dev,
				struct rtw_pci_tx_ring *ring,
				u32 total_desc, u32 desc_size)
{
	ring->desc = kzalloc(total_desc * desc_size, 0);
	if (!ring->desc)
		return -1;

	ring->dma = 0;
	ring->len = total_desc * desc_size;
	ring->total_desc = total_desc;
	ring->desc_size = desc_size;
	ring->idx = 0;
	ring->cur_head = 0;
	ring->cur_tail = 0;
	ring->q_len = 0;

	ring->buffer = kzalloc(sizeof(struct rtw_pci_tx_buffer) * total_desc, 0);
	if (!ring->buffer) {
		kfree(ring->desc);
		return -1;
	}

	return 0;
}

static int rtw_pci_init_rx_ring(struct rtw_dev *dev,
				struct rtw_pci_rx_ring *ring,
				u32 total_desc, u32 desc_size,
				u32 buf_size)
{
	int i;

	ring->desc = kzalloc(total_desc * desc_size, 0);
	if (!ring->desc)
		return -1;

	ring->dma = 0;
	ring->len = total_desc * desc_size;
	ring->total_desc = total_desc;
	ring->desc_size = desc_size;
	ring->buf_size = buf_size;
	ring->idx = 0;

	ring->buffer = kzalloc(sizeof(struct rtw_pci_rx_buffer) * total_desc, 0);
	if (!ring->buffer) {
		kfree(ring->desc);
		return -1;
	}

	for (i = 0; i < total_desc; i++) {
		ring->buffer[i].data = kzalloc(buf_size, 0);
		if (!ring->buffer[i].data)
			return -1;
		ring->buffer[i].offset = 0;
		ring->buffer[i].dma = 0;
		ring->buffer[i].dma_len = buf_size;
	}

	return 0;
}

int rtw_pci_setup(struct rtw_dev *dev, void *provider)
{
	const struct rtw_chip_info *chip = dev->chip;
	u8 tx_desc_sz = chip->tx_pkt_desc_sz ? chip->tx_pkt_desc_sz : 40;
	u8 rx_desc_sz = chip->rx_pkt_desc_sz ? chip->rx_pkt_desc_sz : 24;
	int i, ret;

	(void)provider;

	for (i = 0; i < RTK_MAX_TX_QUEUE_NUM; i++) {
		ret = rtw_pci_init_tx_ring(dev, &dev->pci.tx_rings[i],
					  PCI_TX_DESC_NUM, tx_desc_sz);
		if (ret) {
			pr_err("Failed to init TX ring %d\n", i);
			return ret;
		}
	}

	for (i = 0; i < RTK_MAX_RX_QUEUE_NUM; i++) {
		ret = rtw_pci_init_rx_ring(dev, &dev->pci.rx_rings[i],
					  PCI_RX_DESC_NUM, rx_desc_sz,
					  PCI_RX_BUF_SZ);
		if (ret) {
			pr_err("Failed to init RX ring %d\n", i);
			return ret;
		}
	}

	pr_info("PCI DMA rings initialized\n");
	return 0;
}

void rtw_pci_teardown(struct rtw_dev *dev)
{
	int i, j;

	for (i = 0; i < RTK_MAX_TX_QUEUE_NUM; i++) {
		struct rtw_pci_tx_ring *ring = &dev->pci.tx_rings[i];
		if (ring->buffer) {
			for (j = 0; j < ring->total_desc; j++) {
				if (ring->buffer[j].data)
					kfree(ring->buffer[j].data);
			}
			kfree(ring->buffer);
		}
		if (ring->desc)
			kfree(ring->desc);
	}

	for (i = 0; i < RTK_MAX_RX_QUEUE_NUM; i++) {
		struct rtw_pci_rx_ring *ring = &dev->pci.rx_rings[i];
		if (ring->buffer) {
			for (j = 0; j < ring->total_desc; j++) {
				if (ring->buffer[j].data)
					kfree(ring->buffer[j].data);
			}
			kfree(ring->buffer);
		}
		if (ring->desc)
			kfree(ring->desc);
	}
}

void rtw_pci_irq_handler(struct rtw_dev *dev)
{
	u32 hisr, himr;

	hisr = rtw_read32(dev, REG_HISR0);
	if (!hisr)
		return;

	rtw_write32(dev, REG_HISR0, hisr);

	if (hisr & BIT(0)) {
		struct rtw_pci_tx_ring *ring = &dev->pci.tx_rings[RTW_TX_QUEUE_H2C];
		ring->cur_head = (ring->cur_head + 1) % ring->total_desc;
		ring->q_len--;
	}

	if (hisr & BIT(1)) {
		struct rtw_pci_rx_ring *ring = &dev->pci.rx_rings[RTW_RX_QUEUE_MPDU];
		ring->idx = (ring->idx + 1) % ring->total_desc;
	}
}
