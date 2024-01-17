TARGET = screencapture
SECMEM := 0
OUT_DIR ?= .

SRC = v4l2_vdin.cpp ScreenCapture.cpp ScreenCaptureTest.cpp

# rules
all: $(TARGET)

CXXFLAGS+=-std=c++14 -Wall -Wextra -fPIC -Os
LDFLAGS += -L$(STAGING_DIR)/usr/lib -Wall -Wextra -fPIC -Os -lstdc++ -lm -lpthread

$(TARGET): $(OBJ)
	$(CC) $(TARGET_CFLAGS) $(CXXFLAGS) -D_FILE_OFFSET_BITS=64 $(SRC) $(LDFLAGS) -o $(OUT_DIR)/$@

.PHONY: clean

clean:
	rm -f *.o $(OUT_DIR)/$(TARGET)

install:
	cp $(OUT_DIR)/$(TARGET) $(TARGET_DIR)/usr/bin

