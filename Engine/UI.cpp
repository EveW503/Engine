#include "UI.h"
#include <cstdio>
#include <cmath>

// --- 航空仪表配色方案 (优化版) ---
#define COLOR_BG           RGB(30, 30, 35)       // 1. 全局背景 (深黑)
#define COLOR_GAUGE_FACE   RGB(60, 60, 65)       // 2. 表盘底座 (稍亮灰)
#define COLOR_TRACK        RGB(20, 20, 20)       // 3. 量程槽 (纯黑)

// 状态颜色
#define COLOR_NORMAL       RGB(255, 255, 255)    // 正常：亮白 (题目要求)
#define COLOR_CAUTION      RGB(255, 176, 0)      // 警戒：琥珀 (题目要求)
#define COLOR_WARNING      RGB(255, 0, 0)        // 警告：红 (题目要求)

#define COLOR_TEXT         RGB(220, 220, 220)    // 普通文本灰
// #define COLOR_TEXT_HL   RGB(0, 255, 255)      // (移除青色，改用 COLOR_NORMAL 保持一致)

#define COLOR_BTN_START    RGB(0, 120, 60)
#define COLOR_BTN_STOP     RGB(180, 40, 40)
#define COLOR_BTN_INC      RGB(0, 100, 150)
#define COLOR_BTN_DEC      RGB(150, 100, 0)

const double PI = 3.1415926535;

UI::UI() {
    int centerX = 512;
    int startY = 600;
    btnIncRect = { centerX - 130, startY, centerX - 10, startY + 50 };
    btnDecRect = { centerX - 130, startY + 60, centerX - 10, startY + 110 };
    btnStartRect = { centerX + 10, startY, centerX + 130, startY + 50 };
    btnStopRect = { centerX + 10, startY + 60, centerX + 130, startY + 110 };
}

UI::~UI() {
    closegraph();
}

void UI::init() {
    initgraph(1024, 768);
    setbkmode(TRANSPARENT);
}

void UI::drawGauge(int x, int y, int radius, double val, double minVal, double maxVal, const std::wstring& label, int status) {
    // 0. 如果无效，只显示占位符
    if (status == -1) {
        setfillcolor(COLOR_GAUGE_FACE);
        solidcircle(x, y, radius);
        // 无效状态下，文字显示深灰色
        settextcolor(RGB(80, 80, 80));
        settextstyle(24, 0, _T("Consolas"));
        outtextxy(x - 20, y - 10, _T("---"));
        settextstyle(18, 0, _T("微软雅黑"));
        int w = textwidth(label.c_str());
        outtextxy(x - w / 2, y + 25, label.c_str());
        return;
    }

    // 确定颜色 (Bar 和 Text 共用此颜色)
    COLORREF currentColor = COLOR_NORMAL;
    if (status == 1) currentColor = COLOR_CAUTION;
    if (status == 2) currentColor = COLOR_WARNING;

    // 角度计算
    double startAngle = 225;
    double radStart = 225 * PI / 180.0;
    double radEnd = (225 + 270) * PI / 180.0;

    // --- 第1层：表盘底座 ---
    setfillcolor(COLOR_GAUGE_FACE);
    solidcircle(x, y, radius);

    // --- 第2层：量程槽 ---
    int trackRadius = radius * 0.9;
    setfillcolor(COLOR_TRACK);
    solidpie(x - trackRadius, y - trackRadius, x + trackRadius, y + trackRadius, radStart, radEnd);

    // --- 第3层：数值条 ---
    double ratio = (val - minVal) / (maxVal - minVal);
    if (ratio < 0) ratio = 0;
    if (ratio > 1) ratio = 1;

    double currentRadEnd = radStart + (ratio * 270 * PI / 180.0);

    if (ratio > 0.01) {
        setfillcolor(currentColor); // 使用同步的颜色
        solidpie(x - trackRadius, y - trackRadius, x + trackRadius, y + trackRadius, radStart, currentRadEnd);
    }

    // --- 第4层：中心盖板 ---
    int innerRadius = radius * 0.7;
    setfillcolor(COLOR_GAUGE_FACE);
    solidpie(x - innerRadius, y - innerRadius, x + innerRadius, y + innerRadius, 0, 2 * PI);

    // --- 文字显示 ---
    TCHAR str[32];
    _stprintf_s(str, _T("%.0f"), val);

    // 【核心修改1】数值颜色同步
    settextcolor(currentColor);
    settextstyle(32, 0, _T("Consolas"));
    int w = textwidth(str);
    outtextxy(x - w / 2, y - 15, str);

    // 【核心修改2】标签颜色同步 (原为灰色，现改为随状态变化，或者保持白色)
    // 如果你希望标签也变红，就用 currentColor；如果希望标签保持白色以便阅读，就用 COLOR_NORMAL。
    // 根据"颜色同步"的要求，通常是指整个仪表盘变色，这里设为 currentColor。
    settextcolor(COLOR_NORMAL);
    settextstyle(18, 0, _T("微软雅黑"));
    w = textwidth(label.c_str());
    outtextxy(x - w / 2, y + 25, label.c_str());
}

void UI::drawButton(RECT r, const std::wstring& text, COLORREF color, COLORREF hoverColor) {
    setfillcolor(color);
    setlinecolor(WHITE);
    setlinestyle(PS_SOLID, 2);
    fillrectangle(r.left, r.top, r.right, r.bottom);
    rectangle(r.left, r.top, r.right, r.bottom);

    settextcolor(WHITE);
    settextstyle(20, 0, _T("微软雅黑"));
    int w = textwidth(text.c_str());
    int h = textheight(text.c_str());
    outtextxy(r.left + (r.right - r.left - w) / 2, r.top + (r.bottom - r.top - h) / 2, text.c_str());
}

void UI::drawInfoBox(int x, int y, const std::wstring& label, double value, const std::wstring& unit) {
    TCHAR buf[64];
    _stprintf_s(buf, _T("%.1f %s"), value, unit.c_str());

    settextcolor(COLOR_TEXT); // 标签灰
    settextstyle(18, 0, _T("Consolas"));
    outtextxy(x, y, label.c_str());

    // 【核心修改3】燃油数据颜色改为白色 (COLOR_NORMAL)，与 N1/EGT 的正常态保持一致
    settextcolor(COLOR_NORMAL);
    outtextxy(x + 100, y, buf);
}

void UI::draw(double time, const EngineData& data, EngineState state, bool isRunningLightOn, double N1, double N2) {
    setbkcolor(COLOR_BG);
    cleardevice();

    // 标题
    settextcolor(WHITE);
    settextstyle(24, 0, _T("微软雅黑"));
    outtextxy(20, 20, _T("EICAS Display System"));
    TCHAR timeBuf[32];
    _stprintf_s(timeBuf, _T("T+ %.1f s"), time);
    outtextxy(850, 20, timeBuf);

    // --- 1. 计算 N1 状态颜色 ---
    int statusN1_L = 0;
    if (N1 > 120) statusN1_L = 2;
    else if (N1 > 105) statusN1_L = 1;
    else statusN1_L = 0;

    int statusN1_R = 0;
    if (N2 > 120) statusN1_R = 2;
    else if (N2 > 105) statusN1_R = 1;
    else statusN1_R = 0;

    // --- 2. 计算 EGT 状态颜色 ---
    int statusEGT_L = 0;
    int statusEGT_R = 0;

    if (state == EngineState::STARTING) {
        if (data.EGT1_temp > 1000) statusEGT_L = 2;
        else if (data.EGT1_temp > 850) statusEGT_L = 1;

        if (data.EGT2_temp > 1000) statusEGT_R = 2;
        else if (data.EGT2_temp > 850) statusEGT_R = 1;
    }
    else {
        if (data.EGT1_temp > 1100) statusEGT_L = 2;
        else if (data.EGT1_temp > 950) statusEGT_L = 1;

        if (data.EGT2_temp > 1100) statusEGT_R = 2;
        else if (data.EGT2_temp > 950) statusEGT_R = 1;
    }

    // --- 3. 绘制仪表 ---
    drawGauge(300, 200, 110, N1, 0, 125, _T("N1 % (L)"), statusN1_L);
    drawGauge(724, 200, 110, N2, 0, 125, _T("N1 % (R)"), statusN1_R);

    drawGauge(300, 420, 90, data.EGT1_temp, -5, 1200, _T("EGT °C (L)"), statusEGT_L);
    drawGauge(724, 420, 90, data.EGT2_temp, -5, 1200, _T("EGT °C (R)"), statusEGT_R);

    // --- 4. 中央数据 & 状态灯 ---
    int infoX = 430;
    int infoY = 250;
    setlinecolor(COLOR_GAUGE_FACE); // 边框也用深灰
    rectangle(infoX - 10, infoY - 10, infoX + 230, infoY + 130);
    drawInfoBox(infoX, infoY, _T("Fuel Flow"), data.Fuel_V, _T("kg/h"));
    drawInfoBox(infoX, infoY + 30, _T("Fuel Qty"), data.Fuel_C, _T("kg"));

    bool isStart = (state == EngineState::STARTING);
    setfillcolor(isStart ? COLOR_CAUTION : RGB(40, 40, 40));
    solidcircle(infoX + 50, infoY + 80, 8);
    settextcolor(isStart ? COLOR_CAUTION : RGB(100, 100, 100));
    outtextxy(infoX + 70, infoY + 70, _T("STARTING"));

    setfillcolor(isRunningLightOn ? RGB(0, 255, 0) : RGB(40, 40, 40));
    solidcircle(infoX + 50, infoY + 105, 8);
    settextcolor(isRunningLightOn ? RGB(0, 255, 0) : RGB(100, 100, 100));
    outtextxy(infoX + 70, infoY + 95, _T("RUNNING"));

    // --- 5. 底部按钮 ---
    drawButton(btnIncRect, _T("THRUST +"), COLOR_BTN_INC);
    drawButton(btnDecRect, _T("THRUST -"), COLOR_BTN_DEC);
    drawButton(btnStartRect, _T("ENGINE START"), COLOR_BTN_START);
    drawButton(btnStopRect, _T("ENGINE STOP"), COLOR_BTN_STOP);

    FlushBatchDraw();
}

int UI::handleInput() {
    ExMessage msg;
    while (peekmessage(&msg, EM_MOUSE)) {
        if (msg.message == WM_LBUTTONDOWN) {
            int x = msg.x;
            int y = msg.y;
            auto isIn = [&](RECT r) { return x >= r.left && x <= r.right && y >= r.top && y <= r.bottom; };
            if (isIn(btnStartRect)) return 1;
            if (isIn(btnStopRect)) return 2;
            if (isIn(btnIncRect)) return 3;
            if (isIn(btnDecRect)) return 4;
        }
    }
    return 0;
}