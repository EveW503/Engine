#include "Simulator.h"
#include <cmath>
#include <cstdlib>

Simulator::Simulator()
{
    phase_timer = 0.0;
    current_state = EngineState::OFF;
    error_type = ErrorType::NONE;
    N1 = 0;
    N2 = 0;

    // 初始化 EngineData
    eng_data.rpm_1 = 0;
    eng_data.rpm_2 = 0;
    eng_data.EGT1_temp = 20;
    eng_data.EGT2_temp = 20;
    eng_data.Fuel_V = 0;
    eng_data.Fuel_C = 20000;

    // 【初始化】物理真值
    real_Fuel_C = 20000.0;
    real_Fuel_V = 0.0;
    real_rpm_1 = 0.0;
    real_rpm_2 = 0.0;
    real_EGT1 = 20.0;
    real_EGT2 = 20.0;
}

Simulator::~Simulator() {}

void Simulator::startEngine()
{
    if (current_state == EngineState::OFF || current_state == EngineState::STOPPING) {
        current_state = EngineState::STARTING;
        phase_timer = 0.0;
    }
}

void Simulator::stopEngine()
{
    if (current_state != EngineState::OFF) {
        current_state = EngineState::STOPPING;
        phase_timer = 0.0;
        // 记录当前的物理真值作为基准，而不是记录可能已经出错的 eng_data
        record_N = real_rpm_1;
        record_EGT = real_EGT1;
    }
}

void Simulator::update()
{
    if (current_state == EngineState::STARTING || current_state == EngineState::STOPPING) {
        phase_timer += DT;
    }

    // 1. 物理计算：消耗真实燃油
    if (real_Fuel_C > 0) {
        real_Fuel_C -= real_Fuel_V * DT;
    }

    // 2. 物理判定：真实没油了才停车
    if (real_Fuel_C <= 0.0 && current_state != EngineState::STOPPING && current_state != EngineState::OFF) {
        real_Fuel_C = 0.0;
        stopEngine();
    }

    // 3. 物理状态机更新 (全程使用 real_ 变量)
    switch (current_state) {
    case EngineState::STARTING:
        if (phase_timer <= 2.0) {
            // 累加逻辑，现在安全了
            real_rpm_1 += 50;
            real_rpm_2 += 50;
            real_Fuel_V += 0.025;
        }
        else {
            double t = phase_timer;
            if (t > 1.0) {
                real_Fuel_V = 42.0 * log10(t - 1.0) + 10.0;
                real_rpm_1 = 23000.0 * log10(t - 1.0) + 20000.0;
                real_rpm_2 = 23000.0 * log10(t - 1.0) + 20000.0;
                real_EGT1 = 900.0 * log10(t - 1.0) + 20.0;
                real_EGT2 = 900.0 * log10(t - 1.0) + 20.0;
            }

            if (real_rpm_1 >= 40000 * 0.95) {
                current_state = EngineState::RUNNING;
                record_N = real_rpm_1;
                record_EGT = real_EGT1;
                record_Fuel_V = real_Fuel_V;
            }
        }
        break;

    case EngineState::RUNNING:
    {
        double noise = (rand() % 600 - 300) / 10000.0;

        real_rpm_1 = record_N * (1.0 + noise);
        real_rpm_2 = real_rpm_1;
        real_EGT1 = record_EGT * (1.0 + noise);
        real_EGT2 = record_EGT * (1.0 + noise);
        real_Fuel_V = record_Fuel_V * (1.0 + noise);
        break;
    }
    case EngineState::STOPPING:
        real_Fuel_V = 0;
        if (phase_timer >= 10.0) {
            real_rpm_1 = 0;
            real_rpm_2 = 0;
            real_EGT1 = 20.0;
            real_EGT2 = 20.0;
            current_state = EngineState::OFF;
        }
        else {
            double base = 0.6;
            real_rpm_1 = record_N * std::pow(base, phase_timer);
            real_rpm_2 = real_rpm_1;
            real_EGT1 = (record_EGT - 20.0) * std::pow(base, phase_timer) + 20.0;
            real_EGT2 = (record_EGT - 20.0) * std::pow(base, phase_timer) + 20.0;
        }
        break;
    case EngineState::OFF:
        real_rpm_1 = 0.0;
        real_rpm_2 = 0.0;
        real_EGT1 = 20.0;
        real_EGT2 = 20.0;
        real_Fuel_V = 0.0;
        break;
    }

    // --- 【关键同步步骤】 ---
    // 将计算好的“物理真值”同步给“传感器读数”
    // 这是在应用任何故障之前的基础数据
    eng_data.Fuel_C = real_Fuel_C;
    eng_data.Fuel_V = real_Fuel_V;
    eng_data.rpm_1 = real_rpm_1;
    eng_data.rpm_2 = real_rpm_2;
    eng_data.EGT1_temp = real_EGT1;
    eng_data.EGT2_temp = real_EGT2;

    // 计算 N1, N2 百分比 (用于 UI 和判断)
    N1 = (eng_data.rpm_1 / max_rpm) * 100.0;
    N2 = (eng_data.rpm_2 / max_rpm) * 100.0;

    // 默认传感器都是好的
    eng_data.is_N_sensor_valid[0] = true;
    eng_data.is_N_sensor_valid[1] = true;
    eng_data.is_N_sensor_valid[2] = true;
    eng_data.is_N_sensor_valid[3] = true;
    eng_data.is_EGT_sensor_valid[0] = true;
    eng_data.is_EGT_sensor_valid[1] = true;
    eng_data.is_EGT_sensor_valid[2] = true;
    eng_data.is_EGT_sensor_valid[3] = true;
    eng_data.is_Fuel_valid = true;

    // 4. 故障注入：只修改 eng_data (传感器读数)，不影响 next frame 的物理计算
    switch (error_type)
    {
    case ErrorType::NONE:
        break;
    case ErrorType::SENSOR_N_ONE:
        eng_data.is_N_sensor_valid[0] = false;
        break;
    case ErrorType::SENSOR_N_TWO:
        eng_data.rpm_1 = -1.0;
        N1 = -0.0;// 只改读数，real_rpm_1 保持正常
        eng_data.is_N_sensor_valid[0] = false;
        eng_data.is_N_sensor_valid[1] = false;
        break;
    case ErrorType::SENSOR_EGT_ONE:
        eng_data.is_EGT_sensor_valid[0] = false;
        break;
    case ErrorType::SENSOR_EGT_TWO:
        eng_data.EGT1_temp = -50.0; // 只改读数
        eng_data.is_EGT_sensor_valid[0] = false;
        eng_data.is_EGT_sensor_valid[1] = false;
        break;
    case ErrorType::SENSOR_ALL:
        eng_data.EGT1_temp = -500.0;
        eng_data.EGT2_temp = -500.0;
        eng_data.is_EGT_sensor_valid[0] = false;
        eng_data.is_EGT_sensor_valid[1] = false;
        eng_data.is_EGT_sensor_valid[2] = false;
        eng_data.is_EGT_sensor_valid[3] = false;
        // 注意：转速传感器故障通常不直接改数值为 -1，而是依赖 is_valid 标记
        // 但如果您之前的逻辑是双发失效停车，UI 可能会显示 ---
        break;
    case ErrorType::SENSOR_FUEL:
        eng_data.Fuel_C = -0.0; // 只改读数
        eng_data.is_Fuel_valid = false;
        break;
    case ErrorType::OVERSPEED_N1_1:
        eng_data.rpm_1 = 42400.0; // 模拟传感器读数偏高或测试用例
        N1 = 106.0;
        break;
    case ErrorType::OVERSPEED_N1_2:
        eng_data.rpm_1 = 50000.0;
        N1 = 125.0;
        break;
    case ErrorType::OVERHEAT_EGT_1:
        eng_data.EGT1_temp = 900.0;
        break;
    case ErrorType::OVERHEAT_EGT_2:
        eng_data.EGT2_temp = 1050.0;
        break;
    case ErrorType::OVERHEAT_EGT_3:
        eng_data.EGT1_temp = 1000.0;
        break;
    case ErrorType::OVERHEAT_EGT_4:
        eng_data.EGT2_temp = 1250.0;
        break;
    case ErrorType::LOW_FUEL:
        eng_data.Fuel_C = 500.0; // 这里只改显示，模拟“燃油低”报警测试
        break;
    case ErrorType::OVERSPEED_FUEL:
        eng_data.Fuel_V = 55.0; // 模拟高流量报警
        break;
    default:
        break;
    }
}

void Simulator::addDash()
{

    if (current_state == EngineState::RUNNING) {
        // 1. 燃油增加 1 单位
        record_Fuel_V += 1.0;

        // 2. 转速和温度 随机增加 3% ~ 5%
        // rand()%201 -> 0~200; /10000.0 -> 0.0~0.02; +0.03 -> 0.03~0.05
        double jump = 0.03 + (rand() % 201) / 10000.0;

        record_N = record_N * (1.0 + jump);
        record_EGT = record_EGT * (1.0 + jump);

        // 建议加个上限，防止爆表
        if (record_N > 50000) record_N = 50000;
    }
}

void Simulator::reduceDash()
{
    if (current_state == EngineState::RUNNING) {
        // 1. 燃油减少 1 单位
        record_Fuel_V -= 1.0;
        if (record_Fuel_V < 0) record_Fuel_V = 0;

        // 2. 转速和温度 随机减少 3% ~ 5%
        double jump = 0.03 + (rand() % 201) / 10000.0;

        record_N = record_N * (1.0 - jump);
        record_EGT = record_EGT * (1.0 - jump);

        // 防止减成负数
        if (record_N < 0) record_N = 0;
    }
}

bool Simulator::isStabilized()
{
    // 直接返回布尔表达式，代码更简洁
    // 注意：这里用到了 max_rpm 成员变量，这正是逻辑应该放在这里的原因（UI不应该知道max_rpm是多少）
    return (current_state == EngineState::RUNNING &&
        eng_data.rpm_1 >= max_rpm * 0.95 &&
        eng_data.rpm_2 >= max_rpm * 0.95);
}

EngineState Simulator::getState()
{
    return current_state;
}

EngineData Simulator::getData()
{ 
	return eng_data;
}

double Simulator::getN1()
{
    return N1;
}

double Simulator::getN2()
{
    return N2;
}

void Simulator::setErrorType(ErrorType type)
{
    error_type = type;
}