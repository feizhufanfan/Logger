# 1. 编译Windows
生成工程
```shell
cd Logger
cmake -G "Visual Studio 16 2019" -A x64 -B build -DCMAKE_BUILD_TYPE=Release .
```
打开2019兼容性开发工具
x64 Native Tools Command Prompt for VS 2019


```shell
#切换到项目目录
cd /d 项目目录
msbuild build/ALL_BUILD.vcxproj -p:Configuration=Release #编译示例程序
```


## 使用示例

```C++
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

```