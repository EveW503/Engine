#pragma once
#include <ctime>
#include <cmath>
#include "DataStructrue.h"

class Simulator {
private:
    EngineData eng_data;
    double phase_timer;
    EngineState current_state;
    ErrorType error_type;

    double n1;
    double n2;

    double record_n;
    double record_egt;
    double record_fuel_v;

    // 物理真值变量
    double real_fuel_c;
    double real_fuel_v;
    double real_rpm_1;
    double real_rpm_2;
    double real_egt1;
    double real_egt2;

    const double max_rpm = 40000.0;
    const double dt = 0.005;

public:
    Simulator();
    ~Simulator();
    void startEngine();
    void stopEngine();
    void update();
    void addDash();
    void reduceDash();
    bool isStabilized();
    double getN1();
    double getN2();
    void setErrorType(ErrorType type);
    EngineState getState();
    EngineData getData();
};