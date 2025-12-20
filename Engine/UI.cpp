#include "UI.h"
#include <cmath>
#include <cstdio>

#define COLOR_BG RGB(30, 30, 35)
#define COLOR_GAUGE_FACE RGB(60, 60, 65)
#define COLOR_TRACK RGB(20, 20, 20)

#define COLOR_NORMAL RGB(255, 255, 255)
#define COLOR_CAUTION RGB(255, 176, 0)
#define COLOR_WARNING RGB(255, 0, 0)

#define COLOR_TEXT RGB(220, 220, 220)

#define COLOR_BTN_START RGB(0, 120, 60)
#define COLOR_BTN_STOP RGB(180, 40, 40)
#define COLOR_BTN_INC RGB(0, 100, 150)
#define COLOR_BTN_DEC RGB(150, 100, 0)

#define COLOR_BTN_FAULT RGB(100, 40, 40)
#define COLOR_CAS_BG RGB(20, 20, 20)
#define COLOR_CAS_TEXT RGB(255, 50, 50)

const double PI = 3.1415926535;

COLORREF getAlertColor(ErrorType error)
{
    switch (error)
    {
    case ErrorType::SENSOR_ALL:
    case ErrorType::SENSOR_FUEL:
    case ErrorType::OVERSPEED_N1_2:
    case ErrorType::OVERHEAT_EGT_2:
    case ErrorType::OVERHEAT_EGT_4:
        return RGB(255, 50, 50);

    case ErrorType::SENSOR_N_TWO:
    case ErrorType::SENSOR_EGT_TWO:
    case ErrorType::LOW_FUEL:
    case ErrorType::OVERSPEED_FUEL:
    case ErrorType::OVERSPEED_N1_1:
    case ErrorType::OVERHEAT_EGT_1:
    case ErrorType::OVERHEAT_EGT_3:
        return RGB(255, 176, 0);

    case ErrorType::SENSOR_N_ONE:
    case ErrorType::SENSOR_EGT_ONE:
        return RGB(200, 200, 200);

    default:
        return RGB(255, 255, 255);
    }
}

std::wstring UI::getErrorString(ErrorType error)
{
    switch (error)
    {
    case ErrorType::SENSOR_N_ONE:
        return _T("ADVISORY: N1 SENSOR FAULT");
    case ErrorType::SENSOR_N_TWO:
        return _T("CAUTION: ENG N1 SENSOR FAIL");
    case ErrorType::SENSOR_EGT_ONE:
        return _T("ADVISORY: EGT SENSOR FAULT");
    case ErrorType::SENSOR_EGT_TWO:
        return _T("CAUTION: ENG EGT SENSOR FAIL");
    case ErrorType::SENSOR_FUEL:
        return _T("WARNING: FUEL SENSOR FAIL");
    case ErrorType::SENSOR_ALL:
        return _T("WARNING: DUAL ENG FAIL");

    case ErrorType::LOW_FUEL:
        return _T("CAUTION: LOW FUEL QTY");
    case ErrorType::OVERSPEED_FUEL:
        return _T("CAUTION: HIGH FUEL FLOW");

    case ErrorType::OVERSPEED_N1_1:
        return _T("CAUTION: N1 OVERSPEED");
    case ErrorType::OVERSPEED_N1_2:
        return _T("WARNING: ENG OVERSPEED");

    case ErrorType::OVERHEAT_EGT_1:
        return _T("CAUTION: EGT OVERHEAT");
    case ErrorType::OVERHEAT_EGT_2:
        return _T("WARNING: EGT CRITICAL");
    case ErrorType::OVERHEAT_EGT_3:
        return _T("CAUTION: EGT OVERHEAT");
    case ErrorType::OVERHEAT_EGT_4:
        return _T("WARNING: EGT CRITICAL");

    default:
        return _T("");
    }
}

UI::UI()
{
    int center_x = 512;
    int start_y = 540;

    btn_inc_rect = {center_x - 130, start_y, center_x - 10, start_y + 45};
    btn_dec_rect = {center_x - 130, start_y + 55, center_x - 10, start_y + 100};
    btn_start_rect = {center_x + 10, start_y, center_x + 130, start_y + 45};
    btn_stop_rect = {center_x + 10, start_y + 55, center_x + 130, start_y + 100};

    int f_btn_w = 120;
    int f_btn_h = 25;
    int gap = 8;
    int grid_start_x = (1024 - (7 * f_btn_w + 6 * gap)) / 2;
    int grid_start_y = 670;

    for (int i = 0; i < 14; i++)
    {
        int row = i / 7;
        int col = i % 7;

        int x = grid_start_x + col * (f_btn_w + gap);
        int y = grid_start_y + row * (f_btn_h + gap);

        fault_buttons[i] = {x, y, x + f_btn_w, y + f_btn_h};
    }

    static const wchar_t *names[] = {
        _T("N1_1 Fail"),       _T("N1_ALL Fail"),  _T("EGT_1 Fail"),    _T("EGT_ALL Fail"), _T("Fuel Fail"),
        _T("All Sens Fail"),   _T("Low Fuel"),     _T("N1 >105"),       _T("N1 >120"),      _T("EGT WARN START"),
        _T("EGT ERROR START"), _T("EGT WARN RUN"), _T("EGT ERROR RUN"), _T("Fuel Leak")};

    for (int i = 0; i < 14; i++)
        fault_labels[i] = names[i];
}

UI::~UI()
{
    closegraph();
}

void UI::init()
{
    initgraph(1024, 768);
    setbkmode(TRANSPARENT);
}

void UI::drawGauge(int x, int y, int radius, double val, double min_val, double max_val, const std::wstring &label,
                   int status)
{
    if (status == -1)
    {
        setfillcolor(COLOR_GAUGE_FACE);
        solidcircle(x, y, radius);
        settextcolor(RGB(80, 80, 80));
        settextstyle(24, 0, _T("Consolas"));
        outtextxy(x - 20, y - 10, _T("---"));
        settextstyle(18, 0, _T("Î¢ÈíÑÅºÚ"));
        int w = textwidth(label.c_str());
        outtextxy(x - w / 2, y + 25, label.c_str());
        return;
    }

    COLORREF current_color = COLOR_NORMAL;
    if (status == 1)
        current_color = COLOR_CAUTION;
    if (status == 2)
        current_color = COLOR_WARNING;

    double rad_start = 225 * PI / 180.0;
    double rad_end = (225 + 270) * PI / 180.0;

    setfillcolor(COLOR_GAUGE_FACE);
    solidcircle(x, y, radius);

    int track_radius = radius * 0.9;
    setfillcolor(COLOR_TRACK);
    solidpie(x - track_radius, y - track_radius, x + track_radius, y + track_radius, rad_start, rad_end);

    double ratio = (val - min_val) / (max_val - min_val);
    if (ratio < 0)
        ratio = 0;
    if (ratio > 1)
        ratio = 1;

    double current_rad_end = rad_start + (ratio * 270 * PI / 180.0);

    if (ratio > 0.01)
    {
        setfillcolor(current_color);
        solidpie(x - track_radius, y - track_radius, x + track_radius, y + track_radius, rad_start, current_rad_end);
    }

    int inner_radius = radius * 0.7;
    setfillcolor(COLOR_GAUGE_FACE);
    solidpie(x - inner_radius, y - inner_radius, x + inner_radius, y + inner_radius, 0, 2 * PI);

    TCHAR str[32];
    _stprintf_s(str, _T("%.0f"), val);

    settextcolor(current_color);
    settextstyle(32, 0, _T("Consolas"));
    int w = textwidth(str);
    outtextxy(x - w / 2, y - 15, str);

    settextcolor(COLOR_NORMAL);
    settextstyle(18, 0, _T("Î¢ÈíÑÅºÚ"));
    w = textwidth(label.c_str());
    outtextxy(x - w / 2, y + 25, label.c_str());
}

void UI::drawButton(RECT r, const std::wstring &text, COLORREF color, COLORREF hover_color)
{
    setfillcolor(color);
    setlinecolor(WHITE);
    setlinestyle(PS_SOLID, 1);
    fillrectangle(r.left, r.top, r.right, r.bottom);
    rectangle(r.left, r.top, r.right, r.bottom);

    settextcolor(WHITE);
    if (r.bottom - r.top < 40)
    {
        settextstyle(14, 0, _T("Î¢ÈíÑÅºÚ"));
    }
    else
    {
        settextstyle(20, 0, _T("Î¢ÈíÑÅºÚ"));
    }

    int w = textwidth(text.c_str());
    int h = textheight(text.c_str());
    outtextxy(r.left + (r.right - r.left - w) / 2, r.top + (r.bottom - r.top - h) / 2, text.c_str());
}

void UI::drawInfoBox(int x, int y, const std::wstring &label, double value, const std::wstring &unit, bool is_valid)
{
    settextcolor(COLOR_TEXT);
    settextstyle(18, 0, _T("Consolas"));
    outtextxy(x, y, label.c_str());

    if (!is_valid)
    {
        settextcolor(RGB(80, 80, 80));
        outtextxy(x + 100, y, _T("---"));
    }
    else
    {
        TCHAR buf[64];
        _stprintf_s(buf, _T("%.1f %s"), value, unit.c_str());
        settextcolor(COLOR_NORMAL);
        outtextxy(x + 100, y, buf);
    }
}

void UI::draw(double time, const EngineData &data, EngineState state, bool is_running_light_on, double n1, double n2,
              const std::vector<ErrorType> &detected_errors)
{

    setbkcolor(COLOR_BG);
    cleardevice();

    settextcolor(WHITE);
    settextstyle(24, 0, _T("Î¢ÈíÑÅºÚ"));
    outtextxy(20, 20, _T("EICAS Display System"));
    TCHAR time_buf[32];
    _stprintf_s(time_buf, _T("T+ %.1f s"), time);
    outtextxy(850, 20, time_buf);

    int status_n1_l = 0;
    if (!data.is_n_sensor_valid[0] && !data.is_n_sensor_valid[1])
    {
        status_n1_l = -1;
    }
    else
    {
        if (n1 > 120)
            status_n1_l = 2;
        else if (n1 > 105)
            status_n1_l = 1;
        else
            status_n1_l = 0;
    }

    int status_n1_r = 0;
    if (!data.is_n_sensor_valid[2] && !data.is_n_sensor_valid[3])
    {
        status_n1_r = -1;
    }
    else
    {
        if (n2 > 120)
            status_n1_r = 2;
        else if (n2 > 105)
            status_n1_r = 1;
        else
            status_n1_r = 0;
    }

    int status_egt_l = 0;
    if (!data.is_egt_sensor_valid[0] && !data.is_egt_sensor_valid[1])
    {
        status_egt_l = -1;
    }
    else
    {
        if (state == EngineState::STARTING)
        {
            if (data.egt1_temp > 1000)
                status_egt_l = 2;
            else if (data.egt1_temp > 850)
                status_egt_l = 1;
        }
        else
        {
            if (data.egt1_temp > 1100)
                status_egt_l = 2;
            else if (data.egt1_temp > 950)
                status_egt_l = 1;
        }
    }

    int status_egt_r = 0;
    if (!data.is_egt_sensor_valid[2] && !data.is_egt_sensor_valid[3])
    {
        status_egt_r = -1;
    }
    else
    {
        if (state == EngineState::STARTING)
        {
            if (data.egt2_temp > 1000)
                status_egt_r = 2;
            else if (data.egt2_temp > 850)
                status_egt_r = 1;
        }
        else
        {
            if (data.egt2_temp > 1100)
                status_egt_r = 2;
            else if (data.egt2_temp > 950)
                status_egt_r = 1;
        }
    }

    drawGauge(300, 200, 110, n1, 0, 125, _T("N1 % (L)"), status_n1_l);
    drawGauge(724, 200, 110, n2, 0, 125, _T("N1 % (R)"), status_n1_r);
    drawGauge(300, 420, 90, data.egt1_temp, -5, 1200, _T("EGT ¡ãC (L)"), status_egt_l);
    drawGauge(724, 420, 90, data.egt2_temp, -5, 1200, _T("EGT ¡ãC (R)"), status_egt_r);

    for (int i = 0; i < 14; i++)
    {
        drawButton(fault_buttons[i], fault_labels[i], COLOR_BTN_FAULT);
    }

    drawCASList(detected_errors);

    int info_x = 430;
    int info_y = 250;
    setlinecolor(COLOR_GAUGE_FACE);

    drawInfoBox(info_x, info_y, _T("Fuel Flow"), data.fuel_v, _T("kg/h"), true);
    drawInfoBox(info_x, info_y + 30, _T("Fuel Qty"), data.fuel_c, _T("kg"), data.is_fuel_valid);

    bool is_start = (state == EngineState::STARTING);
    setfillcolor(is_start ? COLOR_CAUTION : RGB(40, 40, 40));
    solidcircle(info_x + 50, info_y + 80, 8);
    settextcolor(is_start ? COLOR_CAUTION : RGB(100, 100, 100));
    settextstyle(16, 0, _T("Î¢ÈíÑÅºÚ"));
    outtextxy(info_x + 70, info_y + 70, _T("STARTING"));

    setfillcolor(is_running_light_on ? RGB(0, 255, 0) : RGB(40, 40, 40));
    solidcircle(info_x + 50, info_y + 105, 8);
    settextcolor(is_running_light_on ? RGB(0, 255, 0) : RGB(100, 100, 100));
    outtextxy(info_x + 70, info_y + 95, _T("RUNNING"));

    drawButton(btn_inc_rect, _T("THRUST +"), COLOR_BTN_INC);
    drawButton(btn_dec_rect, _T("THRUST -"), COLOR_BTN_DEC);
    drawButton(btn_start_rect, _T("ENGINE START"), COLOR_BTN_START);
    drawButton(btn_stop_rect, _T("ENGINE STOP"), COLOR_BTN_STOP);

    FlushBatchDraw();
}

void UI::drawCASList(const std::vector<ErrorType> &errors)
{
    if (errors.empty())
        return;

    int start_x = 362;
    int start_y = 80;
    int item_height = 35;
    int box_width = 300;

    settextstyle(22, 0, _T("Consolas"));

    for (size_t i = 0; i < errors.size(); i++)
    {
        ErrorType err = errors[i];
        std::wstring msg = getErrorString(err);
        if (msg.empty())
            continue;

        COLORREF color = getAlertColor(err);
        int current_y = start_y + (int)i * item_height;

        setfillcolor(RGB(20, 20, 20));
        setlinecolor(color);
        setlinestyle(PS_SOLID, 2);

        fillrectangle(start_x, current_y, start_x + box_width, current_y + item_height);
        rectangle(start_x, current_y, start_x + box_width, current_y + item_height);

        settextcolor(color);

        int text_w = textwidth(msg.c_str());
        int text_h = textheight(msg.c_str());
        int text_x = start_x + (box_width - text_w) / 2;
        int text_y = current_y + (item_height - text_h) / 2;

        outtextxy(text_x, text_y, msg.c_str());
    }
}

int UI::handleInput()
{
    ExMessage msg;
    while (peekmessage(&msg, EM_MOUSE))
    {
        if (msg.message == WM_LBUTTONDOWN)
        {
            int x = msg.x;
            int y = msg.y;
            auto is_in = [&](RECT r) { return x >= r.left && x <= r.right && y >= r.top && y <= r.bottom; };
            if (is_in(btn_start_rect))
                return 1;
            if (is_in(btn_stop_rect))
                return 2;
            if (is_in(btn_inc_rect))
                return 3;
            if (is_in(btn_dec_rect))
                return 4;

            for (int i = 0; i < 14; i++)
            {
                if (is_in(fault_buttons[i]))
                {
                    return 100 + i;
                }
            }
        }
    }
    return 0;
}