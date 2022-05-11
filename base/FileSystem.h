//
// Created by 樱吹雪 on 2022/4/26.
//

#ifndef KHPNL_FILESYSTEM_H
#define KHPNL_FILESYSTEM_H

#include "LogStream.h"
#include "StringPiece.h"
#include "sys/mman.h"
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <map>
#include <memory>

/*封装mmap方法*/
namespace base {

enum FILE_STATE{
    IS_FILE,
    IS_DIR,
    NO_FILE,
    CANNOT_ACCESS,
};
extern map<FILE_STATE, StringPiece> file_state_str;

class FileSystem: public Copyable {
    class FileAddr: public Noncopyable{
    public:
        FileAddr(int fd ,size_t len){
            mFileLength = len;
            mAddr = (char *) mmap(nullptr, mFileLength,
                                  PROT_READ, MAP_PRIVATE, fd, 0);
        }
        char *Addr(){ return mAddr; }
        ~FileAddr(){
            if(mAddr){
                munmap(mAddr, mFileLength);
                mAddr = nullptr;
            }
        }
    private:
        char *mAddr;
        size_t mFileLength;
    };

public:
    typedef std::shared_ptr<FileAddr> FilePtr;

    FileSystem(){ mFileLength = 0; }

    FILE_STATE Open(const StringPiece &path);
    char* getAddr(){
        if (mFile != nullptr) {
            return mFile->Addr();
        }else{
            return nullptr;
        }
    }
    size_t getLength(){ return mFileLength; }

private:
    StringPiece mPath;
    size_t mFileLength;
    struct stat mFileStat;
    FilePtr mFile;
};

// 重载输出FILE_STATE类型变量的方法
inline LogStream& operator<<(LogStream& logger, FILE_STATE state){
    logger << file_state_str[state];
    return logger;
}

}

#endif //KHPNL_FILESYSTEM_H
