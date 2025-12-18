#include "Simulator.h"
#include <cmath>

Simulator::Simulator()
{
    phase_timer = 0.0;
    current_state = EngineState::OFF;
    eng_data.N1_rpm = 0;
    eng_data.N2_rpm = 0;
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
        shutdown_initial_N = eng_data.N1_rpm;
        shutdown_initial_EGT= eng_data.EGT1_temp;
	}

}

void Simulator::update()
{
    if (current_state == EngineState::STARTING || current_state == EngineState::STOPPING) {
        phase_timer += DT;
    }

    switch (current_state) {
    case EngineState::STARTING:
        if (phase_timer <= 2.0) {
            // 题目：每秒 10^4 转 -> 5ms 增加 50 转 (正确)
            eng_data.N1_rpm += 50;
            eng_data.N2_rpm += 50; // 假设双发同步

            // 题目：每秒 5 单位 -> 5ms 增加 0.025 (正确)
            eng_data.Fuel_V += 0.025;

        }
        else {
            double t = phase_timer;

            if (t > 1.0) {
                eng_data.Fuel_V = 42.0 * log10(t - 1.0) + 10.0;
                eng_data.N1_rpm = 23000.0 * log10(t - 1.0) + 20000.0;
                eng_data.N2_rpm = eng_data.N1_rpm; // 暂时双发同步

                // 温度公式 (参考图片): T = 900 * lg(t - 1) + T0
                eng_data.EGT1_temp = 900.0 * log10(t - 1.0) + 20.0;
                eng_data.EGT2_temp = 900.0 * log10(t - 1.0) + 20.0;
            }

            // --- 检查是否达到稳态 ---
            // 题目要求：达到额定转速的 95% 时切入稳态
            // 假设 max_rpm 是 40000
            if (eng_data.N1_rpm >= 40000 * 0.95) {
                current_state = EngineState::RUNNING;
            }
        }
        break;

    case EngineState::RUNNING:
        // 稳态逻辑：在 +/- 3% 范围内波动
        // 需要引入随机数
        break;

    case EngineState::STOPPING:
        // A. 燃油流速直接归零 (题目要求)
        eng_data.Fuel_V = 0;

        // B. 检查是否超时 (10秒内停止)
        if (phase_timer >= 10.0) {
            // 超过10秒，强制彻底关机
            eng_data.N1_rpm = 0;
            eng_data.N2_rpm = 0;
            eng_data.EGT1_temp = 20.0;
            eng_data.EGT2_temp = 20.0;// 恢复室温
            current_state = EngineState::OFF;
        }
        else {
            // C. 对数方式下降 (指数衰减公式)
            // 公式：当前值 = 初始值 * (底数 ^ 时间)

            // 设定底数 a (0 < a < 1)。
            // a 越小，下降越快。建议 0.6 左右，这样 10秒后 0.6^10 ≈ 0.006 (降到初始值的0.6%)
            double base = 0.6;

            // 计算新的转速
            eng_data.N1_rpm = shutdown_initial_N * std::pow(base, phase_timer);
            eng_data.N2_rpm = eng_data.N1_rpm; // 双发同步

            // 计算新的温度 (假设室温是 20度，不能降到0度)
            // 温度衰减模型：(当前温差 * 衰减系数) + 室温
            eng_data.EGT1_temp = (shutdown_initial_EGT - 20.0) * std::pow(base, phase_timer) + 20.0;
            eng_data.EGT2_temp = (shutdown_initial_EGT - 20.0) * std::pow(base, phase_timer) + 20.0;
        }
        break;
    case EngineState::OFF:
        // 确保数据归零
        eng_data.N1_rpm = 0.0;
        eng_data.N2_rpm = 0.0;
        eng_data.EGT1_temp = 20.0;
        eng_data.EGT2_temp = 20.0;
        eng_data.Fuel_V = 0.0;
        break;
    }

    // 3. 通用计算 (所有状态都要做)

    // 计算燃油消耗 (积分)
    // 燃油减少量 = 流速 * 时间
    // 题目中 Fuel_V 单位是 "单位每秒"，所以要乘以 dt (0.005)
    if (eng_data.Fuel_C > 0) {
        eng_data.Fuel_C -= eng_data.Fuel_V * DT;
    }
}


EngineData Simulator::getData()
{ 
	return eng_data;
}