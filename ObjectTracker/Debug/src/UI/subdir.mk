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
	g++ -I/home/steven/workspace/ARDrone_SDK_1_8/ARDroneLib -I/home/steven/workspace/ARDrone_SDK_1_8/ARDroneLib/VP_SDK/VP_Stages -I/home/steven/workspace/ARDrone_SDK_1_8/ARDroneLib/VP_SDK/VP_Os/linux -I/home/steven/workspace/ARDrone_SDK_1_8/ARDroneLib/Soft/Common -I/home/steven/workspace/ARDrone_SDK_1_8/ARDroneLib/VP_SDK -I/home/steven/workspace/ARDrone_SDK_1_8/ARDroneLib/Soft/Lib -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/UI/%.o: ../src/UI/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/home/steven/workspace/ARDrone_SDK_1_8/ARDroneLib/Soft/Lib -I/home/steven/workspace/ARDrone_SDK_1_8/ARDroneLib/VP_SDK/VP_Stages -I/home/steven/workspace/ARDrone_SDK_1_8/ARDroneLib -I/home/steven/workspace/ARDrone_SDK_1_8/ARDroneLib/VP_SDK/VP_Os/linux -I/home/steven/workspace/ARDrone_SDK_1_8/ARDroneLib/VP_SDK -I/home/steven/workspace/ARDrone_SDK_1_8/ARDroneLib/Soft/Common -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


