#
#  Be sure to run `pod spec lint rapidjson.podspec' to ensure this is a
#  valid spec and to remove all comments including this before submitting the spec.
#
#  To learn more about Podspec attributes see https://guides.cocoapods.org/syntax/podspec.html
#  To see working Podspecs in the CocoaPods repo see https://github.com/CocoaPods/Specs/
#

Pod::Spec.new do |spec|

  spec.name = "rapidjson"
  spec.version = "1.1.0"
  spec.summary = "A fast JSON parser/generator for C++ with both SAX/DOM style API"
  spec.description = "RapidJSON is a JSON parser and generator for C++. It was inspired by RapidXml.\n\nRapidJSON is small but complete. It supports both SAX and DOM style API. The SAX parser is only a half thousand lines of code.\nRapidJSON is fast. Its performance can be comparable to strlen(). It also optionally supports SSE2/SSE4.2 for acceleration.\nRapidJSON is self-contained and header-only. It does not depend on external libraries such as BOOST. It even does not depend on STL.\nRapidJSON is memory-friendly. Each JSON value occupies exactly 16 bytes for most 32/64-bit machines (excluding text string). By default it uses a fast memory allocator, and the parser allocates memory compactly during parsing.\nRapidJSON is Unicode-friendly. It supports UTF-8, UTF-16, UTF-32 (LE & BE), and their detection, validation and transcoding internally. For example, you can read a UTF-8 file and let RapidJSON transcode the JSON strings into UTF-16 in the DOM. It also supports surrogates and \"\u0000\" (null character)."
  spec.homepage = "http://rapidjson.org/"
  spec.license = {
      :type => "MIT",
      :file => "license.txt"
}
  spec.authors = "Milo Yip"
  spec.platforms = {
    :ios => "8.0"
  }
  spec.source = {
    :git => "https://github.com/miloyip/rapidjson.git",
    :tag => "version1.1.0"
  }
  spec.source_files = "include/rapidjson/**/*.h"
  spec.header_mappings_dir = "include/rapidjson"
  spec.requires_arc = false

end
