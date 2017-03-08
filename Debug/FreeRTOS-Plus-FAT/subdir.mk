################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../FreeRTOS-Plus-FAT/ff_crc.c \
../FreeRTOS-Plus-FAT/ff_dev_support.c \
../FreeRTOS-Plus-FAT/ff_dir.c \
../FreeRTOS-Plus-FAT/ff_error.c \
../FreeRTOS-Plus-FAT/ff_fat.c \
../FreeRTOS-Plus-FAT/ff_file.c \
../FreeRTOS-Plus-FAT/ff_format.c \
../FreeRTOS-Plus-FAT/ff_ioman.c \
../FreeRTOS-Plus-FAT/ff_locking.c \
../FreeRTOS-Plus-FAT/ff_memory.c \
../FreeRTOS-Plus-FAT/ff_stdio.c \
../FreeRTOS-Plus-FAT/ff_string.c \
../FreeRTOS-Plus-FAT/ff_sys.c \
../FreeRTOS-Plus-FAT/ff_time.c 

OBJS += \
./FreeRTOS-Plus-FAT/ff_crc.o \
./FreeRTOS-Plus-FAT/ff_dev_support.o \
./FreeRTOS-Plus-FAT/ff_dir.o \
./FreeRTOS-Plus-FAT/ff_error.o \
./FreeRTOS-Plus-FAT/ff_fat.o \
./FreeRTOS-Plus-FAT/ff_file.o \
./FreeRTOS-Plus-FAT/ff_format.o \
./FreeRTOS-Plus-FAT/ff_ioman.o \
./FreeRTOS-Plus-FAT/ff_locking.o \
./FreeRTOS-Plus-FAT/ff_memory.o \
./FreeRTOS-Plus-FAT/ff_stdio.o \
./FreeRTOS-Plus-FAT/ff_string.o \
./FreeRTOS-Plus-FAT/ff_sys.o \
./FreeRTOS-Plus-FAT/ff_time.o 

C_DEPS += \
./FreeRTOS-Plus-FAT/ff_crc.d \
./FreeRTOS-Plus-FAT/ff_dev_support.d \
./FreeRTOS-Plus-FAT/ff_dir.d \
./FreeRTOS-Plus-FAT/ff_error.d \
./FreeRTOS-Plus-FAT/ff_fat.d \
./FreeRTOS-Plus-FAT/ff_file.d \
./FreeRTOS-Plus-FAT/ff_format.d \
./FreeRTOS-Plus-FAT/ff_ioman.d \
./FreeRTOS-Plus-FAT/ff_locking.d \
./FreeRTOS-Plus-FAT/ff_memory.d \
./FreeRTOS-Plus-FAT/ff_stdio.d \
./FreeRTOS-Plus-FAT/ff_string.d \
./FreeRTOS-Plus-FAT/ff_sys.d \
./FreeRTOS-Plus-FAT/ff_time.d 


# Each subdirectory must supply rules for building sources it contributes
FreeRTOS-Plus-FAT/%.o: ../FreeRTOS-Plus-FAT/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wpadded -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal  -g3 -DDEBUG -DUSE_FULL_ASSERT -DSTM32F746xx -DUSE_HAL_DRIVER -DHSE_VALUE=25000000 -DAPB1_PERPH_FREQ=50000000 -DAPB2_PERPH_FREQ=100000000 -DAPB1_TIM_FREQ=100000000 -DAPB2_TIM_FREQ=200000000 -D__VFP_FP__ -I"../include" -I"E:\PROGRAMOWANIE\STM32_TOOLCHAIN\ECLIPSE_DEV\shell\bsp\include" -I"E:\PROGRAMOWANIE\STM32_TOOLCHAIN\ECLIPSE_DEV\shell\FreeRTOS\include" -I"E:\PROGRAMOWANIE\STM32_TOOLCHAIN\ECLIPSE_DEV\shell\FreeRTOS\portable\GCC" -I"E:\PROGRAMOWANIE\STM32_TOOLCHAIN\ECLIPSE_DEV\shell\FreeRTOS\portable\MemMang" -I"E:\PROGRAMOWANIE\STM32_TOOLCHAIN\ECLIPSE_DEV\shell\managment" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f7-hal" -I"E:\PROGRAMOWANIE\STM32_TOOLCHAIN\ECLIPSE_DEV\shell\FreeRTOS-Plus-FAT\include" -I"E:\PROGRAMOWANIE\STM32_TOOLCHAIN\ECLIPSE_DEV\shell\FreeRTOS-Plus-FAT" -I"E:\PROGRAMOWANIE\STM32_TOOLCHAIN\ECLIPSE_DEV\shell\FreeRTOS-Plus-FAT\portable\common" -std=gnu11 -Wmissing-prototypes -Wstrict-prototypes -Wbad-function-cast -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


