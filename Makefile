APP_NAME = pb_sim
APP_VERSION = 1.0.0
API_LEVEL = 0
ICON = src/icon.png

# Source files
SRC = $(wildcard src/*.c)

# Toolchain
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

CFLAGS = -std=c11 -Wall -Wextra -Os -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -fno-common -fdata-sections -ffunction-sections
LDFLAGS = -Wl,--gc-sections -nostartfiles

# nwlink for packaging
NWLINK = npx nwlink@0.0.16

.PHONY: all clean run

all: $(APP_NAME).nwa

$(APP_NAME).nwa: $(SRC) $(ICON)
	@echo "Building $(APP_NAME)..."
	$(NWLINK) install-nwlink-lld
	$(NWLINK) link-nwa --icon $(ICON) --api-level $(API_LEVEL) $(SRC) -o $@

run: $(APP_NAME).nwa
	$(NWLINK) run-nwa $<

clean:
	rm -f $(APP_NAME).nwa
