#include "EICAS.h"
#include "Logger.h"
#include "Simulator.h"
#include "Timer.h"
#include "UI.h"
#include <Windows.h>
#include <comdef.h>

int main()
{
    Simulator sim;
    EICAS eicas;
    UI ui;
    Logger logger;
    Timer timer(0.005);

    srand((unsigned int)time(0));

    ui.init();
    BeginBatchDraw();

    bool running = true;
    while (running)
    {
        timer.tick();

        int cmd = ui.handleInput();

        if (cmd == 1)
            sim.startEngine();
        if (cmd == 2)
            sim.stopEngine();
        if (cmd == 3)
            sim.addDash();
        if (cmd == 4)
            sim.reduceDash();

        // 注入故障
        if (cmd >= 100 && cmd < 114)
        {
            int fault_index = cmd - 100;
            ErrorType types[] = {ErrorType::SENSOR_N_ONE,   ErrorType::SENSOR_N_TWO,   ErrorType::SENSOR_EGT_ONE,
                                 ErrorType::SENSOR_EGT_TWO, ErrorType::SENSOR_FUEL,    ErrorType::SENSOR_ALL,
                                 ErrorType::LOW_FUEL,       ErrorType::OVERSPEED_N1_1, ErrorType::OVERSPEED_N1_2,
                                 ErrorType::OVERHEAT_EGT_1, ErrorType::OVERHEAT_EGT_2, ErrorType::OVERHEAT_EGT_3,
                                 ErrorType::OVERHEAT_EGT_4, ErrorType::OVERSPEED_FUEL};
            sim.setErrorType(types[fault_index]);
        }

        while (timer.consumeStep())
        {
            sim.update();
            logger.log(timer.getSimulationTime(), sim.getData());
        }

        EngineData raw_data = sim.getData();
        EngineState eng_state = sim.getState();

        std::vector<ErrorType> detected_errors = eicas.judge(raw_data, eng_state, timer.getSimulationTime());

        // 自动停车保护逻辑
        bool critical_failure = false;
        for (const auto &err : detected_errors)
        {
            if (err == ErrorType::SENSOR_ALL || err == ErrorType::OVERSPEED_N1_2 || err == ErrorType::OVERHEAT_EGT_2 ||
                err == ErrorType::OVERHEAT_EGT_4)
            {
                critical_failure = true;
                break;
            }
        }

        if (critical_failure)
        {
            if (eng_state != EngineState::OFF && eng_state != EngineState::STOPPING)
            {
                sim.stopEngine();
                logger.logAlert(timer.getSimulationTime(), "SYSTEM: AUTO SHUTDOWN TRIGGERED");
            }
        }

        for (const auto &err : detected_errors)
        {
            std::wstring w_msg = ui.getErrorString(err);
            std::string msg = (const char *)_bstr_t(w_msg.c_str());
            logger.logAlert(timer.getSimulationTime(), msg);
        }

        ui.draw(timer.getSimulationTime(), raw_data, sim.getState(), sim.isStabilized(), sim.getN1(), sim.getN2(),
                detected_errors);

        if (GetAsyncKeyState(VK_ESCAPE))
            running = false;
    }

    EndBatchDraw();
    return 0;
}