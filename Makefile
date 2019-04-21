CC=gcc
CXX=g++
ECHO=echo
LINT=cpplint
RM=rm -rvf
MKDIR=mkdir -p
CD=cd

# Binary name
BIN=TDES

# Generated directories
BUILD_DIR=$(ROOT_DIR)/build
LIB_DIR=$(ROOT_DIR)/lib
GEN_DIRS=$(BUILD_DIR) $(LIB_DIR)

# Directories 
ROOT_DIR = .
SRC_DIR = $(ROOT_DIR)/src
EXTERN_DIR = $(ROOT_DIR)/external

# Include
INC_DIRS = $(SRC_DIR)
INC_FLAGS=$(foreach d, $(INC_DIRS), -I$d)

# Compilation and linking flags
DEBUG_FLAGS=-Wall -Wextra -ggdb3 -fstack-protector-all
C_FLAGS=$(DEBUG_FLAGS)
CXX_FLAGS=$(DEBUG_FLAGS) -std=c++11
LD_FLAGS=-lm -lncurses 

# Sources, objects, and dependencies
C_SRC=$(shell find $(SRC_DIR) -name '*.c')
CXX_SRC=$(shell find $(SRC_DIR) -name '*.cc')
OBJ=$(C_SRC:%.c=$(BUILD_DIR)/%.o) \
	$(CXX_SRC:%.cc=$(BUILD_DIR)/%.o)
DEP=$(OBJ:%.o=%.d)

all: extern $(BIN)

$(BIN): $(OBJ)
	@$(ECHO) Linking compiled files... 
	@$(CXX) $(LDFLAGS) $^ -o $(@F)
	@$(ECHO) Done. 

-include $(DEP)

$(BUILD_DIR)/%.o: %.c
	@$(ECHO) Compiling $<
	@$(MKDIR) $(@D)
	@$(CC) $(CFLAGS) -MMD -c $< -o $@

$(BUILD_DIR)/%.o: %.cc
	@$(ECHO) Compiling $<
	@$(MKDIR) $(@D)
	@$(CXX) $(CXXFLAGS) -MMD -c $< -o $@

.PHONY: extern clean lint
extern:
	@$(CD) $(EXTERN_DIR) && $(MAKE) -s

clean:
	@$(ECHO) Removing all generated files and executables...
	@$(RM) $(BUILD_DIR) $(LIB_DIR) $(BIN) \
		core vgcore.* valgrind*
	@$(ECHO) Done.

lint:
	@$(ECHO) Linting source files per Google\'s CXX Styleguide...
	@$(LINT) $(C_SRC) $(CXX_SRC)
	@$(ECHO) Done.
	
#$(shell mkdir -p $(GEN_DIRS))
