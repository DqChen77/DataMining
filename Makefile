CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I/opt/homebrew/Cellar/pugixml/1.15/include -fsanitize=address -g -fno-omit-frame-pointer
LDFLAGS = -lpugixml -L/opt/homebrew/Cellar/pugixml/1.15/lib -fsanitize=address

SRCS = main.cpp dblp_reader.cpp
OBJ_DIR = target
OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRCS))
TARGET = dblp_mining

.PHONY: all clean run

all: $(TARGET) run

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

run: $(TARGET)
	# ./$(TARGET)

$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -f $(OBJS) $(TARGET)
	rm -rf $(OBJ_DIR)