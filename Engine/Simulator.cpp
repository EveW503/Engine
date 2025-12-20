#include "Simulator.h"
#include <cmath>
#include <cstdlib>

Simulator::Simulator()
{
    phase_timer = 0.0;
    current_state = EngineState::OFF;
    error_type = ErrorType::NONE;
    n1 = 0;
    n2 = 0;

    // 初始化引擎数据
    eng_data.rpm_1 = 0;
    eng_data.rpm_2 = 0;
    eng_data.egt1_temp = 20;
    eng_data.egt2_temp = 20;
    eng_data.fuel_v = 0;
    eng_data.fuel_c = 20000;

    // 初始化物理真值
    real_fuel_c = 20000.0;
    real_fuel_v = 0.0;
    real_rpm_1 = 0.0;
    real_rpm_2 = 0.0;
    real_egt1 = 20.0;
    real_egt2 = 20.0;
}

Simulator::~Simulator() {}

void Simulator::startEngine()
{
    if (current_state == EngineState::OFF || current_state == EngineState::STOPPING)
    {
        current_state = EngineState::STARTING;
        phase_timer = 0.0;
    }
}

void Simulator::stopEngine()
{
    if (current_state != EngineState::OFF)
    {
        current_state = EngineState::STOPPING;
        phase_timer = 0.0;
        // 记录停止前的状态基准
        record_n = real_rpm_1;
        record_egt = real_egt1;
    }
}

void Simulator::update()
{
    if (current_state == EngineState::STARTING || current_state == EngineState::STOPPING)
    {
        phase_timer += dt;
    }

    // 消耗燃油
    if (real_fuel_c > 0)
    {
        real_fuel_c -= real_fuel_v * dt;
    }

    // 燃油耗尽导致停车
    if (real_fuel_c <= 0.0 && current_state != EngineState::STOPPING && current_state != EngineState::OFF)
    {
        real_fuel_c = 0.0;
        stopEngine();
    }

    switch (current_state)
    {
    case EngineState::STARTING:
        if (phase_timer <= 2.0)
        {
            real_rpm_1 += 50;
            real_rpm_2 += 50;
            real_fuel_v += 0.025;
        }
        else
        {
            double t = phase_timer;
            if (t > 1.0)
            {
                real_fuel_v = 42.0 * log10(t - 1.0) + 10.0;
                real_rpm_1 = 23000.0 * log10(t - 1.0) + 20000.0;
                real_rpm_2 = 23000.0 * log10(t - 1.0) + 20000.0;
                real_egt1 = 900.0 * log10(t - 1.0) + 20.0;
                real_egt2 = 900.0 * log10(t - 1.0) + 20.0;
            }

            if (real_rpm_1 >= 40000 * 0.95)
            {
                current_state = EngineState::RUNNING;
                record_n = real_rpm_1;
                record_egt = real_egt1;
                record_fuel_v = real_fuel_v;
            }
        }
        break;

    case EngineState::RUNNING:
    {
        double noise = (rand() % 600 - 300) / 10000.0;

        real_rpm_1 = record_n * (1.0 + noise);
        real_rpm_2 = real_rpm_1;
        real_egt1 = record_egt * (1.0 + noise);
        real_egt2 = record_egt * (1.0 + noise);
        real_fuel_v = record_fuel_v * (1.0 + noise);
        break;
    }
    case EngineState::STOPPING:
        real_fuel_v = 0;
        if (phase_timer >= 10.0)
        {
            real_rpm_1 = 0;
            real_rpm_2 = 0;
            real_egt1 = 20.0;
            real_egt2 = 20.0;
            current_state = EngineState::OFF;
        }
        else
        {
            double base = 0.6;
            real_rpm_1 = record_n * std::pow(base, phase_timer);
            real_rpm_2 = real_rpm_1;
            real_egt1 = (record_egt - 20.0) * std::pow(base, phase_timer) + 20.0;
            real_egt2 = (record_egt - 20.0) * std::pow(base, phase_timer) + 20.0;
        }
        break;
    case EngineState::OFF:
        real_rpm_1 = 0.0;
        real_rpm_2 = 0.0;
        real_egt1 = 20.0;
        real_egt2 = 20.0;
        real_fuel_v = 0.0;
        break;
    }

    // 将真值同步给传感器数据
    eng_data.fuel_c = real_fuel_c;
    eng_data.fuel_v = real_fuel_v;
    eng_data.rpm_1 = real_rpm_1;
    eng_data.rpm_2 = real_rpm_2;
    eng_data.egt1_temp = real_egt1;
    eng_data.egt2_temp = real_egt2;

    n1 = (eng_data.rpm_1 / max_rpm) * 100.0;
    n2 = (eng_data.rpm_2 / max_rpm) * 100.0;

    // 默认传感器为正常
    eng_data.is_n_sensor_valid[0] = true;
    eng_data.is_n_sensor_valid[1] = true;
    eng_data.is_n_sensor_valid[2] = true;
    eng_data.is_n_sensor_valid[3] = true;
    eng_data.is_egt_sensor_valid[0] = true;
    eng_data.is_egt_sensor_valid[1] = true;
    eng_data.is_egt_sensor_valid[2] = true;
    eng_data.is_egt_sensor_valid[3] = true;
    eng_data.is_fuel_valid = true;

    // 注入故障
    switch (error_type)
    {
    case ErrorType::NONE:
        break;
    case ErrorType::SENSOR_N_ONE:
        eng_data.is_n_sensor_valid[0] = false;
        break;
    case ErrorType::SENSOR_N_TWO:
        eng_data.rpm_1 = -1.0;
        n1 = -0.0;
        eng_data.is_n_sensor_valid[0] = false;
        eng_data.is_n_sensor_valid[1] = false;
        break;
    case ErrorType::SENSOR_EGT_ONE:
        eng_data.is_egt_sensor_valid[0] = false;
        break;
    case ErrorType::SENSOR_EGT_TWO:
        eng_data.egt1_temp = -50.0;
        eng_data.is_egt_sensor_valid[0] = false;
        eng_data.is_egt_sensor_valid[1] = false;
        break;
    case ErrorType::SENSOR_ALL:
        eng_data.egt1_temp = -500.0;
        eng_data.egt2_temp = -500.0;
        eng_data.is_egt_sensor_valid[0] = false;
        eng_data.is_egt_sensor_valid[1] = false;
        eng_data.is_egt_sensor_valid[2] = false;
        eng_data.is_egt_sensor_valid[3] = false;
        break;
    case ErrorType::SENSOR_FUEL:
        eng_data.fuel_c = -0.0;
        eng_data.is_fuel_valid = false;
        break;
    case ErrorType::OVERSPEED_N1_1:
        eng_data.rpm_1 = 42400.0;
        n1 = 106.0;
        break;
    case ErrorType::OVERSPEED_N1_2:
        eng_data.rpm_1 = 50000.0;
        n1 = 125.0;
        break;
    case ErrorType::OVERHEAT_EGT_1:
        eng_data.egt1_temp = 900.0;
        break;
    case ErrorType::OVERHEAT_EGT_2:
        eng_data.egt2_temp = 1050.0;
        break;
    case ErrorType::OVERHEAT_EGT_3:
        eng_data.egt1_temp = 1000.0;
        break;
    case ErrorType::OVERHEAT_EGT_4:
        eng_data.egt2_temp = 1250.0;
        break;
    case ErrorType::LOW_FUEL:
        eng_data.fuel_c = 500.0;
        break;
    case ErrorType::OVERSPEED_FUEL:
        eng_data.fuel_v = 55.0;
        break;
    default:
        break;
    }
}

void Simulator::addDash()
{
    if (current_state == EngineState::RUNNING)
    {
        record_fuel_v += 1.0;
        double jump = 0.03 + (rand() % 201) / 10000.0;
        record_n = record_n * (1.0 + jump);
        record_egt = record_egt * (1.0 + jump);
        if (record_n > 50000)
            record_n = 50000;
    }
}

void Simulator::reduceDash()
{
    if (current_state == EngineState::RUNNING)
    {
        record_fuel_v -= 1.0;
        if (record_fuel_v < 0)
            record_fuel_v = 0;
        double jump = 0.03 + (rand() % 201) / 10000.0;
        record_n = record_n * (1.0 - jump);
        record_egt = record_egt * (1.0 - jump);
        if (record_n < 0)
            record_n = 0;
    }
}

bool Simulator::isStabilized()
{
    return (current_state == EngineState::RUNNING && eng_data.rpm_1 >= max_rpm * 0.95 &&
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
    return n1;
}

double Simulator::getN2()
{
    return n2;
}

void Simulator::setErrorType(ErrorType type)
{
    error_type = type;
}