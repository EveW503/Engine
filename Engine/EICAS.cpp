// 文件：EICAS.cpp
#include "EICAS.h"

EICAS::EICAS() {};
EICAS::~EICAS() {};

std::vector<ErrorType> EICAS::judge(const EngineData& data, EngineState state)
{
    std::vector<ErrorType> error_list; // 用于收集本帧所有故障

    // --- 1. 传感器故障统计 ---
    int fail_N1 = 0;
    int fail_N2 = 0;
    int fail_EGT1 = 0;
    int fail_EGT2 = 0;

    for (int i = 0; i < 4; i++) {
        if (!data.is_N_sensor_valid[i])   (i < 2) ? fail_N1++ : fail_N2++;
        if (!data.is_EGT_sensor_valid[i]) (i < 2) ? fail_EGT1++ : fail_EGT2++;
    }

    // --- 2. 红色警告 (RED WARNING) - 优先级最高 ---
    // 按顺序 push_back，自然形成优先级 (UI显示时先画出来的在最上面)

    // 2.1 双发传感器失效
    if ((fail_N1 == 2 && fail_N2 == 2) || (fail_EGT1 == 2 && fail_EGT2 == 2))
        error_list.push_back(ErrorType::SENSOR_ALL);

    // 2.2 燃油传感器失效
    if (!data.is_Fuel_valid)
        error_list.push_back(ErrorType::SENSOR_FUEL);

    // 2.3 转速严重超限
    const double LIMIT_N_RED = 48000.0;
    if (data.rpm_1 > LIMIT_N_RED || data.rpm_2 > LIMIT_N_RED)
        error_list.push_back(ErrorType::OVERSPEED_N1_2);

    // 2.4 EGT 严重超温
    double limit_egt_red = (state == EngineState::STARTING) ? 1000.0 : 1100.0;
    if (data.EGT1_temp > limit_egt_red || data.EGT2_temp > limit_egt_red) {
        error_list.push_back((state == EngineState::STARTING) ? ErrorType::OVERHEAT_EGT_2 : ErrorType::OVERHEAT_EGT_4);
    }

    // --- 3. 琥珀色警告 (AMBER CAUTION) ---

    // 3.1 单发传感器失效
    if (fail_N1 == 2 || fail_N2 == 2) error_list.push_back(ErrorType::SENSOR_N_TWO);
    if (fail_EGT1 == 2 || fail_EGT2 == 2) error_list.push_back(ErrorType::SENSOR_EGT_TWO);

    // 3.2 燃油相关
    if (data.Fuel_C < 1000.0) error_list.push_back(ErrorType::LOW_FUEL);
    if (data.Fuel_V > 50.0) error_list.push_back(ErrorType::OVERSPEED_FUEL);

    // 3.3 转速超限 (>105%)
    const double LIMIT_N_AMBER = 42000.0;
    // 如果已经报了红色超限，通常不需要再报黄色，加个判断避免重复显示
    bool has_red_overspeed = (data.rpm_1 > LIMIT_N_RED || data.rpm_2 > LIMIT_N_RED);
    if (!has_red_overspeed) {
        if (data.rpm_1 > LIMIT_N_AMBER || data.rpm_2 > LIMIT_N_AMBER)
            error_list.push_back(ErrorType::OVERSPEED_N1_1);
    }

    // 3.4 EGT 超温
    double limit_egt_amber = (state == EngineState::STARTING) ? 850.0 : 950.0;
    bool has_red_egt = (data.EGT1_temp > limit_egt_red || data.EGT2_temp > limit_egt_red);
    if (!has_red_egt) {
        if (data.EGT1_temp > limit_egt_amber || data.EGT2_temp > limit_egt_amber) {
            error_list.push_back((state == EngineState::STARTING) ? ErrorType::OVERHEAT_EGT_1 : ErrorType::OVERHEAT_EGT_3);
        }
    }

    // --- 4. 白色咨询信息 (WHITE ADVISORY) ---
    if ((fail_N1 + fail_N2) > 0 && fail_N1 != 2 && fail_N2 != 2) error_list.push_back(ErrorType::SENSOR_N_ONE);
    if ((fail_EGT1 + fail_EGT2) > 0 && fail_EGT1 != 2 && fail_EGT2 != 2) error_list.push_back(ErrorType::SENSOR_EGT_ONE);

    return error_list;
}