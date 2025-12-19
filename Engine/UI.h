#pragma once
#include <graphics.h>
#include "DataStructrue.h"
#include <string>

class UI {
public:
    UI();
    ~UI();

    // 初始化窗口
    void init();

    // 绘制所有界面元素
    void draw(double time, const EngineData& data, EngineState state, bool isRunningLightOn, double N1, double N2);

    // 处理鼠标点击
    // 返回值: 0-无, 1-Start, 2-Stop, 3-Inc, 4-Dec
    int handleInput();

private:
    // 优化的仪表盘绘制函数
    void drawGauge(int x, int y, int radius, double val, double minVal, double maxVal, const std::wstring& label);

    // 优化的按钮绘制函数
    void drawButton(RECT r, const std::wstring& text, COLORREF bgColor, COLORREF hoverColor = 0);

    // 绘制数据框
    void drawInfoBox(int x, int y, const std::wstring& label, double value, const std::wstring& unit);

    // 按钮区域
    RECT btnStartRect;
    RECT btnStopRect;
    RECT btnIncRect; // 增加推力
    RECT btnDecRect; // 减小推力
};