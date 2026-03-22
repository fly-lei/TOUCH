################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../elab/driver/elab_device.c 

OBJS += \
./elab/driver/elab_device.o 

C_DEPS += \
./elab/driver/elab_device.d 


# Each subdirectory must supply rules for building sources it contributes
elab/driver/%.o elab/driver/%.su elab/driver/%.cyclo: ../elab/driver/%.c elab/driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DQ_SPY -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I"E:/STM32F407/TOUCH/qpc/include" -I"E:/STM32F407/TOUCH/qpc/ports/arm-cm/qv/gnu" -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"E:/STM32F407/TOUCH/elab/driver" -I"E:/STM32F407/TOUCH/elab/init" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-elab-2f-driver

clean-elab-2f-driver:
	-$(RM) ./elab/driver/elab_device.cyclo ./elab/driver/elab_device.d ./elab/driver/elab_device.o ./elab/driver/elab_device.su

.PHONY: clean-elab-2f-driver

