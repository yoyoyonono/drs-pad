CXX = clang++
CXXFLAGS = -Iwebsocketpp/ -Iboost_1_84_0/

SRC_DIR = .
BUILD_DIR = build

$(BUILD_DIR)/main: $(SRC_DIR)/main.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

run : $(BUILD_DIR)/main
	$(BUILD_DIR)/main