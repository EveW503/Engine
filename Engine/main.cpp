#include "Simulator.h"
#include "UI.h"
#include "Logger.h"
#include "Timer.h"
#include "EICAS.h"
#include <Windows.h>
#include <comdef.h> // 用于 _bstr_t

int main() {
    // 1. 实例化模块
    Simulator sim;
    EICAS eicas;
    UI ui;
    Logger logger;
    Timer timer(0.005);

    // 随机数种子
    srand((unsigned int)time(0));

    ui.init();
    BeginBatchDraw();

    bool running = true;
    while (running) {
        timer.tick();

        // --- A. 处理 UI 输入 ---
        int cmd = ui.handleInput();

        if (cmd == 1) sim.startEngine();
        if (cmd == 2) sim.stopEngine();
        if (cmd == 3) sim.addDash();
        if (cmd == 4) sim.reduceDash();

        // 故障注入指令 (100 ~ 113)
        if (cmd >= 100 && cmd < 114) {
            int faultIndex = cmd - 100;
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
            sim.setErrorType(types[faultIndex]);
        }

        // --- B. 物理更新 ---
        while (timer.consumeStep()) {
            sim.update();
            logger.log(timer.getSimulationTime(), sim.getData());
        }

        // --- C. EICAS 逻辑判定 ---
        EngineData rawData = sim.getData();
        EngineState eng_state = sim.getState();
        // 获取所有检测到的故障
        std::vector<ErrorType> detectedErrors = eicas.judge(rawData, eng_state);

        // --- 【新增】 自动停车逻辑 (Auto-Shutdown) ---
        // 题目要求：以下4种红色警告必须触发发动机停车
        // 1. SENSOR_ALL (双发传感器故障)
        // 2. OVERSPEED_N1_2 (转速 > 120)
        // 3. OVERHEAT_EGT_2 (启动超温 > 1000)
        // 4. OVERHEAT_EGT_4 (稳态超温 > 1100)

        bool criticalFailure = false;
        for (const auto& err : detectedErrors) {
            if (err == ErrorType::SENSOR_ALL ||
                err == ErrorType::OVERSPEED_N1_2 ||
                err == ErrorType::OVERHEAT_EGT_2 ||
                err == ErrorType::OVERHEAT_EGT_4) {
                criticalFailure = true;
                break;
            }
        }

        // 如果发生致命故障，且引擎尚未停车或关机，则强制停车
        if (criticalFailure) {
            if (eng_state != EngineState::OFF && eng_state != EngineState::STOPPING) {
                sim.stopEngine();
                // 可选：记录一条额外的日志说明系统触发了自动保护
                logger.logAlert(timer.getSimulationTime(), "SYSTEM: AUTO SHUTDOWN TRIGGERED");
            }
        }

        // --- D. 日志记录报警信息 ---
        for (const auto& err : detectedErrors) {
            std::wstring wMsg = ui.getErrorString(err);
            std::string msg = (const char*)_bstr_t(wMsg.c_str());
            logger.logAlert(timer.getSimulationTime(), msg);
        }

        // --- E. 绘图 ---
        ui.draw(timer.getSimulationTime(),
            rawData,
            sim.getState(),
            sim.isStabilized(),
            sim.getN1(),
            sim.getN2(),
            detectedErrors);

        if (GetAsyncKeyState(VK_ESCAPE)) running = false;
    }

    EndBatchDraw();
    return 0;
}