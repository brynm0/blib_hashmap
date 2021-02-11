cmake_minimum_required(VERSION 3.0.0)
project(hashmap VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(hashmap hashmap.cpp)

target_compile_options(hashmap PRIVATE "/W0" "/wd4201" "/DH_DEBUG=0")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
