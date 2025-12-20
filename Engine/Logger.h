#pragma once
#include <string>
#include <fstream>
#include <map>
#include "DataStructrue.h"

class Logger {
public:
    Logger();
    ~Logger();

    // 记录每帧的数值数据
    void log(double time, const EngineData& data);

    // 记录报警事件
    void logAlert(double time, const std::string& alert_msg);

private:
    std::ofstream out_file;
    std::string filename;

    // 用于记录报警信息的去重时间戳
    std::map<std::string, double> last_alert_times;
};