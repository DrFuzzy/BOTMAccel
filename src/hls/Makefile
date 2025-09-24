#######################################################################################
.PHONY: help
help:
	@echo "Makefile Usage:"
	@echo "  make all"
	@echo "      Command to run all targets: csim, csynth, cosim, and export in turn."
	@echo ""
	@echo "  make csim"
	@echo "      Command to run only simulation (csim)."
	@echo ""
	@echo "  make csynth"
	@echo "      Command to run only synthesis (csynth)."
	@echo ""
	@echo "  make cosim"
	@echo "      Command to run cosimulation (cosim)."
	@echo ""
	@echo "  make export"
	@echo "      Command to export the HLS design as an IP package."
	@echo ""
	@echo "  make clean"
	@echo "      Command to remove the generated files."
	@echo ""
	@echo "  make help"
	@echo "      Print this help."
	@echo ""
#######################################################################################

SHELL 			:= bash
SOURCES         := aco.cpp aco.h   # List your source files
SOURCES_TB      := ${SOURCES} aco_tb.cpp  # Testbench files

CONFIG 			:= hls_config.cfg  # HLS configuration file
WORK_DIR 		:= hls_component/aco  # Change to your project directory
COMMON_OPTIONS 	:= --mode hls --config ${CONFIG} --work_dir ${WORK_DIR}

# The default target: runs csim, csynth, cosim, and export in sequence
all: csim csynth cosim export

# Synthesis step (csynth)
csynth: ${SOURCES} ${CONFIG}
	v++ --compile ${COMMON_OPTIONS}

# Simulation step (csim) - Forces clean before running csim
csim: clean
csim: ${SOURCES_TB} ${CONFIG}
	vitis-run --csim ${COMMON_OPTIONS}

# Cosimulation step (cosim)
cosim: ${SOURCES_TB} ${CONFIG}
	vitis-run --cosim ${COMMON_OPTIONS}

# Export step - Export the design as an IP package
export: ${SOURCES} ${CONFIG}
	vitis-run --package ${COMMON_OPTIONS}

# Clean step to remove generated files
.PHONY: clean
clean:
	rm -rf ${WORK_DIR} vitis_c++/ .Xil/ xcd.log
