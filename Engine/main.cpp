#include "Simulator.h"
#include "UI.h"
#include "Logger.h"
#include "Timer.h"

int main() {

    srand((unsigned int)time(0));
    // 1. 初始化各模块
    Simulator sim;
    UI ui;
    Logger logger;
    Timer timer(0.005); // 设定物理步长 5ms

    ui.init();
    BeginBatchDraw(); // 开启 EasyX 双缓冲，防闪烁

    bool isAppRunning = true;

    while (isAppRunning) {
        // --- A. 时间步进 ---
        timer.tick();

        // --- B. 处理用户输入 ---
        int cmd = ui.handleInput();
        if (cmd == 1) sim.startEngine();
        if (cmd == 2) sim.stopEngine();
        if (cmd == 3) sim.addDash();    // 点击了 THRUST +
        if (cmd == 4) sim.reduceDash(); // 点击了 THRUST -

        // --- C. 物理逻辑更新 (定长 5ms) ---
        // 核心循环：如果画面卡顿，这里会连续执行多次 update 以追赶时间
        while (timer.consumeStep()) {
            sim.update(); // 每次执行代表物理世界过去了 5ms

            // 记录日志 (题目要求每 5ms 记录一次)
            // 获取当前仿真时间
            logger.log(timer.getSimulationTime(), sim.getData());
        }

        // --- D. 渲染画面 ---
        // 获取当前数据并绘制
        // 注意：UI 绘制可以按照屏幕刷新率来，不需要每 5ms 画一次（那样太快人眼看不清）
        // 这里每一帧循环画一次即可

        // 这里的 Simulator 类需要添加一个 getCurrentState() 方法方便 UI 判断状态
        // 既然你 Simulator.h 里 current_state 是 private，
        // 你可以在 Simulator 类里加一个 `EngineState getState() { return current_state; }`
        // 或者直接传 getData() 里的数据如果包含状态的话。
        // *为了编译通过，我现在假设你会在 Simulator.h 加一个 getter，或者仅仅传 STARTING/RUNNING 给 UI*
        // *临时方案：修改 Simulator.h 把 current_state 设为 public，或者加个 getter*

        // 我们假设你在 Simulator.h 加了这个函数：
        // EngineState getState() { return current_state; }

        // 由于我现在没法改你的 Simulator.h，我们暂时假设 UI 的 state 参数无效，或者你去 Simulator.h 加一行。
        // 这里模拟一个状态传入 (你需要去 Simulator.h 加: EngineState getState() { return current_state; })
        bool stable = sim.isStabilized();
        ui.draw(timer.getSimulationTime(), sim.getData(), sim.getState(), stable);
        // ^ 注意：要把 EngineState::OFF 改成 sim.getState() 才能看到状态灯变化！

        // 如果想按 ESC 退出
        if (GetAsyncKeyState(VK_ESCAPE)) isAppRunning = false;
    }

    EndBatchDraw();
    return 0;
}