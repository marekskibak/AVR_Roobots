################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include 1-Wire/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
Uarttt.lss \

FLASH_IMAGE += \
Uarttt.hex \

EEPROM_IMAGE += \
Uarttt.eep \

SIZEDUMMY += \
sizedummy \


# All Target
all: Uarttt.elf secondary-outputs

# Tool invocations
Uarttt.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,Uarttt.map -mmcu=atmega16 -o"Uarttt.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

Uarttt.lss: Uarttt.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S Uarttt.elf  >"Uarttt.lss"
	@echo 'Finished building: $@'
	@echo ' '

Uarttt.hex: Uarttt.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -O ihex Uarttt.elf  "Uarttt.hex"
	@echo 'Finished building: $@'
	@echo ' '

Uarttt.eep: Uarttt.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex Uarttt.elf  "Uarttt.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: Uarttt.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=atmega16 Uarttt.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(ASM_DEPS)$(EEPROM_IMAGE)$(FLASH_IMAGE)$(ELFS)$(LSS)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS) Uarttt.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
