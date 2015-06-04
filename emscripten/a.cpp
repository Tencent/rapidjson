#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>

#define private public
#include "rapidjson/document.h"

int main() {
    // std::ifstream in_file("config.json", std::ios::in);
    rapidjson::Document json_config_;
    // std::string json_string(
    //         (std::istreambuf_iterator<char>(in_file)),
    //         std::istreambuf_iterator<char>());
    std::string json_string("{\"float_test\": 75.0}");
    // if (!json_string.empty()) {
        json_config_.Parse(json_string.c_str());
    // }

    printf("%p\n", &json_config_);
    printf("%p\n", &json_config_.data_.o.members[0].value.data_.n.d);

    std::cout << json_config_["float_test"].GetDouble() << std::endl;
}