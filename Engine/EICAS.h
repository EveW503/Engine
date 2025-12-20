#pragma once
#include "DataStructrue.h"
#include <vector>

struct AlertMsg {
    ErrorType type;
    double expire_time; // 消息消失的时间
};

class EICAS {
private:
    std::vector<ErrorType> last_raw_errors;
    std::vector<AlertMsg> active_msgs;

public:
    EICAS();
    ~EICAS();

    std::vector<ErrorType> judge(const EngineData& data, EngineState state, double current_time);
};