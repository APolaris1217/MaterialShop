#pragma once
#define _HAS_STD_BYTE 0 
#include "Material.h"
#include "Payment.h" 
#include <vector>
#include <string>
#include <set>
#include <map> 

using namespace std;

class ShopSystem {
private:
    vector<Material*> materials;

    Material* currentSelection;
    map<Material*, set<int>> shoppingCart;

    bool isPaid;

    PaymentMethod* currentPaymentMethod;

    vector<PaymentMethod*> availablePayments;

    // 记录上次支付的实际支付金额（用于发货单显示）
    double lastPaidAmount = 0.0;

public:
    ShopSystem();
    ~ShopSystem(); // 需要修改析构函数来释放 PaymentMethod 内存

    bool loadFromJSON(const string& filename);
    const vector<Material*>& getMaterials() const;

    // 设置当前查看哪个科目（不清除购物车）
    void setViewingMaterial(int index);
    Material* getViewingMaterial() const { return currentSelection; }

    // 修改当前查看科目的年份选择
    void toggleYear(int year);

    // 判断某一年是否被选中（用于UI高亮）
    bool isYearSelected(int year) const;
    bool isMaterialInCart(Material* m) const;

    // 计算购物车总价（遍历 map）
    double getTotalPrice();

    // 计算在选中支付方式下的最终应付金额（含手续费/折扣）
    double getTotalPriceWithPayment();

    // 支付逻辑 (现在不需要 inputMoney，依赖选中的方式)
    bool pay();

    // 设置选中的支付方式
    void setPaymentMethod(int index);
    // 获取支付方式列表
    const vector<PaymentMethod*>& getAvailablePayments() const { return availablePayments; }
    // 获取当前选中的支付方式
    PaymentMethod* getCurrentPaymentMethod() const { return currentPaymentMethod; }

    // 生成包含所有科目的发货单
    string getDeliverMessage();

    // 获取购物车内容（用于 UI 显示）
    const map<Material*, set<int>>& getShoppingCart() const { return shoppingCart; }

    void clearCart();
};