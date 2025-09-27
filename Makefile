# === COMPILE PROJECT ===
CXX := g++
CXXFLAGS := -std=c++20 -O2
INCLUDES := -Iinclude -Iinclude/BEngine -Iinclude/BEngine/imgui -Iinclude/BEngine/tfd
LDFLAGS := -Llib -lengine -lopengl32 -lglfw3 -lgdi32 -lole32 -luuid -loleaut32 -lcomdlg32
SRCS := src/main.cpp include/BEngine/engine.cpp include/BEngine/engine_editor.cpp
OUT := engine.exe

test: project run

all: support-lib project run

project:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRCS) -o $(OUT) $(LDFLAGS)

run:
	./$(OUT)

# === SUPPORT LIBRARY ===

LIB_INCLUDES := -Iinclude -Iinclude/BEngine -Iinclude/BEngine/imgui -Iinclude/BEngine/tfd

ENGINE_SRCS := $(wildcard src/BEngine/*.cpp)
ENGINE_OBJS := $(subst /,\,$(ENGINE_SRCS:.cpp=.o))
ENGINE_LIB := lib\libengine.a

support-lib: $(ENGINE_LIB)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(LIB_INCLUDES) -c $< -o $@

$(ENGINE_LIB): $(ENGINE_OBJS)
	ar rcs $@ $^

clear:
	for %%f in ($(ENGINE_OBJS)) do del %%f
