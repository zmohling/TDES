CC=gcc
CXX=g++
ECHO=echo
LINT=cpplint
RM=rm -rvf
MKDIR=mkdir -p
CD=cd
INSTALL=install

# Binary name
BIN=tdes

# Directories 
DEST_DIR=/
ROOT_DIR=.
LIB_DIR=$(ROOT_DIR)/lib
SRC_BASE_DIR=$(ROOT_DIR)/src
SRC_SUB_DIRS=$(sort $(dir $(wildcard $(SRC_BASE_DIR)/*/)))
INSTALL_DIR=usr/local/bin

# Generated directories
BUILD_DIR=$(ROOT_DIR)/build

# Libaries
LIBS=$(shell find $(LIB_DIR) -name '*.a')

# Include
INC_DIRS=$(SRC_BASE_DIR)
INC_FLAGS=$(foreach d, $(INC_DIRS), -I$d)

# Compilation and linking flags
DEBUG_FLAGS=-O3
RELEASE_FLAGS=-O3
C_FLAGS=$(INC_FLAGS) $(DEBUG_FLAGS)
CXX_FLAGS=$(INC_FLAGS) $(DEBUG_FLAGS) -std=c++11 -pthread
LD_FLAGS=-lm -lpthread -lssl -lcrypto

# Sources, objects, and dependencies
C_SRC=$(shell find $(SRC_BASE_DIR) -name '*.c')
CXX_SRC=$(shell find $(SRC_BASE_DIR) -name '*.cc')
OBJ=$(patsubst %.c, $(BUILD_DIR)/%.o, $(filter %.c,$(subst /, , $(C_SRC)))) \
	$(patsubst %.cc, $(BUILD_DIR)/%.o, $(filter %.cc,$(subst /, , $(CXX_SRC))))
DEP=$(OBJ:%.o=%.d)

all: $(BIN)

$(BIN): $(OBJ)
	@$(ECHO) Linking compiled files... 
	@$(CXX) $(LD_FLAGS) $^ -o $(@F)

-include $(DEP)

$(BUILD_DIR)/%.o: $(SRC_SUB_DIRS)%.c | $(BUILD_DIR)
	@$(ECHO) Compiling $<
	@$(CC) $(C_FLAGS) -MMD -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_SUB_DIRS)%.cc | $(BUILD_DIR)
	@$(ECHO) Compiling $<
	@$(CXX) $(CXX_FLAGS) -MMD -c $< -o $@

.PHONY: clean lint install uninstall
clean:
	@$(ECHO) Removing all generated files and executables...
	@$(RM) $(BUILD_DIR) $(BIN) *.txt *.mp4 core vgcore.* valgrind*

lint:
	@$(ECHO) Linting source files per Google\'s CXX Styleguide...
	@$(LINT) $(C_SRC) $(CXX_SRC)

install:
	@$(ECHO) "Installing to $(DEST_DIR)$(INSTALL_DIR)"
	@$(INSTALL) $(BIN) $(DEST_DIR)$(INSTALL_DIR)

uninstall:
	@$(RM) $(DEST_DIR)$(INSTALL_DIR)/$(BIN)

# Directory generation
$(BUILD_DIR):
	@$(MKDIR) $(BUILD_DIR)
