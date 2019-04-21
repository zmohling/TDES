CC=gcc
CXX=g++
ECHO=echo
LINT=cpplint
RM=rm -rvf
MKDIR=mkdir -p
CD=cd

# Binary name
BIN=../test

# Directories 
ROOT_DIR=..
SRC_DIR=$(ROOT_DIR)/src
EXTERN_DIR=$(ROOT_DIR)/external
LIB_DIR=$(ROOT_DIR)/lib
TESTS_DIR=$(ROOT_DIR)/tests
BUILD_DIR=$(ROOT_DIR)/build

# Google Test & Mock
GTEST_DIR=$(EXTERN_DIR)/googletest
GMOCK_DIR=$(EXTERN_DIR)/googlemock
G_FLAGS=-isystem $(GTEST_DIR)/include \
			-isystem $(GMOCK_DIR)/include -pthread
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
				$(GTEST_DIR)/include/gtest/internal/*.h
GMOCK_HEADERS = $(GMOCK_DIR)/include/gmock/*.h \
                $(GMOCK_DIR)/include/gmock/internal/*.h \
                $(GTEST_HEADERS)

# Libaries
LIBS=$(shell find $(LIB_DIR) -name '*.a')

# Include
INC_DIRS=$(SRC_DIR)
INC_FLAGS=$(foreach d, $(INC_DIRS), -I$d)

# Compilation and linking flags
DEBUG_FLAGS=-Wall -Wextra -ggdb3 -fstack-protector-all
CXX_FLAGS=$(INC_FLAGS) $(DEBUG_FLAGS) -std=c++11
LD_FLAGS=-lm

# Sources, objects, and dependencies
TESTS_SRC = $(shell find $(TESTS_DIR) -name '*.cc')
TESTS_OBJ=$(patsubst %.cc, $(BUILD_DIR)/%.o, $(strip $(filter %.cc, $(subst /, , $(TESTS_SRC)))))
OBJ=$(wildcard $(BUILD_DIR)/*.o)
MAINS=$(BUILD_DIR)/main.o
DEP=$(OBJ:%.o=%.d)

$(BIN): $(OBJ) $(TESTS_OBJ) $(LIBS)
	@$(ECHO) Linking compiled tests... 
	@$(CXX) $(G_FLAGS) $(CXX_FLAGS) -L$(LIB_DIR) -lgtest_main \
		-lpthread $(filter-out $(MAINS), $^) -o $@

$(BUILD_DIR)/%.o: $(TEST_DIR)%.cc $(GMOCK_HEADERS)
	@$(ECHO) Compiling $<
	@$(CXX) $(G_FLAGS) $(CXX_FLAGS) -c $< -o $@
