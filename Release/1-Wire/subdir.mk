################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../1-Wire/crc8.c \
../1-Wire/ds18x20.c \
../1-Wire/onewire.c 

OBJS += \
./1-Wire/crc8.o \
./1-Wire/ds18x20.o \
./1-Wire/onewire.o 

C_DEPS += \
./1-Wire/crc8.d \
./1-Wire/ds18x20.d \
./1-Wire/onewire.d 


# Each subdirectory must supply rules for building sources it contributes
1-Wire/%.o: ../1-Wire/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -Os -fpack-struct -fshort-enums -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega16 -DF_CPU=11059200UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


