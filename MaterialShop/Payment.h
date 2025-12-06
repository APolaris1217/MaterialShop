#pragma once
#define _HAS_STD_BYTE 0 
#include <string>
#include <iostream>
#include <iomanip> 

using namespace std;

// ==========================================
// 抽象基类：支付方式
// ==========================================
class PaymentMethod {
protected:
    string name;

public:
    PaymentMethod(const string& n) : name(n) {}
    virtual ~PaymentMethod() {}

    // 纯虚函数：执行支付逻辑，返回支付是否成功
    virtual bool pay(double amount) const = 0;

    // 返回该支付方式对应的优惠/手续费说明（用于 UI 显示）
    virtual string getPolicy() const { return string("(无)"); }

    // 获取支付方式名称
    string getName() const { return name; }
};

// ==========================================
// 具体实现类 1：支付宝
// ==========================================
class Alipay : public PaymentMethod {
public:
    Alipay() : PaymentMethod("支付宝") {}

    // 假设支付宝有 0.5% 的手续费 (仅为示例)
    bool pay(double amount) const override {
        double fee = amount * 0.005;
        double total = amount + fee;

        cout << fixed << setprecision(2);

        if (total > 0) {
            cout << "使用 " << name << " 支付 " << amount << " 元，手续费 " << fee << " 元，实扣 " << total << " 元。" << endl;
            return true;
        }
        return false;
    }

    string getPolicy() const override { return string("收取 0.5% 手续费"); }
};

// ==========================================
// 具体实现类 2：微信支付
// ==========================================
class WechatPay : public PaymentMethod {
public:
    WechatPay() : PaymentMethod("微信支付") {}

    // 模拟微信支付成功
    bool pay(double amount) const override {
        if (amount > 0) {
            cout << "使用 " << name << " 支付 " << amount << " 元。" << endl;
            return true;
        }
        return false;
    }

    string getPolicy() const override { return string("无额外优惠/手续费"); }
};

// ==========================================
// 具体实现类 3：校园卡 (满 100 减 10)
// ==========================================
class CampusCard : public PaymentMethod {
public:
    CampusCard() : PaymentMethod("校园卡") {}

    bool pay(double amount) const override {
        double finalAmount = amount;
        string discountMsg = "";

        if (amount >= 10.0) {
            finalAmount -= 1.0; // 满 100 减 10
            discountMsg = " (已减 1.00 元)";
        }

        cout << fixed << setprecision(2);

        if (finalAmount > 0) {
            cout << "使用 " << name << " 支付 " << amount << " 元" << discountMsg << "，实付 " << finalAmount << " 元。" << endl;
            return true;
        }
        return false;
    }

    string getPolicy() const override { return string("满 10 减 1 元"); }
};

// ==========================================
// 具体实现类 4：农业银行卡 (无优惠/手续费)
// ==========================================
class AgriBankCard : public PaymentMethod {
public:
    AgriBankCard() : PaymentMethod("农业银行卡") {}

    bool pay(double amount) const override {
        if (amount > 0) {
            cout << "使用 " << name << " 支付 " << amount << " 元。" << endl;
            return true;
        }
        return false;
    }

    string getPolicy() const override { return string("无额外优惠/手续费"); }
};