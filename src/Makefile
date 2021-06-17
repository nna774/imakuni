TARGET := imakuni
RM := rm -f
CP := cp -f
LIB_DIR := ../lib
SRCS := main.cpp png.cpp pnm.cpp gif.cpp deflate.cpp lzw.cpp image.cpp jpg.cpp
OBJS := $(SRCS:%.cpp=%.o)
DEPS := $(SRCS:%.cpp=%.d)
CFLAGS := -Wall -Wextra -std=c++17 -g3
-include $(DEPS)

all: build

build: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CFLAGS) -c -MMD -MP $<

deflate.o:
	$(CXX) -I$(LIB_DIR) -c -MMD -MP deflate.cpp

clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

.PHONY: clean
