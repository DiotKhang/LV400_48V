################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
cllc/%.obj: ../cllc/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs2031/ccs/tools/compiler/ti-cgt-c2000_22.6.2.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla2 --float_support=fpu64 --tmu_support=tmu0 --vcu_support=vcu2 -O3 --opt_for_speed=5 --fp_mode=relaxed --include_path="C:/Users/nimdu/workspace_ccstheia/LV400_48V" --include_path="C:/Users/nimdu/workspace_ccstheia/LV400_48V/device" --include_path="C:/Users/nimdu/workspace_ccstheia/LV400_48V/cllc" --include_path="C:/Users/nimdu/workspace_ccstheia/LV400_48V/libraries" --include_path="C:/Users/nimdu/workspace_ccstheia/LV400_48V/libraries/CLAmath" --include_path="C:/Users/nimdu/workspace_ccstheia/LV400_48V/device/driverlib" --include_path="C:/ti/ccs2031/ccs/tools/compiler/ti-cgt-c2000_22.6.2.LTS/include" --define=_DEBUG --define=_FLASH --define=_TI_EABI_ --define=LARGE_MODEL --define=CPU1 --define=F28X_DEVICE --define=CLA_DEBUG=0 --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --cla_background_task=on --preproc_with_compile --preproc_dependency="cllc/$(basename $(<F)).d_raw" --include_path="C:/Users/nimdu/workspace_ccstheia/LV400_48V/CPU1_FLASH/syscfg" --obj_directory="cllc" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

cllc/%.obj: ../cllc/%.cla $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs2031/ccs/tools/compiler/ti-cgt-c2000_22.6.2.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla2 --float_support=fpu64 --tmu_support=tmu0 --vcu_support=vcu2 -O3 --opt_for_speed=5 --fp_mode=relaxed --include_path="C:/Users/nimdu/workspace_ccstheia/LV400_48V" --include_path="C:/Users/nimdu/workspace_ccstheia/LV400_48V/device" --include_path="C:/Users/nimdu/workspace_ccstheia/LV400_48V/cllc" --include_path="C:/Users/nimdu/workspace_ccstheia/LV400_48V/libraries" --include_path="C:/Users/nimdu/workspace_ccstheia/LV400_48V/libraries/CLAmath" --include_path="C:/Users/nimdu/workspace_ccstheia/LV400_48V/device/driverlib" --include_path="C:/ti/ccs2031/ccs/tools/compiler/ti-cgt-c2000_22.6.2.LTS/include" --define=_DEBUG --define=_FLASH --define=_TI_EABI_ --define=LARGE_MODEL --define=CPU1 --define=F28X_DEVICE --define=CLA_DEBUG=0 --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --cla_background_task=on --preproc_with_compile --preproc_dependency="cllc/$(basename $(<F)).d_raw" --include_path="C:/Users/nimdu/workspace_ccstheia/LV400_48V/CPU1_FLASH/syscfg" --obj_directory="cllc" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


