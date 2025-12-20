// 文件：evew503/engine/Engine-0afa75b00eef08f05a15a7ed406d5f619f090826/Engine/EICAS.h
#pragma once
#include "DataStructrue.h"
#include <vector>

// 定义一个内部结构记录活跃的消息
struct AlertMsg {
    ErrorType type;
    double expireTime; // 消息应该消失的时间点
};

class EICAS {
private:
    // 记录上一帧检测到的原始故障，用于判断“新故障”
    std::vector<ErrorType> last_raw_errors;

    // 当前正在显示的消息队列
    std::vector<AlertMsg> active_msgs;

public:
    EICAS();
    ~EICAS();

    // 【修改】增加 currentTime 参数
    std::vector<ErrorType> judge(const EngineData& data, EngineState state, double currentTime);
};