#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <cstdint>
#include <cstddef>

using namespace rapidjson;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    std::string json(reinterpret_cast<const char*>(data), size);
    Document d;
    d.Parse(json.c_str());
    return 0;
}
