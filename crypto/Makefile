CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall
INCLUDES = -I. -Icore -Iinclude -Iinclude/cipher

SRCS = main.cpp \
       core/mgr.cpp \
       core/ui.cpp \
       core/errors.cpp \
       ciphers/chacha20.cpp \
       ciphers/rabbit.cpp \
       ciphers/caesar.cpp \
       ciphers/shamir.cpp \

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
	./$(TARGET) || true

.PHONY: all clean run
