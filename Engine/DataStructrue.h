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

enum class ErrorType {
    NONE = 0,
    // --- 传感器故障 ---
    SENSOR_N_ONE, SENSOR_N_TWO,
    SENSOR_EGT_ONE, SENSOR_EGT_TWO,
    SENSOR_ALL,
    SENSOR_FUEL,
    // --- 超限故障 ---
    OVERSPEED_N1_1, OVERSPEED_N1_2,
    OVERHEAT_EGT_1, OVERHEAT_EGT_2,
    OVERHEAT_EGT_3, OVERHEAT_EGT_4,
    //FUEL
    LOW_FUEL,OVERSPEED_FUEL,
};

struct EngineData {
    double rpm_1;
    double rpm_2;
    double EGT1_temp;
    double EGT2_temp;// 排气温度
    double Fuel_C;     // 燃油余量
    double Fuel_V;     // 燃油流速

    // 传感器状态标记 (用于模拟故障)
    bool is_N_sensor_valid[4];   // 两个转速传感器是否正常
    bool is_EGT_sensor_valid[4]; // 两个EGT传感器是否正常
    bool is_Fuel_valid;

    // 当前计算出的警告信息
    std::vector<std::string> active_alerts;
};