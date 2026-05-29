# RealtekWifi - macOS driver for Realtek WiFi cards

Port of Linux `rtw88` (WiFi 5) and `rtw89` (WiFi 6) drivers to macOS IOKit.

## Supported chipsets

**WiFi 5 (rtw88):** RTL8821CE, RTL8822BE, RTL8822CE, RTL8723DE, RTL8812AE, RTL8814AE  
**WiFi 6 (rtw89):** RTL8852AE, RTL8852BE, RTL8852CE, RTL8922AE

## Requirements

- macOS Ventura 13.0+
- Xcode 15+ (for kernel extension development)
- SIP disabled (for loading unsigned kexts during development)

## Building

```bash
make
```

## Installing

```bash
make install
```

Or manually:

```bash
sudo cp -R build/RealtekWifi.kext /Library/Extensions/
sudo chown -R root:wheel /Library/Extensions/RealtekWifi.kext
sudo kextutil -v /Library/Extensions/RealtekWifi.kext
```

## Firmware

Download firmware blobs:

```bash
./tools/fetch_fw.sh
```

## Architecture

```
RealtekWifi.kext
├── RealtekWifi         - IOKit driver (IOEthernetController subclass)
├── rtw88/              - Ported Linux rtw88 driver (WiFi 5)
├── rtw89/              - Ported Linux rtw89 driver (WiFi 6)
├── platform/           - OS abstraction layer
└── firmware/           - Firmware binaries
```

The driver registers as an Ethernet controller (like itlwm), compatible with HeliPort or any standard network management on macOS.
