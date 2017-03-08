################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../system/src/stm32f7-hal/stm32f7xx_hal.c \
../system/src/stm32f7-hal/stm32f7xx_hal_cortex.c \
../system/src/stm32f7-hal/stm32f7xx_hal_crc_ex.c \
../system/src/stm32f7-hal/stm32f7xx_hal_dma.c \
../system/src/stm32f7-hal/stm32f7xx_hal_dma2d.c \
../system/src/stm32f7-hal/stm32f7xx_hal_flash.c \
../system/src/stm32f7-hal/stm32f7xx_hal_gpio.c \
../system/src/stm32f7-hal/stm32f7xx_hal_iwdg.c \
../system/src/stm32f7-hal/stm32f7xx_hal_ltdc.c \
../system/src/stm32f7-hal/stm32f7xx_hal_pwr.c \
../system/src/stm32f7-hal/stm32f7xx_hal_pwr_ex.c \
../system/src/stm32f7-hal/stm32f7xx_hal_rcc.c \
../system/src/stm32f7-hal/stm32f7xx_hal_rcc_ex.c \
../system/src/stm32f7-hal/stm32f7xx_hal_sd.c \
../system/src/stm32f7-hal/stm32f7xx_hal_sdram.c \
../system/src/stm32f7-hal/stm32f7xx_hal_smartcard_ex.c \
../system/src/stm32f7-hal/stm32f7xx_hal_sram.c \
../system/src/stm32f7-hal/stm32f7xx_ll_fmc.c \
../system/src/stm32f7-hal/stm32f7xx_ll_sdmmc.c 

OBJS += \
./system/src/stm32f7-hal/stm32f7xx_hal.o \
./system/src/stm32f7-hal/stm32f7xx_hal_cortex.o \
./system/src/stm32f7-hal/stm32f7xx_hal_crc_ex.o \
./system/src/stm32f7-hal/stm32f7xx_hal_dma.o \
./system/src/stm32f7-hal/stm32f7xx_hal_dma2d.o \
./system/src/stm32f7-hal/stm32f7xx_hal_flash.o \
./system/src/stm32f7-hal/stm32f7xx_hal_gpio.o \
./system/src/stm32f7-hal/stm32f7xx_hal_iwdg.o \
./system/src/stm32f7-hal/stm32f7xx_hal_ltdc.o \
./system/src/stm32f7-hal/stm32f7xx_hal_pwr.o \
./system/src/stm32f7-hal/stm32f7xx_hal_pwr_ex.o \
./system/src/stm32f7-hal/stm32f7xx_hal_rcc.o \
./system/src/stm32f7-hal/stm32f7xx_hal_rcc_ex.o \
./system/src/stm32f7-hal/stm32f7xx_hal_sd.o \
./system/src/stm32f7-hal/stm32f7xx_hal_sdram.o \
./system/src/stm32f7-hal/stm32f7xx_hal_smartcard_ex.o \
./system/src/stm32f7-hal/stm32f7xx_hal_sram.o \
./system/src/stm32f7-hal/stm32f7xx_ll_fmc.o \
./system/src/stm32f7-hal/stm32f7xx_ll_sdmmc.o 

C_DEPS += \
./system/src/stm32f7-hal/stm32f7xx_hal.d \
./system/src/stm32f7-hal/stm32f7xx_hal_cortex.d \
./system/src/stm32f7-hal/stm32f7xx_hal_crc_ex.d \
./system/src/stm32f7-hal/stm32f7xx_hal_dma.d \
./system/src/stm32f7-hal/stm32f7xx_hal_dma2d.d \
./system/src/stm32f7-hal/stm32f7xx_hal_flash.d \
./system/src/stm32f7-hal/stm32f7xx_hal_gpio.d \
./system/src/stm32f7-hal/stm32f7xx_hal_iwdg.d \
./system/src/stm32f7-hal/stm32f7xx_hal_ltdc.d \
./system/src/stm32f7-hal/stm32f7xx_hal_pwr.d \
./system/src/stm32f7-hal/stm32f7xx_hal_pwr_ex.d \
./system/src/stm32f7-hal/stm32f7xx_hal_rcc.d \
./system/src/stm32f7-hal/stm32f7xx_hal_rcc_ex.d \
./system/src/stm32f7-hal/stm32f7xx_hal_sd.d \
./system/src/stm32f7-hal/stm32f7xx_hal_sdram.d \
./system/src/stm32f7-hal/stm32f7xx_hal_smartcard_ex.d \
./system/src/stm32f7-hal/stm32f7xx_hal_sram.d \
./system/src/stm32f7-hal/stm32f7xx_ll_fmc.d \
./system/src/stm32f7-hal/stm32f7xx_ll_sdmmc.d 


# Each subdirectory must supply rules for building sources it contributes
system/src/stm32f7-hal/%.o: ../system/src/stm32f7-hal/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wpadded -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal  -g3 -DDEBUG -DUSE_FULL_ASSERT -DSTM32F746xx -DUSE_HAL_DRIVER -DHSE_VALUE=25000000 -DAPB1_PERPH_FREQ=50000000 -DAPB2_PERPH_FREQ=100000000 -DAPB1_TIM_FREQ=100000000 -DAPB2_TIM_FREQ=200000000 -D__VFP_FP__ -I"../include" -I"E:\PROGRAMOWANIE\STM32_TOOLCHAIN\ECLIPSE_DEV\shell\bsp\include" -I"E:\PROGRAMOWANIE\STM32_TOOLCHAIN\ECLIPSE_DEV\shell\FreeRTOS\include" -I"E:\PROGRAMOWANIE\STM32_TOOLCHAIN\ECLIPSE_DEV\shell\FreeRTOS\portable\GCC" -I"E:\PROGRAMOWANIE\STM32_TOOLCHAIN\ECLIPSE_DEV\shell\FreeRTOS\portable\MemMang" -I"E:\PROGRAMOWANIE\STM32_TOOLCHAIN\ECLIPSE_DEV\shell\managment" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f7-hal" -I"E:\PROGRAMOWANIE\STM32_TOOLCHAIN\ECLIPSE_DEV\shell\FreeRTOS-Plus-FAT\include" -I"E:\PROGRAMOWANIE\STM32_TOOLCHAIN\ECLIPSE_DEV\shell\FreeRTOS-Plus-FAT" -I"E:\PROGRAMOWANIE\STM32_TOOLCHAIN\ECLIPSE_DEV\shell\FreeRTOS-Plus-FAT\portable\common" -std=gnu11 -Wmissing-prototypes -Wstrict-prototypes -Wbad-function-cast -Wno-bad-function-cast -Wno-conversion -Wno-sign-conversion -Wno-unused-parameter -Wno-sign-compare -Wno-missing-prototypes -Wno-missing-declarations -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


