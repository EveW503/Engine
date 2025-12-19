#include "UI.h"
#include <cstdio>
#include <cmath>

// --- 航空仪表配色方案 (优化版) ---
#define COLOR_BG           RGB(30, 30, 35)       // 1. 全局背景 (深黑)
#define COLOR_GAUGE_FACE   RGB(60, 60, 65)       // 2. 表盘底座 (稍亮灰)，与背景区分
#define COLOR_TRACK        RGB(20, 20, 20)       // 3. 量程槽 (纯黑)，让数值条更显眼
// 状态颜色
#define COLOR_NORMAL       RGB(255, 255, 255)    // 正常：亮白
#define COLOR_CAUTION      RGB(255, 176, 0)      // 警戒：琥珀
#define COLOR_WARNING      RGB(255, 0, 0)        // 警告：红
// 其他颜色保持不变...
#define COLOR_TEXT         RGB(220, 220, 220)
#define COLOR_TEXT_HL      RGB(0, 255, 255)
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
        solidcircle(x, y, radius); // 画个灰底表示仪表存在但无数据
        settextcolor(RGB(100, 100, 100));
        settextstyle(24, 0, _T("Consolas"));
        outtextxy(x - 20, y - 10, _T("OFF"));
        outtextxy(x - textwidth(label.c_str()) / 2, y + 25, label.c_str());
        return;
    }

    // 确定数值条颜色
    COLORREF barColor = COLOR_NORMAL;
    if (status == 1) barColor = COLOR_CAUTION;
    if (status == 2) barColor = COLOR_WARNING;

    // 角度定义：
    // 题目要求 0~210度。通常仪表习惯：左下(135度/7点钟)为起点，顺时针扫过270度。
    // 这里我们设定：起点 225度 (左下)，扫过 270度 到 -45度 (右下)。
    double startAngle = 225;
    double sweepAngle = -270; // EasyX 顺时针可能需要负值或者特定处理，这里用正统弧度计算

    // 转换为 EasyX 弧度 (EasyX: X轴正向0，顺时针正向)
    // 我们手动计算弧度：225度 = 1.25 PI (左下)。
    // 结束角 = 225 + 270 = 495 = 135度 (右下)。
    // 注意：EasyX solidpie 的参数是 stangle, endangle。
    // 为了画出从左下顺时针到右下的扇形：
    double radStart = 225 * PI / 180.0;
    double radEnd = (225 + 270) * PI / 180.0;

    // --- 第1层：表盘底座 (Gauge Face) ---
    // 画一个实心圆，区别于全局背景
    setfillcolor(COLOR_GAUGE_FACE);
    solidcircle(x, y, radius);

    // --- 第2层：量程槽 (Track) ---
    // 画一个深黑色的扇形，作为进度条的轨道
    // 稍微比底座小一点，留出边框
    int trackRadius = radius * 0.9;
    setfillcolor(COLOR_TRACK);
    solidpie(x - trackRadius, y - trackRadius, x + trackRadius, y + trackRadius, radStart, radEnd);

    // --- 第3层：数值条 (Value Bar) ---
    // 计算比例
    double ratio = (val - minVal) / (maxVal - minVal);
    if (ratio < 0) ratio = 0;
    if (ratio > 1) ratio = 1;

    // 计算数值条的结束角度
    double currentRadEnd = radStart + (ratio * 270 * PI / 180.0);

    // 只有当 ratio > 0 时才画，避免画出一条线
    if (ratio > 0.01) {
        setfillcolor(barColor);
        solidpie(x - trackRadius, y - trackRadius, x + trackRadius, y + trackRadius, radStart, currentRadEnd);
    }

    // --- 第4层：中心盖板 (Center Cap) ---
    // 盖住扇形中心，形成圆环
    int innerRadius = radius * 0.7;
    setfillcolor(COLOR_GAUGE_FACE); // 颜色与底座一致，或者用 COLOR_BG 镂空
    solidpie(x - innerRadius, y - innerRadius, x + innerRadius, y + innerRadius, 0, 2 * PI);

    // --- 文字显示 ---
    TCHAR str[32];
    _stprintf_s(str, _T("%.0f"), val);

    settextcolor(barColor); // 文字颜色与状态同步
    settextstyle(32, 0, _T("Consolas"));
    int w = textwidth(str);
    outtextxy(x - w / 2, y - 15, str);

    settextcolor(RGB(180, 180, 180));
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
    settextcolor(RGB(200, 200, 200));
    settextstyle(18, 0, _T("Consolas"));
    outtextxy(x, y, label.c_str());
    settextcolor(COLOR_NORMAL);
    outtextxy(x + 100, y, buf);
}

// 【核心修改】Draw 主函数包含告警逻辑判断
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
    // 逻辑：N1 > 120 红, N1 > 105 琥珀, 否则 白
    int statusN1_L = 0;
    if (N1 > 120) statusN1_L = 2;       // Red
    else if (N1 > 105) statusN1_L = 1;  // Amber
    else statusN1_L = 0;                // White

    int statusN1_R = 0;
    if (N2 > 120) statusN1_R = 2;
    else if (N2 > 105) statusN1_R = 1;
    else statusN1_R = 0;

    // --- 2. 计算 EGT 状态颜色 ---
    // 启动过程 (STARTING): >1000 红, >850 琥珀
    // 稳态过程 (RUNNING/其他): >1100 红, >950 琥珀
    int statusEGT_L = 0;
    int statusEGT_R = 0;

    if (state == EngineState::STARTING) {
        // 左发
        if (data.EGT1_temp > 1000) statusEGT_L = 2;
        else if (data.EGT1_temp > 850) statusEGT_L = 1;

        // 右发
        if (data.EGT2_temp > 1000) statusEGT_R = 2;
        else if (data.EGT2_temp > 850) statusEGT_R = 1;
    }
    else {
        // 稳态 limits
        if (data.EGT1_temp > 1100) statusEGT_L = 2;
        else if (data.EGT1_temp > 950) statusEGT_L = 1;

        if (data.EGT2_temp > 1100) statusEGT_R = 2;
        else if (data.EGT2_temp > 950) statusEGT_R = 1;
    }

    // --- 3. 绘制仪表 ---
    // N1 量程 0-125%
    drawGauge(300, 200, 110, N1, 0, 125, _T("N1 % (L)"), statusN1_L);
    drawGauge(724, 200, 110, N2, 0, 125, _T("N1 % (R)"), statusN1_R);

    // EGT 量程 -5 到 1200 C
    drawGauge(300, 420, 90, data.EGT1_temp, -5, 1200, _T("EGT °C (L)"), statusEGT_L);
    drawGauge(724, 420, 90, data.EGT2_temp, -5, 1200, _T("EGT °C (R)"), statusEGT_R);

    // --- 4. 中央数据 & 状态灯 ---
    int infoX = 430;
    int infoY = 250;
    setlinecolor(COLOR_BG);
    rectangle(infoX - 10, infoY - 10, infoX + 230, infoY + 130);
    drawInfoBox(infoX, infoY, _T("Fuel Flow"), data.Fuel_V, _T("kg/h"));
    drawInfoBox(infoX, infoY + 30, _T("Fuel Qty"), data.Fuel_C, _T("kg"));

    bool isStart = (state == EngineState::STARTING);
    setfillcolor(isStart ? COLOR_CAUTION : RGB(40, 40, 40)); // 启动用琥珀色/黄色
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
            if (isIn(btnStopRect))  return 2;
            if (isIn(btnIncRect))   return 3;
            if (isIn(btnDecRect))   return 4;
        }
    }
    return 0;
}