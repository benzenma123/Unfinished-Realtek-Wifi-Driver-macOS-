# Unfinished-Realtek-Wifi-Driver-macOS-
# NOTE: this project is reversed engineering from linux source code so expect some issue and also there's a bit of "vibecoding" (not too much) in this project by opencode
This is a unfinised version of Realtek WiFi driver that I made, you can use this repo and try to continue my project. I cannot continue my project due to school and a bunch of work, feel free to ask me anything. If you manage to get it work then please contact me, thank you.
So far here's what i manage to do:
- Ported Linux rtw89 PCIe DMA initialization sequence to macOS IOKit kext
- Fixed wrong BAR mapping (index 1 instead of index 0) to access 1MB register space
- Implemented MMIO read/write wrappers (8/16/32-bit) for PCIe config and BAR2 access
- Reverse-engineered firmware binary format: v0 header (32 bytes), 26 sections, section header structure (16 bytes each)
- Decoded H2C mailbox protocol: 4-byte header (`__le16 hdr, __le16 len`), `struct rtw89_txwd_body` (6 dwords), `struct rtw89_pci_tx_bd_32` (8 bytes)
- Decoded NORM-mode TX BD format: `dword0 = length | (opt << 16)`, `dword1 = dma_lo`, `opt = LS | (dma_hi << 6)`
- Mapped firmware download from mailbox (H2C path dead) to DMA Channel 12
- Implemented full `mac_pre_init` PCIe PHY/power sequence: l1off_pwroff, aphy_pwrcut, hci_ldo, power_wake, set_sic, set_dbg, set_keep_reg
- Implemented DMA stop, BD/WD index clear, mode_op (INIT_CFG1), BDRAM reset, per-channel stop/start
- Embedded 1MB firmware binary in kext and implemented section-by-section DMA delivery
- Added 40+ missing register definitions: `SYS_SDIO_CTRL`, `HCI_OPT_CTRL`, `PS_CTRL`, `TX_ADDRESS_INFO_MODE`, `PKTIN_SETTING`, etc.
- Property-based debugging via IORegistry (no IOLog visible on 13+ without dev account)
- Identified root block: DMA engine never fetches BD — HW read pointer stays at 0 after doorbell write, despite correct BD format, ring address, and channel config

This project set me back for few weeks btw
