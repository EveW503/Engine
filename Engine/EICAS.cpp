#include "EICAS.h"

EICAS::EICAS() {};
EICAS::~EICAS() {};

ErrorType EICAS::judge(const EngineData& data, EngineState state)
{
    // --- 1. 传感器故障统计 ---
    int fail_N1 = 0;   // 左发 N 转速传感器失效数 (Max 2)
    int fail_N2 = 0;   // 右发 N 转速传感器失效数 (Max 2)
    int fail_EGT1 = 0; // 左发 EGT 失效数
    int fail_EGT2 = 0; // 右发 EGT 失效数

    for (int i = 0; i < 4; i++) {
        if (!data.is_N_sensor_valid[i])   (i < 2) ? fail_N1++ : fail_N2++;
        if (!data.is_EGT_sensor_valid[i]) (i < 2) ? fail_EGT1++ : fail_EGT2++;
    }

    // --- 2. 红色警告 (RED WARNING) - 优先级最高 ---
    // 2.1 传感器双发完全失效 (题目: 发动机停车, 红色警告)
    // 逻辑：两台引擎的传感器都坏了(>=3个坏或者两边都坏)
    if ((fail_N1 == 2 && fail_N2 == 2) || (fail_EGT1 == 2 && fail_EGT2 == 2))
        return ErrorType::SENSOR_ALL;

    // 2.2 燃油传感器完全失效 (题目: 红色警告)
    if (!data.is_Fuel_valid)
        return ErrorType::SENSOR_FUEL;

    // 2.3 转速严重超限 (>120%)
    const double LIMIT_N_RED = 48000.0; // 120%
    if (data.rpm_1 > LIMIT_N_RED || data.rpm_2 > LIMIT_N_RED)
        return ErrorType::OVERSPEED_N1_2;

    // 2.4 EGT 严重超温 (根据阶段判断)
    double limit_egt_red = (state == EngineState::STARTING) ? 1000.0 : 1100.0;
    // 注意：题目中 EGT 红线对应 ErrorType::OVERHEAT_EGT_2 (启动红) 和 OVERHEAT_EGT_4 (稳态红)
    if (data.EGT1_temp > limit_egt_red || data.EGT2_temp > limit_egt_red) {
        return (state == EngineState::STARTING) ? ErrorType::OVERHEAT_EGT_2 : ErrorType::OVERHEAT_EGT_4;
    }

    // --- 3. 琥珀色警告 (AMBER CAUTION) - 优先级中等 ---
    // 3.1 单发传感器完全失效 (题目: 单发转速传感器故障... 琥珀色)
    // 逻辑：左发坏完 OR 右发坏完
    if (fail_N1 == 2 || fail_N2 == 2) return ErrorType::SENSOR_N_TWO;
    if (fail_EGT1 == 2 || fail_EGT2 == 2) return ErrorType::SENSOR_EGT_TWO;

    // 3.2 燃油相关
    if (data.Fuel_C < 1000.0) return ErrorType::LOW_FUEL;
    if (data.Fuel_V > 50.0) return ErrorType::OVERSPEED_FUEL;

    // 3.3 转速超限 (>105%)
    const double LIMIT_N_AMBER = 42000.0; // 105%
    if (data.rpm_1 > LIMIT_N_AMBER || data.rpm_2 > LIMIT_N_AMBER)
        return ErrorType::OVERSPEED_N1_1;

    // 3.4 EGT 超温
    double limit_egt_amber = (state == EngineState::STARTING) ? 850.0 : 950.0;
    if (data.EGT1_temp > limit_egt_amber || data.EGT2_temp > limit_egt_amber) {
        return (state == EngineState::STARTING) ? ErrorType::OVERHEAT_EGT_1 : ErrorType::OVERHEAT_EGT_3;
    }

    // --- 4. 白色咨询信息 (WHITE ADVISORY) - 优先级最低 ---
    // 4.1 单个传感器失效 (题目: 一切指示正常... 白色)
    if ((fail_N1 + fail_N2) > 0) return ErrorType::SENSOR_N_ONE;
    if ((fail_EGT1 + fail_EGT2) > 0) return ErrorType::SENSOR_EGT_ONE;

    return ErrorType::NONE;
}