# 流

在RapidJSON中，`rapidjson::Stream`是用於读写JSON的概念（概念是指C++的concept）。在这里我们先介绍如何使用RapidJSON提供的各种流。然后再看看如何自行定义流。

[TOC]

# 内存流 {#MemoryStreams}

内存流把JSON存储在内存之中。

## StringStream（输入）{#StringStream}

`StringStream`是最基本的输入流，它表示一个完整的、只读的、存储于内存的JSON。它在`rapidjson/rapidjson.h`中定义。

~~~~~~~~~~cpp
#include "rapidjson/document.h" // 会包含 "rapidjson/rapidjson.h"

using namespace rapidjson;

// ...
const char json[] = "[1, 2, 3, 4]";
StringStream s(json);

Document d;
d.ParseStream(s);
~~~~~~~~~~

由于这是非常常用的用法，RapidJSON提供`Document::Parse(const char*)`去做完全相同的事情：

~~~~~~~~~~cpp
// ...
const char json[] = "[1, 2, 3, 4]";
Document d;
d.Parse(json);
~~~~~~~~~~

需要注意，`StringStream`是`GenericStringStream<UTF8<> >`的typedef，使用者可用其他编码类去代表流所使用的字符集。

## StringBuffer（输出）{#StringBuffer}

`StringBuffer`是一个简单的输出流。它分配一个内存缓冲区，供写入整个JSON。可使用`GetString()`来获取该缓冲区。

~~~~~~~~~~cpp
#include "rapidjson/stringbuffer.h"

StringBuffer buffer;
Writer<StringBuffer> writer(buffer);
d.Accept(writer);

const char* output = buffer.GetString();
~~~~~~~~~~

当缓冲区满溢，它将自动增加容量。缺省容量是256个字符（UTF8是256字节，UTF16是512字节等）。使用者能自行提供分配器及初始容量。

~~~~~~~~~~cpp
StringBuffer buffer1(0, 1024); // 使用它的分配器，初始大小 = 1024
StringBuffer buffer2(allocator, 1024);
~~~~~~~~~~

如无设置分配器，`StringBuffer`会自行实例化一个内部分配器。

相似地，`StringBuffer`是`GenericStringBuffer<UTF8<> >`的typedef。

# 文件流 {#FileStreams}

当要从文件解析一个JSON，你可以把整个JSON读入内存并使用上述的`StringStream`。

然而，若JSON很大，或是内存有限，你可以改用`FileReadStream`。它只会从文件读取一部分至缓冲区，然后让那部分被解析。若缓冲区的字符都被读完，它会再从文件读取下一部分。

## FileReadStream（输入） {#FileReadStream}

`FileReadStream`通过`FILE`指针读取文件。使用者需要提供一个缓冲区。

~~~~~~~~~~cpp
#include "rapidjson/filereadstream.h"
#include <cstdio>

using namespace rapidjson;

FILE* fp = fopen("big.json", "rb"); // 非Windows平台使用"r"

char readBuffer[65536];
FileReadStream is(fp, readBuffer, sizeof(readBuffer));

Document d;
d.ParseStream(is);

fclose(fp);
~~~~~~~~~~

与`StringStreams`不一样，`FileReadStream`是一个字节流。它不处理编码。若文件并非UTF-8编码，可以把字节流用`EncodedInputStream`包装。我们很快会讨论这个问题。

除了读取文件，使用者也可以使用`FileReadStream`来读取`stdin`。

## FileWriteStream（输出）{#FileWriteStream}

`FileWriteStream`是一个含缓冲功能的输出流。它的用法与`FileReadStream`非常相似。

~~~~~~~~~~cpp
#include "rapidjson/filewritestream.h"
#include <cstdio>

using namespace rapidjson;

Document d;
d.Parse(json);
// ...

FILE* fp = fopen("output.json", "wb"); // 非Windows平台使用"w"

char writeBuffer[65536];
FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));

Writer<FileWriteStream> writer(os);
d.Accept(writer);

fclose(fp);
~~~~~~~~~~

它也可以把输出导向`stdout`。

# 编码流 {#EncodedStreams}

编码流（encoded streams）本身不存储JSON，它们是通过包装字节流来提供基本的编码／解码功能。

如上所述，我们可以直接读入UTF-8字节流。然而，UTF-16及UTF-32有字节序（endian）问题。要正确地处理字节序，需要在读取时把字节转换成字符（如对UTF-16使用`wchar_t`），以及在写入时把字符转换为字节。

除此以外，我们也需要处理[字节顺序标记（byte order mark, BOM）](http://en.wikipedia.org/wiki/Byte_order_mark)。当从一个字节流读取时，需要检测BOM，或者仅仅是把存在的BOM消去。当把JSON写入字节流时，也可选择写入BOM。

若一个流的编码在编译期已知，你可使用`EncodedInputStream`及`EncodedOutputStream`。若一个流可能存储UTF-8、UTF-16LE、UTF-16BE、UTF-32LE、UTF-32BE的JSON，并且编码只能在运行时得知，你便可以使用`AutoUTFInputStream`及`AutoUTFOutputStream`。这些流定义在`rapidjson/encodedstream.h`。

注意到，这些编码流可以施于文件以外的流。例如，你可以用编码流包装内存中的文件或自定义的字节流。

## EncodedInputStream {#EncodedInputStream}

`EncodedInputStream`含两个模板参数。第一个是`Encoding`类型，例如定义于`rapidjson/encodings.h`的`UTF8`、`UTF16LE`。第二个参数是被包装的流的类型。

~~~~~~~~~~cpp
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"   // FileReadStream
#include "rapidjson/encodedstream.h"    // EncodedInputStream
#include <cstdio>

using namespace rapidjson;

FILE* fp = fopen("utf16le.json", "rb"); // 非Windows平台使用"r"

char readBuffer[256];
FileReadStream bis(fp, readBuffer, sizeof(readBuffer));

EncodedInputStream<UTF16LE<>, FileReadStream> eis(bis);  // 用eis包装bis

Document d; // Document为GenericDocument<UTF8<> > 
d.ParseStream<0, UTF16LE<> >(eis);  // 把UTF-16LE文件解析至内存中的UTF-8

fclose(fp);
~~~~~~~~~~

## EncodedOutputStream {#EncodedOutputStream}

`EncodedOutputStream`也是相似的，但它的构造函数有一个`bool putBOM`参数，用于控制是否在输出字节流写入BOM。

~~~~~~~~~~cpp
#include "rapidjson/filewritestream.h"  // FileWriteStream
#include "rapidjson/encodedstream.h"    // EncodedOutputStream
#include <cstdio>

Document d;         // Document为GenericDocument<UTF8<> > 
// ...

FILE* fp = fopen("output_utf32le.json", "wb"); // 非Windows平台使用"w"

char writeBuffer[256];
FileWriteStream bos(fp, writeBuffer, sizeof(writeBuffer));

typedef EncodedOutputStream<UTF32LE<>, FileWriteStream> OutputStream;
OutputStream eos(bos, true);   // 写入BOM

Writer<OutputStream, UTF32LE<>, UTF8<>> writer(eos);
d.Accept(writer);   // 这里从内存的UTF-8生成UTF32-LE文件

fclose(fp);
~~~~~~~~~~

## AutoUTFInputStream {#AutoUTFInputStream}

有时候，应用软件可能需要㲃理所有可支持的JSON编码。`AutoUTFInputStream`会先使用BOM来检测编码。若BOM不存在，它便会使用合法JSON的特性来检测。若两种方法都失败，它就会倒退至构造函数提供的UTF类型。

由于字符（编码单元／code unit）可能是8位、16位或32位，`AutoUTFInputStream` 需要一个能至少储存32位的字符类型。我们可以使用`unsigned`作为模板参数：

~~~~~~~~~~cpp
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"   // FileReadStream
#include "rapidjson/encodedstream.h"    // AutoUTFInputStream
#include <cstdio>

using namespace rapidjson;

FILE* fp = fopen("any.json", "rb"); // 非Windows平台使用"r"

char readBuffer[256];
FileReadStream bis(fp, readBuffer, sizeof(readBuffer));

AutoUTFInputStream<unsigned, FileReadStream> eis(bis);  // 用eis包装bis

Document d;         // Document为GenericDocument<UTF8<> > 
d.ParseStream<0, AutoUTF<unsigned> >(eis); // 把任何UTF编码的文件解析至内存中的UTF-8

fclose(fp);
~~~~~~~~~~

当要指定流的编码，可使用上面例子中`ParseStream()`的参数`AutoUTF<CharType>`。

你可以使用`UTFType GetType()`去获取UTF类型，并且用`HasBOM()`检测输入流是否含有BOM。

## AutoUTFOutputStream {#AutoUTFOutputStream}

相似地，要在运行时选择输出的编码，我们可使用`AutoUTFOutputStream`。这个类本身并非「自动」。你需要在运行时指定UTF类型，以及是否写入BOM。

~~~~~~~~~~cpp
using namespace rapidjson;

void WriteJSONFile(FILE* fp, UTFType type, bool putBOM, const Document& d) {
    char writeBuffer[256];
    FileWriteStream bos(fp, writeBuffer, sizeof(writeBuffer));

    typedef AutoUTFOutputStream<unsigned, FileWriteStream> OutputStream;
    OutputStream eos(bos, type, putBOM);
    
    Writer<OutputStream, UTF8<>, AutoUTF<> > writer;
    d.Accept(writer);
}
~~~~~~~~~~

`AutoUTFInputStream`／`AutoUTFOutputStream`是比`EncodedInputStream`／`EncodedOutputStream`方便。但前者会产生一点运行期额外开销。

# 自定义流 {#CustomStream}

除了内存／文件流，使用者可创建自行定义适配RapidJSON API的流类。例如，你可以创建网络流、从压缩文件读取的流等等。

RapidJSON利用模板结合不同的类型。只要一个类包含所有所需的接口，就可以作为一个流。流的接合定义在`rapidjson/rapidjson.h`的注释里：

~~~~~~~~~~cpp
concept Stream {
    typename Ch;    //!< 流的字符类型

    //! 从流读取当前字符，不移动读取指针（read cursor）
    Ch Peek() const;

    //! 从流读取当前字符，移动读取指针至下一字符。
    Ch Take();

    //! 获取读取指针。
    //! \return 从开始以来所读过的字符数量。
    size_t Tell();

    //! 从当前读取指针开始写入操作。
    //! \return 返回开始写入的指针。
    Ch* PutBegin();

    //! 写入一个字符。
    void Put(Ch c);

    //! 清空缓冲区。
    void Flush();

    //! 完成写作操作。
    //! \param begin PutBegin()返回的开始写入指针。
    //! \return 已写入的字符数量。
    size_t PutEnd(Ch* begin);
}
~~~~~~~~~~

输入流必须实现`Peek()`、`Take()`及`Tell()`。
输出流必须实现`Put()`及`Flush()`。
`PutBegin()`及`PutEnd()`是特殊的接口，仅用于原位（*in situ*）解析。一般的流不需实现它们。然而，即使接口不需用于某些流，仍然需要提供空实现，否则会产生编译错误。

## 例子：istream的包装类 {#ExampleIStreamWrapper}

以下的例子是`std::istream`的包装类，它只需现3个函数。

~~~~~~~~~~cpp
class IStreamWrapper {
public:
    typedef char Ch;

    IStreamWrapper(std::istream& is) : is_(is) {
    }

    Ch Peek() const { // 1
        int c = is_.peek();
        return c == std::char_traits<char>::eof() ? '\0' : (Ch)c;
    }

    Ch Take() { // 2
        int c = is_.get();
        return c == std::char_traits<char>::eof() ? '\0' : (Ch)c;
    }

    size_t Tell() const { return (size_t)is_.tellg(); } // 3

    Ch* PutBegin() { assert(false); return 0; }
    void Put(Ch) { assert(false); }
    void Flush() { assert(false); }
    size_t PutEnd(Ch*) { assert(false); return 0; }

private:
    IStreamWrapper(const IStreamWrapper&);
    IStreamWrapper& operator=(const IStreamWrapper&);

    std::istream& is_;
};
~~~~~~~~~~

使用者能用它来包装`std::stringstream`、`std::ifstream`的实例。

~~~~~~~~~~cpp
const char* json = "[1,2,3,4]";
std::stringstream ss(json);
IStreamWrapper is(ss);

Document d;
d.ParseStream(is);
~~~~~~~~~~

但要注意，由于标准库的内部开销问，此实现的性能可能不如RapidJSON的内存／文件流。

## 例子：ostream的包装类 {#ExampleOStreamWrapper}

以下的例子是`std::istream`的包装类，它只需实现2个函数。

~~~~~~~~~~cpp
class OStreamWrapper {
public:
    typedef char Ch;

    OStreamWrapper(std::ostream& os) : os_(os) {
    }

    Ch Peek() const { assert(false); return '\0'; }
    Ch Take() { assert(false); return '\0'; }
    size_t Tell() const {  }

    Ch* PutBegin() { assert(false); return 0; }
    void Put(Ch c) { os_.put(c); }                  // 1
    void Flush() { os_.flush(); }                   // 2
    size_t PutEnd(Ch*) { assert(false); return 0; }

private:
    OStreamWrapper(const OStreamWrapper&);
    OStreamWrapper& operator=(const OStreamWrapper&);

    std::ostream& os_;
};
~~~~~~~~~~

使用者能用它来包装`std::stringstream`、`std::ofstream`的实例。

~~~~~~~~~~cpp
Document d;
// ...

std::stringstream ss;
OSStreamWrapper os(ss);

Writer<OStreamWrapper> writer(os);
d.Accept(writer);
~~~~~~~~~~

但要注意，由于标准库的内部开销问，此实现的性能可能不如RapidJSON的内存／文件流。

# 总结 {#Summary}

本节描述了RapidJSON提供的各种流的类。内存流很简单。若JSON存储在文件中，文件流可减少JSON解析及生成所需的内存量。编码流在字节流和字符流之间作转换。最后，使用者可使用一个简单接口创建自定义的流。
