#include "EICAS.h"
#include <algorithm>

EICAS::EICAS() {};
EICAS::~EICAS() {};

std::vector<ErrorType> EICAS::judge(const EngineData &data, EngineState state, double current_time)
{
    std::vector<ErrorType> current_raw_errors;

    // 统计传感器故障数量
    int fail_n1 = 0;
    int fail_n2 = 0;
    int fail_egt1 = 0;
    int fail_egt2 = 0;

    for (int i = 0; i < 4; i++)
    {
        if (!data.is_n_sensor_valid[i])
            (i < 2) ? fail_n1++ : fail_n2++;
        if (!data.is_egt_sensor_valid[i])
            (i < 2) ? fail_egt1++ : fail_egt2++;
    }

    // 判断红色警告
    if ((fail_n1 == 2 && fail_n2 == 2) || (fail_egt1 == 2 && fail_egt2 == 2))
        current_raw_errors.push_back(ErrorType::SENSOR_ALL);

    if (!data.is_fuel_valid)
        current_raw_errors.push_back(ErrorType::SENSOR_FUEL);

    const double limit_n_red = 48000.0;
    if (data.rpm_1 > limit_n_red || data.rpm_2 > limit_n_red)
        current_raw_errors.push_back(ErrorType::OVERSPEED_N1_2);

    double limit_egt_red = (state == EngineState::STARTING) ? 1000.0 : 1100.0;
    if (data.egt1_temp > limit_egt_red || data.egt2_temp > limit_egt_red)
    {
        current_raw_errors.push_back((state == EngineState::STARTING) ? ErrorType::OVERHEAT_EGT_2
                                                                      : ErrorType::OVERHEAT_EGT_4);
    }

    // 判断琥珀色警告
    if (fail_n1 == 2 || fail_n2 == 2)
        current_raw_errors.push_back(ErrorType::SENSOR_N_TWO);
    if (fail_egt1 == 2 || fail_egt2 == 2)
        current_raw_errors.push_back(ErrorType::SENSOR_EGT_TWO);

    if (data.fuel_c < 1000.0)
        current_raw_errors.push_back(ErrorType::LOW_FUEL);
    if (data.fuel_v > 50.0)
        current_raw_errors.push_back(ErrorType::OVERSPEED_FUEL);

    const double limit_n_amber = 42000.0;
    bool has_red_overspeed = (data.rpm_1 > limit_n_red || data.rpm_2 > limit_n_red);
    if (!has_red_overspeed)
    {
        if (data.rpm_1 > limit_n_amber || data.rpm_2 > limit_n_amber)
            current_raw_errors.push_back(ErrorType::OVERSPEED_N1_1);
    }

    double limit_egt_amber = (state == EngineState::STARTING) ? 850.0 : 950.0;
    bool has_red_egt = (data.egt1_temp > limit_egt_red || data.egt2_temp > limit_egt_red);
    if (!has_red_egt)
    {
        if (data.egt1_temp > limit_egt_amber || data.egt2_temp > limit_egt_amber)
        {
            current_raw_errors.push_back((state == EngineState::STARTING) ? ErrorType::OVERHEAT_EGT_1
                                                                          : ErrorType::OVERHEAT_EGT_3);
        }
    }

    // 判断白色咨询信息
    if ((fail_n1 + fail_n2) > 0 && fail_n1 != 2 && fail_n2 != 2)
        current_raw_errors.push_back(ErrorType::SENSOR_N_ONE);
    if ((fail_egt1 + fail_egt2) > 0 && fail_egt1 != 2 && fail_egt2 != 2)
        current_raw_errors.push_back(ErrorType::SENSOR_EGT_ONE);

    // 维护显示队列
    for (const auto &err : current_raw_errors)
    {
        bool is_new = true;
        for (const auto &last : last_raw_errors)
        {
            if (last == err)
            {
                is_new = false;
                break;
            }
        }

        if (is_new)
        {
            bool already_showing = false;
            for (auto &msg : active_msgs)
            {
                if (msg.type == err)
                {
                    msg.expire_time = current_time + 5.0; // 刷新过期时间
                    already_showing = true;
                    break;
                }
            }
            if (!already_showing)
            {
                active_msgs.push_back({err, current_time + 5.0});
            }
        }
    }

    // 清理已过期的消息
    auto it = active_msgs.begin();
    while (it != active_msgs.end())
    {
        if (current_time > it->expire_time)
        {
            it = active_msgs.erase(it);
        }
        else
        {
            ++it;
        }
    }

    last_raw_errors = current_raw_errors;

    std::vector<ErrorType> output;
    for (const auto &msg : active_msgs)
    {
        output.push_back(msg.type);
    }

    return output;
}