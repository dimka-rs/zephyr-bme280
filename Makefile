BOARD ?= ebyte_nrf52805
OPENOCD ?= $(HOME)/repos/openocd

all:
	west build -b $(BOARD) -- -DBOARD_ROOT=.

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

flashst: build/zephyr/zephyr.hex
	@echo Flashing: $<
	$(OPENOCD)/src/openocd -s $(OPENOCD)/tcl -f interface/stlink.cfg -f target/nrf52.cfg \
		-c init \
		-c "reset init" \
		-c halt \
		-c "nrf5 mass_erase" \
		-c "flash write_image $<" \
		-c reset \
		-c exit

