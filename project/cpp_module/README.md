# CPP Module

C++ 学习模块集合，涵盖了从 C++ 基础语法到高级特性的完整学习路径。这是一个系统性的 C++ 教程项目，通过实际编程练习来掌握现代 C++ 的核心概念。

## 项目概述

本项目包含了 CPP00 到 CPP09 共十个模块，每个模块专注于 C++ 的特定主题和概念。通过循序渐进的练习，从简单的语法开始，逐步深入到模板、STL、异常处理等高级主题。

## 模块结构

### CPP00 - C++ 基础与命名空间
**主要内容**：C++ 基础语法、命名空间、I/O 流
```
CPP00/
├── ex00/  # Megaphone - 字符串操作和I/O
├── ex01/  # PhoneBook - 类和对象基础
└── ex02/  # Account - 静态成员和封装
```

**学习要点**：
- `std::cout`, `std::cin` 的使用
- `std::string` 类型操作
- 基本的类定义和对象创建
- 访问控制符 (public, private, protected)

### CPP01 - 内存分配、引用与指针
**主要内容**：动态内存管理、引用、指针
```
CPP01/
├── ex00/  # BraiiiiiiinnnzzzZ - 堆栈分配对比
├── ex01/  # Moar brainz! - 动态数组
├── ex02/  # HI THIS IS BRAIN - 引用与指针
├── ex03/  # Unnecessary violence - 类的组合
├── ex04/  # Sed is for losers - 文件操作
├── ex05/  # Harl 2.0 - 成员函数指针
└── ex06/  # Harl filter - 条件编译
```

**学习要点**：
- `new` 和 `delete` 操作符
- 引用 (`&`) 与指针 (`*`) 的区别
- 成员函数指针的使用
- 文件流操作 (`std::ifstream`, `std::ofstream`)

### CPP02 - 运算符重载与拷贝控制
**主要内容**：运算符重载、拷贝构造函数、赋值运算符
```
CPP02/
├── ex00/  # Fixed-point numbers - 基础运算符重载
├── ex01/  # Towards a more useful fixed-point number class
├── ex02/  # Now we're talking - 比较运算符
└── ex03/  # BSP - 几何算法应用
```

**学习要点**：
- 算术运算符重载 (`+`, `-`, `*`, `/`)
- 比较运算符重载 (`<`, `>`, `==`, `!=`)
- 拷贝构造函数和赋值运算符
- `const` 修饰符的正确使用

### CPP03 - 继承
**主要内容**：类继承、虚函数、多态性
```
CPP03/
├── ex00/  # ClapTrap - 基础类设计
├── ex01/  # SereTrap - 继承基础
├── ex02/  # FragTrap - 多重继承
└── ex03/  # DiamondTrap - 钻石继承问题
```

**学习要点**：
- 公有继承 (`public` inheritance)
- 虚函数 (`virtual`) 和纯虚函数
- 多重继承和虚继承
- 钻石继承问题的解决

### CPP04 - 多态性与抽象类
**主要内容**：抽象类、接口、运行时多态
```
CPP04/
├── ex00/  # Polymorphism - 多态基础
├── ex01/  # I don't want to set the world on fire - 深拷贝
├── ex02/  # Abstract class - 抽象类设计
└── ex03/  # Interface & recap - 接口设计
```

**学习要点**：
- 纯虚函数和抽象基类
- 虚析构函数的重要性
- 深拷贝vs浅拷贝
- 接口设计模式

### CPP05 - 异常处理
**主要内容**：异常机制、RAII 原则
```
CPP05/
├── ex00/  # Mommy, when I grow up, I want to be a bureaucrat!
├── ex01/  # Form up, maggots! - 异常安全
├── ex02/  # No, you need form 28B, not 28C... - 继承与异常
└── ex03/  # At least this beats coffee-making - 复杂异常处理
```

**学习要点**：
- `try`, `catch`, `throw` 语句
- 自定义异常类
- 异常安全编程
- RAII (Resource Acquisition Is Initialization) 原则

### CPP06 - 类型转换
**主要内容**：C++ 风格的类型转换
```
CPP06/
├── ex00/  # Conversion of scalar types - 基础类型转换
├── ex01/  # Serialization - 对象序列化
└── ex02/  # Identify real type - 运行时类型识别
```

**学习要点**：
- `static_cast`, `dynamic_cast`, `const_cast`, `reinterpret_cast`
- 类型转换运算符
- RTTI (Run-Time Type Identification)
- 序列化和反序列化

### CPP07 - 函数模板
**主要内容**：函数模板、泛型编程基础
```
CPP07/
├── ex00/  # Start with a few functions - 函数模板基础
├── ex01/  # Iter - 迭代器模板
└── ex02/  # Array - 类模板
```

**学习要点**：
- 函数模板定义和实例化
- 模板参数推导
- 类模板设计
- 模板特化

### CPP08 - STL 容器
**主要内容**：标准模板库容器的使用
```
CPP08/
├── ex00/  # Easy find - 算法和容器
├── ex01/  # Span - 自定义容器设计
└── ex02/  # Mutantstack - 容器适配器
```

**学习要点**：
- STL 容器 (`vector`, `list`, `deque`, `stack`, `queue`)
- 迭代器 (iterator) 的使用
- STL 算法函数
- 容器适配器设计

### CPP09 - STL 算法与高级容器
**主要内容**：STL 算法、关联容器
```
CPP09/
├── ex00/  # Bitcoin Exchange - 数据处理
├── ex01/  # Reverse Polish Notation - 栈应用
└── ex02/  # PmergeMe - 排序算法优化
```

**学习要点**：
- 关联容器 (`map`, `set`, `multimap`, `multiset`)
- STL 算法库 (`<algorithm>`)
- 函数对象 (functor)
- 性能优化技巧

## 编译与运行

每个练习都有独立的 Makefile：

```bash
# 编译单个练习
cd CPP00/ex00
make

# 运行程序
./megaphone

# 清理
make clean
make fclean
```

### 通用编译标准

所有模块都遵循相同的编译标准：

```makefile
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
```

## 核心概念进阶

### 1. RAII 原则示例

```cpp
class ResourceManager {
private:
    int* _resource;
    
public:
    ResourceManager() : _resource(new int[100]) {}
    
    ~ResourceManager() {
        delete[] _resource;  // 自动清理
    }
    
    // 禁用拷贝避免双重释放
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
};
```

### 2. 模板设计模式

```cpp
template<typename T>
class Array {
private:
    T* _array;
    unsigned int _size;
    
public:
    template<typename U>
    Array(const Array<U>& other) {  // 模板拷贝构造
        // 类型安全的拷贝
    }
    
    T& operator[](unsigned int index) {
        if (index >= _size)
            throw std::out_of_range("Index out of bounds");
        return _array[index];
    }
};
```

### 3. STL 风格迭代器

```cpp
class MutantStack : public std::stack<T> {
public:
    typedef typename std::stack<T>::container_type::iterator iterator;
    
    iterator begin() { 
        return this->c.begin(); 
    }
    
    iterator end() { 
        return this->c.end(); 
    }
};
```

## 实际应用示例

### 金融数据处理 (CPP09/ex00)

```cpp
class BitcoinExchange {
private:
    std::map<std::string, double> _database;
    
public:
    void loadDatabase(const std::string& filename);
    double getPrice(const std::string& date) const;
    void processTransactions(const std::string& filename) const;
};
```

### 逆波兰表达式计算器 (CPP09/ex01)

```cpp
class RPN {
private:
    std::stack<double> _stack;
    
public:
    double calculate(const std::string& expression) {
        std::istringstream iss(expression);
        std::string token;
        
        while (iss >> token) {
            if (isOperator(token))
                performOperation(token);
            else
                _stack.push(std::stod(token));
        }
        
        return _stack.top();
    }
};
```

## 学习路径建议

### 初学者路径
1. **CPP00-CPP01**: C++ 基础语法和内存管理
2. **CPP02-CPP03**: 面向对象编程核心
3. **CPP04**: 多态性和抽象设计

### 进阶路径  
4. **CPP05**: 异常处理和错误管理
5. **CPP06**: 类型系统深入理解
6. **CPP07**: 泛型编程入门

### 高级路径
7. **CPP08-CPP09**: STL 库的精通和实际应用

## 最佳实践

1. **遵循 RAII 原则**：资源获取即初始化
2. **使用 const 正确性**：明确数据的可修改性
3. **异常安全编程**：保证程序的健壮性
4. **模板编程**：编写可复用的泛型代码
5. **STL 优先**：充分利用标准库的强大功能

---

这个 CPP Module 项目提供了完整的 C++ 学习体验，从基础语法到高级特性，每个概念都通过实际的编程练习来巩固，是掌握现代 C++ 编程的理想学习资源。