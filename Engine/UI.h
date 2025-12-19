#pragma once
#include <graphics.h>
#include "DataStructrue.h"
#include <string>

class UI {
public:
    UI();
    ~UI();

    void init();
    // 保持 draw 参数不变，符合你 main.cpp 的调用
    void draw(double time, const EngineData& data, EngineState state, bool isRunningLightOn, double N1, double N2);
    int handleInput();

private:
    // 【核心修改】增加 status 参数：0=正常(白), 1=警戒(琥珀), 2=警告(红), -1=无效
    void drawGauge(int x, int y, int radius, double val, double minVal, double maxVal, const std::wstring& label, int status);

    void drawButton(RECT r, const std::wstring& text, COLORREF bgColor, COLORREF hoverColor = 0);
    void drawInfoBox(int x, int y, const std::wstring& label, double value, const std::wstring& unit);

    RECT btnStartRect;
    RECT btnStopRect;
    RECT btnIncRect;
    RECT btnDecRect;
};