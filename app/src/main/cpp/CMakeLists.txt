cmake_minimum_required(VERSION 3.22.1)
project("native_hook")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fvisibility=hidden")

# 下载 Dobby Hook 框架
include(FetchContent)
FetchContent_Declare(
    Dobby
    GIT_REPOSITORY https://github.com/jmpews/Dobby.git
    GIT_TAG master
)
FetchContent_MakeAvailable(Dobby)

# 创建共享库
add_library(
    native_hook
    SHARED
    native_hook.cpp
)

# 包含 Dobby 头文件
target_include_directories(native_hook PRIVATE ${Dobby_SOURCE_DIR}/include)

# 链接库
target_link_libraries(
    native_hook
    dobby
)
