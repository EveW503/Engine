#include "EICAS.h"

EICAS::EICAS() {};
EICAS::~EICAS() {};

ErrorType EICAS::judge(const EngineData& data)
{
	int sensor_failed_count_N1 = 0;
	int sensor_failed_count_N2 = 0;
	int sensor_failed_count_EGT1 = 0;
	int sensor_failed_count_EGT2 = 0;

	for (int i = 0; i < 4; i++)
	{
		if (data.is_N_sensor_valid[i] == false && i < 2)
			sensor_failed_count_N1 = sensor_failed_count_N1 + 1;
		if (data.is_EGT_sensor_valid[i] == false && i < 2)
			sensor_failed_count_EGT1 = sensor_failed_count_EGT1 + 1;
		if (data.is_N_sensor_valid[i] == false && i > 1)
			sensor_failed_count_N2 = sensor_failed_count_N2 + 1;
		if (data.is_EGT_sensor_valid[i] == false && i > 1)
			sensor_failed_count_EGT2 = sensor_failed_count_EGT2 + 1;
	}


}