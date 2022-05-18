cmake_minimum_required(VERSION 3.17)
project(BombermanServer)

set(CMAKE_CXX_STANDARD 20)

set(BOOST_ROOT "opt/local/include/boost")

find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(BombermanServer main.cpp)
TARGET_LINK_LIBRARIES(BombermanServer LINK_PUBLIC ${Boost_LIBRARIES} )
