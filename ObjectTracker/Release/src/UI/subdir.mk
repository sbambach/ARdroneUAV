################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/UI/gamepad.cpp \
../src/UI/planner.cpp 

C_SRCS += \
../src/UI/ui.c 

OBJS += \
./src/UI/gamepad.o \
./src/UI/planner.o \
./src/UI/ui.o 

C_DEPS += \
./src/UI/ui.d 

CPP_DEPS += \
./src/UI/gamepad.d \
./src/UI/planner.d 


# Each subdirectory must supply rules for building sources it contributes
src/UI/%.o: ../src/UI/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/UI/%.o: ../src/UI/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


