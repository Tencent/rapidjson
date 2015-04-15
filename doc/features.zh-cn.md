# 特点

## 总体

* 跨平台
 * 编译器：Visual Studio、gcc、clang等
 * 架构：x86、x64、ARM等
 * 操作系统：Windows、Mac OS X、Linux、iOS、Android等
* 容易安装
 * 只有头文件的库。只需把头文件复制至你的项目中。
* 独立、最小依赖
 * 不需依赖STL、BOOST等。
 * 只包含`<cstdio>`, `<cstdlib>`, `<cstring>`, `<inttypes.h>`, `<new>`, `<stdint.h>`。 
* 没使用C++异常、RTTI
* 高性能
 * 使用模版及内联函数去降低函数调用开销。
 * 内部经优化的Grisu2及浮点数解析实现。
 * 可选的SSE2/SSE4.2支持。

## 符合标准

* RapidJSON应完全符合RFC4627/ECMA-404标准。
* 支持Unicod代理对（surrogate pair）。
* 支持空字符（`"\u0000"`）。
 * 例如，可以优雅地解析及处理`["Hello\u0000World"]`。含读写字符串长度的API。

## Unicode

* 支持UTF-8、UTF-16、UTF-32编码，包括小端序和大端序。
 * 这些编码用于输入输出流，以及内存中的表示。
* 支持从输入流自动检测编码。
* 内部支持编码的转换。
 * 例如，你可以读取一个UTF-8文件，让RapidJSON把JSON字符串转换至UTF-16的DOM。
* 内部支持编码校验。
 * 例如，你可以读取一个UTF-8文件，让RapidJSON检查是否所有JSON字符串是合法的UTF-8字节序列。
* 支持自定义的字符类型。
 * 预设的字符类型是：UTF-8为`char`，UTF-16为`wchar_t`，UTF32为`uint32_t`。
* 支持自定义的编码。

## API风格

* SAX（Simple API for XML）风格API
 * 类似于[SAX](http://en.wikipedia.org/wiki/Simple_API_for_XML), RapidJSON提供一个事件循序访问的解析器API（`rapidjson::GenericReader`）。RapidJSON也提供一个生成器API（`rapidjson::Writer`），可以处理相同的事件集合。
* DOM（Document Object Model）风格API
 * 类似于HTML／XML的[DOM](http://en.wikipedia.org/wiki/Document_Object_Model)，RapidJSON可把JSON解析至一个DOM表示方式（`rapidjson::GenericDocument`），以方便操作。如有需要，可把DOM转换（stringify）回JSON。
 * DOM风格API（`rapidjson::GenericDocument`）实际上是由SAX风格API（`rapidjson::GenericReader`）实现的。SAX更快，但有时DOM更易用。用户可根据情况作出选择。

## 解析

* 递归式（预设）及迭代式解析器
 * 递归式解析器较快，但在极端情况下可出现堆栈溢出。
 * 迭代式解析器使用自定义的堆栈去维持解析状态。
* 支持原位（*in situ*）解析。
 * 把JSON字符串的值解析至原JSON之中，然后让DOM指向那些字符串。
 * 比常规分析更快：不需字符串的内存分配、不需复制（如字符串不含转义符）、缓存友好。
* 对于JSON数字类型，支持32-bit/64-bit的有号／无号整数，以及`double`。
* 错误处理
 * 支持详尽的解析错误代号。
 * 支持本地化错误信息。

## DOM (Document)

* RapidJSON在类型转换时会检查数值的范围。
* 字符串字面量的优化
 * 只储存指针，不作复制
* 优化“短”字符串
 * 在`Value`内储存短字符串，无需额外分配。
 * 对UTF-8字符串来说，32位架构下可存储最多11字符，64位下15字符。
* 可选地支持`std::string`（定义`RAPIDJSON_HAS_STDSTRING=1`）

## 生成

* 支持`rapidjson::PrettyWriter`去加入换行及缩进。

## 输入输出流

* 支持`rapidjson::GenericStringBuffer`，把输出的JSON储存于字符串内。
* 支持`rapidjson::FileReadStream`及`rapidjson::FileWriteStream`，使用`FILE`对象作输入输出。
* 支持自定义输入输出流。

## 内存

* 最小化DOM的内存开销。
 * 对大部分32／64位机器而言，每个JSON值只占16或20字节（不包含字符串）。
* 支持快速的预设分配器。
 * 它是一个堆栈形式的分配器（顺序分配，不容许单独释放，适合解析过程之用）。
 * 使用者也可提供一个预分配的缓冲区。（有可能达至无需CRT分配就能解析多个JSON）
* 支持标准CRT（C-runtime）分配器。
* 支持自定义分配器。

## 其他

* 一些C++11的支持（可选）
 * 右值引用（rvalue reference）
 * `noexcept`修饰符
