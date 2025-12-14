RACK_DIR ?= ../..
CXXFLAGS += -std=c++17
SOURCES += $(wildcard src/*.cpp) $(wildcard src/comp/*.cpp)
DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)
include $(RACK_DIR)/plugin.mk
