# STM32F446RE UART command shell
# arm-none-eabi-gcc, no HAL

TOOLCHAIN_PATH = /Applications/ArmGNUToolchain/15.2.rel1/arm-none-eabi/bin
CC       = $(TOOLCHAIN_PATH)/arm-none-eabi-gcc
AS       = $(TOOLCHAIN_PATH)/arm-none-eabi-gcc
OBJCOPY  = $(TOOLCHAIN_PATH)/arm-none-eabi-objcopy
SIZE     = $(TOOLCHAIN_PATH)/arm-none-eabi-size

SRC_DIR     = Src
INC_DIR     = Inc
STARTUP_DIR = Startup
CMSIS_DIR   = CMSIS
BUILD_DIR   = build

C_SRCS  = $(wildcard $(SRC_DIR)/*.c)
AS_SRCS = $(STARTUP_DIR)/startup_stm32f446retx.s

C_OBJS  = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(C_SRCS))
AS_OBJS = $(BUILD_DIR)/startup_stm32f446retx.o
OBJS    = $(C_OBJS) $(AS_OBJS)

TARGET  = $(BUILD_DIR)/firmware

MCU_FLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16

CFLAGS  = $(MCU_FLAGS)
CFLAGS += -DSTM32F446xx
CFLAGS += -Wall -Wextra -g -O0
CFLAGS += -I$(INC_DIR)
CFLAGS += -I$(CMSIS_DIR)/Include
CFLAGS += -I$(CMSIS_DIR)/Device/ST/STM32F4xx/Include
CFLAGS += -std=c11

ASFLAGS = $(MCU_FLAGS) -g

LDFLAGS  = $(MCU_FLAGS)
LDFLAGS += -TSTM32F446RETX_FLASH.ld
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -specs=nano.specs
LDFLAGS += -lc -lm

SERIAL_PORT ?= $(shell ls /dev/cu.usbmodem* 2>/dev/null | head -1)

.PHONY: all flash clean size serial

all: $(TARGET).elf $(TARGET).bin size

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/startup_stm32f446retx.o: $(STARTUP_DIR)/startup_stm32f446retx.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

flash: $(TARGET).bin
	st-flash write $(TARGET).bin 0x08000000

size: $(TARGET).elf
	$(SIZE) --format=berkeley $<

clean:
	rm -rf $(BUILD_DIR)

serial:
	@echo "Connecting to $(SERIAL_PORT) at 115200..."
	screen $(SERIAL_PORT) 115200
