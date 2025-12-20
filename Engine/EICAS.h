// 文件：EICAS.h
#pragma once
#include "DataStructrue.h"
#include <vector> // 必须引入 vector

class EICAS {
public:
    EICAS();
    ~EICAS();

    // 【修改】返回值改为 vector，支持返回多条故障
    std::vector<ErrorType> judge(const EngineData& data, EngineState state);
};