#pragma once
#include <ctime>
#include <cmath>
#include "DataStructrue.h"

class Simulator {
private:
	EngineData eng_data;
	double phase_timer; // 当前阶段运行了多久
	EngineState current_state;
	ErrorType error_type;

	double N1;
	double N2;// 转速

	double record_N;
	double record_EGT;
	double record_Fuel_V;

	const double max_rpm = 40000.0;
	const double DT = 0.005; // 模拟步长

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
	EngineState getState();
	EngineData getData();
};