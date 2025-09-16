# === Configuration ===
# === Compiler ===
CXX := g++
CC := gcc
CXXFLAGS := -std=c++20 -O2
CFLAGS := -O2

# === Paths ===
INCLUDES := -Iinclude -I. -Iinclude/BEngine -Iinclude/BEngine/imgui
LDFLAGS := -Llib -lglfw3dll

# === Files ===
SRCS := src/main.cpp
ENGINE_SCRS := $(wildcard include/BEngine/*.cpp) $(wildcard include/BEngine/glad/*.cpp) $(wildcard include/BEngine/stb/*.cpp)


ALL_SRCS:= $(SRCS) $(ENGINE_SCRS)

OUT := engine.exe

# === Targets ===

compile: test run

test:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(ALL_SRCS) -o $(OUT) $(LDFLAGS)

run:
	./$(OUT)







# === SUPPORT LIBRARY ===

SUPPORT_CPP_SRCS := \
	include/BEngine/stb/stb.cpp \
	include/BEngine/imgui/imgui.cpp \
	include/BEngine/imgui/imgui_draw.cpp \
	include/BEngine/imgui/imgui_tables.cpp \
	include/BEngine/imgui/imgui_widgets.cpp \
	include/BEngine/imgui/imgui_impl_opengl3.cpp

SUPPORT_C_SRCS := include/BEngine/glad/glad.c

SUPPORT_CPP_OBJS := $(SUPPORT_CPP_SRCS:.cpp=.o)
SUPPORT_C_OBJS := $(SUPPORT_C_SRCS:.c=.o)
SUPPORT_OBJS := $(SUPPORT_CPP_OBJS) $(SUPPORT_C_OBJS)

SUPPORT_LIB := libengine_support.a

support-lib: $(SUPPORT_LIB)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
	
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(SUPPORT_LIB): $(SUPPORT_OBJS)
	ar rcs $@ $^
	rm -f $(SUPPORT_OBJS)

clean-support:
	sh -c "rm -f $(SUPPORT_OBJS)"
