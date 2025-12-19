#include "Logger.h"
#include <ctime>
#include <iostream>
#include <iomanip> // 用于格式化输出

Logger::Logger() {
    // 生成带时间戳的文件名，例如 log_20231027_123000.csv
    time_t now = time(0);
    struct tm tstruct;
    localtime_s(&tstruct, &now);
    char buf[80];
    strftime(buf, sizeof(buf), "log_%Y%m%d_%H%M%S.csv", &tstruct);
    filename = buf;

    outFile.open(filename);
    if (outFile.is_open()) {
        // 写入表头
        outFile << "Time(s),N1(RPM),N2(RPM),EGT1(C),EGT2(C),Fuel_Flow,Fuel_Quantity\n";
    }
}

Logger::~Logger() {
    if (outFile.is_open()) {
        outFile.close();
    }
}

void Logger::log(double time, const EngineData& data) {
    if (outFile.is_open()) {
        // 使用 CSV 格式写入，逗号分隔
        outFile << std::fixed << std::setprecision(3)
            << time << ","
            << data.N1_rpm << ","
            << data.N2_rpm << ","
            << data.EGT1_temp << ","
            << data.EGT2_temp << ","
            << data.Fuel_V << ","
            << data.Fuel_C << "\n";
    }
}