#include "Timer.h"

using namespace std::chrono;

Timer::Timer(double step) : fixed_dt(step)
{
    reset();
}

void Timer::reset()
{
    last_time = Clock::now();
    accumulator = 0.0;
    total_sim_time = 0.0;
}

void Timer::tick()
{
    TimePoint current_time = Clock::now();

    // 计算两帧之间的时间差
    duration<double> diff = current_time - last_time;
    double frame_time = diff.count();

    last_time = current_time;

    // 防止因为断点或拖动窗口导致单帧时间过长，造成死循环
    if (frame_time > 0.25)
    {
        frame_time = 0.25;
    }

    accumulator += frame_time;
}

bool Timer::consumeStep()
{
    // 只要累积时间超过固定步长，就允许执行一次物理更新
    if (accumulator >= fixed_dt)
    {
        accumulator -= fixed_dt;
        total_sim_time += fixed_dt;
        return true;
    }
    return false;
}

double Timer::getSimulationTime() const
{
    return total_sim_time;
}

double Timer::getFixedStep() const
{
    return fixed_dt;
}