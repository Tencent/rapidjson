#include <rapidjson/document.h>

int main() {
    rapidjson::Document document;
    document.Parse("{\"answer\":42}");
    return !document.HasParseError() && document["answer"].GetInt() == 42 ? 0 : 1;
}
