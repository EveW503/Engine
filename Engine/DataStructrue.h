#pragma once
#include <vector>
#include <string>

enum class EngineState {
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
    SENSOR_N_ONE, //有一个转速传感器发生故障
    SENSOR_N_TWO, //同一个发动机的两个转速传感器均发生故障
    SENSOR_EGT_ONE, //有一个温度传感器发生故障
    SENSOR_EGT_TWO, //同一个发动机的两个温度传感器均发生故障
    SENSOR_ALL, //两个发动机的全部转速传感器或全部温度传感器发生故障
    SENSOR_FUEL,//燃油余量传感器发生故障
    // --- 超限故障 ---
    OVERSPEED_N1_1, //有一个发动机的转速超过警戒值
    OVERSPEED_N1_2, //有一个发动机的转速超过警告值
    OVERHEAT_EGT_1, //在启动时有一个发动机排气温度超过警戒值
    OVERHEAT_EGT_2, //在启动时有一个发动机排气温度超过警告值
    OVERHEAT_EGT_3, //在稳态时有一个发动机排气温度超过警戒值
    OVERHEAT_EGT_4, //在稳态时有一个发动机排气温度超过警告值
    //FUEL
    LOW_FUEL,//燃油余量小于500
    OVERSPEED_FUEL,//燃油流速大于50
};

struct EngineData {
    double rpm_1;
    double rpm_2;
    double egt1_temp; // 修改为下划线
    double egt2_temp; // 修改为下划线
    double fuel_c;    // 修改为下划线
    double fuel_v;    // 修改为下划线

    // 传感器状态标记 (用于模拟故障)
    bool is_n_sensor_valid[4];   // 修改为下划线
    bool is_egt_sensor_valid[4]; // 修改为下划线
    bool is_fuel_valid;          // 修改为下划线

    // 当前计算出的警告信息
    std::vector<std::string> active_alerts;
};