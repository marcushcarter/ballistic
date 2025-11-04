# =========================================================
# Super crude one-shot Makefile
# =========================================================
CXX = g++ -g -O0
CXXFLAGS = -std=c++20 -Iinclude \
           -Iexternal
LDFLAGS = 
SRC := src/main.cpp
TARGET := bin/release/main.exe

all: $(TARGET)

$(TARGET):
	@if not exist "bin\release" mkdir bin\release
#	@copy "external\fmod\lib\-.dll" "bin\release\"
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	@if exist bin rmdir /S /Q bin
