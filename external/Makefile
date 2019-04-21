CC=gcc
CXX=g++
ECHO=echo
RM=rm -rvf
MKDIR=mkdir -p

ROOT_DIR= ..
TEST_DIR=$(ROOT_DIR)/tests
LIB_DIR=$(ROOT_DIR)/lib
BUILD_DIR=$(ROOT_DIR)/build

# Google Test and Google Mock
GTEST_DIR=googletest
GMOCK_DIR=googlemock
USER_DIR=$(TEST_DIR)

# Google Test libraries
GTEST_LIBS = $(LIB_DIR)/libgtest.a $(LIB_DIR)/libgtest_main.a \
			 $(LIB_DIR)/libgmock.a $(LIB_DIR)/libgmock_main.a

# All Google Test headers.
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

# All Google Mock headers.
GMOCK_HEADERS = $(GMOCK_DIR)/include/gmock/*.h \
                $(GMOCK_DIR)/include/gmock/internal/*.h \
				$(GTEST_HEADERS)

# Google Test and Mock sources
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)
GMOCK_SRCS_ = $(GMOCK_DIR)/src/*.cc $(GMOCK_HEADERS)

# Tests
#TESTS=$(shell find $(TEST_DIR) -name '*.cc')

# Flags passed to the preprocessor
CPPFLAGS=-isystem $(GTEST_DIR)/include -isystem $(GMOCK_DIR)/include

DEBUG_FLAGS=-Wall -Wextra -ggdb3 -fstack-protector-all
CXXFLAGS=$(CPPFLAGS) $(DEBUG_FLAGS) -std=c++11

all: $(GTEST_LIBS) #$(TESTS)

$(BUILD_DIR)/gtest-all.o: $(GTEST_SRCS_) | $(BUILD_DIR)
	@$(ECHO) Compiling $(*F)
	@$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) $(CXXFLAGS) \
            -c $(GTEST_DIR)/src/gtest-all.cc -o $(BUILD_DIR)/$@

$(BUILD_DIR)/gtest_main.o: $(GTEST_SRCS_) | $(BUILD_DIR)
	@$(ECHO) Compiling $(*F)
	@$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) $(CXXFLAGS) \
            -c $(GTEST_DIR)/src/gtest_main.cc -o $(BUILD_DIR)/$@

$(BUILD_DIR)/gmock-all.o: $(GMOCK_SRCS_) | $(BUILD_DIR)
	@$(ECHO) Compiling $(*F)
	@$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) $(CXXFLAGS) \
            -c $(GMOCK_DIR)/src/gmock-all.cc -o $(BUILD_DIR)/$@

$(BUILD_DIR)/gmock_main.o: $(GMOCK_SRCS_) | $(BUILD_DIR)
	@$(ECHO) Compiling $(*F)
	@$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) $(CXXFLAGS) \
            -c $(GMOCK_DIR)/src/gmock_main.cc -o $(BUILD_DIR)/$@

$(LIB_DIR)/libgtest.a: $(BUILD_DIR)/gtest-all.o | $(LIB_DIR)
	@$(AR) -r $@ $^ &> /dev/null

$(LIB_DIR)/libgtest_main.a: $(BUILD_DIR)/gtest_main.o | $(LIB_DIR)
	@$(AR) -r $@ $^ &> /dev/null

$(LIB_DIR)/libgmock.a: $(BUILD_DIR)/gmock-all.o | $(LIB_DIR)
	@$(AR) -r $@ $^ &> /dev/null

$(LIB_DIR)/libgmock_main.a: $(BUILD_DIR)/gmock_main.o | $(LIB_DIR)
	@$(AR) -r $@ $^ &> /dev/null

# Directory creation
$(LIB_DIR):
	@$(ECHO) Creating external libraries... 
	@$(MKDIR) $(LIB_DIR) 

$(BUILD_DIR):
	@$(ECHO) Compiling external sources...
	@$(MKDIR) $(BUILD_DIR) 
