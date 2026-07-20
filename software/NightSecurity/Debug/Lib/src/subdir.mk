################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Lib/src/buzzer.c \
../Lib/src/dht11.c \
../Lib/src/esp8266.c \
../Lib/src/finger.c \
../Lib/src/fire.c \
../Lib/src/key.c \
../Lib/src/mq2.c \
../Lib/src/mq7.c \
../Lib/src/oled.c \
../Lib/src/pir.c \
../Lib/src/radar.c \
../Lib/src/rgb.c \
../Lib/src/voice.c 

OBJS += \
./Lib/src/buzzer.o \
./Lib/src/dht11.o \
./Lib/src/esp8266.o \
./Lib/src/finger.o \
./Lib/src/fire.o \
./Lib/src/key.o \
./Lib/src/mq2.o \
./Lib/src/mq7.o \
./Lib/src/oled.o \
./Lib/src/pir.o \
./Lib/src/radar.o \
./Lib/src/rgb.o \
./Lib/src/voice.o 

C_DEPS += \
./Lib/src/buzzer.d \
./Lib/src/dht11.d \
./Lib/src/esp8266.d \
./Lib/src/finger.d \
./Lib/src/fire.d \
./Lib/src/key.d \
./Lib/src/mq2.d \
./Lib/src/mq7.d \
./Lib/src/oled.d \
./Lib/src/pir.d \
./Lib/src/radar.d \
./Lib/src/rgb.d \
./Lib/src/voice.d 


# Each subdirectory must supply rules for building sources it contributes
Lib/src/%.o Lib/src/%.su Lib/src/%.cyclo: ../Lib/src/%.c Lib/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -I"D:/keil.project2/NightSecurityProject/NightSecurityProject/software/NightSecurity/Lib/inc" -I"D:/keil.project2/NightSecurityProject/NightSecurityProject/software/NightSecurity/Task/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Lib-2f-src

clean-Lib-2f-src:
	-$(RM) ./Lib/src/buzzer.cyclo ./Lib/src/buzzer.d ./Lib/src/buzzer.o ./Lib/src/buzzer.su ./Lib/src/dht11.cyclo ./Lib/src/dht11.d ./Lib/src/dht11.o ./Lib/src/dht11.su ./Lib/src/esp8266.cyclo ./Lib/src/esp8266.d ./Lib/src/esp8266.o ./Lib/src/esp8266.su ./Lib/src/finger.cyclo ./Lib/src/finger.d ./Lib/src/finger.o ./Lib/src/finger.su ./Lib/src/fire.cyclo ./Lib/src/fire.d ./Lib/src/fire.o ./Lib/src/fire.su ./Lib/src/key.cyclo ./Lib/src/key.d ./Lib/src/key.o ./Lib/src/key.su ./Lib/src/mq2.cyclo ./Lib/src/mq2.d ./Lib/src/mq2.o ./Lib/src/mq2.su ./Lib/src/mq7.cyclo ./Lib/src/mq7.d ./Lib/src/mq7.o ./Lib/src/mq7.su ./Lib/src/oled.cyclo ./Lib/src/oled.d ./Lib/src/oled.o ./Lib/src/oled.su ./Lib/src/pir.cyclo ./Lib/src/pir.d ./Lib/src/pir.o ./Lib/src/pir.su ./Lib/src/radar.cyclo ./Lib/src/radar.d ./Lib/src/radar.o ./Lib/src/radar.su ./Lib/src/rgb.cyclo ./Lib/src/rgb.d ./Lib/src/rgb.o ./Lib/src/rgb.su ./Lib/src/voice.cyclo ./Lib/src/voice.d ./Lib/src/voice.o ./Lib/src/voice.su

.PHONY: clean-Lib-2f-src

