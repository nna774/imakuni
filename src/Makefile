TARGET := imakuni
RM := rm -f
CP := cp -f
LIB_DIR := ../lib
SRCS := main.cpp png.cpp deflate.cpp
OBJS := $(SRCS:%.cpp=%.o)
DEPS := $(SRCS:%.cpp=%.d)
CFLAGS := -Wall -Wextra
-include $(DEPS)

all: build

build: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CFLAGS) -c -MMD -MP $<

deflate.o:
	$(CXX) -I$(LIB_DIR) -c -MMD -MP deflate.cpp

clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

.PHONY: clean
