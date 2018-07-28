TARGET := imakuni
RM := rm -f
CP := cp -f
LIB_DIR := ../lib
SRCS := main.cpp png.cpp
OBJS := $(SRCS:%.cpp=%.o)
DEPS := $(SRCS:%.cpp=%.d)
CFLAGS := -Wall -Wextra -I$(LIB_DIR)
-include $(DEPS)

all: build

build: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CFLAGS) -c -MMD -MP $<

clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

.PHONY: clean
