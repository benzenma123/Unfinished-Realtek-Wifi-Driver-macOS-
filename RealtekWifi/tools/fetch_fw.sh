#!/bin/bash
# fetch_fw.sh - Download Realtek firmware files from Linux firmware repository
set -e

FW_DIR="$(dirname "$0")/../firmware"
LINUX_FW_URL="https://git.kernel.org/pub/scm/linux/kernel/git/firmware/linux-firmware.git/plain/rtw88"

mkdir -p "$FW_DIR"

declare -a FW_FILES=(
    "rtw8821c_fw.bin"
    "rtw8822b_fw.bin"
    "rtw8822c_fw.bin"
    "rtw8723d_fw.bin"
    "rtw8812a_fw.bin"
    "rtw8822c_wow_fw.bin"
)

echo "Downloading rtw88 firmware..."
for fw in "${FW_FILES[@]}"; do
    echo "  $fw..."
    curl -sL "$LINUX_FW_URL/$fw" -o "$FW_DIR/$fw" || echo "  WARNING: Failed to download $fw"
done

LINUX_FW_URL89="https://git.kernel.org/pub/scm/linux/kernel/git/firmware/linux-firmware.git/plain/rtw89"

declare -a FW_FILES89=(
    "rtw8852a_fw.bin"
    "rtw8852b_fw.bin"
    "rtw8852c_fw.bin"
    "rtw8922a_fw.bin"
)

echo "Downloading rtw89 firmware..."
for fw in "${FW_FILES89[@]}"; do
    echo "  $fw..."
    curl -sL "$LINUX_FW_URL89/$fw" -o "$FW_DIR/$fw" || echo "  WARNING: Failed to download $fw"
done

echo "Firmware download complete."
ls -la "$FW_DIR/"
