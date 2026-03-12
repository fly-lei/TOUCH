################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../qpc/src/qs/qs.c \
../qpc/src/qs/qs_64bit.c \
../qpc/src/qs/qs_fp.c \
../qpc/src/qs/qs_rx.c \
../qpc/src/qs/qstamp.c \
../qpc/src/qs/qutest.c 

OBJS += \
./qpc/src/qs/qs.o \
./qpc/src/qs/qs_64bit.o \
./qpc/src/qs/qs_fp.o \
./qpc/src/qs/qs_rx.o \
./qpc/src/qs/qstamp.o \
./qpc/src/qs/qutest.o 

C_DEPS += \
./qpc/src/qs/qs.d \
./qpc/src/qs/qs_64bit.d \
./qpc/src/qs/qs_fp.d \
./qpc/src/qs/qs_rx.d \
./qpc/src/qs/qstamp.d \
./qpc/src/qs/qutest.d 


# Each subdirectory must supply rules for building sources it contributes
qpc/src/qs/%.o qpc/src/qs/%.su qpc/src/qs/%.cyclo: ../qpc/src/qs/%.c qpc/src/qs/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DQ_SPY -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I"E:/STM32F407/TOUCH/qpc/include" -I"E:/STM32F407/TOUCH/qpc/ports/arm-cm/qv/gnu" -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-qpc-2f-src-2f-qs

clean-qpc-2f-src-2f-qs:
	-$(RM) ./qpc/src/qs/qs.cyclo ./qpc/src/qs/qs.d ./qpc/src/qs/qs.o ./qpc/src/qs/qs.su ./qpc/src/qs/qs_64bit.cyclo ./qpc/src/qs/qs_64bit.d ./qpc/src/qs/qs_64bit.o ./qpc/src/qs/qs_64bit.su ./qpc/src/qs/qs_fp.cyclo ./qpc/src/qs/qs_fp.d ./qpc/src/qs/qs_fp.o ./qpc/src/qs/qs_fp.su ./qpc/src/qs/qs_rx.cyclo ./qpc/src/qs/qs_rx.d ./qpc/src/qs/qs_rx.o ./qpc/src/qs/qs_rx.su ./qpc/src/qs/qstamp.cyclo ./qpc/src/qs/qstamp.d ./qpc/src/qs/qstamp.o ./qpc/src/qs/qstamp.su ./qpc/src/qs/qutest.cyclo ./qpc/src/qs/qutest.d ./qpc/src/qs/qutest.o ./qpc/src/qs/qutest.su

.PHONY: clean-qpc-2f-src-2f-qs

