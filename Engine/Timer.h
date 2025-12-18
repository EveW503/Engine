#pragma once // Visual Studio 防止头文件重复包含的指令
#include <chrono>

class Timer {
public:
    // 构造函数：默认步长 0.005秒 (5ms)
    Timer(double step = 0.005);

    // 重置计时器
    void reset();

    // 每一帧调用一次：计算距离上一帧经过的真实时间
    void tick();

    // 物理步进判断：是否积攒了足够的时间来执行一次更新
    // 返回 true 代表需要更新，返回 false 代表等待
    bool consumeStep();

    // 获取当前的仿真总时间 (t)
    double getSimulationTime() const;

    // 获取固定的物理时间步长 (dt)
    double getFixedStep() const;

private:
    // 使用高精度时钟类型定义
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    TimePoint last_time;       // 上一次 tick 的系统时间
    double accumulator;        // 时间累加池
    double total_sim_time;     // 仿真逻辑运行的总时长
    const double FIXED_DT;     // 固定的时间步长 (const 常量)
};