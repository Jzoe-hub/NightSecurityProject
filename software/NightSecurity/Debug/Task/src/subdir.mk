################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Task/src/task_alarm.c \
../Task/src/task_comm.c \
../Task/src/task_config.c \
../Task/src/task_finger.c \
../Task/src/task_log.c \
../Task/src/task_ota.c \
../Task/src/task_security.c \
../Task/src/task_sensor.c \
../Task/src/task_ui.c \
../Task/src/task_watchdog.c 

OBJS += \
./Task/src/task_alarm.o \
./Task/src/task_comm.o \
./Task/src/task_config.o \
./Task/src/task_finger.o \
./Task/src/task_log.o \
./Task/src/task_ota.o \
./Task/src/task_security.o \
./Task/src/task_sensor.o \
./Task/src/task_ui.o \
./Task/src/task_watchdog.o 

C_DEPS += \
./Task/src/task_alarm.d \
./Task/src/task_comm.d \
./Task/src/task_config.d \
./Task/src/task_finger.d \
./Task/src/task_log.d \
./Task/src/task_ota.d \
./Task/src/task_security.d \
./Task/src/task_sensor.d \
./Task/src/task_ui.d \
./Task/src/task_watchdog.d 


# Each subdirectory must supply rules for building sources it contributes
Task/src/%.o Task/src/%.su Task/src/%.cyclo: ../Task/src/%.c Task/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -I"D:/keil.project2/NightSecurityProject/NightSecurityProject/software/NightSecurity/Lib/inc" -I"D:/keil.project2/NightSecurityProject/NightSecurityProject/software/NightSecurity/Task/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Task-2f-src

clean-Task-2f-src:
	-$(RM) ./Task/src/task_alarm.cyclo ./Task/src/task_alarm.d ./Task/src/task_alarm.o ./Task/src/task_alarm.su ./Task/src/task_comm.cyclo ./Task/src/task_comm.d ./Task/src/task_comm.o ./Task/src/task_comm.su ./Task/src/task_config.cyclo ./Task/src/task_config.d ./Task/src/task_config.o ./Task/src/task_config.su ./Task/src/task_finger.cyclo ./Task/src/task_finger.d ./Task/src/task_finger.o ./Task/src/task_finger.su ./Task/src/task_log.cyclo ./Task/src/task_log.d ./Task/src/task_log.o ./Task/src/task_log.su ./Task/src/task_ota.cyclo ./Task/src/task_ota.d ./Task/src/task_ota.o ./Task/src/task_ota.su ./Task/src/task_security.cyclo ./Task/src/task_security.d ./Task/src/task_security.o ./Task/src/task_security.su ./Task/src/task_sensor.cyclo ./Task/src/task_sensor.d ./Task/src/task_sensor.o ./Task/src/task_sensor.su ./Task/src/task_ui.cyclo ./Task/src/task_ui.d ./Task/src/task_ui.o ./Task/src/task_ui.su ./Task/src/task_watchdog.cyclo ./Task/src/task_watchdog.d ./Task/src/task_watchdog.o ./Task/src/task_watchdog.su

.PHONY: clean-Task-2f-src

