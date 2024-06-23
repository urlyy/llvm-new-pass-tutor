/**
 * 思路来自 https://github.com/banach-space/llvm-tutor?tab=readme-ov-file#mbaadd
 * 原文也强调了 "only valid for 8 bit integers"
 * 下方代码没有太多意义，仅作为代码混淆的展示例子
 */
#include <iostream>

int mba_add(int8_t a, int8_t b) {
    int8_t xor_result = a ^ b;
    int8_t and_result =a & b;
    int8_t result = (((xor_result + 2 * and_result) * 39 + 23) * 151 + 111);
    return result;
}

int add(int8_t a, int8_t b){
    return a+b;
}

int main() {
    int8_t a = 100;
    int8_t b = 20;
    std::cout << add(a,b) << " " << mba_add(a,b);
    return 0;
}