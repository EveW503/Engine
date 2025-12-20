// Simulator.h
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

    double N1;
    double N2;

    double record_N;
    double record_EGT;
    double record_Fuel_V;

    // 【核心修改】引入全套物理真值变量
    double real_Fuel_C;    // 真实燃油量
    double real_Fuel_V;    // 真实流速
    double real_rpm_1;     // 左发真实转速
    double real_rpm_2;     // 右发真实转速
    double real_EGT1;      // 左发真实温度
    double real_EGT2;      // 右发真实温度

    const double max_rpm = 40000.0;
    const double DT = 0.005;

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