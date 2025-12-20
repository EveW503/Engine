#pragma once
#include <graphics.h>
#include "DataStructrue.h"
#include <string>
#include <vector>

class UI {
public:
    UI();
    ~UI();

    void init();
    // 增加 detectedError 参数，用于显示 EICAS 判定出的错误信息
    void draw(double time, const EngineData& data, EngineState state,
        bool isRunningLightOn, double N1, double N2, ErrorType detectedError);

    // 返回值扩展：>100 代表故障按钮索引
    int handleInput();

private:
    void drawGauge(int x, int y, int radius, double val, double minVal, double maxVal, const std::wstring& label, int status);
    void drawButton(RECT r, const std::wstring& text, COLORREF color, COLORREF hoverColor = 0);
    void drawInfoBox(int x, int y, const std::wstring& label, double value, const std::wstring& unit);

    // 【新增】绘制 CAS 消息区 (Crew Alerting System)
    void drawCASMessage(ErrorType error);

    // 【新增】辅助函数：获取错误的文字描述
    std::wstring getErrorString(ErrorType error);

    // 现有按钮
    RECT btnStartRect;
    RECT btnStopRect;
    RECT btnIncRect;
    RECT btnDecRect;

    // 【新增】14个故障注入按钮的区域数组
    // 布局设计：2行 x 7列
    RECT faultButtons[14];
    // 对应的故障类型数组，方便映射
    ErrorType faultTypes[14];
};