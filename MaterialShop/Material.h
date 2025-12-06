#pragma once
#define _HAS_STD_BYTE 0 
#include <string>
#include <vector>
#include <map> // 必须包含
#include<algorithm>
using namespace std;

class Material {
protected:
    string name;
    double basePrice;
    // 修改核心：存储 年份 -> 链接 的映射
    map<int, string> yearResources;

public:
    Material(string n, double p, map<int, string> resources)
        : name(n), basePrice(p), yearResources(resources) {
    }
    virtual ~Material() {}

    virtual double calculatePrice(int count) = 0;

    // 修改接口：根据年份获取链接
    virtual string getLinkByYear(int year) = 0;
    virtual string getDiscountRule() = 0;

    string getName() const { return name; }
    double getBasePrice() const { return basePrice; }

    // 辅助函数：提取所有年份给GUI生成按钮用
    vector<int> getAvailableYears() const {
        vector<int> years;
        // 遍历 map，把 key (年份) 存入 vector
        for (auto const& entry : yearResources) {
            years.push_back(entry.first);
        }
        // 让年份倒序排列（2024在前面）
        sort(years.rbegin(), years.rend());
        return years;
    }
};

class JsonConfiguredMaterial : public Material {
private:
    int discountThreshold;
    double discountRate;

public:
    // 构造函数参数变了
    JsonConfiguredMaterial(string n, double p, map<int, string> resources, int discThreshold, double discRate);

    double calculatePrice(int count) override;
    string getLinkByYear(int year) override;
	string getDiscountRule() override;
};