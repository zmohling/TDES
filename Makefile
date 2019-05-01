CC=gcc
CXX=g++
ECHO=echo
LINT=cpplint
RM=rm -rvf
MKDIR=mkdir -p
CD=cd

# Binary name
BIN=TDES

# Directories 
ROOT_DIR=.
SRC_DIR=$(ROOT_DIR)/src
SUB_DIRS=$(sort $(dir $(wildcard $(SRC_DIR)/*/)))

# Generated directories
BUILD_DIR=$(ROOT_DIR)/build
LIB_DIR=$(ROOT_DIR)/lib
#GEN_DIRS=$(BUILD_DIR) $(LIB_DIR)

# Libaries
LIBS=$(shell find $(LIB_DIR) -name '*.a')

# Include
INC_DIRS=$(SRC_DIR)
INC_FLAGS=$(foreach d, $(INC_DIRS), -I$d)

# Compilation and linking flags
DEBUG_FLAGS=-Wall -ggdb3
C_FLAGS=$(INC_FLAGS) $(DEBUG_FLAGS)
CXX_FLAGS=$(INC_FLAGS) $(DEBUG_FLAGS) -std=c++11
LD_FLAGS=-lm

# Sources, objects, and dependencies
C_SRC=$(shell find $(SRC_DIR) -name '*.c')
CXX_SRC=$(shell find $(SRC_DIR) -name '*.cc')
OBJ=$(patsubst %.c, $(BUILD_DIR)/%.o, $(filter %.c,$(subst /, , $(C_SRC)))) \
	$(patsubst %.cc, $(BUILD_DIR)/%.o, $(filter %.cc,$(subst /, , $(CXX_SRC))))
MAINS=$(BUILD_DIR)/main.o
DEP=$(OBJ:%.o=%.d)

all: $(BIN)

$(BIN): $(OBJ)
	@$(ECHO) Linking compiled files... 
	@$(CXX) $(LD_FLAGS) $^ -o $(@F)

-include $(DEP)

$(BUILD_DIR)/%.o: $(SUB_DIRS)%.c | $(BUILD_DIR)
	@$(ECHO) Compiling $<
	@$(CC) $(C_FLAGS) -MMD -c $< -o $@

$(BUILD_DIR)/%.o: $(SUB_DIRS)%.cc | $(BUILD_DIR)
	@$(ECHO) Compiling $<
	@$(CXX) $(CXX_FLAGS) -MMD -c $< -o $@

.PHONY: clean lint uninstall
clean:
	@$(ECHO) Removing all generated files and executables...
	@$(RM) $(BUILD_DIR) $(BIN) core vgcore.* valgrind*

deep_clean:
	@$(ECHO) Removing all generated files and executables...
	@$(RM) $(BUILD_DIR) $(BIN) core vgcore.* valgrind*
	@$(ECHO) Removing libraries...
	@$(RM) $(LIB_DIR)

lint:
	@$(ECHO) Linting source files per Google\'s CXX Styleguide...
	@$(LINT) $(C_SRC) $(CXX_SRC)

# Directory generation
$(BUILD_DIR):
	@$(MKDIR) $(BUILD_DIR)
