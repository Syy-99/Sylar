cmake_minimum_required(VERSION 2.8)
project(sylar)

set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")


set(LIB_SRC
    sylar/log.cc
    sylar/util.cc
    sylar/config.cc
    )

add_library(sylar SHARED ${LIB_SRC})    # 生成.so文件

# 测试日志
add_executable(test tests/test.cc)
add_dependencies(test sylar)
target_link_libraries(test sylar)

# 测试config
add_executable(test_config tests/test_config.cc)
add_dependencies(test_config sylar)
target_link_libraries(test_config sylar)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)