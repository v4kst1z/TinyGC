# CMakeList.txt: TinyGC 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

project ("heapmap")

IF (NOT WIN32)
    SET(CMAKE_CXX_FLAGS -pthread)
ENDIF()

add_executable (heapmap "../TinyGC.cpp" "../TinyGC.h" "heapmap.cpp" "../Common.h" "../GCObject.h" "../Visitor.h" "../Container.h" "../Work.h"  "../Tgc.h")
add_executable (linknode "../TinyGC.cpp" "../TinyGC.h" "linknode.cpp" "../Common.h" "../GCObject.h" "../Visitor.h" "../Container.h" "../Work.h"  "../Tgc.h")


