# 常见问题

[TOC]

## 一般问题

1. RapidJSON是什么？

   RapidJSON是一个C++库，用于解析及生成JSON。读者可参考它的所有[特点](doc/features.zh-cn.md)。

2. 为什么称作RapidJSON？

   它的灵感来自于[RapidXML](http://rapidxml.sourceforge.net/)，RapidXML是一个高速的XML DOM解析器。

3. RapidJSON与RapidXML相似么？

   RapidJSON借镜了RapidXML的一些设计, 包括原位（*in situ*）解析、只有头文件的库。但两者的API是完全不同的。此外RapidJSON也提供许多RapidXML没有的特点。

4. RapidJSON是免费的么？

   是的，它在MIT特許條款下免费。它可用于商业软件。详情请参看[license.txt](https://github.com/miloyip/rapidjson/blob/master/license.txt)。

5. RapidJSON很小么？它有何依赖？

   是的。在Windows上，一个解析JSON并打印出统计的可执行文件少于30KB。

   RapidJSON仅依赖于C++标准库。

6. 怎样安装RapidJSON？

   见[安装一节](readme.zh-cn.md)。

7. RapidJSON能否运行于我的平台？

   社区已在多个操作系统／编译器／CPU架构的组合上测试RapidJSON。但我们无法确保它能运行于你特定的平台上。只需要生成及执行单元测试便能获取答案。

8. RapidJSON支持C++03么？C++11呢？

   RapidJSON开始时在C++03上实现。后来加入了可选的C++11特性支持（如转移构造函数、`noexcept`）。RapidJSON应该兼容所有遵从C++03或C++11的编译器。

9. RapidJSON是否真的用于实际应用？

   是的。它被配置于前台及后台的真实应用中。一个社区成员说RapidJSON在他们的系统中每日解析5千万个JSON。

10. RapidJSON是如何被测试的？

   RapidJSON包含一组单元测试去执行自动测试。[Travis](https://travis-ci.org/miloyip/rapidjson/)（供Linux平台）及[AppVeyor](https://ci.appveyor.com/project/miloyip/rapidjson/)（供Windows平台）会对所有修改进行编译及执行单元测试。在Linux下还会使用Valgrind去检测内存泄漏。

11. RapidJSON是否有完整的文档？

   RapidJSON提供了使用手册及API说明文档。

12. 有没有其他替代品？

   有许多替代品。例如nativejson-benchmark](https://github.com/miloyip/nativejson-benchmark)列出了一些开源的C/C++ JSON库。[json.org](http://www.json.org/)也有一个列表。

## JSON

1. 什么是JSON？

   JSON (JavaScript Object Notation)是一个轻量的数据交换格式。它使用人类可读的文本格式。更多关于JSON的细节可考[RFC7159](http://www.ietf.org/rfc/rfc7159.txt)及[ECMA-404](http://www.ecma-international.org/publications/standards/Ecma-404.htm)。

2. JSON有什么应用场合？

   JSON常用于网页应用程序，以传送结构化数据。它也可作为文件格式用于数据持久化。

2. RapidJSON是否符合JSON标准？

   是。RapidJSON完全符合[RFC7159](http://www.ietf.org/rfc/rfc7159.txt)及[ECMA-404](http://www.ecma-international.org/publications/standards/Ecma-404.htm)。它能处理一些特殊情况，例如支持JSON字符串中含有空字符及代理对（surrogate pair）。

3. RapidJSON是否支持宽松的语法？

   现时不支持。RapidJSON只支持严格的标准格式。宽松语法现时在这[issue](https://github.com/miloyip/rapidjson/issues/36)中进行讨论。

## DOM与SAX

1. 什么是DOM风格API？

   Document Object Model（DOM）是一个储存于内存的JSON表示方式，用于查询及修改JSON。

2. 什么是SAX风格API?

   SAX是一个事件驱动的API，用于解析及生成JSON。

3. 我应用DOM还是SAX？

   DOM易于查询及修改。SAX则是非常快及省内存的，但通常较难使用。

4. 什么是原位（*in situ*）解析？

   原位解析会把JSON字符串直接解码至输入的JSON中。这是一个优化，可减少内存消耗及提升性能，但输入的JSON会被更改。进一步细节请参考[原位解析](doc/dom.md) 。

5. 什么时候会产生解析错误？

   当输入的JSON包含非法语法，或不能表示一个值（如Number太大），或解析器的处理器中断解析过程，解析器都会产生一个错误。详情请参考[解析错误](doc/dom.md)。

6. 有什么错误信息？

   错误信息存储在`ParseResult`，它包含错误代号及偏移值（从JSON开始至错误处的字符数目）。可以把错误代号翻译为人类可读的错误讯息。

7. 为可不只使用`double`去表示JSON number？

   一些应用需要使用64位无号／有号整数。这些整数不能无损地转换成`double`。因此解析器会检测一个JSON number是否能转换至各种整数类型及`double`。

## Document/Value (DOM)

1. 什么是转移语意？为什么？

   `Value`不用复制语意，而使用了转移语意。这是指，当把来源值赋值于目标值时，来源值的所有权会转移至目标值。

   由于转移快于复制，此设计决定强迫使用者注意到复制的消耗。

2. 怎样去复制一个值？

   有两个API可用：含allocator的构造函数，以及`CopyFrom()`。可参考[深复制Value](doc/tutorial.md)里的用例。

3. 为什么我需要提供字符串的长度？

   由于C字符串是空字符结尾的，需要使用`strlen()`去计算其长度，这是线性复杂度的操作。若使用者已知字符串的长度，对很多操作来说会造成不必要的消耗。

   此外，RapidJSON可处理含有`\u0000`（空字符）的字符串。若一个字符串含有空字符，`strlen()`便不能返回真正的字符串长度。在这种情况下使用者必须明确地提供字符串长度。

4. 为什么在许多DOM操作API中要提供分配器作为参数？

   由于这些API是`Value`的成员函数，我们不希望为每个`Value`储存一个分配器指针。

5. 它会转换各种数值类型么？

   当使用`GetInt()`、`GetUint()`等API时，可能会发生转换。对于整数至整数转换，仅当保证转换安全才会转换（否则会断言失败）。然而，当把一个64位有号／无号整数转换至double时，它会转换，但有可能会损失精度。含有小数的数字、或大于64位的整数，都只能使用`GetDouble()`获取其值。

## Reader/Writer (SAX)

1. 为什么不仅仅用`printf`输出一个JSON？为什么需要`Writer`？

   最重要的是，`Writer`能确保输出的JSON是格式正确的。错误地调用SAX事件（如`StartObject()`错配`EndArray()`）会造成断言失败。此外，`Writer`会把字符串进行转义（如`\n`）。最后，`printf()`的数值输出可能并不是一个合法的JSON number，特别是某些locale会有数字分隔符。而且`Writer`的数值字符串转换是使用非常快的算法来实现的，胜过`printf()`及`iostream`。

2. 我能否暂停解析过程，并在稍后继续？

   基于性能考虑，目前版本并不直接支持此功能。然而，若执行环境支持多线程，使用者可以在另一线程解析JSON，并通过阻塞输入流去暂停。

## Unicode

1. 它是否支持UTF-8、UTF-16及其他格式？

   是。它完全支持UTF-8、UTF-16（大端／小端）、UTF-32（大端／小端）及ASCII。

2. 它能否检测编码的合法性？

   能。只需把`kParseValidateEncodingFlag`参考传给`Parse()`。若发现在输入流中有非法的编码，它就会产生`kParseErrorStringInvalidEncoding`错误。

3. 什么是代理对（surrogate pair)？RapidJSON是否支持？

   JSON使用UTF-16编码去转义Unicode字符，例如`\u5927`表示中文字“大”。要处理基本多文种平面（basic multilingual plane，BMP）以外的字符时，UTF-16会把那些字符编码成两个16位值，这称为UTF-16代理对。例如，绘文字字符U+1F602在JSON中可被编码成`\uD83D\uDE02`。

   RapidJSON完全支持解析及生成UTF-16代理对。 

4. 它能否处理JSON字符串中的`\u0000`（空字符）？

   能。RapidJSON完全支持JSON字符串中的空字符。然而，使用者需要注意到这件事，并使用`GetStringLength()`及相关API去取得字符串真正长度。

5. 能否对所有非ASCII字符输出成`\uxxxx`形式？

   可以。只要在`Writer`中使用`ASCII<>`作为输出编码参数，就可以强逼转义那些字符。

## 流

1. 我有一个很大的JSON文件。我应否把它整个载入内存中？

   使用者可使用`FileReadStream`去逐块读入文件。但若使用于原位解析，必须载入整个文件。

2. 我能否解析一个从网络上串流进来的JSON？

   可以。使用者可根据`FileReadStream`的实现，去实现一个自定义的流。

3. 我不知道一些JSON将会使用哪种编码。怎样处理它们？

   你可以使用`AutoUTFInputStream`，它能自动检测输入流的编码。然而，它会带来一些性能开销。

4. 什么是BOM？RapidJSON怎样处理它？

   [字节顺序标记（byte order mark, BOM）](http://en.wikipedia.org/wiki/Byte_order_mark)有时会出现于文件／流的开始，以表示其UTF编码类型。

   RapidJSON的`EncodedInputStream`可检测／跳过BOM。`EncodedOutputStream`可选择是否写入BOM。可参考[编码流](doc/stream.md)中的例子。

5. 为什么会涉及大端／小端？

   流的大端／小端是UTF-16及UTF-32流要处理的问题，而UTF-8不需要处理。

## 性能

1. RapidJSON是否真的快？

   是。它可能是最快的开源JSON库。有一个[评测](https://github.com/miloyip/nativejson-benchmark)评估C/C++ JSON库的性能。

2. 为什么它会快？

   RapidJSON的许多设计是针对时间／空间性能来设计的，这些决定可能会影响API的易用性。此外，它也使用了许多底层优化（内部函数／intrinsic、SIMD）及特别的算法（自定义的double至字符串转换、字符串至double的转换）。

3. 什是是SIMD？它如何用于RapidJSON？

   [SIMD](http://en.wikipedia.org/wiki/SIMD)指令可以在现代CPU中执行并行运算。RapidJSON支持了Intel的SSE2/SSE4.2去加速跳过空白字符。在解析含缩进的JSON时，这能提升性能。只要定义名为`RAPIDJSON_SSE2`或`RAPIDJSON_SSE42`的宏，就能启动这个功能。然而，若在不支持这些指令集的机器上执行这些可执行文件，会导致崩溃。

4. 它会消耗许多内存么？

   RapidJSON的设计目标是减低内存占用。

   在SAX API中，`Reader`消耗的内存与JSON树深度加上最长JSON字符成正比。

   在DOM API中，每个`Value`在32/64位架构下分别消耗16/24字节。RapidJSON也使用一个特殊的内存分配器去减少分配的额外开销。

5. 高性能的意义何在？

   有些应用程序需要处理非常大的JSON文件。而有些后台应用程序需要处理大量的JSON。达到高性能同时改善延时及吞吐量。更广义来说，这也可以节省能源。

## 八挂

1. 谁是RapidJSON的开发者？

   叶劲峰（Milo Yip，[miloyip](https://github.com/miloyip)）是RapidJSON的原作者。全世界许多贡献者一直在改善RapidJSON。Philipp A. Hartmann（[pah](https://github.com/pah)）实现了许多改进，也设置了自动化测试，而且还参与许多社区讨论。丁欧南（Don Ding，[thebusytypist](https://github.com/thebusytypist)）实现了迭代式解析器。Andrii Senkovych（[jollyroger](https://github.com/jollyroger)）完成了向CMake的迁移。Kosta（[Kosta-Github](https://github.com/Kosta-Github)）提供了一个非常灵巧的短字符串优化。也需要感谢其他献者及社区成员。

2. 为何你要开发RapidJSON？

   在2011年开始这项目是，它仅一个兴趣项目。Milo Yip是一个游戏程序员，他在那时候认识到JSON并希望在未来的项目中使用。由于JSON好像很简单，他希望写一个仅有头文件并且快速的程序库。

3. 为什么开发中段有一段长期空档？

   主要是个人因素，例如加入新家庭成员。另外，Milo Yip也花了许多业馀时间去翻译Jason Gregory的《Game Engine Architecture》至中文版《游戏引擎架构》。

4. 为什么这个项目从Google Code搬到GitHub？

   这是大势所趋，而且GitHub更为强大及方便。
