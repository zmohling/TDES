CC=gcc
CXX=g++
ECHO=echo
LINT=cpplint
RM=rm -rvf
MKDIR=mkdir -p

BIN=encryptor

ROOT_DIR= .
BUILD_DIR=$(ROOT_DIR)/build
SRC_DIR=$(ROOT_DIR)/src
INC_DIRS=$(SRC_DIR)

INC_FLAGS=$(foreach d, $(INC_DIRS), -I$d)
CFLAGS=-Wall -Wextra -ggdb3 -funroll-loops\
	   -fstack-protector-all
CXXFLAGS=-std=c++11 -Wall -Wextra -ggdb3 -funroll-loops\
		 -fstack-protector-all
LDFLAGS=-lm -lncurses 

C_SRC=$(shell find $(SRC_DIR) -name '*.c')
CXX_SRC=$(shell find $(SRC_DIR) -name '*.cc')
#OBJ=$(patsubst %.c, $(BUILD_DIR)/%.o, $(lastword $(subst /, , $(C_SRC)))) \
	$(patsubst %.cc, $(BUILD_DIR)/%.o, $(lastword $(subst /, , $(CXX_SRC))))
OBJ=$(C_SRC:%.c=$(BUILD_DIR)/%.o) \
	$(CXX_SRC:%.cc=$(BUILD_DIR)/%.o)
DEP=$(OBJ:%.o=%.d)

$(BIN): $(OBJ)
	@$(ECHO) Linking compiled files... 
	@$(CXX) $(LDFLAGS) $^ -o $(@F)
	@$(ECHO) Done. 

-include $(DEP)

$(BUILD_DIR)/%.o: %.c
	@$(ECHO) Compiling $<...
	@$(MKDIR) $(@D)
	@$(CC) $(CFLAGS) -MMD -c $< -o $@

$(BUILD_DIR)/%.o: %.cc
	@$(ECHO) Compiling $<...
	@$(MKDIR) $(@D)
	@$(CXX) $(CXXFLAGS) -MMD -c $< -o $@

.PHONY: clean lint
clean:
	@$(ECHO) Removing all generated files and executables...
	@$(RM) $(BUILD_DIR) $(BIN) $(SRC_DIR)/lex.yy.c $(SRC_DIR)/def_compiler.tab.* \
		core vgcore.* valgrind*
	@$(ECHO) Done.

lint:
	@$(ECHO) Linting source files per Google\'s CXX Styleguide...
	@$(LINT) $(SRC_DIR)/*.c $(SRC_DIR)/*.cc $(SRC_DIR)/*.h
	@$(ECHO) Done.
