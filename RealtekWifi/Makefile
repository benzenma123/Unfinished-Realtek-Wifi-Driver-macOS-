SDK_VERSION := macosx

SYS_ROOT := $(shell xcrun --sdk $(SDK_VERSION) --show-sdk-path)
KERNEL_FW := $(SYS_ROOT)/System/Library/Frameworks/Kernel.framework

CC := xcrun -sdk $(SDK_VERSION) clang
CXX := xcrun -sdk $(SDK_VERSION) clang++
LD := xcrun -sdk $(SDK_VERSION) ld

HOST_ARCH := $(shell uname -m)
ARCH_FLAGS := -arch $(HOST_ARCH)

KERNEL_FLAGS := -nostdlib -nostdinc -mkernel -fno-builtin -fno-rtti \
                -fno-exceptions -fno-stack-protector -funsigned-char \
                -fasm-blocks -fno-common -Wno-deprecated-declarations \
                -Wno-unused-parameter -DIOS_KERNEL -DKERNEL \
                -D__MAC_OS_X_VERSION_MIN_REQUIRED=140000

CFLAGS := $(ARCH_FLAGS) $(KERNEL_FLAGS) \
          -Iinclude -Isrc -I. \
          -I$(KERNEL_FW)/Headers \
          -I$(KERNEL_FW)/PrivateHeaders \
          -I$(SYS_ROOT)/usr/include \
          -O2 -g

CXXFLAGS := $(CFLAGS) -std=c++11
LDFLAGS  := $(ARCH_FLAGS) -kext -keep_private_externs -undefined dynamic_lookup

# ------ Sources -----------------------------------------------------------
HAL_C_SRCS   := $(wildcard src/rtw88/*.c) $(wildcard src/rtw89/*.c) src/fw/rtw8852b_fw.c
HAL_C_OBJS   := $(HAL_C_SRCS:.c=.o)

# ------ RealtekWifi.kext (IOEthernetController, uses HeliPort) ------------
RW_TARGET := RealtekWifi
RW_SRCS   := src/RealtekWifi.cpp
RW_OBJS   := $(RW_SRCS:.cpp=.o) $(HAL_C_OBJS)
RW_BINARY := build/$(RW_TARGET).kext/Contents/MacOS/$(RW_TARGET)

# ------ AirportRtlwm.kext (IO80211Controller, native WiFi UI) -------------
AR_TARGET := AirportRtlwm
AR_SRCS   := AirportRtlwm/AirportRtlwm.cpp
AR_OBJS   := $(AR_SRCS:.cpp=.o) $(HAL_C_OBJS)
AR_BINARY := build/$(AR_TARGET).kext/Contents/MacOS/$(AR_TARGET)

# ------ Rules -------------------------------------------------------------
.PHONY: all clean install

all: $(RW_BINARY) $(AR_BINARY)

$(RW_BINARY): $(RW_OBJS) Resources/Info.plist
	mkdir -p $(dir $@) build/$(RW_TARGET).kext/Contents/Resources
	$(LD) $(LDFLAGS) -o $@ $(filter %.o,$^)
	cp Resources/Info.plist build/$(RW_TARGET).kext/Contents/Info.plist
	@echo "Built $(RW_TARGET).kext"

$(AR_BINARY): $(AR_OBJS) AirportRtlwm/Info.plist
	mkdir -p $(dir $@) build/$(AR_TARGET).kext/Contents/Resources
	$(LD) $(LDFLAGS) -o $@ $(filter %.o,$^)
	cp AirportRtlwm/Info.plist build/$(AR_TARGET).kext/Contents/Info.plist
	@echo "Built $(AR_TARGET).kext"

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

install-rw: $(RW_BINARY)
	sudo rm -rf /Library/Extensions/$(RW_TARGET).kext
	sudo cp -R build/$(RW_TARGET).kext /Library/Extensions/
	sudo chown -R root:wheel /Library/Extensions/$(RW_TARGET).kext
	sudo kextutil -v /Library/Extensions/$(RW_TARGET).kext

install-ar: $(AR_BINARY)
	sudo rm -rf /Library/Extensions/$(AR_TARGET).kext
	sudo cp -R build/$(AR_TARGET).kext /Library/Extensions/
	sudo chown -R root:wheel /Library/Extensions/$(AR_TARGET).kext
	sudo kextutil -v /Library/Extensions/$(AR_TARGET).kext

install: install-ar

clean:
	rm -rf build $(RW_OBJS) $(AR_OBJS) $(HAL_C_OBJS)

distclean: clean
	rm -rf firmware/*.bin
