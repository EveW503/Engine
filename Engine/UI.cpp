#include "UI.h"
#include <cstdio>
#include <cmath>

// --- 配色方案 (EICAS 风格) ---
#define COLOR_BG           RGB(30, 30, 35)       // 全局深色背景
#define COLOR_PANEL_BG     RGB(50, 50, 55)       // 仪表背景灰
#define COLOR_TEXT         RGB(220, 220, 220)    // 浅灰文字
#define COLOR_TEXT_HL      RGB(0, 255, 255)      // 高亮文字(青色)
#define COLOR_GAUGE_FILL   RGB(0, 200, 0)        // 仪表填充绿
#define COLOR_GAUGE_WARN   RGB(200, 150, 0)      // 仪表警告橙
#define COLOR_BTN_START    RGB(0, 120, 60)       // 启动绿
#define COLOR_BTN_STOP     RGB(180, 40, 40)      // 停止红
#define COLOR_BTN_INC      RGB(0, 100, 150)      // 加推力蓝
#define COLOR_BTN_DEC      RGB(150, 100, 0)      // 减推力橙

const double PI = 3.1415926535;

UI::UI() {
    // 布局优化：将按钮集中在底部中央，形成控制面板
    // 按钮尺寸 120x50，间隔 20
    int centerX = 512;
    int startY = 600;

    // 左侧：推力控制
    btnIncRect = { centerX - 130, startY, centerX - 10, startY + 50 };
    btnDecRect = { centerX - 130, startY + 60, centerX - 10, startY + 110 };

    // 右侧：启停控制
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

// 优化的仪表盘：270度扫描 (左下 135度 -> 右下 405度)
void UI::drawGauge(int x, int y, int radius, double val, double maxVal, const std::wstring& label) {
    // EasyX 角度：0=东, 90=南, 180=西. 顺时针增加.
    // 我们希望从 "西南(135度)" 转到 "东南(45度)"，跨度 270度
    double startRad = 135 * PI / 180.0;
    double sweepRad = 270 * PI / 180.0;

    // 1. 画仪表盘底色 (深灰扇形)
    setfillcolor(COLOR_PANEL_BG);
    solidpie(x - radius, y - radius, x + radius, y + radius, startRad, startRad + sweepRad);

    // 2. 画动态数据 (绿色扇形)
    double ratio = val / maxVal;
    if (ratio < 0) ratio = 0;
    if (ratio > 1) ratio = 1;

    // 警告色逻辑：超过 95% 变橙色
    if (ratio > 0.95) setfillcolor(COLOR_GAUGE_WARN);
    else setfillcolor(COLOR_GAUGE_FILL);

    solidpie(x - radius, y - radius, x + radius, y + radius, startRad, startRad + (sweepRad * ratio));

    // 3. 画中心遮罩 (做成圆环效果，看起来更高级)
    int innerR = radius * 0.7;
    setfillcolor(COLOR_BG);
    solidpie(x - innerR, y - innerR, x + innerR, y + innerR, 0, 2 * PI);

    // 4. 显示数值和标签
    TCHAR str[32];
    settextcolor(COLOR_TEXT_HL);
    settextstyle(24, 0, _T("Consolas")); // 加大字体

    _stprintf_s(str, _T("%.0f"), val);

    // 居中计算
    int w = textwidth(str);
    outtextxy(x - w / 2, y - 10, str);

    settextcolor(COLOR_TEXT);
    settextstyle(18, 0, _T("微软雅黑"));
    w = textwidth(label.c_str());
    outtextxy(x - w / 2, y + 20, label.c_str());
}

// 绘制带边框的按钮
void UI::drawButton(RECT r, const std::wstring& text, COLORREF color, COLORREF hoverColor) {
    setfillcolor(color);
    setlinecolor(WHITE);
    setlinestyle(PS_SOLID, 2);

    fillrectangle(r.left, r.top, r.right, r.bottom);
    rectangle(r.left, r.top, r.right, r.bottom);

    settextcolor(WHITE);
    settextstyle(20, 0, _T("微软雅黑"));

    // 文字居中
    int w = textwidth(text.c_str());
    int h = textheight(text.c_str());
    int x = r.left + (r.right - r.left - w) / 2;
    int y = r.top + (r.bottom - r.top - h) / 2;
    outtextxy(x, y, text.c_str());
}

// 绘制数据信息框
void UI::drawInfoBox(int x, int y, const std::wstring& label, double value, const std::wstring& unit) {
    TCHAR buf[64];
    _stprintf_s(buf, _T("%.2f %s"), value, unit.c_str());

    settextcolor(COLOR_TEXT);
    settextstyle(18, 0, _T("Consolas"));
    outtextxy(x, y, label.c_str());

    settextcolor(COLOR_TEXT_HL);
    outtextxy(x + 100, y, buf);
}

void UI::draw(double time, const EngineData& data, EngineState state, bool isRunningLightOn,double N1,double N2) {
    // 1. 绘制全屏背景
    setbkcolor(COLOR_BG);
    cleardevice();

    // 2. 标题栏
    settextcolor(WHITE);
    settextstyle(30, 0, _T("微软雅黑"));
    outtextxy(20, 20, _T("Twin-Engine EICAS Simulator"));

    // 运行时间 (右上角)
    TCHAR timeBuf[32];
    _stprintf_s(timeBuf, _T("T+ %.2f s"), time);
    outtextxy(850, 20, timeBuf);

    // 3. 绘制仪表区 (两行两列布局)
    // 第一行：N1 转速
    drawGauge(300, 200, 110, N1, 125, _T("N1 (L)"));
    drawGauge(724, 200, 110, N2, 125, _T("N1 (R)"));

    // 第二行：EGT 温度
    drawGauge(300, 420, 90, data.EGT1_temp, 1200, _T("EGT (L)"));
    drawGauge(724, 420, 90, data.EGT2_temp, 1200, _T("EGT (R)"));

    // 4. 中央数据面板 (放在两个引擎中间)
    int infoX = 430;
    int infoY = 250;
    setlinecolor(COLOR_PANEL_BG);
    rectangle(infoX - 10, infoY - 10, infoX + 230, infoY + 120);

    drawInfoBox(infoX, infoY, _T("Fuel Flow"), data.Fuel_V, _T("kg/h"));
    drawInfoBox(infoX, infoY + 30, _T("Fuel Qty"), data.Fuel_C, _T("kg"));

    // 状态灯
    bool isStart = (state == EngineState::STARTING);
    bool isRun = isRunningLightOn;

    setfillcolor(isStart ? RGB(255, 200, 0) : RGB(40, 40, 40));
    solidcircle(infoX + 50, infoY + 80, 10);
    settextcolor(isStart ? RGB(255, 200, 0) : RGB(100, 100, 100));
    outtextxy(infoX + 70, infoY + 70, _T("STARTING"));

    setfillcolor(isRun ? RGB(0, 255, 0) : RGB(40, 40, 40));
    solidcircle(infoX + 50, infoY + 105, 10);
    settextcolor(isRun ? RGB(0, 255, 0) : RGB(100, 100, 100));
    outtextxy(infoX + 70, infoY + 95, _T("RUNNING"));

    // 5. 绘制控制面板 (底部)
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

            // 辅助 Lambda：判断点是否在矩形内
            auto isIn = [&](RECT r) { return x >= r.left && x <= r.right && y >= r.top && y <= r.bottom; };

            if (isIn(btnStartRect)) return 1;
            if (isIn(btnStopRect))  return 2;
            if (isIn(btnIncRect))   return 3;
            if (isIn(btnDecRect))   return 4;
        }
    }
    return 0;
}