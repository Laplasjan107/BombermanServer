cmake_minimum_required(VERSION 3.17)
project(BombermanServer)

set(CMAKE_CXX_STANDARD 20)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(BOOST_ROOT "opt/local/include/boost")

find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(BombermanServer main.cpp messages/server-client/ClientMessageType.h messages/EventType.h Direction.h messages/IMessage.h client.h messages/server-client/JoinMessage.h messages/server-client/PlaceBombMessage.h messages/server-client/PlaceBlockMessage.h messages/server-client/MoveMessage.h messages/server-client/HelloMessage.h messages/server-client/AcceptedPlayerMessage.h messages/server-client/GameStartedMessage.h messages/server-client/TurnMessage.h messages/server-client/GameEndedMessage.h)
target_link_libraries(BombermanServer LINK_PUBLIC Threads::Threads)
target_link_libraries(BombermanServer LINK_PUBLIC ${Boost_LIBRARIES} )
