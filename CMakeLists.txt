cmake_minimum_required(VERSION 2.8)
project(sylar)

set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")


set(LIB_SRC
    sylar/log.cc
    sylar/util.cc
    sylar/config.cc
    sylar/thread.cc
    sylar/fiber.cc
    sylar/mutex.cc
    sylar/scheduler.cc
    sylar/iomanager.cc
    sylar/timer.cc
    sylar/hook.cc
    sylar/fd_manager.cc
    sylar/address.cc
    sylar/socket.cc
    sylar/bytearray.cc
    sylar/http/http.cc
    sylar/http/http11_parser.rl.cc
    sylar/http/httpclient_parser.rl.cc
    sylar/http/http_parser.cc
    )

add_library(sylar SHARED ${LIB_SRC})    # 生成.so文件

set(LIB_LIB 
    sylar
    pthread
    yaml-cpp
    dl    
)

# 测试日志
add_executable(test tests/test.cc)
add_dependencies(test sylar)
target_link_libraries(test ${LIB_LIB})

# 测试config + yaml
add_executable(test_config tests/test_config.cc)
add_dependencies(test_config sylar)
target_link_libraries(test_config ${LIB_LIB})

# 测试线程模块
add_executable(test_thread tests/test_thread.cc)
add_dependencies(test_thread sylar)
target_link_libraries(test_thread ${LIB_LIB})

# 测试协程模块
add_executable(test_util tests/test_util.cc)
add_dependencies(test_util sylar)
target_link_libraries(test_util ${LIB_LIB})

# 测试协程模块
add_executable(test_fiber tests/test_fiber.cc)
add_dependencies(test_fiber sylar)
target_link_libraries(test_fiber ${LIB_LIB})

# 测试协程调度器
add_executable(test_scheduler tests/test_scheduler.cc)
add_dependencies(test_scheduler sylar)
target_link_libraries(test_scheduler ${LIB_LIB})

add_executable(test_iomanager tests/test_iomanager.cc)
add_dependencies(test_iomanager sylar)
target_link_libraries(test_iomanager ${LIB_LIB})


add_executable(test_hook tests/test_hook.cc)
add_dependencies(test_hook sylar)
target_link_libraries(test_hook ${LIB_LIB})

# 测试address
add_executable(test_address tests/test_address.cc)
add_dependencies(test_address sylar)
target_link_libraries(test_address ${LIB_LIB})

# 测试socke
add_executable(test_socket tests/test_socket.cc)
add_dependencies(test_socket sylar)
target_link_libraries(test_socket ${LIB_LIB})

# 测试bytearray
add_executable(test_bytearray tests/test_bytearray.cc)
add_dependencies(test_bytearray sylar)
target_link_libraries(test_bytearray ${LIB_LIB})

# 测试HTTP数据结构
add_executable(test_http tests/test_http.cc)
add_dependencies(test_http sylar)
target_link_libraries(test_http ${LIB_LIB})


# 测试HTTP报文解析
add_executable(test_http_parser tests/test_http_parser.cc)
add_dependencies(test_http_parser sylar)
target_link_libraries(test_http_parser ${LIB_LIB})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)