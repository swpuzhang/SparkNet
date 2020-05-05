#ifndef FILEDIR_H
#define FILEDIR_H
#ifdef WIN32
#include <io.h>
#include <direct.h> 
#else
#include <unistd.h>
#include <sys/stat.h>

#endif
#include <stdint.h>
#include <string>
#include <dirent.h>

constexpr auto MAX_PATH_LEN = 256;

#ifdef WIN32
#define ACCESS(fileName,accessMode) _access(fileName,accessMode)
#define MKDIR(path) _mkdir(path)
#else
#define ACCESS(fileName,accessMode) access(fileName,accessMode)
#define MKDIR(path) mkdir(path,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#endif

std::string GetPwd()
{
    char buf[MAX_PATH_LEN];
    getcwd(buf, MAX_PATH_LEN);
    return buf;
}

// 从左到右依次判断文件夹是否存在,不存在就创建
// 注意:最后一个如果是文件夹的话,需要加上 '\' 或者 '/'
int32_t CreateDirectory(const std::string& directoryPath)
{
   
    auto dirPathLen = directoryPath.length();
    if (dirPathLen > MAX_PATH_LEN)
    {
        return -1;
    }
    char tmpDirPath[MAX_PATH_LEN] = { 0 };
    for (size_t i = 0; i < dirPathLen; ++i)
    {
        tmpDirPath[i] = directoryPath[i];
        if (tmpDirPath[i] == '\\' || tmpDirPath[i] == '/')
        {
            if (ACCESS(tmpDirPath, 0) != 0)
            {
                int32_t ret = MKDIR(tmpDirPath);
                if (ret != 0)
                {
                    return ret;
                }
            }
        }
    }
    return 0;
}
#endif // !FILEDIR_H

