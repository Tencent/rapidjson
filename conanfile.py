from conans import ConanFile, tools
import os


class RapidjsonConan(ConanFile):
    name = "rapidjson"
    description = "A fast JSON parser/generator for C++ with both SAX/DOM style API"
    homepage = "http://rapidjson.org/"
    url = "https://github.com/systelab/rapidjson"
    author = "CSW <csw@werfen.com>"
    license = "MIT"
    exports = ["LICENSE.md"]
    no_copy_source = True
    _source_subfolder = "source_subfolder"

    def package(self):
        source_archive_url = "{0}/archive/v{1}.tar.gz".format(self.url, self.version)
        self.output.info(("Downloading sources from '%s'...") % source_archive_url)
        tools.get(source_archive_url)

        extracted_dir = self.name + "-" + self.version
        os.rename(extracted_dir, self._source_subfolder)

        include_folder = os.path.join(self._source_subfolder, "include")
        self.copy(pattern="license.txt", dst="licenses", src=self._source_subfolder)
        self.copy(pattern="*", dst="include", src=include_folder)

    def package_id(self):
        self.info.header_only()
