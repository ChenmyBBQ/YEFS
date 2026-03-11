# YEFS C++ 编程规范 (基于 C++ Core Guidelines 与 Qt6 适配)

当前项目是基于 Qt6/QML/C++ 的桌面 GIS 应用。核心的 C++ 逻辑代码需遵循现代 C++ (C++17/20) 标准（源于 [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)），同时必须兼顾 Qt 框架的特有机制（如信号槽、对象树等）。

## 1. 核心与跨领域原则 (Cross-Cutting Principles)
- **RAII（资源获取即初始化）**: 必须确保内存、文件句柄、锁等锁资源通过对象生命周期自动管理。
- **默认不可变 (Immutability)**: 变量和方法默认应加上 `const` / `constexpr`。只有在确认需要发生突变（Mutation）时，才移除 const。
- **类型安全**: 充分利用编译期静态类型检查防止错误（如 `static_assert`）。
- **极简与明确意图**: 优先选择结构清晰简单的代码，避免过度聪明的极客写法。

## 2. 内存、资源使用与 Qt 适配 (Resource Management)
- **禁止裸管理**: 绝对避免手写零散的 `new` / `delete` 与 `malloc()` / `free()`。
- **原生 C++ 对象**: 使用 `std::unique_ptr` 表示独占所有权，使用 `std::shared_ptr` 表示共享所有权。
- **Qt 派生对象专属规则（QObject 父子树）**: 当且仅当构造派生自 `QObject` 的对象（如视窗组件、管理服务等）并**传入了明确的父级对象 (parent)** 时，允许且必须直接使用 `new Object(parent)`。因为 Qt 的对象树会在销毁时自动回收。
- **观察者指针**: 原生 `T*` （裸指针）只用于非持有的观察（如传入不归该方法管的引用实例），绝对不要通过裸指针传递所有权！

## 3. 函数设计 (Functions)
- **单一逻辑**: 函数必须小巧且用途唯一。
- **参数传递 (F.16)**:
  - 基础数值类型（如 `int`, `double`）或简单枚举：按**值传递**。
  - QObject、大型结构或容器：按 **`const T&` (常量引用)** 传递（例如 `const QString&`，`const QList<int>&`）。
  - 若函数明确消耗（sink）该对象，则按值传递并配合 `std::move`。
- **输出结果 (F.20, F.21)**: 摒弃用于输出的参数引用 (`out` 参数)。使用结构体 (`struct`) 或 `std::tuple` 打包返回多个返回值。
- **不可变抛出**: 绝不可返回指向函数内部局部变量的指针或引用。

## 4. 类与层级 (Classes & Hierarchies)
- **零/五法则 (Rule of Zero / Five)**: 
  - 尽量避免手写特殊函数。若无涉及裸资源的深拷贝或特殊清理，完全依赖编译器默认生成（Rule of Zero）。
  - 若需要自管理底层资源，则必须实现全部的五个方法：析构、拷贝构造、拷贝赋值、移动构造、移动赋值（Rule of Five）。
- **多态类约束**: 若包含 `virtual` 方法，基类的析构函数必须为 `public virtual` 或 `protected non-virtual`，并立刻抑制默认公开复制/移动。
- **覆写说明**: 子类继承方法只能并且必须显式写 `override` 或 `final`，切勿重复写 `virtual`。
- **避免隐式转换**: 只有 1 个入参的构造函数必须带上 `explicit`。

## 5. 常量、表达式与安全 (Expressions & Statements)
- **强制初始化**: 变量在声明时必须立即初始化。推荐大括号法 `{}`，如 `int size{0};`。
- **安全替换**:
  - 用 `nullptr`，绝不能用 `NULL` 宏或整数 `0`。
  - 用 `enum class`（强作用域枚举），禁止用弱类型的纯 `enum`。
- **避免转换**:
  - 严禁 C 风格静态转换流（例如 `(int)var`），请使用 `static_cast`、`qobject_cast`（针对 QObject）或 `const_cast`。
  - 避免窄化丢失精度的数值转换。

## 6. Qt标准库与 C++ 融合
- 针对 UI 涉及展示的字符串，优先使用 `QString` 拥有，使用 `QStringView` 或 `const QString&` 观察。业务无涉 Qt 底层的部分仍可使用 `std::string_view`。
- Qt 内部的轻量连续容器推荐 `QList`或 `std::vector`。
- 面向终端 I/O 调试：优先调用内部日志 (例如 `qDebug()` 及应用级 log)，而不是原始的 `std::cout` 或者带有强制刷新的 `std::endl`（直接用 `\n`）。

## 7. 文件规范与代码风格 (Source Files & Naming)
- 头文件（`.h`）使用 `#pragma once` 等价于防依赖机制。
- 禁止在头文件的全局命名空间滥用 `using namespace std;`。
- **命名规范**: 
  - 尽量不要用匈牙利法来编码类型信息（如 `bIsFinished` / `strName`，这是被摒弃的）。
  - 类名推荐与 Qt 类似的 `PascalCase`（帕斯卡命名法，大驼峰）。
  - 变量和函数推荐 `camelCase`（小驼峰）。
  - 私有成员使用统一的前后缀标识（例如 `m_xxx` 或 `xxx_`）。
  - `ALL_CAPS` 仅留给无法避免的纯宏定义。

## 8. 分析与重构限制 (代码导航与工具使用)
- 在重命名符号、修改函数签名或梳理 C/C++ 代码上下文时，**强制使用 `GetSymbolReferences_CppTools`** (找引用)、**`GetSymbolInfo_CppTools`** (看定义) 与 **`GetSymbolCallHierarchy_CppTools`** (找调用链) 这三个 C++ 语义级专用工具。
- **严禁使用**文本搜索工具 (`grep_search` / `file_search`) 来寻找 C++ 函数或变量的引用。这会极大且致命地引发由于多态覆写 (`override`)、重载与类作用域同名 (`update`/`draw`) 导致的漏改或误改灾难。