cmake_minimum_required(VERSION 3.22.1)
project("native_hook")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fvisibility=hidden")

include(FetchContent)
FetchConte