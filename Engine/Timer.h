#pragma once
#include <chrono>

class Timer
{
public:
    Timer(double step = 0.005);

    void reset();

    // 更新系统时间
    void tick();

    // 检查是否满足物理更新步长
    bool consumeStep();

    double getSimulationTime() const;

    double getFixedStep() const;

private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    TimePoint last_time;   // 上一帧的系统时间
    double accumulator;    // 累积时间池
    double total_sim_time; // 仿真逻辑运行的总时间
    const double fixed_dt; // 固定时间步长
};