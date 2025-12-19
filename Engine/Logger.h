#pragma once
#include <string>
#include <fstream>
#include "DataStructrue.h"

class Logger {
public:
    Logger();
    ~Logger();

    // 每帧调用，将数据写入缓冲区或文件
    void log(double time, const EngineData& data);

private:
    std::ofstream outFile;
    std::string filename;
};