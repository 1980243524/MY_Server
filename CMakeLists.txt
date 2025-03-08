cmake_minimum_required(VERSION 3.20)
project(MyWebServer)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 20)
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

add_library(Server-lib
    source/dbPool.cpp
    source/timer.cpp
    source/chatserver.cpp
    source/threadPool.cpp
    source/util.cpp
)

# 查找 MySQL 客户端库
find_library(MYSQLCLIENT_LIB mysqlclient)
if(NOT MYSQLCLIENT_LIB)
    message(FATAL_ERROR "MySQL client library not found!")
endif()

find_package(Threads REQUIRED)

# add_executable(main main.cpp)

# target_include_directories(main PRIVATE head)

# target_link_libraries(  main PRIVATE
#                         Server-lib
#                         ${MYSQLCLIENT_LIB}
#                         Threads::Threads
#                         )

# add_executable(test test/test_thread.cpp)
# target_include_directories(test PRIVATE head)



# target_link_libraries(  test PRIVATE
#                         Server-lib
#                         ${MYSQLCLIENT_LIB}
#                         Threads::Threads
#                         )

# add_executable(timer test/test_timer.cpp)
# target_include_directories(timer PRIVATE head)

# target_link_libraries(  timer PRIVATE
#                         Server-lib
#                         ${MYSQLCLIENT_LIB}
#                         Threads::Threads
#                         )

add_executable(server test/test_server.cpp)
target_include_directories(server PRIVATE head)

target_link_libraries(  server PRIVATE
                        Server-lib
                        ${MYSQLCLIENT_LIB}
                        Threads::Threads
                        )

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
