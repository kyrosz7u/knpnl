add_library(net Socket.cpp TcpConnection.cpp Acceptor.cpp TcpServer.cpp)

add_subdirectory(http)

target_link_libraries(net event base)
target_link_libraries(http net)

#target_include_directories(net INTERFACE Connection.h)

