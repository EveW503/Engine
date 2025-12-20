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
            << data.rpm_1 << ","
            << data.rpm_2 << ","
            << data.EGT1_temp << ","
            << data.EGT2_temp << ","
            << data.Fuel_V << ","
            << data.Fuel_C << "\n";
    }
}

void Logger::logAlert(double time, const std::string& alertMsg) {
    if (!outFile.is_open() || alertMsg.empty()) return;

    // --- 5秒去重逻辑 ---
    // 检查该消息是否记录过，且距离上次记录是否小于 5.0秒
    if (lastAlertTimes.find(alertMsg) != lastAlertTimes.end()) {
        double lastTime = lastAlertTimes[alertMsg];
        if (time - lastTime < 5.0) {
            return; // 5秒内重复，忽略，不记录
        }
    }

    // 更新该消息的最后记录时间
    lastAlertTimes[alertMsg] = time;

    // --- 写入文件 ---
    // 这里我们选择在 CSV 中添加一行特殊的注释行，或者您可以单独开一个 log_alert.txt
    // 为了方便，这里直接写进同一个文件，用 "ALERT" 标记开头
    outFile << "ALERT," << std::fixed << std::setprecision(1) << time
        << ",MESSAGE:," << alertMsg << "\n";

    // 也可以在控制台打印一下，方便调试
    // std::cout << "[LOG] " << time << "s: " << alertMsg << std::endl;
}