#include "ShopSystem.h"
#include "GUI.h"
#include <Windows.h>
int main() {
    // 1. 实例化系统
    ShopSystem system;
    // 启动时弹出游戏规则对话框
    MessageBoxW(NULL,
        L"欢迎来到期末资料获取系统！\n\n规则：\n1. 在左侧选择科目和年份。\n2. 在购物车中选择要购买的科目。\n3. 选择支付方式并付款。\n4. 付款后将显示下载链接。\n\n祝你使用愉快！",
        L"游戏规则",
        MB_OK | MB_ICONINFORMATION);

    // 2. 加载JSON配置文件
    // 注意：如果直接在VS里运行，文件要放在项目目录下
    if (!system.loadFromJSON("data.json")) {
        MessageBox(NULL, "无法找到 data.json 配置文件！\n请确认文件位于项目文件夹中。", "启动错误", MB_OK | MB_ICONERROR);
        return -1;
    }

    // 3. 启动GUI
    GUI gui(&system);
    gui.init();

    // 4. 主循环
    while (gui.isRunning) {
        gui.update();
        gui.render();
    }

	EndBatchDraw();

    closegraph();
    return 0;
}