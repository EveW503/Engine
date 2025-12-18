#include "Timer.h"

// 在 .cpp 里可以使用 using namespace，不会影响其他文件
using namespace std::chrono;

// 构造函数实现：使用初始化列表给 const 成员赋值
Timer::Timer(double step) : FIXED_DT(step) {
    reset();
}

void Timer::reset() {
    last_time = Clock::now();
    accumulator = 0.0;
    total_sim_time = 0.0;
}

void Timer::tick() {
    TimePoint current_time = Clock::now();

    // 计算当前帧与上一帧的时间差 (duration 自动转为秒)
    duration<double> diff = current_time - last_time;
    double frame_time = diff.count();

    last_time = current_time;

    // 【防卡死机制】
    // 如果窗口被拖动或断点调试导致停顿太久，强制将单帧时间限制在 0.25秒内。
    // 否则 accumulator 会变得巨大，导致程序为了追赶进度而“死循环”更新。
    if (frame_time > 0.25) {
        frame_time = 0.25;
    }

    accumulator += frame_time;
}

bool Timer::consumeStep() {
    // 只要累加池里的时间够扣 5ms，就返回 true
    if (accumulator >= FIXED_DT) {
        accumulator -= FIXED_DT;
        total_sim_time += FIXED_DT; // 只有发生物理更新时，仿真时间才增加
        return true;
    }
    return false;
}

double Timer::getSimulationTime() const {
    return total_sim_time;
}

double Timer::getFixedStep() const {
    return FIXED_DT;
}