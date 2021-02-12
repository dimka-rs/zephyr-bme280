# SPDX-License-Identifier: Apache-2.0

## I could probably use stlink+hla, but
## zephyr/boards/common/openocd-nrf5.board.cmake
## only describes swd

#set(OPENOCD_NRF5_INTERFACE "stlink")

board_runner_args(nrfjprog "--nrf-family=NRF52")
board_runner_args(jlink "--device=nrf52" "--speed=4000")
#board_runner_args(pyocd "--target=nrf52" "--frequency=4000000")
#board_runner_args(openocd "")

include(${ZEPHYR_BASE}/boards/common/nrfjprog.board.cmake)
include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)
#include(${ZEPHYR_BASE}/boards/common/pyocd.board.cmake)
#include(${ZEPHYR_BASE}/boards/common/openocd-nrf5.board.cmake)
