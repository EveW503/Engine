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
    void draw(double time, const EngineData& data, EngineState state);

    // 处理鼠标点击，返回用户的操作指令
    // 返回值: 0-无操作, 1-点击Start, 2-点击Stop
    int handleInput();

private:
    // 辅助画图函数：画仪表盘
    void drawGauge(int x, int y, int radius, double val, double maxVal, const std::wstring& label);
    // 辅助画图函数：画按钮
    void drawButton(int x, int y, int w, int h, const std::wstring& text, COLORREF color);

    // 按钮的坐标区域 (简单的矩形碰撞检测)
    RECT btnStartRect;
    RECT btnStopRect;
};