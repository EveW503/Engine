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
    void draw(double time, const EngineData& data, EngineState state,
        bool isRunningLightOn, double N1, double N2, ErrorType detectedError);

    int handleInput();

private:
    void drawGauge(int x, int y, int radius, double val, double minVal, double maxVal, const std::wstring& label, int status);
    void drawButton(RECT r, const std::wstring& text, COLORREF color, COLORREF hoverColor = 0);
    void drawInfoBox(int x, int y, const std::wstring& label, double value, const std::wstring& unit);
    void drawCASMessage(ErrorType error);
    std::wstring getErrorString(ErrorType error);

    // 现有按钮区域
    RECT btnStartRect;
    RECT btnStopRect;
    RECT btnIncRect;
    RECT btnDecRect;

    // 故障注入按钮区域
    RECT faultButtons[14];

    // 【新增】故障按钮的文字标签数组
    const wchar_t* faultLabels[14];
};