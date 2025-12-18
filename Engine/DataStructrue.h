#pragma once
#include <vector>
#include <string>

enum class EngineState{
    OFF,        // 关机
    STARTING,   // 启动阶段（线性 -> 对数增长）
    RUNNING,    // 稳态（随机波动）
    STOPPING,   // 停车阶段（对数下降）
    SHUTDOWN    // 故障停机
};

enum class AlertLevel {
    NORMAL,     // 白色
    CAUTION,    // 琥珀色
    WARNING,    // 红色
    INVALID     // 无效值 (--)
};

struct EngineData {
    double N1_rpm; 
    double N2_rpm;// 实际转速
    double EGT1_temp;
    double EGT2_temp;// 排气温度
    double Fuel_C;     // 燃油余量
    double Fuel_V;     // 燃油流速

    // 传感器状态标记 (用于模拟故障)
    bool is_N_sensor_valid[2];   // 两个转速传感器是否正常
    bool is_EGT_sensor_valid[2]; // 两个EGT传感器是否正常
    bool is_Fuel_valid;

    // 当前计算出的警告信息
    std::vector<std::string> active_alerts;
};