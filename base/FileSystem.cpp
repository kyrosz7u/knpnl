//
// Created by 樱吹雪 on 2022/4/26.
//

#include "FileSystem.h"
#include <memory>

using namespace base;

map<FILE_STATE,StringPiece> file_state_str={
        {IS_FILE, "IS_FILE"},
        {IS_DIR, "IS_DIR"},
        {NO_FILE, "NO_FILE"},
        {CANNOT_ACCESS, "CANNOT_ACCESS"}
};

FILE_STATE FileSystem::Open(const StringPiece &path){
    mPath=path;
    if(stat(mPath.data(),&mFileStat)){
        return NO_FILE;
    }
    if(!(mFileStat.st_mode&S_IROTH)){
        return CANNOT_ACCESS;
    }
    if(S_ISDIR(mFileStat.st_mode)){
        return IS_DIR;
    }
    mFileLength = mFileStat.st_size;

    int fd = open(mPath.data(), O_RDONLY);
    mFile = shared_ptr<FileAddr>(new FileAddr(fd, mFileLength));
    close(fd);
    return IS_FILE;
}

