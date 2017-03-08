################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../bsp/lcd/stm32746g_discovery_lcd.c 

OBJS += \
./bsp/lcd/stm32746g_discovery_lcd.o 

C_DEPS += \
./bsp/lcd/stm32746g_discovery_lcd.d 


# Each subdirectory must supply rules for building sources it contributes
bsp/lcd/%.o: ../bsp/lcd/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wpadded -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal  -g3 -DDEBUG -DUSE_FULL_ASSERT -DSTM32F746xx -DUSE_HAL_DRIVER -DHSE_VALUE=25000000 -DAPB1_PERPH_FREQ=50000000 -DAPB2_PERPH_FREQ=100000000 -DAPB1_TIM_FREQ=100000000 -DAPB2_TIM_FREQ=200000000 -D__VFP_FP__ -I"../include" -I"E:\PROGRAMOWANIE\STM32_TOOLCHAIN\ECLIPSE_DEV\shell\bsp\include" -I"E:\PROGRAMOWANIE\STM32_TOOLCHAIN\ECLIPSE_DEV\shell\FreeRTOS\include" -I"E:\PROGRAMOWANIE\STM32_TOOLCHAIN\ECLIPSE_DEV\shell\FreeRTOS\portable\GCC" -I"E:\PROGRAMOWANIE\STM32_TOOLCHAIN\ECLIPSE_DEV\shell\FreeRTOS\portable\MemMang" -I"E:\PROGRAMOWANIE\STM32_TOOLCHAIN\ECLIPSE_DEV\shell\managment" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f7-hal" -I"E:\PROGRAMOWANIE\STM32_TOOLCHAIN\ECLIPSE_DEV\shell\FreeRTOS-Plus-FAT\include" -I"E:\PROGRAMOWANIE\STM32_TOOLCHAIN\ECLIPSE_DEV\shell\FreeRTOS-Plus-FAT" -I"E:\PROGRAMOWANIE\STM32_TOOLCHAIN\ECLIPSE_DEV\shell\FreeRTOS-Plus-FAT\portable\common" -std=gnu11 -Wmissing-prototypes -Wstrict-prototypes -Wbad-function-cast -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


