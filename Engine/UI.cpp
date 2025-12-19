#include "UI.h"
#include <cstdio>
#include <cmath>

// 简单的颜色定义
#define COLOR_BG BLACK
#define COLOR_TEXT WHITE
#define COLOR_GAUGE_BG RGB(50, 50, 50)
#define COLOR_GAUGE_FILL GREEN
#define COLOR_BTN_START RGB(0, 100, 200)
#define COLOR_BTN_STOP RGB(200, 50, 50)

UI::UI() {
    // 定义按钮位置
    btnStartRect = { 300, 500, 420, 550 }; // Left, Top, Right, Bottom
    btnStopRect = { 450, 500, 570, 550 };
}

UI::~UI() {
    closegraph();
}

void UI::init() {
    initgraph(1024, 768); // 创建窗口
    setbkmode(TRANSPARENT); // 文字背景透明
}

// 辅助函数：将角度转为弧度
const double PI = 3.1415926535;

void UI::drawGauge(int x, int y, int radius, double val, double maxVal, const std::wstring& label) {
    // 1. 画背景扇形 (0 ~ 210度)
    // EasyX 的 pie 函数参数是：left, top, right, bottom, startAngle, endAngle
    // 这里的角度是弧度制。0度是3点钟方向，顺时针为负（EasyX坐标系问题需注意，通常逆时针为正）
    // 为了简单，我们用 solidpie 画一个灰色的底
    setfillcolor(COLOR_GAUGE_BG);
    solidpie(x - radius, y - radius, x + radius, y + radius, 0, 210 * PI / 180);

    // 2. 画动态数据扇形
    // 计算当前值对应的角度
    double percentage = val / maxVal;
    if (percentage > 1.0) percentage = 1.0;
    if (percentage < 0.0) percentage = 0.0;

    double angle = percentage * 210.0; // 0 ~ 210

    setfillcolor(COLOR_GAUGE_FILL);
    // 注意：EasyX 的角度起始也是弧度。
    solidpie(x - radius, y - radius, x + radius, y + radius, 0, angle * PI / 180);

    // 3. 显示文字数值
    TCHAR valStr[32];
    _stprintf_s(valStr, _T("%.0f"), val);

    settextcolor(COLOR_TEXT);
    settextstyle(20, 0, _T("Consolas"));
    outtextxy(x - 20, y + radius + 10, valStr);
    outtextxy(x - 20, y - 20, label.c_str());
}

void UI::drawButton(int x, int y, int w, int h, const std::wstring& text, COLORREF color) {
    setfillcolor(color);
    fillrectangle(x, y, x + w, y + h);

    settextcolor(WHITE);
    settextstyle(25, 0, _T("微软雅黑"));
    // 简单居中
    outtextxy(x + 10, y + 10, text.c_str());
}

void UI::draw(double time, const EngineData& data, EngineState state) {
    cleardevice(); // 清屏

    // 1. 绘制 N1 表盘 (左发、右发)
    // 参数：圆心x, 圆心y, 半径, 当前值, 最大值, 标签
    drawGauge(200, 200, 100, data.N1_rpm, 45000, _T("N1 Left"));
    drawGauge(600, 200, 100, data.N2_rpm, 45000, _T("N1 Right"));

    // 2. 绘制 EGT 表盘 (放在中间下面一点)
    drawGauge(300, 350, 80, data.EGT1_temp, 1200, _T("EGT Left"));
    drawGauge(500, 350, 80, data.EGT2_temp, 1200, _T("EGT Right"));

    // 3. 绘制数字信息 (Fuel Flow, Time)
    TCHAR buf[64];
    settextcolor(WHITE);
    settextstyle(20, 0, _T("Consolas"));

    _stprintf_s(buf, _T("Time: %.2f s"), time);
    outtextxy(800, 20, buf);

    _stprintf_s(buf, _T("Fuel Flow: %.2f"), data.Fuel_V);
    outtextxy(350, 600, buf);

    _stprintf_s(buf, _T("Fuel Qty: %.0f"), data.Fuel_C);
    outtextxy(350, 630, buf);

    // 4. 绘制状态灯 (Start / Run)
    // 简单画两个矩形框，根据状态改变填充色
    bool isStarting = (state == EngineState::STARTING);
    bool isRunning = (state == EngineState::RUNNING);

    setfillcolor(isStarting ? YELLOW : RGB(50, 50, 50));
    fillrectangle(50, 50, 150, 90); // Start 窗口
    settextcolor(BLACK);
    outtextxy(60, 60, _T("START"));

    setfillcolor(isRunning ? GREEN : RGB(50, 50, 50));
    fillrectangle(160, 50, 260, 90); // Run 窗口
    outtextxy(180, 60, _T("RUN"));

    // 5. 绘制交互按钮
    drawButton(btnStartRect.left, btnStartRect.top, 120, 50, _T("START"), COLOR_BTN_START);
    drawButton(btnStopRect.left, btnStopRect.top, 120, 50, _T("STOP"), COLOR_BTN_STOP);

    FlushBatchDraw(); // 显存交换
}

int UI::handleInput() {
    ExMessage msg;
    // 使用 peekmessage 不阻塞
    while (peekmessage(&msg, EM_MOUSE)) {
        if (msg.message == WM_LBUTTONDOWN) {
            // 检查是否点击了 Start
            if (msg.x >= btnStartRect.left && msg.x <= btnStartRect.right &&
                msg.y >= btnStartRect.top && msg.y <= btnStartRect.bottom) {
                return 1;
            }
            // 检查是否点击了 Stop
            if (msg.x >= btnStopRect.left && msg.x <= btnStopRect.right &&
                msg.y >= btnStopRect.top && msg.y <= btnStopRect.bottom) {
                return 2;
            }
        }
    }
    return 0;
}