cmake_minimum_required(VERSION 3.17)
project(robots-client)

set(CMAKE_CXX_STANDARD 20)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(BOOST_ROOT "opt/local/include/boost")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath -Wl,/opt/gcc-11.2/lib64")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wconversion")

find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${Boost_INCLUDE_DIRS})

add_executable(robots-client main.cpp)
target_link_libraries(robots-client LINK_PUBLIC Threads::Threads)
target_link_libraries(robots-client LINK_PUBLIC ${Boost_LIBRARIES} )
