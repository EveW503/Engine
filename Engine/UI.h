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
        bool is_running_light_on, double n1, double n2, const std::vector<ErrorType>& detected_errors);

    std::wstring getErrorString(ErrorType error);

    int handleInput();

private:
    void drawGauge(int x, int y, int radius, double val, double min_val, double max_val, const std::wstring& label, int status);
    void drawButton(RECT r, const std::wstring& text, COLORREF color, COLORREF hover_color = 0);
    void drawInfoBox(int x, int y, const std::wstring& label, double value, const std::wstring& unit, bool is_valid = true);
    void drawCASList(const std::vector<ErrorType>& errors);

    RECT btn_start_rect;
    RECT btn_stop_rect;
    RECT btn_inc_rect;
    RECT btn_dec_rect;

    RECT fault_buttons[14];
    const wchar_t* fault_labels[14];
};