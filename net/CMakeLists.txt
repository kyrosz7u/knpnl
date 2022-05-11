add_library(net Socket.cpp TcpConnection.cpp Acceptor.cpp TcpServer.cpp)
add_library(http HttpServer.cpp HttpRequest.cpp HttpResponse.cpp HttpContext.cpp)
target_link_libraries(net event base)
target_link_libraries(http net)

#target_include_directories(net INTERFACE Connection.h)

