#pragma once
#include <ctime>
#include <cmath>
#include "DataStructrue.h"

class Simulator {
private:
	EngineData eng_data;
	double phase_timer; // 当前阶段运行了多久
	EngineState current_state;  
	double shutdown_initial_N;
	double shutdown_initial_EGT;
	const double max_rpm = 40000.0;
	const double DT = 0.005; // 模拟步长

public:
	Simulator();
	~Simulator();
	void startEngine();
	void stopEngine();
	void update();
	EngineData getData();
};