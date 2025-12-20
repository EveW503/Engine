// Logger.h
#pragma once
#include <string>
#include <fstream>
#include <map> // 新增：用于记录上次报警时间
#include "DataStructrue.h"

class Logger {
public:
    Logger();
    ~Logger();

    // 现有的：记录数值数据 (CSV)
    void log(double time, const EngineData& data);

    // 【新增】记录报警信息 (txt 或 追加到 CSV)
    void logAlert(double time, const std::string& alertMsg);

private:
    std::ofstream outFile;
    std::string filename;

    // 【新增】用于 5s 去重逻辑
    // Key: 报警内容, Value: 上次记录该报警的时间
    std::map<std::string, double> lastAlertTimes;
};