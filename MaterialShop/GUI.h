#pragma once
#define _HAS_STD_BYTE 0 
#include <graphics.h>
#include "ShopSystem.h"
#include <string>
#include <vector>

using namespace std;

struct Button {
    int x, y, w, h;
    string text;
    COLORREF color;

    int value = 0;

    bool isClicked(int mx, int my) {  //判断给定的鼠标坐标是否在按钮的可点击区域内
        return (mx >= x && mx <= x + w && my >= y && my <= y + h);
    }

    void draw(bool active) {
        setfillcolor(active ? RGB(255, 140, 0) : color); // active=橙色
        fillrectangle(x, y, x + w, y + h);

        setbkmode(TRANSPARENT);
        settextcolor(WHITE);
        settextstyle(20, 0, "微软雅黑");

        RECT r = { x, y, x + w, y + h };
        drawtext(text.c_str(), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
};

class GUI {
private:
    ShopSystem* shop;

    // 控件列表
    vector<Button> itemButtons;     // 左侧科目按钮
    vector<Button> yearButtons;     // 右侧年份按钮
    vector<Button> paymentButtons; // <<< 重新启用：支付方式按钮列表

    Button payButton;               // 确认支付按钮
    string statusMsg;               // 底部状态栏文字

public:
    GUI(ShopSystem* s);

    void init();    // 初始化
    void update();  // 逻辑更新
    void render();  // 绘图

    void refreshYearButtons(); // 刷新右侧年份
    void initPaymentButtons(); // <<< 新增：刷新支付方式按钮

    bool isRunning;
};