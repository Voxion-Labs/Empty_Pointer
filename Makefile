CXX ?= g++
EMXX ?= em++

APP_NAME := Empty_Pointer
BUILD_DIR := build
PUBLIC_DIR := public
RAYLIB_SRC ?= raylib/src

CPPFLAGS := -Iinclude
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -pedantic
SOURCES := src/main.cpp src/game.cpp src/entity.cpp src/math_utils.cpp

WEB_FLAGS := \
	-std=c++17 \
	-Os \
	-DPLATFORM_WEB \
	-Iinclude \
	-I$(RAYLIB_SRC) \
	-L$(RAYLIB_SRC) \
	-lraylib \
	-s USE_GLFW=3 \
	-s ASYNCIFY \
	-s ALLOW_MEMORY_GROWTH=1 \
	-s INITIAL_MEMORY=67108864 \
	-s ASSERTIONS=0 \
	--shell-file shell.html

.PHONY: all native web clean

all: native

native: $(BUILD_DIR)/$(APP_NAME)

$(BUILD_DIR)/$(APP_NAME): $(SOURCES)
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(SOURCES) -lraylib -o $@

web: $(PUBLIC_DIR)/index.html

$(PUBLIC_DIR)/index.html: $(SOURCES) shell.html
	mkdir -p $(PUBLIC_DIR)
	$(EMXX) $(SOURCES) $(WEB_FLAGS) -o $@
	mkdir -p $(PUBLIC_DIR)/assets
	cp assets/logo.svg $(PUBLIC_DIR)/assets/logo.svg
	touch $(PUBLIC_DIR)/.nojekyll

clean:
	rm -rf $(BUILD_DIR) $(PUBLIC_DIR)
