cmake_minimum_required(VERSION 3.20)
project(BallisticProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add third-party libraries
add_subdirectory(Thirdparty)

# Add your projects
add_subdirectory(Engine)
add_subdirectory(EngineRuntime)
add_subdirectory(EngineEditor)
