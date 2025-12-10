CXX = g++
CXXFLAGS = -O2 -IBallistic/src -IBallisticEditor/src -IBallisticRuntime/src

LIBS = Ballistic/libBallistic.a Ballistic/Thirdparty/glfw/lib/libglfw3.a \
 		-lopengl32 -lgdi32 -luser32 -lkernel32

OUT_DIR = out

EDITOR_LIBS = Ballistic/libBallistic.a BallisticEditor/libBallisticEditor.a Ballistic/Thirdparty/glfw/lib/libglfw3.a \
 		-lopengl32 -lgdi32 -luser32 -lkernel32

EDITOR_SRCS := BallisticEditor/src/EditorMain.cpp
EDITOR_OBJS := $(EDITOR_SRCS:.cpp=.o)
EDITOR_TARGET = $(OUT_DIR)\BallisticEditor.exe

RUNTIME_LIBS = Ballistic/libBallistic.a BallisticRuntime/libBallisticRuntime.a Ballistic/Thirdparty/glfw/lib/libglfw3.a \
 		-lopengl32 -lgdi32 -luser32 -lkernel32
 		
RUNTIME_SRCS := BallisticRuntime/src/RuntimeMain.cpp
RUNTIME_OBJS := $(RUNTIME_SRCS:.cpp=.o)
RUNTIME_TARGET = $(OUT_DIR)\BallisticRuntime.exe

all: libs editor runtime

editor: libs $(EDITOR_TARGET)

$(EDITOR_TARGET): $(EDITOR_OBJS)
	@if not exist "$(OUT_DIR)" mkdir "$(OUT_DIR)"
	$(CXX) $(CXXFLAGS) -o $@ $^ $(EDITOR_LIBS)

run-editor: editor
	$(EDITOR_TARGET)

runtime: libs $(RUNTIME_TARGET)

$(RUNTIME_TARGET): $(RUNTIME_OBJS)
	@if not exist "$(OUT_DIR)" mkdir "$(OUT_DIR)"
	$(CXX) $(CXXFLAGS) -o $@ $^ $(RUNTIME_LIBS)

run-runtime: runtime
	$(RUNTIME_TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

libs:
	cd Ballistic/ && $(MAKE) -f make.mk
	cd BallisticEditor/ && $(MAKE) -f make.mk
	cd BallisticRuntime/ && $(MAKE) -f make.mk

clean:
	if exist "$(OUT_DIR)" rmdir /s /q "$(OUT_DIR)"

.PHONY: all editor runtime run-editor run-runtime libs clean