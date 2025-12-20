#pragma once
#include "DataStructrue.h"

class EICAS {
private:
public:
	EICAS();
	~EICAS();

	ErrorType judge(const EngineData& data, EngineState state);
};