#include "RPN.hpp"
#include <stack>
#include <sstream>
#include <iostream>
#include <cctype>

RPN::RPN() {}

int RPN::calculate(const std::string& expression) {
    std::stack<int> stk;
    std::istringstream iss(expression);
    std::string token;

    while (iss >> token) {
        if (token.length() == 1 && std::isdigit(token[0])) {
            // 数字，入栈
            int num = token[0] - '0';
            stk.push(num);
        }
        else if (token.length() == 1 && (token == "+" || token == "-" || token == "*" || token == "/")) {
            // 操作符，弹出两个操作数计算
            if (stk.size() < 2)
                throw std::runtime_error("Error");

            int b = stk.top(); stk.pop();
            int a = stk.top(); stk.pop();

            int res;
            if (token == "+") res = a + b;
            else if (token == "-") res = a - b;
            else if (token == "*") res = a * b;
            else {
                if (b == 0) throw std::runtime_error("Error");
                res = a / b;
            }
            stk.push(res);
        }
        else {
            // 非法字符
            throw std::runtime_error("Error");
        }
    }
    if (stk.size() != 1) {
        throw std::runtime_error("Error");
    }
    return stk.top();
}

