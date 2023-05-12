#include <iostream>
#include "Logger.h"

#if ISSINGNEL
std::mutex Logger::mutex_;
#endif
int main() {



#if ISSINGNEL
    //单例模式 ISSINGNEL 1
    Logger &logger=Logger::instance("./");
#else
    //一般模式 ISSINGNEL 0
    Logger logger("./");
#endif
    logger.PrintLog("%s:%d 日志初始化完成",FILENAME,__LINE__);


    std::getchar();
    return 0;

}
