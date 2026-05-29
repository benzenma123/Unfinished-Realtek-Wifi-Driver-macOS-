#include "rtw89.h"

#define FW_HDR_LEN_V0               (8 * 4)
#define FW_HDR_LEN_V1               (12 * 4)

static int read_poll_fwdl_path_rdy(struct rtw89_dev *rtwdev, bool is_h2c)
{
    u32 mask = is_h2c ? B_AX_H2C_PATH_RDY : B_AX_FWDL_PATH_RDY;
    u32 val;
    int ret;

    ret = read_poll_timeout(rtw89_read8, val, val & mask,
                            1, FWDL_WAIT_CNT, rtwdev, R_AX_WCPU_FW_CTRL);
    return ret;
}

int rtw89_fw_check_rdy(struct rtw89_dev *rtwdev)
{
    u32 val;
    int ret;

    ret = read_poll_timeout(rtw89_read32, val,
                            (val & B_AX_WCPU_FWDL_STS_MASK) ==
                            (RTW89_FWDL_WCPU_FW_INIT_RDY << 5),
                            500, 5000000, rtwdev, R_AX_WCPU_FW_CTRL);
    return ret;
}

int rtw89_fwdl_enable_wcpu(struct rtw89_dev *rtwdev, bool en)
{
    if (en) {
        rtw89_write32(rtwdev, R_AX_UDM1, 0);
        rtw89_write32(rtwdev, R_AX_UDM2, 0);
        rtw89_write32(rtwdev, R_AX_HALT_H2C_CTRL, 0);
        rtw89_write32(rtwdev, R_AX_HALT_C2H_CTRL, 0);
        rtw89_write32(rtwdev, R_AX_HALT_H2C, 0);
        rtw89_write32(rtwdev, R_AX_HALT_C2H, 0);
        rtw89_write32_set(rtwdev, R_AX_SYS_CLK_CTRL, B_AX_CPU_CLK_EN);

        u32 val = rtw89_read32(rtwdev, R_AX_WCPU_FW_CTRL);
        val &= ~(B_AX_WCPU_FWDL_EN | B_AX_H2C_PATH_RDY | B_AX_FWDL_PATH_RDY);
        val &= ~B_AX_WCPU_FWDL_STS_MASK;
        val |= B_AX_WCPU_FWDL_EN;
        rtw89_write32(rtwdev, R_AX_WCPU_FW_CTRL, val);

        if (rtwdev->chip && rtwdev->chip->chip_id == RTL8852B)
            rtw89_write32_mask(rtwdev, R_AX_SEC_CTRL,
                                B_AX_SEC_IDMEM_SIZE_CONFIG_MASK, 0x2);

        rtw89_write16_mask(rtwdev, R_AX_BOOT_REASON,
                            B_AX_BOOT_REASON_MASK, 0);

        rtw89_write32_set(rtwdev, R_AX_PLATFORM_ENABLE, B_AX_WCPU_EN);
    } else {
        rtw89_write32_clr(rtwdev, R_AX_WCPU_FW_CTRL, B_AX_WCPU_FWDL_EN);
    }

    return 0;
}

int rtw89_fw_download(struct rtw89_dev *rtwdev, const u8 *fw_data, u32 fw_size)
{
    int ret;
    u32 val;

    if (!fw_data || fw_size < 64) {
        IOLog("rtw89: Invalid firmware data (size=%d)\n", fw_size);
        return -1;
    }

    ret = read_poll_fwdl_path_rdy(rtwdev, true);
    if (ret) {
        IOLog("rtw89: H2C_PATH_RDY timeout\n");
        val = rtw89_read32(rtwdev, R_AX_WCPU_FW_CTRL);
        IOLog("rtw89: WCPU_FW_CTRL=0x%08x\n", val);
        return -1;
    }

    for (int retry = 0; retry < 3; retry++) {
        u32 w3 = *(const u32 *)(fw_data + 12);
        u32 w6 = *(const u32 *)(fw_data + 24);
        u32 w7 = *(const u32 *)(fw_data + 28);
        u8 hdr_ver = (w3 >> 24) & 0xFF;
        u8 sec_num = (w6 >> 8) & 0xFF;
        u16 orig_part_size = w7 & 0xFFFF;
        u32 hdr_len = (hdr_ver == 0) ? FW_HDR_LEN_V0 : FW_HDR_LEN_V1;
        u32 sec_hdr_size = sizeof(struct rtw89_fw_hdr_section);

        IOLog("rtw89: FW v%d sections=%d hdr_len=%d part_size=%d->%d\n",
              hdr_ver, sec_num, hdr_len, orig_part_size,
              FWDL_SECTION_PER_PKT_LEN);

        u8 hdr_copy[48];
        if (hdr_len > sizeof(hdr_copy)) hdr_len = sizeof(hdr_copy);
        memcpy(hdr_copy, fw_data, hdr_len);
        u32 *w7_ptr = (u32 *)(hdr_copy + 28);
        *w7_ptr = (*w7_ptr & ~(u32)0xFFFF) | (u32)FWDL_SECTION_PER_PKT_LEN;

        ret = rtw89_pci_ch12_send_h2c(rtwdev, hdr_copy, hdr_len, true);
        if (ret) {
            IOLog("rtw89: Header DMA send failed ret=%d\n", ret);
            continue;
        }

        IODelay(200);

        ret = read_poll_fwdl_path_rdy(rtwdev, false);
        if (ret) {
            IOLog("rtw89: FWDL_PATH_RDY not set after header\n");
            val = rtw89_read32(rtwdev, R_AX_WCPU_FW_CTRL);
            IOLog("rtw89: WCPU_FW_CTRL=0x%08x\n", val);
            continue;
        }

        IOLog("rtw89: FWDL_PATH_RDY set! Sending sections...\n");

        u32 data_offset = hdr_len + sec_num * sec_hdr_size;
        bool section_ok = true;

        for (u32 i = 0; i < sec_num && i < FWDL_SECTION_MAX_NUM; i++) {
            const u8 *sec_hdr = fw_data + hdr_len + i * sec_hdr_size;
            u32 w1 = *(const u32 *)(sec_hdr + 4);
            u32 sec_size = w1 & 0xFFFFFF;

            if (sec_size == 0) continue;

            IOLog("rtw89: Section %d: size=%d\n", i, sec_size);

            u32 offset = 0;
            while (offset < sec_size) {
                u32 chunk = sec_size - offset;
                if (chunk > FWDL_SECTION_PER_PKT_LEN)
                    chunk = FWDL_SECTION_PER_PKT_LEN;

                ret = rtw89_pci_ch12_send_h2c(rtwdev,
                                               fw_data + data_offset + offset,
                                               chunk, true);
                if (ret) {
                    IOLog("rtw89: Section %d chunk dma failed\n", i);
                    section_ok = false;
                    break;
                }

                offset += chunk;
                IODelay(50);
            }

            if (!section_ok) break;
            data_offset += sec_size;
        }

        if (!section_ok) continue;

        IODelay(5000);
        ret = rtw89_fw_check_rdy(rtwdev);
        if (ret == 0) {
            rtwdev->flags |= RTW89_FLAG_FW_RDY;
            IOLog("rtw89: Firmware ready\n");
            return 0;
        }

        IOLog("rtw89: FW download retry %d failed\n", retry);
    }

    IOLog("rtw89: Firmware download failed after retries\n");
    return -1;
}
