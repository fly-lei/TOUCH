################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../qpc/ports/arm-cm/qv/gnu/qv_port.c 

OBJS += \
./qpc/ports/arm-cm/qv/gnu/qv_port.o 

C_DEPS += \
./qpc/ports/arm-cm/qv/gnu/qv_port.d 


# Each subdirectory must supply rules for building sources it contributes
qpc/ports/arm-cm/qv/gnu/%.o qpc/ports/arm-cm/qv/gnu/%.su qpc/ports/arm-cm/qv/gnu/%.cyclo: ../qpc/ports/arm-cm/qv/gnu/%.c qpc/ports/arm-cm/qv/gnu/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I"E:/STM32F407/TOUCH/qpc/include" -I"E:/STM32F407/TOUCH/qpc/ports/arm-cm/qv/gnu" -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-qpc-2f-ports-2f-arm-2d-cm-2f-qv-2f-gnu

clean-qpc-2f-ports-2f-arm-2d-cm-2f-qv-2f-gnu:
	-$(RM) ./qpc/ports/arm-cm/qv/gnu/qv_port.cyclo ./qpc/ports/arm-cm/qv/gnu/qv_port.d ./qpc/ports/arm-cm/qv/gnu/qv_port.o ./qpc/ports/arm-cm/qv/gnu/qv_port.su

.PHONY: clean-qpc-2f-ports-2f-arm-2d-cm-2f-qv-2f-gnu

