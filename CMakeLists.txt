cmake_minimum_required(VERSION 3.17)
project(BombermanServer)

set(CMAKE_CXX_STANDARD 20)

set(BOOST_ROOT "opt/local/include/boost")

find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(BombermanServer main.cpp messages/server-client/ClientMessageType.h messages/EventType.h Direction.h messages/IMessage.h client.h messages/server-client/Join.h messages/server-client/PlaceBomb.h messages/server-client/PlaceBlock.h messages/server-client/Move.h)
TARGET_LINK_LIBRARIES(BombermanServer LINK_PUBLIC ${Boost_LIBRARIES} )
