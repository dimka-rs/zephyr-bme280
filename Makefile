BOARD ?= ebyte_nrf52805
OPENOCD ?= $(HOME)/repos/openocd

all:
	west build -b $(BOARD) -- -DBOARD_ROOT=$(CURDIR)

clean:
	rm -rf ./build

flash:
	west flash

menuconfig:
	west build -t menuconfig

guiconfig:
	west build -t guiconfig

init:
	@echo "source $(HOME)/zephyrproject/zephyr/zephyr-env.sh"

mcuboot:
	west build -s $(HOME)/zephyrproject/bootloader/mcuboot/boot/zephyr/ -d build/mcuboot -b $(BOARD) -- -DBOARD_ROOT=$(CURDIR)

flash_mcuboot: build/mcuboot/zephyr/zephyr.hex
	@echo Flashing: $<
	$(OPENOCD)/src/openocd \
		-s $(OPENOCD)/tcl \
		-f interface/stlink.cfg \
		-f target/nrf52.cfg \
		-c init \
		-c "reset init" \
		-c halt \
		-c "nrf5 mass_erase" \
		-c "flash write_image $<" \
		-c reset \
		-c exit

flashst: build/zephyr/zephyr.hex
	@echo Flashing: $<
	$(OPENOCD)/src/openocd \
		-s $(OPENOCD)/tcl \
		-f interface/stlink.cfg \
		-f target/nrf52.cfg \
		-c init \
		-c "reset init" \
		-c halt \
		-c "nrf5 mass_erase" \
		-c "flash write_image $<" \
		-c reset \
		-c exit

reset:
	$(OPENOCD)/src/openocd \
		-s $(OPENOCD)/tcl \
		-f interface/stlink.cfg \
		-f target/nrf52.cfg \
		-c init \
		-c reset \
		-c exit
