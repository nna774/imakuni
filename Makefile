TARGET := ./imakuni
SRCDIR := src
RM := rm -f
CP := cp -f
DIFF := imagediff # https://gist.github.com/nna774/9b06603599bd26986126af6fa50436f4
TESTS_IMAGE_DIR := tests/img
TESTS := lenna 1012
TEMPDIR := tmp

all: $(TARGET)

$(TARGET): build
	$(CP) $(SRCDIR)/$(TARGET) .

build:
	$(MAKE) -C $(SRCDIR) build

clean: clean_src clean_tmp
	$(RM) $(TARGET)

clean_src:
	$(MAKE) -C $(SRCDIR) clean

clean_tmp:
	$(RM) -r $(TEMPDIR)

$(TEMPDIR):
	mkdir -p $(TEMPDIR)

test: $(TARGET) $(TEMPDIR)
	for f in $(TESTS); do \
	  $(TARGET) $(TESTS_IMAGE_DIR)/$$f.png $(TEMPDIR)/$$f.png; \
	  $(DIFF) $(TESTS_IMAGE_DIR)/$$f.png $(TEMPDIR)/$$f.png; \
	  $(TARGET) $(TESTS_IMAGE_DIR)/$$f.png $(TEMPDIR)/$$f.pnm; \
	  $(DIFF) $(TESTS_IMAGE_DIR)/$$f.png $(TEMPDIR)/$$f.pnm; \
	done

.PHONY: clean clean_src test
