# === Configuration ===
# === Compiler ===
CXX := g++
CXXFLAGS := -std=c++20 -g -lm 

# === Paths ===
INCLUDES := -Iinclude -I.
LDFLAGS := -Llib -lglfw3dll

# === Files ===
SRCS := src/main.cpp
ENGINE_SCRS := $(wildcard include/*.cpp) $(wildcard include/glad/*.c)

ALL_SRCS:= $(SRCS) $(ENGINE_SCRS)

OUT := engine.exe

# === Targets ===

compile: test run

test:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(ALL_SRCS) -o $(OUT) $(LDFLAGS)

run:
	./$(OUT)