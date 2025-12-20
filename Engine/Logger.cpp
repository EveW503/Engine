#include "Logger.h"
#include <ctime>
#include <iomanip>
#include <iostream> 

Logger::Logger()
{
    // 生成带时间戳的文件名
    time_t now = time(0);
    struct tm tstruct;
    localtime_s(&tstruct, &now);
    char buf[80];
    strftime(buf, sizeof(buf), "log_%Y%m%d_%H%M%S.csv", &tstruct);
    filename = buf;

    out_file.open(filename);
    if (out_file.is_open())
    {
        // 写入CSV表头
        out_file << "Time(s),N1(RPM),N2(RPM),EGT1(C),EGT2(C),Fuel_Flow,Fuel_Quantity\n";
    }
}

Logger::~Logger()
{
    if (out_file.is_open())
    {
        out_file.close();
    }
}

void Logger::log(double time, const EngineData &data)
{
    if (out_file.is_open())
    {
        // 格式化输出数据，保留三位小数
        out_file << std::fixed << std::setprecision(3) << time << "," << data.rpm_1 << "," << data.rpm_2 << ","
                 << data.egt1_temp << "," << data.egt2_temp << "," << data.fuel_v << "," << data.fuel_c << "\n";
    }
}

void Logger::logAlert(double time, const std::string &alert_msg)
{
    if (!out_file.is_open() || alert_msg.empty())
        return;

    // 5秒内的重复报警不记录
    if (last_alert_times.find(alert_msg) != last_alert_times.end())
    {
        double last_time = last_alert_times[alert_msg];
        if (time - last_time < 5.0)
        {
            return;
        }
    }

    last_alert_times[alert_msg] = time;

    // 写入报警日志
    out_file << "ALERT," << std::fixed << std::setprecision(1) << time << ",MESSAGE:," << alert_msg << "\n";
}