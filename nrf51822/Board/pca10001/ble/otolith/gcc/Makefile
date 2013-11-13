BOARD := BOARD_PCA10001

C_SOURCE_FILES += main.c
#C_SOURCE_FILES += app_error.c
C_SOURCE_FILES += nrf_delay.c

OUTPUT_FILENAME := otolith_gcc
SDK_PATH = ../../../../../
MKCM_PATH = ../../../../../

#Uncomment correct line if you have softdevice programmed on the chip.
USE_SOFTDEVICE := S110
#USE_SOFTDEVICE := S210

include $(MKCM_PATH)Source/templates/gcc/Makefile.common
