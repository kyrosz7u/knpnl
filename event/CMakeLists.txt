add_library(event EventLoop.cpp LoopThread.cpp LoopThreadPool.cpp Channel.cpp Poller.cpp Timer.cpp TimerQueue.cpp)

target_link_libraries(event base)