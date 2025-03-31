DLL导出类（DLL-Exported Class）
在Windows平台开发中，DLL（动态链接库） 是一种模块化代码共享的方式。当需要在DLL中暴露一个类（供其他程序或模块使用）时，需要将该类标记为“导出”（Export），这就是所谓的 DLL导出类。

为什么需要导出类？
符号可见性：默认情况下，DLL中的符号（类、函数、变量）对外不可见。导出类的声明会告诉编译器：“这个类需要被外部调用”。
跨模块使用：如果其他模块（如EXE或其他DLL）要使用DLL中的类，必须明确导出它，否则会导致链接错误或运行时崩溃。
内存管理一致性：导出类需要确保类的内存布局（如虚表、成员变量偏移）在DLL和使用者模块中一致，否则可能引发未定义行为。
如何导出类？
在Windows中，使用 __declspec(dllexport) 关键字标记类为导出：

<CPP>
// 在DLL项目编译时，标记为导出
class __declspec(dllexport) MyClass {
    // 类定义
};
在外部调用该DLL的代码中，需用 __declspec(dllimport) 声明导入：

<CPP>
// 在使用DLL的代码中，标记为导入
class __declspec(dllimport) MyClass {
    // 类定义（通常通过头文件共享）
};
通常通过预定义宏简化跨DLL/EXE的声明：

<CPP>
#ifdef MYDLL_EXPORTS
    #define MY_API __declspec(dllexport)
#else
    #define MY_API __declspec(dllimport)
#endif
class MY_API MyClass { /* ... */ };
用户代码中的关键点分析
在用户提供的 LogStream 代码中：

基类未导出引发警告：

<CPP>
class GLOG_EXPORT LogStream : public std::ostream { /* ... */ };
std::ostream 是标准库的类，未标记为DLL导出（标准库本身是独立的模块）。
当派生类（LogStream）被导出时，MSVC会警告 C4275，提示基类未导出可能引发二进制兼容性问题。
为何可以忽略此警告？

标准库（如std::ostream）是 与编译器绑定的。只要调用方和使用方使用 相同版本的编译器，内存布局即一致。
微软官方文档指出，如果基类是标准库类型（如std::ostream），可以安全忽略此警告。
导出类的注意事项
二进制兼容性（ABI）：

如果导出类的公共接口（如虚函数、成员变量布局）发生变更，所有使用该DLL的模块必须重新编译，否则会导致崩溃。
关键原则：保持DLL接口稳定，或通过Pimpl模式隐藏实现细节。
STL类的导出问题：

导出包含STL成员（如std::string）的类可能导致兼容性问题，因为不同编译器的STL实现不同。
解决方案：避免在DLL接口中直接使用STL容器，改用原始数据（如char*）或自定义类型。
虚函数与虚表：

虚函数会导致类中包含虚表指针。如果DLL和使用者的虚表布局不一致（如不同编译器），调用虚函数会崩溃。
解决方法：使用纯接口类（仅包含虚函数，无成员变量）作为基类。
