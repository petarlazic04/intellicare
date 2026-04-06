# IntelliCare Test Makefile

CXX = g++
CXXFLAGS = -std=c++17 -I. -Ithird_party
LDFLAGS = -lmosquitto

# Source files
HUB_SRC = main_hub.cpp
SIMULATOR_SRC = main_house.cpp

# Output executables
HUB_EXE = test_hub
SIMULATOR_EXE = test_house

# Default target - build both tests
all: $(HUB_EXE) $(SIMULATOR_EXE)

# Build hub test
$(HUB_EXE): $(HUB_SRC)
	$(CXX) $(CXXFLAGS) $(HUB_SRC) $(LDFLAGS) -o $(HUB_EXE)

# Build simulator/actuators test
$(SIMULATOR_EXE): $(SIMULATOR_SRC)
	$(CXX) $(CXXFLAGS) $(SIMULATOR_SRC) $(LDFLAGS) -o $(SIMULATOR_EXE)

# Clean build artifacts
clean:
	rm -f $(HUB_EXE) $(SIMULATOR_EXE)

# Rebuild everything
rebuild: clean all

# Show help
help:
	@echo "IntelliCare Test Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all       - Build both test executables (default)"
	@echo "  hub       - Build only the hub test"
	@echo "  simulator - Build only the simulatortest"
	@echo "  clean     - Remove built executables"
	@echo "  rebuild   - Clean and rebuild everything"
	@echo "  help      - Show this help message"
	@echo ""
	@echo "Usage:"
	@echo "  make              # Build both tests"
	@echo "  make hub          # Build only hub test"
	@echo "  make simulator    # Build only simulator test"
	@echo "  make clean        # Clean built files"
	@echo "  make rebuild      # Full rebuild"

# Alternative target names
hub: $(HUB_EXE)
simulator: $(SIMULATOR_EXE)

.PHONY: all clean rebuild help hub simulator