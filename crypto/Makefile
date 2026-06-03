CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall
INCLUDES = -I. -Iincludes -Icore
SRCS = main.cpp core/mgr.cpp core/ui.cpp ciphers/rabbit.cpp ciphers/chacha20.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = encrypt_fw

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
