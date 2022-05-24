cmake_minimum_required(VERSION 3.17)
project(BombermanServer)

set(CMAKE_CXX_STANDARD 20)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(BOOST_ROOT "opt/local/include/boost")

find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${Boost_INCLUDE_DIRS})

add_executable(BombermanClient main.cpp messages/server-client/ClientMessageType.h Direction.h messages/IMessage.h client.h messages/server-client/JoinMessage.h messages/server-client/PlaceBombMessage.h messages/server-client/PlaceBlockMessage.h messages/server-client/MoveMessage.h messages/server-client/HelloMessage.h messages/server-client/AcceptedPlayerMessage.h messages/server-client/GameStartedMessage.h messages/server-client/TurnMessage.h messages/server-client/GameEndedMessage.h messages/server-client/ServerMessageType.h types.h common.h messages/messages.h messages/common_includes.h Player.h messages/client-gui/DrawMessageType.h messages/client-gui/InputMessageType.h messages/client-gui/LobbyMessage.h MapSettings.h messages/client-gui/UDPMessage.h event/events.h event/EventType.h event/BombPlaced.h event/BombExploded.h event/PlayerMoved.h event/BlockPlaced.h Position.h event/Event.h event/event_factory.h GameStatus.h Bomb.h UnexpectedMessageException.h ClientOptions.h)
target_link_libraries(BombermanClient LINK_PUBLIC Threads::Threads)
target_link_libraries(BombermanClient LINK_PUBLIC ${Boost_LIBRARIES} )
