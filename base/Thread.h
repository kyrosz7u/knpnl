//
// Created by 樱吹雪 on 2022/1/11.
//

#ifndef KYROSWEBSERVER_TREAD_H
#define KYROSWEBSERVER_TREAD_H

#include "Copyable.h"
#include "Types.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <string>

using namespace std;

namespace base{

namespace CurrentThread{
    extern __thread pid_t Tid;
    extern __thread char TidStr[16];
    inline pid_t tid(){
        //__builtin_expect告诉编译器 Tid==0很有可能为假，提高分支预测效率
        if(__builtin_expect(Tid==0,0)){
            Tid=(pid_t)syscall(SYS_gettid);
            sprintf(TidStr,"%d",Tid);
        }
        return Tid;
    }
    inline char const* getStr(){
        if(__builtin_expect(Tid==0,0)){
            tid();
        }
        return TidStr;
    }
};

struct ThreadData{
    string mTidStr="";
    string mName="";
    TaskFunc mTask;
};

class Thread: Noncopyable{
public:
    Thread(TaskFunc t);
    Thread(TaskFunc t, const string &name);
    ~Thread();
    void start();
    void join();
    pid_t tid();
private:
    ThreadData runningData;
    pthread_t mPthreadId=0;
    bool isRunning=false;
    bool isJoin=false;
};

};

#endif
