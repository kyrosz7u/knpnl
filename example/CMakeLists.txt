#SET(EXECUTABLE_OUTPUT_PATH ./bin)

add_executable(thread_test thread_test.cpp)
target_link_libraries(thread_test base)

add_executable(timestamp_test timestamp_test.cpp)
target_link_libraries(timestamp_test base)

add_executable(logger_test logger_test.cpp)
target_link_libraries(logger_test base)

add_executable(threadpool_test threadpool_test.cpp)
target_link_libraries(threadpool_test base)

add_executable(echoserver_test echoserver_test.cpp)
target_link_libraries(echoserver_test base net)

add_executable(eventloop_test eventloop_test.cpp)
target_link_libraries(eventloop_test base event)

add_executable(timerqueue_test timerqueue_test.cpp)
target_link_libraries(timerqueue_test base event)

add_executable(loopthread_test loopthread_test.cpp)
target_link_libraries(loopthread_test base event)

add_executable(LoopThreadPool_test LoopThreadPool_test.cpp)
target_link_libraries(LoopThreadPool_test base event)

add_executable(tcpserver_test tcpserver_test.cpp)
target_link_libraries(tcpserver_test net)
