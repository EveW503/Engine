#include "Simulator.h"
#include <cmath>
#include <cstdlib>

Simulator::Simulator()
{
    phase_timer = 0.0;
    current_state = EngineState::OFF;
    N1 = 0;
    N2 = 0;
    eng_data.rpm_1 = 0;
    eng_data.rpm_2 = 0;
    eng_data.EGT1_temp = 20;
    eng_data.EGT2_temp = 20;
    eng_data.Fuel_V = 0;
    eng_data.Fuel_C = 20000; // 满油
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
        record_N = eng_data.rpm_1;
        record_EGT = eng_data.EGT1_temp;
	}

}

void Simulator::update()
{

    if (current_state == EngineState::STARTING || current_state == EngineState::STOPPING) {
        phase_timer += DT;
    }

    if (eng_data.Fuel_C <= 0.0 && current_state != EngineState::STOPPING && current_state != EngineState::OFF) {
        eng_data.Fuel_C = 0.0;
        stopEngine();
    }

    switch (current_state) {
    case EngineState::STARTING:
        if (phase_timer <= 2.0) {
            eng_data.rpm_1 += 50;
            eng_data.rpm_2 += 50; 

            eng_data.Fuel_V += 0.025;

        }
        else {
            double t = phase_timer;

            if (t > 1.0) {
                eng_data.Fuel_V = 42.0 * log10(t - 1.0) + 10.0;
                eng_data.rpm_1 = 23000.0 * log10(t - 1.0) + 20000.0;
                eng_data.rpm_2 = 23000.0 * log10(t - 1.0) + 20000.0; // 暂时双发同步

                eng_data.EGT1_temp = 900.0 * log10(t - 1.0) + 20.0;
                eng_data.EGT2_temp = 900.0 * log10(t - 1.0) + 20.0;
            }

            if (eng_data.rpm_1 >= 40000 * 0.95) {
                current_state = EngineState::RUNNING;
                record_N = eng_data.rpm_1;      // 记录当前转速作为基准
                record_EGT = eng_data.EGT1_temp; // 记录当前温度作为基准
                record_Fuel_V = eng_data.Fuel_V; // 记录当前油耗作为基准
            }
        }
        break;

    case EngineState::RUNNING:
    {
        double noise = (rand() % 600 - 300) / 10000.0;

        eng_data.rpm_1 = record_N * (1.0 + noise);
        eng_data.rpm_2 = eng_data.rpm_1; // 双发同步

        eng_data.EGT1_temp = record_EGT * (1.0 + noise);
        eng_data.EGT2_temp = record_EGT * (1.0 + noise);

        // 燃油流速在稳态也稍微波动
        // 假设基准流速是启动结束时的值 (大约 42*lg(something)+10)
        // 这里简单给定一个稳态流速基准，比如 80
        eng_data.Fuel_V = record_Fuel_V * (1.0 + noise);
        break;
    }
    case EngineState::STOPPING:
        eng_data.Fuel_V = 0;

        if (phase_timer >= 10.0) {
            eng_data.rpm_1 = 0;
            eng_data.rpm_2 = 0;
            eng_data.EGT1_temp = 20.0;
            eng_data.EGT2_temp = 20.0;
            current_state = EngineState::OFF;
        }
        else {
           
            double base = 0.6;

            eng_data.rpm_1 = record_N * std::pow(base, phase_timer);
            eng_data.rpm_2 = eng_data.rpm_1; // 双发同步

            eng_data.EGT1_temp = (record_EGT - 20.0) * std::pow(base, phase_timer) + 20.0;
            eng_data.EGT2_temp = (record_EGT - 20.0) * std::pow(base, phase_timer) + 20.0;
        }
        break;
    case EngineState::OFF:
        eng_data.rpm_1 = 0.0;
        eng_data.rpm_2 = 0.0;
        eng_data.EGT1_temp = 20.0;
        eng_data.EGT2_temp = 20.0;
        eng_data.Fuel_V = 0.0;
        break;
    }

    if (eng_data.Fuel_C > 0) {
        eng_data.Fuel_C -= eng_data.Fuel_V * DT;
    }

    N1 = (eng_data.rpm_1 / max_rpm) * 100.0;
    N2 = (eng_data.rpm_2 / max_rpm) * 100.0;
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