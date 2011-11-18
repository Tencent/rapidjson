#include "perftest.h"

#if TEST_YAJL

#ifdef _MSC_VER
#include <float.h>
#define isinf !_finite
#define isnan _isnan
#define snprintf _snprintf
#endif

#include "yajl/src/yajl.c"
#include "yajl/src/yajl_alloc.c"
#include "yajl/src/yajl_buf.c"
#include "yajl/src/yajl_encode.c"
#include "yajl/src/yajl_gen.c"
#include "yajl/src/yajl_lex.c"
#include "yajl/src/yajl_parser.c"
#include "yajl/src/yajl_tree.c"
#include "yajl/src/yajl_version.c"

#endif // TEST_YAJL
