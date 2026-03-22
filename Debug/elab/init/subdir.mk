################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../elab/init/elab_export.c 

OBJS += \
./elab/init/elab_export.o 

C_DEPS += \
./elab/init/elab_export.d 


# Each subdirectory must supply rules for building sources it contributes
elab/init/%.o elab/init/%.su elab/init/%.cyclo: ../elab/init/%.c elab/init/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DQ_SPY -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I"E:/STM32F407/TOUCH/qpc/include" -I"E:/STM32F407/TOUCH/qpc/ports/arm-cm/qv/gnu" -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"E:/STM32F407/TOUCH/elab/driver" -I"E:/STM32F407/TOUCH/elab/init" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-elab-2f-init

clean-elab-2f-init:
	-$(RM) ./elab/init/elab_export.cyclo ./elab/init/elab_export.d ./elab/init/elab_export.o ./elab/init/elab_export.su

.PHONY: clean-elab-2f-init

