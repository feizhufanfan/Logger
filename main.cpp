#include <iostream>
#include "Logger.h"

#if ISSINGNEL
std::mutex Logger::mutex_;
#endif
int main() {



#if ISSINGNEL
    //����ģʽ ISSINGNEL 1
    Logger &logger=Logger::instance("./");
#else
    //һ��ģʽ ISSINGNEL 0
    Logger logger("./");
#endif
    logger.PrintLog("%s:%d ��־��ʼ�����",FILENAME,__LINE__);


    std::getchar();
    return 0;

}
