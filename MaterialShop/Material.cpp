#include "Material.h"
#include <algorithm> // 用于 sort

JsonConfiguredMaterial::JsonConfiguredMaterial(string n, double p, map<int, string> resources, int discThreshold, double discRate)
    : Material(n, p, resources), discountThreshold(discThreshold), discountRate(discRate)
{
}

double JsonConfiguredMaterial::calculatePrice(int count) {
    double total = basePrice * count;
    if (discountThreshold > 0 && count >= discountThreshold) {
        return total * discountRate;
    }
    return total;
}

string JsonConfiguredMaterial::getLinkByYear(int year) {
    // 在 map 中查找年份
    if (yearResources.count(year)) {
        return yearResources[year];
    }
    return "链接丢失";
}

string JsonConfiguredMaterial::getDiscountRule() {
    if (discountThreshold <= 0 || discountRate >= 1.0) {
        return "无折扣";
    }
    // 比如：满 3 年 享 9 折
    string rateStr = to_string((int)(discountRate * 100)) + "%"; // 0.9 -> 90%
    string msg = "满 " + to_string(discountThreshold) + " 年 (" + rateStr + " Price)";

    // 或者简单点：满 3 年 9 折
    // 注意：这里 to_string 可能会带小数点，简单处理如下：
    int discountInt = (int)(discountRate * 10); // 0.9 -> 9
    return "满 " + to_string(discountThreshold) + " 年享 " + to_string(discountInt) + " 折";
}