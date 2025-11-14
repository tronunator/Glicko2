# Makefile for Team-Based Glicko-2 Rating System
# Supports both Windows (MSVC, MinGW) and Linux (g++)

# Detect operating system
ifeq ($(OS),Windows_NT)
    # Windows detected
    DETECTED_OS := Windows
    # Try to detect MSVC vs MinGW
    ifeq ($(CXX),cl)
        COMPILER := MSVC
    else
        COMPILER := MinGW
        CXX := g++
    endif
else
    DETECTED_OS := $(shell uname -s)
    COMPILER := GCC
    CXX := g++
endif

# Directory structure
SRC_DIR := src
INC_DIR := include
BUILD_DIR := build
EXAMPLE_DIR := examples

# Targets (will be updated with .exe on Windows)

# Source files - library
LIB_SOURCES := $(SRC_DIR)/TeamGlickoRating.cpp \
               $(SRC_DIR)/TeamRatingAggregator.cpp \
               $(SRC_DIR)/PerformanceWeighting.cpp \
               $(SRC_DIR)/TeamGlicko2System.cpp \
               $(SRC_DIR)/TeamBalancer.cpp

# Object files - library
LIB_OBJECTS := $(BUILD_DIR)/TeamGlickoRating.o \
               $(BUILD_DIR)/TeamRatingAggregator.o \
               $(BUILD_DIR)/PerformanceWeighting.o \
               $(BUILD_DIR)/TeamGlicko2System.o \
               $(BUILD_DIR)/TeamBalancer.o

# Example programs
EXAMPLE_TARGET := $(BUILD_DIR)/example_usage
BATCH_TARGET := $(BUILD_DIR)/batch_processor
BALANCE_TARGET := $(BUILD_DIR)/team_balancing_test

# Compiler flags
ifeq ($(COMPILER),MSVC)
    # MSVC flags
    CXXFLAGS := /EHsc /W3 /std:c++14 /O2 /I$(INC_DIR)
    LDFLAGS :=
    RM := del /Q
    RMDIR := rmdir /S /Q
    MKDIR := if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
    EXAMPLE_TARGET := $(EXAMPLE_TARGET).exe
    BATCH_TARGET := $(BATCH_TARGET).exe
    BALANCE_TARGET := $(BALANCE_TARGET).exe
else
    # GCC/MinGW flags
    CXXFLAGS := -std=c++14 -Wall -Wextra -O2 -I$(INC_DIR)
    LDFLAGS := -lm
    ifeq ($(DETECTED_OS),Windows)
        RM := del /Q
        RMDIR := rmdir /S /Q
        MKDIR := if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
        EXAMPLE_TARGET := $(EXAMPLE_TARGET).exe
        BATCH_TARGET := $(BATCH_TARGET).exe
        BALANCE_TARGET := $(BALANCE_TARGET).exe
    else
        RM := rm -f
        RMDIR := rm -rf
        MKDIR := mkdir -p $(BUILD_DIR)
    endif
endif

# Default target
all: $(BUILD_DIR) $(EXAMPLE_TARGET) $(BATCH_TARGET) $(BALANCE_TARGET)

# Create build directory
$(BUILD_DIR):
	$(MKDIR)

# Build example_usage
$(EXAMPLE_TARGET): $(LIB_OBJECTS) $(BUILD_DIR)/example_usage.o
ifeq ($(COMPILER),MSVC)
	$(CXX) $(CXXFLAGS) $^ /Fe:$@ $(LDFLAGS)
else
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
endif
	@echo Build complete: $@

# Build batch_processor
$(BATCH_TARGET): $(LIB_OBJECTS) $(BUILD_DIR)/batch_processor.o
ifeq ($(COMPILER),MSVC)
	$(CXX) $(CXXFLAGS) $^ /Fe:$@ $(LDFLAGS)
else
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
endif
	@echo Build complete: $@

# Build team_balancing_test
$(BALANCE_TARGET): $(LIB_OBJECTS) $(BUILD_DIR)/team_balancing_test.o
ifeq ($(COMPILER),MSVC)
	$(CXX) $(CXXFLAGS) $^ /Fe:$@ $(LDFLAGS)
else
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
endif
	@echo Build complete: $@

# Compile source files to object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
ifeq ($(COMPILER),MSVC)
	$(CXX) $(CXXFLAGS) /c $< /Fo:$@
else
	$(CXX) $(CXXFLAGS) -c $< -o $@
endif

$(BUILD_DIR)/%.o: $(EXAMPLE_DIR)/%.cpp
ifeq ($(COMPILER),MSVC)
	$(CXX) $(CXXFLAGS) /c $< /Fo:$@
else
	$(CXX) $(CXXFLAGS) -c $< -o $@
endif

# Clean build artifacts
clean:
ifeq ($(DETECTED_OS),Windows)
	-$(RM) $(BUILD_DIR)\*.o $(BUILD_DIR)\*.obj 2>nul
	-$(RM) $(BUILD_DIR)\*.exe 2>nul
	-$(RM) *.o *.obj *.exe 2>nul
else
	$(RM) $(BUILD_DIR)/*.o $(TARGET)
	$(RM) *.o *.exe
endif
	@echo Clean complete

# Deep clean - remove build directory
distclean: clean
ifeq ($(DETECTED_OS),Windows)
	-$(RMDIR) $(BUILD_DIR) 2>nul
else
	-$(RMDIR) $(BUILD_DIR)
endif
	@echo Deep clean complete

# Run the example
run: $(TARGET)
	./$(TARGET)

# Help target
help:
	@echo Team-Based Glicko-2 Rating System - Makefile
	@echo ============================================
	@echo.
	@echo Available targets:
	@echo   all       - Build the example program (default)
	@echo   clean     - Remove build artifacts
	@echo   distclean - Remove build directory completely
	@echo   run       - Build and run the example
	@echo   help      - Show this help message
	@echo.
	@echo Directory Structure:
	@echo   include/ - Header files
	@echo   src/     - Source files
	@echo   examples/- Example programs
	@echo   build/   - Build artifacts (generated)
	@echo.
	@echo Detected OS: $(DETECTED_OS)
	@echo Compiler: $(COMPILER)

# Phony targets
.PHONY: all clean distclean run help

# Dependencies (simplified - in real project use auto-generated dependencies)
$(BUILD_DIR)/TeamGlickoRating.o: $(SRC_DIR)/TeamGlickoRating.cpp $(INC_DIR)/TeamGlickoRating.h $(INC_DIR)/TeamGlicko2Config.h
$(BUILD_DIR)/TeamRatingAggregator.o: $(SRC_DIR)/TeamRatingAggregator.cpp $(INC_DIR)/TeamRatingAggregator.h $(INC_DIR)/TeamGlickoRating.h
$(BUILD_DIR)/PerformanceWeighting.o: $(SRC_DIR)/PerformanceWeighting.cpp $(INC_DIR)/PerformanceWeighting.h $(INC_DIR)/TeamGlicko2Config.h
$(BUILD_DIR)/TeamGlicko2System.o: $(SRC_DIR)/TeamGlicko2System.cpp $(INC_DIR)/TeamGlicko2System.h $(INC_DIR)/TeamGlickoRating.h $(INC_DIR)/TeamRatingAggregator.h $(INC_DIR)/PerformanceWeighting.h
$(BUILD_DIR)/TeamBalancer.o: $(SRC_DIR)/TeamBalancer.cpp $(INC_DIR)/TeamBalancer.h $(INC_DIR)/TeamGlickoRating.h
$(BUILD_DIR)/example_usage.o: $(EXAMPLE_DIR)/example_usage.cpp $(INC_DIR)/TeamGlicko2System.h
$(BUILD_DIR)/team_balancing_test.o: $(EXAMPLE_DIR)/team_balancing_test.cpp $(INC_DIR)/TeamBalancer.h
