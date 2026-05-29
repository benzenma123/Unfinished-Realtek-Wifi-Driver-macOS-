#include "rtw88.h"

/* Firmware header format for rtw88 chips:
 *  - 8 bytes header:
 *    u16 signature (0x92BF, 0x92C0, etc.)
 *    u16 version
 *    u16 sub_version
 *    u16 sub_index
 *  - 4 bytes: total size
 *  - Followed by firmware data sections
 */

#define FW_HEADER_SIZE          12
#define FW_SIGNATURE_88XX       0x92BF
#define FW_SIGNATURE_8822C      0x92C0

#define FW_SECTION_IMEM         0x10
#define FW_SECTION_DMEM         0x11
#define FW_SECTION_EMEM         0x12

#define FW_START_ADDR_IMEM      0x2000
#define FW_START_ADDR_DMEM      0x4000

struct rtw_fw_header {
	u16 signature;
	u16 version;
	u16 sub_version;
	u16 sub_index;
	__le32 total_size;
	u8 reserved[8];
};

struct rtw_fw_section {
	u8 type;
	u8 reserved;
	__le16 size;
	__le32 addr;
};

#define FW_SECTION_HEADER_SIZE  8

static int rtw_fw_send_h2c_cmd(struct rtw_dev *dev, u8 *cmd, u32 len)
{
	u32 val;
	int retries = 100;

	if (len > 8) {
		pr_err("H2C cmd too long: %d\n", len);
		return -1;
	}

	rtw_write32(dev, REG_HMEBOX0, *(u32 *)cmd);
	if (len >= 8)
		rtw_write32(dev, REG_HMEBOX1, *(u32 *)(cmd + 4));

	for (retries = 0; retries < 100; retries++) {
		val = rtw_read32(dev, REG_HMETFR);
		if (val & BIT_INT_BOX0)
			return 0;
		IODelay(100);
	}

	pr_err("H2C cmd timeout\n");
	return -1;
}

static int rtw_fw_section_write(struct rtw_dev *dev,
				struct rtw_fw_section *section,
				u8 *data)
{
	u32 addr = section->addr;
	u16 size = section->size;
	u32 *data32 = (u32 *)data;
	int i;

	pr_info("FW section: type=0x%x addr=0x%08x size=%d\n",
		section->type, addr, size);

	for (i = 0; i < size / 4; i++)
		rtw_write32(dev, addr + i * 4, data32[i]);

	return 0;
}

int rtw_fw_download(struct rtw_dev *dev)
{
	const struct firmware *fw = dev->fw.firmware;
	struct rtw_fw_header *hdr;
	struct rtw_fw_section *section;
	u8 *ptr;
	u32 remain;
	int ret;

	if (!fw || !fw->data || fw->size < FW_HEADER_SIZE) {
		pr_err("No firmware loaded\n");
		return -1;
	}

	hdr = (struct rtw_fw_header *)fw->data;
	ptr = (u8 *)fw->data + FW_HEADER_SIZE;
	remain = fw->size - FW_HEADER_SIZE;

	dev->fw.version = hdr->version;
	dev->fw.sub_version = hdr->sub_version;
	dev->fw.feature = 0;

	pr_info("FW: sig=0x%04x ver=%d sub=%d size=%d\n",
		hdr->signature, hdr->version, hdr->sub_version,
		hdr->total_size);

	rtw_write32(dev, REG_MCUFW_CTRL, BIT_MCUFWDL_EN);
	IODelay(1000);

	while (remain >= FW_SECTION_HEADER_SIZE) {
		section = (struct rtw_fw_section *)ptr;
		ptr += FW_SECTION_HEADER_SIZE;
		remain -= FW_SECTION_HEADER_SIZE;

		if (section->type == 0 || section->size == 0)
			break;

		if (section->size > remain) {
			pr_err("FW section size > remain\n");
			break;
		}

		ret = rtw_fw_section_write(dev, section, ptr);
		if (ret)
			return ret;

		ptr += section->size;
		remain -= section->size;
	}

	rtw_write8(dev, REG_MCUFW_CTRL, 0);

	rtw_write32(dev, REG_MCUFW_CTRL,
		    BIT_MCUFWDL_EN | BIT_CPU_CLK_SEL);

	rtw_write8(dev, REG_MCUFW_CTRL,
		   rtw_read8(dev, REG_MCUFW_CTRL) | BIT_CPU_CLK_SEL);

	rtw_write32(dev, REG_MCUFW_CTRL,
		    BIT_MCUFWDL_EN | BIT_CPU_CLK_SEL | VAL_FW_TRIGGER);

	IODelay(1000);

	{
		u32 fw_ctrl;
		int i;
		for (i = 0; i < 500; i++) {
			fw_ctrl = rtw_read32(dev, REG_MCUFW_CTRL);
			if ((fw_ctrl & FW_READY_MASK) == FW_READY) {
				pr_info("FW ready after %dms\n", i);
				break;
			}
			IODelay(1000);
		}
		if (i == 500) {
			pr_err("FW init timeout\n");
			return -1;
		}
	}

	return 0;
}
