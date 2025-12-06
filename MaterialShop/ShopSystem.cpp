#define _HAS_STD_BYTE 0  
#include "ShopSystem.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <Windows.h> 
#include "Material.h" // 需要引入 Material.h 才能使用 JsonConfiguredMaterial
#include <iomanip> // 用于 cout 输出

using json = nlohmann::json;

// ==========================================
// 辅助函数：UTF-8 转 GBK (防止中文乱码)
// ==========================================
string Utf8ToGbk(const string& str) {
    if (str.empty()) return "";
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    if (len == 0) return str;
    wstring wstr(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], len);
    len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (len == 0) return str;
    string strGbk(len, 0);
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &strGbk[0], len, NULL, NULL);
    if (!strGbk.empty() && strGbk.back() == '\0') strGbk.pop_back();
    return strGbk;
}

// ==========================================
// ShopSystem 实现
// ==========================================

ShopSystem::ShopSystem() {
    currentSelection = nullptr;
    isPaid = false;
    currentPaymentMethod = nullptr;

    // >>> 支付功能初始化：初始化所有支付方式 (解决 LNK2005 冲突的方法定义)
    availablePayments.push_back(new Alipay());
    availablePayments.push_back(new WechatPay());
    availablePayments.push_back(new CampusCard());
    availablePayments.push_back(new AgriBankCard());

    // 默认选中第一个
    if (!availablePayments.empty()) {
        currentPaymentMethod = availablePayments[0];
    }
}

ShopSystem::~ShopSystem() {
    // 释放 Material 内存
    for (auto m : materials) delete m;
    materials.clear();

    // >>> 支付功能清理：释放 PaymentMethod 内存 (解决 LNK2005 冲突的方法定义)
    for (auto p : availablePayments) delete p;
    availablePayments.clear();
}

bool ShopSystem::loadFromJSON(const string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    try {
        json data;
        file >> data;

        for (const auto& item : data) {
            // 1. 读取并转换名称
            string utf8Name = item.value("name", "未知科目");
            string name = Utf8ToGbk(utf8Name);

            double price = item.value("base_price", 0.0);

            // 2. 读取资源映射 (年份 -> 链接)
            map<int, string> resources;
            if (item.contains("resources")) {
                for (auto& element : item["resources"].items()) {
                    try {
                        int year = stoi(element.key()); // "2024" -> 2024
                        string utf8Link = element.value();
                        resources[year] = Utf8ToGbk(utf8Link); // 链接也转换一下防止乱码
                    }
                    catch (...) { continue; }
                }
            }

            // 3. 读取折扣信息
            int dThres = item.value("discount_threshold", 0);
            double dRate = item.value("discount_rate", 1.0);

            // 使用 JsonConfiguredMaterial，确保 Material.h 和 Material.cpp 中有定义
            materials.push_back(new JsonConfiguredMaterial(name, price, resources, dThres, dRate));
        }

        if (!materials.empty()) {
            currentSelection = materials[0]; // 默认选中第一个
        }

        return true;
    }
    catch (...) {
        return false;
    }
}

const vector<Material*>& ShopSystem::getMaterials() const {
    return materials;
}

void ShopSystem::setViewingMaterial(int index) {
    if (index >= 0 && index < materials.size()) {
        currentSelection = materials[index];
    }
}

void ShopSystem::toggleYear(int year) {
    if (!currentSelection) return;
    set<int>& years = shoppingCart[currentSelection];

    if (years.count(year)) {
        years.erase(year);
        if (years.empty()) {
            shoppingCart.erase(currentSelection);
        }
    }
    else {
        years.insert(year);
    }
    isPaid = false;
}

bool ShopSystem::isYearSelected(int year) const {
    if (!currentSelection) return false;
    if (shoppingCart.count(currentSelection)) {
        return shoppingCart.at(currentSelection).count(year) > 0;
    }
    return false;
}

bool ShopSystem::isMaterialInCart(Material* m) const {
    return shoppingCart.count(m) > 0;
}

double ShopSystem::getTotalPrice() {
    double total = 0.0;
    for (auto it = shoppingCart.cbegin(); it != shoppingCart.cend(); ++it) {
        auto mat = it->first;
        const auto& years = it->second;
        total += mat->calculatePrice((int)years.size());
    }
    return total;
}

// >>> 新增：设置选中的支付方式 (解决 LNK2005 冲突的方法定义)
void ShopSystem::setPaymentMethod(int index) {
    if (index >= 0 && index < availablePayments.size()) {
        currentPaymentMethod = availablePayments[index];
    }
}

// >>> 修改：支付逻辑 (现在依赖 currentPaymentMethod，解决 LNK2005 冲突的方法定义)
bool ShopSystem::pay() {
    double price = getTotalPrice();
    if (shoppingCart.empty()) return false;
    if (!currentPaymentMethod) return false; // 必须选择支付方式

    // 调用多态的 pay 接口
    bool success = currentPaymentMethod->pay(price);

    if (success) {
        isPaid = true;
        return true;
    }
    return false;
}

string ShopSystem::getDeliverMessage() {
    if (!isPaid) return "错误：未支付！";
    if (shoppingCart.empty()) return "购物车为空！";

    string msg = "====== 购买清单 ======\n";

    // 遍历购物车中的每个科目
    for (auto const& [mat, years] : shoppingCart) {
        msg += "● " + mat->getName() + "\n";

        // 遍历该科目选中的年份 (反向迭代，从大到小)
        for (auto it = years.rbegin(); it != years.rend(); ++it) {
            int year = *it;
            string link = mat->getLinkByYear(year);
            msg += "   [" + to_string(year) + "] " + link + "\n";
        }
        msg += "----------------------\n";
    }

    // 新增支付信息
    char buf[128];
    sprintf_s(buf, "最终实付: %.2f 元\n", getTotalPrice());
    msg += "支付方式: " + currentPaymentMethod->getName() + "\n";
    msg += string(buf);
    msg += "======================\n";

    // 注意：getDeliverMessage 负责返回处理好的字符串，UI负责在正确的编码环境下显示
    return msg;
}

void ShopSystem::clearCart() {
    shoppingCart.clear(); // 清空 map，价格自然归零
    isPaid = false;       // 重置支付状态
}