// 文件：evew503/engine/Engine-0afa75b00eef08f05a15a7ed406d5f619f090826/Engine/EICAS.cpp
#include "EICAS.h"
#include <algorithm> // 用于 std::find

EICAS::EICAS() {};
EICAS::~EICAS() {};

std::vector<ErrorType> EICAS::judge(const EngineData& data, EngineState state, double currentTime)
{
    // ---------------------------------------------------------
    // 第一步：检测当前帧的所有原始故障 (Raw Detection)
    // ---------------------------------------------------------
    std::vector<ErrorType> current_raw_errors;

    // --- 1. 传感器故障统计 ---
    int fail_N1 = 0; int fail_N2 = 0;
    int fail_EGT1 = 0; int fail_EGT2 = 0;

    for (int i = 0; i < 4; i++) {
        if (!data.is_N_sensor_valid[i])   (i < 2) ? fail_N1++ : fail_N2++;
        if (!data.is_EGT_sensor_valid[i]) (i < 2) ? fail_EGT1++ : fail_EGT2++;
    }

    // 红色警告
    if ((fail_N1 == 2 && fail_N2 == 2) || (fail_EGT1 == 2 && fail_EGT2 == 2))
        current_raw_errors.push_back(ErrorType::SENSOR_ALL);

    if (!data.is_Fuel_valid)
        current_raw_errors.push_back(ErrorType::SENSOR_FUEL);

    const double LIMIT_N_RED = 48000.0;
    if (data.rpm_1 > LIMIT_N_RED || data.rpm_2 > LIMIT_N_RED)
        current_raw_errors.push_back(ErrorType::OVERSPEED_N1_2);

    double limit_egt_red = (state == EngineState::STARTING) ? 1000.0 : 1100.0;
    if (data.EGT1_temp > limit_egt_red || data.EGT2_temp > limit_egt_red) {
        current_raw_errors.push_back((state == EngineState::STARTING) ? ErrorType::OVERHEAT_EGT_2 : ErrorType::OVERHEAT_EGT_4);
    }

    // 琥珀色警告
    if (fail_N1 == 2 || fail_N2 == 2) current_raw_errors.push_back(ErrorType::SENSOR_N_TWO);
    if (fail_EGT1 == 2 || fail_EGT2 == 2) current_raw_errors.push_back(ErrorType::SENSOR_EGT_TWO);

    if (data.Fuel_C < 1000.0) current_raw_errors.push_back(ErrorType::LOW_FUEL);
    if (data.Fuel_V > 50.0) current_raw_errors.push_back(ErrorType::OVERSPEED_FUEL);

    const double LIMIT_N_AMBER = 42000.0;
    bool has_red_overspeed = (data.rpm_1 > LIMIT_N_RED || data.rpm_2 > LIMIT_N_RED);
    if (!has_red_overspeed) {
        if (data.rpm_1 > LIMIT_N_AMBER || data.rpm_2 > LIMIT_N_AMBER)
            current_raw_errors.push_back(ErrorType::OVERSPEED_N1_1);
    }

    double limit_egt_amber = (state == EngineState::STARTING) ? 850.0 : 950.0;
    bool has_red_egt = (data.EGT1_temp > limit_egt_red || data.EGT2_temp > limit_egt_red);
    if (!has_red_egt) {
        if (data.EGT1_temp > limit_egt_amber || data.EGT2_temp > limit_egt_amber) {
            current_raw_errors.push_back((state == EngineState::STARTING) ? ErrorType::OVERHEAT_EGT_1 : ErrorType::OVERHEAT_EGT_3);
        }
    }

    // 白色咨询
    if ((fail_N1 + fail_N2) > 0 && fail_N1 != 2 && fail_N2 != 2) current_raw_errors.push_back(ErrorType::SENSOR_N_ONE);
    if ((fail_EGT1 + fail_EGT2) > 0 && fail_EGT1 != 2 && fail_EGT2 != 2) current_raw_errors.push_back(ErrorType::SENSOR_EGT_ONE);

    // ---------------------------------------------------------
    // 第二步：管理显示队列 (Manage Display Queue)
    // ---------------------------------------------------------

    // 1. 检查新产生的故障 (Rising Edge Detection)
    for (const auto& err : current_raw_errors) {
        // 检查该错误是否在上一帧已经存在
        bool is_new = true;
        for (const auto& last : last_raw_errors) {
            if (last == err) {
                is_new = false;
                break;
            }
        }

        // 如果是新故障，或者是已经在显示列表中但也想刷新时间（可选，这里按题目只在触发时显示5s）
        if (is_new) {
            // 检查是否已经在 active_msgs 里（比如刚消失又触发，或者之前的还没消失）
            bool already_showing = false;
            for (auto& msg : active_msgs) {
                if (msg.type == err) {
                    msg.expireTime = currentTime + 5.0; // 重置计时器
                    already_showing = true;
                    break;
                }
            }
            if (!already_showing) {
                active_msgs.push_back({ err, currentTime + 5.0 }); // 新增消息，5秒后过期
            }
        }
    }

    // 2. 清理过期消息
    // 使用迭代器遍历，删除当前时间 > 过期时间的消息
    auto it = active_msgs.begin();
    while (it != active_msgs.end()) {
        if (currentTime > it->expireTime) {
            it = active_msgs.erase(it); // 只有这里移除了，UI 才会停止绘制
        }
        else {
            ++it;
        }
    }

    // 3. 更新历史记录
    last_raw_errors = current_raw_errors;

    // 4. 生成输出列表
    std::vector<ErrorType> output;
    for (const auto& msg : active_msgs) {
        output.push_back(msg.type);
    }

    return output;
}