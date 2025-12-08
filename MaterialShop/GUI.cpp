#define _HAS_STD_BYTE 0 
#include "GUI.h"
#include <algorithm> 
#include <conio.h>
#include <iostream>
#include <string>
#include <cmath> // 引入 cmath 用于 ceil
#include <sstream>

// 全局布局配置：方便统一调整 GUI 拉长与日志区高度
static const int kBottomLogHeight = 420; // 日志区高度（增大以“往下”）
static const int kMinWindowHeight = 800;
static const int kTopAreaExtra = 80; // 顶部/中部额外保留空间，避免控件覆盖
static const int kLogTopShift = 50; // 日志区上界向下偏移（正值会把日志区上界下移）

GUI::GUI(ShopSystem* s) : shop(s) {
    isRunning = true;
    statusMsg = "欢迎使用！请点击左侧科目，勾选需要的年份（支持跨科目多选）。";
}

// ==========================================
// 初始化界面
// ==========================================
void GUI::init() {
    // 1. 动态计算窗口高度（确保包含左侧项、支付区与底部日志区）
    int count = (int)shop->getMaterials().size();
    // 基于项目数量计算内容区域高度，并确保至少能容纳底部日志区
    int contentH = 100 + count * 60 + kTopAreaExtra;
    int winH = max(kMinWindowHeight, contentH + kBottomLogHeight);

    // 2. 初始化大窗口
    initgraph(1280, winH);
    BeginBatchDraw();
    setbkcolor(RGB(245, 245, 245));

    // 3. 初始化左侧科目按钮
    auto mats = shop->getMaterials();
    int y = 60;
    for (auto m : mats) {
        itemButtons.push_back({ 50, y, 400, 45, m->getName(), RGB(70, 130, 180), 0 });
        y += 60;
    }

    // 使用统一变量表示底部日志区高度，便于后续布局计算（增强健壮性）
    const int bottomLogHeight = kBottomLogHeight;
    // 计算日志区上界（上界下移 kLogTopShift 像素）
    int logTop = getheight() - bottomLogHeight + kLogTopShift;

    // 关键修改：Pay Button Y 坐标基于窗口高度和底部日志区计算，避免硬编码导致的覆盖
    int fixed_payButton_y = logTop - 120; // 保留上方 120px 空间用于价格/支付区

    // 5. 初始化确认支付按钮 
    payButton = { 600, fixed_payButton_y, 300, 60, "确认支付 (Pay)", RGB(34, 139, 34), 0 };

    // 4. 初始化支付方式按钮列表
    initPaymentButtons();
}

// ==========================================
// 辅助函数：初始化/刷新支付方式按钮
// 固定的支付区位置
// ==========================================
void GUI::initPaymentButtons() {
    paymentButtons.clear();

    auto payments = shop->getAvailablePayments();
    // 将年份列整体向左移动 80 像素（从 600 -> 520）以靠近科目列表
    int startX = 600;

    int btnW = 100;
    int btnH = 40;
    int gap = 10;

    // 1. 计算 payment buttons 占据的总行数
    int paymentRows = (int)ceil((double)payments.size() / 3);

    // 2. 反推 startY: 这是支付按钮区域的起始Y坐标
    // payButton.y = startY + paymentRows * (btnH + gap) + 间隔

    // **********************************************
    // 关键修改：提取和修改间隔值
    // **********************************************
    int verticalGap = 90;

    int startY = payButton.y - verticalGap - paymentRows * (btnH + gap);

    // 3. 生成 Payment Buttons (使用计算出的 startY)
    for (size_t i = 0; i < payments.size(); i++) {
        PaymentMethod* p = payments[i];
        int x = startX + (i % 3) * (btnW + gap); // 一行 3 个
        int y = startY + (i / 3) * (btnH + gap); // 换行

        // value 存索引 i，用于 ShopSystem::setPaymentMethod
        paymentButtons.push_back({ x, y, btnW, btnH, p->getName(), RGB(150, 150, 150), (int)i });
    }

    // 3. 重新定位 Pay Button (这里确保 payButton.y 基于 payment 按钮区域动态计算)
    payButton.y = startY + paymentRows * (btnH + gap) + verticalGap; // 使用新的 verticalGap
}


// ==========================================
// 辅助函数：刷新右侧年份按钮 (不变)
// ==========================================
void GUI::refreshYearButtons() {
    yearButtons.clear();

    Material* m = shop->getViewingMaterial();
    if (!m) return;

    vector<int> availYears = m->getAvailableYears();

    int startX = 600;
    int startY = 60;
    int btnW = 70;
    int btnH = 40;
    int gap = 10;
    int col = 0, row = 0;

    for (int year : availYears) {
        int x = startX + col * (btnW + gap);
        int y = startY + row * (btnH + gap);

        yearButtons.push_back({ x, y, btnW, btnH, to_string(year), RGB(150, 150, 150), year });

        col++;
        if (col >= 3) { col = 0; row++; }
    }
}


// ==========================================
// 逻辑更新
// ==========================================
void GUI::update() {
    ExMessage msg;
    while (peekmessage(&msg, EM_MOUSE)) {
        if (msg.message == WM_LBUTTONDOWN) {

            // 1. 处理左侧科目点击
            for (size_t i = 0; i < itemButtons.size(); i++) {
                if (itemButtons[i].isClicked(msg.x, msg.y)) {
                    shop->setViewingMaterial(i);
                    refreshYearButtons();
                    initPaymentButtons();
                    statusMsg = "正在查看: [" + shop->getViewingMaterial()->getName() + "]";
                }
            }

            // 2. 处理右侧年份点击
            for (auto& btn : yearButtons) {
                if (btn.isClicked(msg.x, msg.y)) {
                    shop->toggleYear(btn.value);
                }
            }

            // 3. 处理支付方式选择
            for (auto& btn : paymentButtons) {
                if (btn.isClicked(msg.x, msg.y)) {
                    // 切换选中的支付方式
                    shop->setPaymentMethod(btn.value);
                    statusMsg = "支付方式已切换为：" + shop->getCurrentPaymentMethod()->getName();
                }
            }

            // 4. 处理支付按钮点击 (使用新的 pay() 接口)
            if (payButton.isClicked(msg.x, msg.y)) {
                if (shop->getTotalPrice() <= 0.01) {
                    statusMsg = "错误：购物车为空，请先选择资料！";
                }
                else if (!shop->getCurrentPaymentMethod()) {
                    statusMsg = "错误：请先选择支付方式！";
                }
                else {
                    // 尝试支付
                    if (shop->pay()) {
                        // 1. 获取原始发货单/收据
                        string receipt = shop->getDeliverMessage();

                        std::cout << "\n========== 支付成功！资料清单 ==========" << endl;
                        std::cout << receipt << endl;
                        std::cout << "========================================" << endl;

                        // ====================================================
                        // ★★★ 新增：5秒倒计时逻辑 ★★★
                        // ====================================================
                        for (int i = 5; i >= 1; i--) {
                            // 动态更新状态栏：显示收据 + 倒计时提示
                            statusMsg = receipt;
                            statusMsg += "\n----------------------------------------\n";
                            statusMsg += "[系统] 支付成功！\n";
                            statusMsg += "[倒计时] " + to_string(i) + " 秒后弹出操作菜单...";

                            // 强制渲染：让用户实时看到 5, 4, 3... 的变化
                            render();

                            // 暂停 1 秒
                            Sleep(1000);
                        }
                        // ====================================================

                        // 倒计时结束后，将状态栏重置为纯收据（准备追加新内容）
                        statusMsg = receipt;
                        render(); // 确保弹窗前背景是干净的收据

                        // 弹出选择框
                        int choice = MessageBox(
                            GetHWnd(),
                            "支付成功！\n\n点击【是】清空购物车并继续购买。\n点击【否】退出程序。",
                            "支付确认",
                            MB_YESNO | MB_ICONQUESTION | MB_TOPMOST
                        );

                        if (choice == IDYES) {
                            // --- 用户选择继续 ---
                            shop->clearCart();     // 1. 清空数据
                            refreshYearButtons();  // 2. 刷新按钮状态

                            // 3. 在下方追加提示（保留上面的收据不被清空）
                            statusMsg += "\n----------------------------------------\n";
                            statusMsg += "[系统] 购物车金额已清空。\n";
                            statusMsg += "[系统] 请继续点击左侧科目进行选购...";
                        }
                        else {
                            // --- 用户选择退出 ---
                            isRunning = false;
                            exit(0);
                        }
                    }
                    else {
                        statusMsg = "支付失败：请检查账户余额或其他支付条件！";
                    }
                }
            }
        }
    }
}

// ==========================================
// 界面渲染
// ==========================================
void GUI::render() {
    cleardevice();

    // 统一使用变量表示底部日志区高度，避免硬编码
    const int bottomLogHeight = kBottomLogHeight;
    // 计算日志区上界（上界下移 kLogTopShift 像素）
    int h = getheight();
    int w = getwidth();
    int logTop = h - bottomLogHeight + kLogTopShift;

    // 1. 绘制顶部标题 
    settextcolor(BLACK);
    settextstyle(26, 0, "微软雅黑");
    outtextxy(50, 15, "科目列表 (购物车)");
    outtextxy(600, 15, "年份配置");

    // 关键修改：将 支付方式标题 的 Y 坐标基于计算出的 paymentButtons[0].y
    int paymentTitleY = 340;
    if (!paymentButtons.empty()) {
        // 将标题放在第一个支付按钮的上方 40px 处
        paymentTitleY = paymentButtons[0].y - 40;
    }
    outtextxy(600, paymentTitleY, "支付方式");

    outtextxy(950, 15, "优惠政策");

    // 2. 绘制左侧科目按钮 (不变)
    auto materials = shop->getMaterials();
    for (size_t i = 0; i < itemButtons.size(); i++) {
        Material* m = materials[i];
        bool isViewing = (shop->getViewingMaterial() == m);
        bool isInCart = shop->isMaterialInCart(m);

        if (isViewing) {
            itemButtons[i].color = RGB(255, 140, 0);
        }
        else if (isInCart) {
            itemButtons[i].color = RGB(60, 179, 113);
        }
        else {
            itemButtons[i].color = RGB(70, 130, 180);
        }
        itemButtons[i].draw(false);

        if (isInCart) {
            setfillcolor(RED);
            solidcircle(itemButtons[i].x + itemButtons[i].w - 20, itemButtons[i].y + 22, 5);
        }
    }

    // 3. 绘制中间操作区 (年份选择)
    for (auto& btn : yearButtons) {
        bool isSelected = shop->isYearSelected(btn.value);
        btn.draw(isSelected);
    }

    // 4. 绘制支付方式按钮
    PaymentMethod* currentMethod = shop->getCurrentPaymentMethod();
    for (auto& btn : paymentButtons) {
        // 判断是否为当前选中 (高亮显示)
        bool isSelected = (currentMethod && currentMethod->getName() == btn.text);
        btn.draw(isSelected);
    }

    // 5. 绘制价格与支付区
    // 价格背景框 (位置基于固定的 payButton.y)
    setfillcolor(WHITE);
    // payButton.y 是基于窗口和 payment 区域计算的，这里的绘制位置也是基于该值
    fillrectangle(600, payButton.y - 80, payButton.x + payButton.w, payButton.y - 20);

    // 价格文字
    settextcolor(RED);
    settextstyle(22, 0, "微软雅黑");
    char buf[64];
    sprintf_s(buf, "购物车总价: %.2f 元", shop->getTotalPrice());
    outtextxy(620, payButton.y - 60, buf);

    // 支付按钮 (位置已固定/动态计算)
    payButton.draw(false);

    // 6. 绘制最右侧折扣信息栏 (不变)
    // ... (保持不变)
    int infoX = 950;
    int infoY = 60;
    int infoW = 280;

    setfillcolor(RGB(235, 235, 235));
    // 使用 logTop 避免覆盖底部日志区（将原来 getheight()-bottomLogHeight 替换为 logTop）
    solidrectangle(infoX, 50, infoX + infoW, logTop - 20);

    setbkmode(TRANSPARENT);
    int currentY = infoY;

    for (auto m : materials) {
        settextcolor(BLACK);
        settextstyle(20, 0, "微软雅黑");
        outtextxy(infoX + 10, currentY, m->getName().c_str());

        string rule = m->getDiscountRule();

        if (rule == "无折扣") {
            settextcolor(RGB(150, 150, 150));
        }
        else {
            settextcolor(RGB(220, 20, 60));
        }

        settextstyle(18, 0, "Consolas");
        outtextxy(infoX + 10, currentY + 25, rule.c_str());

        setlinecolor(RGB(200, 200, 200));
        line(infoX + 10, currentY + 50, infoX + infoW - 10, currentY + 50);

        currentY += 65;
    }

    // 在材料折扣列表下方显示各支付方式的优惠/手续费说明
    currentY += 8; // 额外间距
    settextcolor(BLACK);
    settextstyle(20, 0, "微软雅黑");
    outtextxy(infoX + 10, currentY, "支付方式优惠：");
    currentY += 30;

    // 列出所有支付方式及其政策
    auto payments = shop->getAvailablePayments();
    settextstyle(16, 0, "Consolas");
    int policyMaxW = infoW - 20; // 文本可用宽度（左右各留 10px）
    int lineGap = 18;            // 政策行高（像素），根据字体大小微调
    for (auto p : payments) {
        string line = p->getName() + string(" - ") + p->getPolicy();
        outtextxy(infoX + 10, currentY, line.c_str());
        currentY += 22;
        if (currentY > logTop - 40) break; // 防止越界覆盖日志区（使用 logTop）
    }

PAYMENT_DRAW_DONE: ;

    // 6.5 绘制购物车面板（在左侧科目下方，靠近底部日志区之上）
    {
        int cartX = 50;
        int cartW = 400;
        int cartH = 170;

        // 计算安全的 cartY：不应覆盖左侧的最后一个科目按钮，也不应进入日志区
        int defaultCartY = logTop - cartH - 10; // 在日志区之上留 10px 间隔（使用 logTop）

        int cartY = defaultCartY;
        if (!itemButtons.empty()) {
            const Button& lastItem = itemButtons.back();
            int lastItemBottom = lastItem.y + lastItem.h;
            // 如果最后一个科目按钮底部距离默认 cartY 太近，则把 cartY 向下移动，保证间隙
            cartY = max(defaultCartY, lastItemBottom + 10);
            // 如果移动后 cartY 会进入日志区（即超出 allowed area），则调整 cartH 以适配
            if (cartY + cartH > logTop - 5) {
                cartH = max(60, (logTop - 5) - cartY); // 最小高度 60
            }
        }

        // 背景
        setfillcolor(RGB(245, 245, 245));
        solidrectangle(cartX, cartY, cartX + cartW, cartY + cartH);

        // 标题
        settextcolor(BLACK);
        settextstyle(18, 0, "微软雅黑");
        outtextxy(cartX + 10, cartY + 8, "购物车内容：");

        // 列表内容
        settextcolor(BLACK);
        settextstyle(16, 0, "Consolas");

        const auto& cart = shop->getShoppingCart();
        int lineY = cartY + 36;
        if (cart.empty()) {
            outtextxy(cartX + 12, lineY, "(空)");
        }
        else {
            for (auto const& kv : cart) {
                Material* m = kv.first;
                const auto& years = kv.second;

                // 组装年份字符串
                string ys;
                if (years.empty()) ys = "(未选择年份)";
                else {
                    bool first = true;
                    for (int yv : years) {
                        if (!first) ys += ",";
                        ys += to_string(yv);
                        first = false;
                    }
                }

                string line = "- " + m->getName() + " [" + to_string((int)years.size()) + "] : " + ys;
                outtextxy(cartX + 12, lineY, line.c_str());
                lineY += 20;
                if (lineY > cartY + cartH - 20) break; // 超出面板则截断
            }
        }
    }

    // 7. 绘制底部日志区（使用 logTop 作为上界，避免硬编码）
    setfillcolor(RGB(30, 30, 30));
    fillrectangle(0, logTop, w, h);

    settextcolor(GREEN);
    settextstyle(18, 0, "Consolas");
    RECT r = { 20, logTop + 10, w - 20, h - 10 };
    drawtext(statusMsg.c_str(), &r, DT_LEFT | DT_WORDBREAK);

    // 8. 提交绘制
    FlushBatchDraw();
    Sleep(15);
}