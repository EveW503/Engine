#include "EICAS.h"

EICAS::EICAS() {};
EICAS::~EICAS() {};

ErrorType EICAS::judge(const EngineData& data)
{
    int sensor_failed_count_N1 = 0;   // 左发 N 转速传感器故障数
    int sensor_failed_count_N2 = 0;   // 右发 N 转速传感器故障数
    int sensor_failed_count_EGT1 = 0; // 左发 EGT 传感器故障数
    int sensor_failed_count_EGT2 = 0; // 右发 EGT 传感器故障数
    int sensor_failed_count_N = sensor_failed_count_N1 + sensor_failed_count_N2;
    int sensor_failed_count_EGT = sensor_failed_count_EGT1 + sensor_failed_count_EGT2;

    for (int i = 0; i < 4; i++)
    {
        if (data.is_N_sensor_valid[i] == false) {
            if (i < 2) sensor_failed_count_N1++; // 0, 1 是左发
            else       sensor_failed_count_N2++; // 2, 3 是右发
        }

        if (data.is_EGT_sensor_valid[i] == false) {
            if (i < 2) sensor_failed_count_EGT1++;
            else       sensor_failed_count_EGT2++;
        }
    }

    if (sensor_failed_count_N1 == 2 && sensor_failed_count_N2 == 2 &&
        sensor_failed_count_EGT1 == 2 && sensor_failed_count_EGT2 == 2) {
        return ErrorType::SENSOR_ALL;
    }

    if (sensor_failed_count_N == 1)
        return ErrorType::SENSOR_N_ONE;
    if (sensor_failed_count_N2 == 2|| sensor_failed_count_N1 == 2)
        return ErrorType::SENSOR_N_TWO;
    if (sensor_failed_count_EGT == 2)
        return ErrorType::SENSOR_EGT_ONE;
    if (sensor_failed_count_EGT2 == 2 || sensor_failed_count_EGT1 == 2)
        return ErrorType::SENSOR_EGT_TWO;

    if (data.is_Fuel_valid == false)
        return ErrorType::SENSOR_FUEL;

    const double LIMIT_N_ORANGE = 42000.0;
    const double LIMIT_N_RED = 48000.0;
    const double LIMIT_EGT_ORANGE_START = 850.0;
    const double LIMIT_EGT_RED_START = 1000.0;
    const double LIMIT_EGT_ORANGE_RUN =950.0;
    const double LIMIT_EGT_RED_RUN = 1100.0;

    if (data.rpm_1 > LIMIT_N_ORANGE || data.rpm_2 > LIMIT_N_ORANGE)
        return ErrorType::OVERSPEED_N1_1;
    if (data.rpm_1 > LIMIT_N_RED || data.rpm_2 > LIMIT_N_RED)
        return ErrorType::OVERSPEED_N1_2;

    if (data.EGT1_temp > LIMIT_EGT_ORANGE_START || data.EGT2_temp > LIMIT_EGT_ORANGE_START)
        return ErrorType::OVERHEAT_EGT_1;
    if (data.EGT1_temp > LIMIT_EGT_RED_START || data.EGT2_temp > LIMIT_EGT_RED_START)
        return ErrorType::OVERHEAT_EGT_2;
    if (data.EGT1_temp > LIMIT_EGT_ORANGE_RUN || data.EGT2_temp > LIMIT_EGT_ORANGE_RUN)
        return ErrorType::OVERHEAT_EGT_3;
    if (data.EGT1_temp > LIMIT_EGT_RED_RUN || data.EGT2_temp > LIMIT_EGT_RED_RUN)
        return ErrorType::OVERHEAT_EGT_4;

    if (data.Fuel_V > 50.0)
        return ErrorType::OVERSPEED_FUEL;

    if (data.Fuel_C < 1000.0)
        return ErrorType::LOW_FUEL;

    return ErrorType::NONE;
}