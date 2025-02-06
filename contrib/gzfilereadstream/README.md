Add support for reading directly from gzipped file:

Example:

Input json:
```
{"a": 1, "x": 10, "y": [1,2,3], "m": {"c": "z"}}
```

Test:
```
#include <iostream>
#include <zlib.h>

#include "rapidjson/document.h"
#include "rapidjson/gzfilereadstream.h"

using namespace rapidjson;
using namespace std;

int main(int argc, char* argv[]) {
    gzFile_s* fp = gzopen(argv[1], "r");

    char readBuffer[65536];
    GzFileReadStream is(fp, readBuffer, sizeof(readBuffer));

    Document d;
    d.ParseStream(is);
    cout << d["m"].GetObject()["c"].GetString() << endl;

    gzclose(fp);
    return 0;
}
```

NOTE: compile with `-lz`: `g++ -lz main.cpp`
