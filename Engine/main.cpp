#include "Simulator.h"
#include "UI.h"
#include "Logger.h"
#include "Timer.h"
#include "EICAS.h" // 引入 EICAS 类
#include <Windows.h>

int main() {
    // 1. 实例化模块
    Simulator sim;
    EICAS eicas;    // 你的检测逻辑类
    UI ui;
    Logger logger;
    Timer timer(0.005);

    // 随机数种子 (放在这里!)
    srand((unsigned int)time(0));

    ui.init();
    BeginBatchDraw();

    bool running = true;
    while (running) {
        timer.tick();

        // --- A. 处理 UI 输入 (包含故障按钮) ---
        int cmd = ui.handleInput();

        if (cmd == 1) sim.startEngine();
        if (cmd == 2) sim.stopEngine();
        if (cmd == 3) sim.addDash();
        if (cmd == 4) sim.reduceDash();

        // 故障注入指令 (100 ~ 113)
        if (cmd >= 100 && cmd < 114) {
            int faultIndex = cmd - 100;
            // 映射回 ErrorType (需要和 UI.cpp 里的数组顺序一致)
            ErrorType types[] = {
                ErrorType::SENSOR_N_ONE, ErrorType::SENSOR_N_TWO,
                ErrorType::SENSOR_EGT_ONE, ErrorType::SENSOR_EGT_TWO,
                ErrorType::SENSOR_FUEL, ErrorType::SENSOR_ALL,
                ErrorType::LOW_FUEL,
                ErrorType::OVERSPEED_N1_1, ErrorType::OVERSPEED_N1_2,
                ErrorType::OVERHEAT_EGT_1, ErrorType::OVERHEAT_EGT_2,
                ErrorType::OVERHEAT_EGT_3, ErrorType::OVERHEAT_EGT_4,
                ErrorType::OVERSPEED_FUEL
            };

            // 注入故障到模拟器
            sim.setErrorType(types[faultIndex]); // 假设你在 Simulator.h 加了这个接口
            // 或者 sim.setErrorType(types[faultIndex]); 
        }

        // --- B. 物理更新 ---
        while (timer.consumeStep()) {
            sim.update();
            logger.log(timer.getSimulationTime(), sim.getData());
        }

        // --- C. EICAS 逻辑判定 (核心功能) ---
        // 获取模拟器的原始数据
        EngineData rawData = sim.getData();
        EngineState eng_state = sim.getState();

        // EICAS 进行独立判定 (不依赖 Simulator 的 error_type，只看数据)
        ErrorType detectedError = eicas.judge(rawData, eng_state);

        // --- D. 绘图 (闭环显示) ---
        // 将 EICAS 判定出的 detectedError 传给 UI 显示
        ui.draw(timer.getSimulationTime(),
            rawData,
            sim.getState(),
            sim.isStabilized(),
            sim.getN1(),
            sim.getN2(),
            detectedError); // <--- 这里传入检测结果

        if (GetAsyncKeyState(VK_ESCAPE)) running = false;
    }

    EndBatchDraw();
    return 0;
}