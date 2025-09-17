# === COMPILE PROJECT ===
CXX := g++
CXXFLAGS := -std=c++20 -O2
INCLUDES := -Iinclude -Iinclude/BEngine -Iinclude/BEngine/imgui
LDFLAGS := -Llib -lengine -lglfw3dll -lopengl32
SRCS := src/main.cpp include/BEngine/engine.cpp
OUT := engine.exe

test: project run

all: support-lib project run

project:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRCS) -o $(OUT) $(LDFLAGS)

run:
	./$(OUT)

# === SUPPORT LIBRARY ===

ENGINE_SRCS := $(wildcard src/BEngine/*.cpp)
ENGINE_OBJS := $(subst /,\,$(ENGINE_SRCS:.cpp=.o))
ENGINE_LIB := lib\libengine.a

support-lib: $(ENGINE_LIB)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(ENGINE_LIB): $(ENGINE_OBJS)
	ar rcs $@ $^
	for %%f in ($(ENGINE_OBJS)) do del %%f
