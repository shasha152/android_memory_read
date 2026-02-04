# amemory – Zero-overhead Cross-Process Memory Reader (C++)

## 简介

**amemory** 是一个面向 C++ 的高性能跨进程内存读取库，专注于以下场景：

* 需要从**其他进程**读取复杂数据结构
* 数据结构中包含**多层指针嵌套**
* 希望避免多次 `vm_read / process_vm_readv` 带来的系统调用开销
* 希望**只定义结构体即可完成读取**，而不是手写一堆偏移和读逻辑

该库的核心目标是：

> **将“跨进程结构化内存读取”的问题，转化为一个几乎完全在编译期解决的问题。**

---

## 核心特性

### 🚀 一次 `read`，读取整棵指针树

* 根据结构体中 **指针嵌套深度**
* 在编译期展开所有访问路径
* 自动计算需要读取的远端内存布局
* **合并为最少次数的 `vm_read` 调用**

你不再需要：

* 手写多次 `read`
* 手动追踪指针偏移
* 在性能和可维护性之间妥协

---

### 🧠 编译期计算，运行期零分支

* 指针深度、字段数量、偏移展开 **全部在编译期完成**
* 运行期仅做：

    * 地址修正
    * 连续内存读取
    * 指针重绑定
* 不依赖 RTTI / 反射
* 不引入虚函数、动态分配或多余拷贝

---

### 🧩 只需定义结构体

你只需要**像描述远端内存布局一样定义结构体**：

```cpp
struct data4
{
    am::offset_t<std::int64_t, 0x8>  v1;
    am::offset_t<std::int64_t, 0x10> v2;
};

struct data3
{
    am::pointer_t<data4, 0x0> v1;
    am::offset_int32_t<0x10>  v2;
};

struct data2
{
    float v1;
    am::pointer_t<data3, 0x8> v2;
};

struct data1
{
    am::pointer_t<float, 0x8>  v1;
    am::pointer_t<data2, 0x18> v2;
    float v3;
};
```

无需任何额外代码，即可完成跨进程读取。

---

## 使用示例

```cpp
void test_read(pid_t pid)
{
    data1 data;

    am::default_access access(pid);
    am::manager manager(access);

    manager.read(0x7B2B823400, data);

    std::cout << *data.v1 << std::endl;
    std::cout << data.v2->v2->v1->v2 << std::endl;
}
```

### 发生了什么？

* `manager.read(base_address, data)` 会：

    1. 在编译期分析 `data1` 的所有指针路径
    2. 计算最大指针深度
    3. 生成最优的内存读取布局
    4. 使用最少次数的 `vm_read` 拉取数据
    5. 自动修复和绑定所有指针关系

你只是在**像访问本地对象一样访问远端内存**。

---

## 设计理念

### ❌ amemory 不做什么

* 不做运行期反射
* 不做字符串字段解析
* 不在运行期遍历类型信息
* 不为“动态结构”牺牲性能

---

### ✅ amemory 专注什么

* **静态结构**
* **确定的偏移**
* **高频读取**
* **极致性能**

这是一个为：

* 游戏逆向 / 内存分析
* 系统工具
* 调试器
* 高性能监控

等场景设计的库。

---

## 性能说明

* 指针深度、字段展开、偏移计算：**编译期完成**
* 运行期复杂度 ≈ **O(1)**（与指针层级无关）
* `vm_read` 调用次数：**最小化**
* 非常适合高频、循环调用

---

## 编译器与标准要求

* C++20（依赖 concepts / constexpr / lambda template）
* 推荐使用 Clang / GCC
* 强烈建议开启：

  ```bash
  -O2
  ```

调试时可开启：

```bash
-fsanitize=address
```

---

## 适用场景总结

✔ 跨进程读取复杂结构
✔ 多层指针嵌套
✔ 高性能、低延迟
✔ 编译期安全性优先

如果你的需求是：

> “我知道远端内存布局，我只想**快、稳、一次性地把它读出来**”

那么 **amemory** 正是为此而生。
