TARGET := imakuni
SRCDIR := src
RM := rm -f
CP := cp -f

all: build
	$(CP) $(SRCDIR)/$(TARGET) .

build:
	$(MAKE) -C $(SRCDIR) build

clean: clean_src
	$(RM) $(TARGET)

clean_src:
	$(MAKE) -C $(SRCDIR) clean

.PHONY: clean clean_src
