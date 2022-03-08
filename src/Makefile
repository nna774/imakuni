TARGET := imakuni
RM := rm -f
CP := cp -f
LIB_DIR := ../lib
SRCS := main.cpp png.cpp pnm.cpp gif.cpp deflate.cpp lzw.cpp image.cpp jpg.cpp
OBJS := $(SRCS:%.cpp=%.o)
DEPS := $(SRCS:%.cpp=%.d)
CFLAGS := -std=c++17 -g3
CFLAGSWITHWARN := -Wall -Wextra $(CFLAGS)
-include $(DEPS)

all: build

build: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CFLAGSWITHWARN) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CFLAGSWITHWARN) -c -MMD -MP $<

deflate.o:
	$(CXX) $(CFLAGS) -I$(LIB_DIR) -c -MMD -MP deflate.cpp

clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

.PHONY: clean
