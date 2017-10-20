var NAVTREE =
[
  [ "RapidJSON", "index.html", [
    [ "Change Log", "md__c_h_a_n_g_e_l_o_g.html", null ],
    [ "Features", "md_doc_features.html", null ],
    [ "Tutorial", "md_doc_tutorial.html", [
      [ "Value & Document", "md_doc_tutorial.html#ValueDocument", null ],
      [ "Query Value", "md_doc_tutorial.html#QueryValue", [
        [ "Query Array", "md_doc_tutorial.html#QueryArray", null ],
        [ "Query Object", "md_doc_tutorial.html#QueryObject", null ],
        [ "Querying Number", "md_doc_tutorial.html#QueryNumber", null ],
        [ "Query String", "md_doc_tutorial.html#QueryString", null ]
      ] ],
      [ "Create/Modify Values", "md_doc_tutorial.html#CreateModifyValues", [
        [ "Change Value Type", "md_doc_tutorial.html#ChangeValueType", null ],
        [ "Move Semantics", "md_doc_tutorial.html#MoveSemantics", [
          [ "Move semantics and temporary values", "md_doc_tutorial.html#TemporaryValues", null ]
        ] ],
        [ "Create String", "md_doc_tutorial.html#CreateString", null ],
        [ "Modify Array", "md_doc_tutorial.html#ModifyArray", null ],
        [ "Modify Object", "md_doc_tutorial.html#ModifyObject", null ],
        [ "Deep Copy Value", "md_doc_tutorial.html#DeepCopyValue", null ],
        [ "Swap Values", "md_doc_tutorial.html#SwapValues", null ]
      ] ],
      [ "What's next", "md_doc_tutorial.html#WhatsNext", null ]
    ] ],
    [ "Pointer", "md_doc_pointer.html", [
      [ "JSON Pointer", "md_doc_pointer.html#JsonPointer", null ],
      [ "Basic Usage", "md_doc_pointer.html#BasicUsage", null ],
      [ "Helper Functions", "md_doc_pointer.html#HelperFunctions", null ],
      [ "Resolving Pointer", "md_doc_pointer.html#ResolvingPointer", null ],
      [ "Error Handling", "md_doc_pointer.html#ErrorHandling", null ],
      [ "URI Fragment Representation", "md_doc_pointer.html#URIFragment", null ],
      [ "User-Supplied Tokens", "md_doc_pointer.html#UserSuppliedTokens", null ]
    ] ],
    [ "Stream", "md_doc_stream.html", [
      [ "Memory Streams", "md_doc_stream.html#MemoryStreams", [
        [ "StringStream (Input)", "md_doc_stream.html#StringStream", null ],
        [ "StringBuffer (Output)", "md_doc_stream.html#StringBuffer", null ]
      ] ],
      [ "File Streams", "md_doc_stream.html#FileStreams", [
        [ "FileReadStream (Input)", "md_doc_stream.html#FileReadStream", null ],
        [ "FileWriteStream (Output)", "md_doc_stream.html#FileWriteStream", null ]
      ] ],
      [ "iostream Wrapper", "md_doc_stream.html#iostreamWrapper", [
        [ "IStreamWrapper", "md_doc_stream.html#IStreamWrapper", null ],
        [ "OStreamWrapper", "md_doc_stream.html#OStreamWrapper", null ]
      ] ],
      [ "Encoded Streams", "md_doc_stream.html#EncodedStreams", [
        [ "EncodedInputStream", "md_doc_stream.html#EncodedInputStream", null ],
        [ "EncodedOutputStream", "md_doc_stream.html#EncodedOutputStream", null ],
        [ "AutoUTFInputStream", "md_doc_stream.html#AutoUTFInputStream", null ],
        [ "AutoUTFOutputStream", "md_doc_stream.html#AutoUTFOutputStream", null ]
      ] ],
      [ "Custom Stream", "md_doc_stream.html#CustomStream", [
        [ "Example: istream wrapper", "md_doc_stream.html#ExampleIStreamWrapper", null ],
        [ "Example: ostream wrapper", "md_doc_stream.html#ExampleOStreamWrapper", null ]
      ] ],
      [ "Summary", "md_doc_stream.html#Summary", null ]
    ] ],
    [ "Encoding", "md_doc_encoding.html", [
      [ "Unicode", "md_doc_encoding.html#Unicode", [
        [ "Unicode Transformation Format", "md_doc_encoding.html#UTF", null ],
        [ "Character Type", "md_doc_encoding.html#CharacterType", null ],
        [ "AutoUTF", "md_doc_encoding.html#AutoUTF", null ],
        [ "ASCII", "md_doc_encoding.html#ASCII", null ]
      ] ],
      [ "Validation & Transcoding", "md_doc_encoding.html#ValidationTranscoding", [
        [ "Transcoder", "md_doc_encoding.html#Transcoder", null ]
      ] ]
    ] ],
    [ "DOM", "md_doc_dom.html", [
      [ "Template", "md_doc_dom.html#Template", [
        [ "Encoding", "md_doc_dom.html#Encoding", null ],
        [ "Allocator", "md_doc_dom.html#Allocator", null ]
      ] ],
      [ "Parsing", "md_doc_dom.html#Parsing", [
        [ "Parse Error", "md_doc_dom.html#ParseError", null ],
        [ "In Situ Parsing", "md_doc_dom.html#InSituParsing", null ],
        [ "Transcoding and Validation", "md_doc_dom.html#TranscodingAndValidation", null ]
      ] ],
      [ "Techniques", "md_doc_dom.html#Techniques", [
        [ "User Buffer", "md_doc_dom.html#UserBuffer", null ]
      ] ]
    ] ],
    [ "SAX", "md_doc_sax.html", [
      [ "Reader", "md_doc_sax.html#Reader", [
        [ "Handler", "md_doc_sax.html#Handler", null ],
        [ "GenericReader", "md_doc_sax.html#GenericReader", null ],
        [ "Parsing", "md_doc_sax.html#SaxParsing", null ],
        [ "Token-by-Token Parsing", "md_doc_sax.html#TokenByTokenParsing", null ]
      ] ],
      [ "Writer", "md_doc_sax.html#Writer", [
        [ "Template", "md_doc_sax.html#WriterTemplate", null ],
        [ "PrettyWriter", "md_doc_sax.html#PrettyWriter", null ],
        [ "Completeness and Reset", "md_doc_sax.html#CompletenessReset", null ]
      ] ],
      [ "Techniques", "md_doc_sax.html#SaxTechniques", [
        [ "Parsing JSON to Custom Data Structure", "md_doc_sax.html#CustomDataStructure", null ],
        [ "Filtering of JSON", "md_doc_sax.html#Filtering", null ]
      ] ]
    ] ],
    [ "Schema", "md_doc_schema.html", null ],
    [ "Performance", "md_doc_performance.html", null ],
    [ "Internals", "md_doc_internals.html", [
      [ "Architecture", "md_doc_internals.html#Architecture", null ],
      [ "Value", "md_doc_internals.html#Value", [
        [ "Data Layout", "md_doc_internals.html#DataLayout", null ],
        [ "Flags", "md_doc_internals.html#Flags", null ],
        [ "Short-String Optimization", "md_doc_internals.html#ShortString", null ]
      ] ],
      [ "Allocator", "md_doc_internals.html#InternalAllocator", [
        [ "MemoryPoolAllocator", "md_doc_internals.html#MemoryPoolAllocator", null ]
      ] ],
      [ "Parsing Optimization", "md_doc_internals.html#ParsingOptimization", [
        [ "Skip Whitespaces with SIMD", "md_doc_internals.html#SkipwhitespaceWithSIMD", null ],
        [ "Local Stream Copy", "md_doc_internals.html#LocalStreamCopy", null ],
        [ "Parsing to Double", "md_doc_internals.html#ParsingDouble", null ]
      ] ],
      [ "Generation Optimization", "md_doc_internals.html#GenerationOptimization", [
        [ "Integer-to-String conversion", "md_doc_internals.html#itoa", null ],
        [ "Double-to-String conversion", "md_doc_internals.html#dtoa", null ]
      ] ],
      [ "Parser", "md_doc_internals.html#Parser", [
        [ "Iterative Parser", "md_doc_internals.html#IterativeParser", [
          [ "Grammar", "md_doc_internals.html#IterativeParserGrammar", null ],
          [ "Parsing Table", "md_doc_internals.html#IterativeParserParsingTable", null ],
          [ "Implementation", "md_doc_internals.html#IterativeParserImplementation", null ]
        ] ]
      ] ]
    ] ],
    [ "FAQ", "md_doc_faq.html", null ],
    [ "Modules", "modules.html", "modules" ],
    [ "Namespace Members", "namespacemembers.html", [
      [ "All", "namespacemembers.html", null ],
      [ "Functions", "namespacemembers_func.html", null ],
      [ "Typedefs", "namespacemembers_type.html", null ],
      [ "Enumerations", "namespacemembers_enum.html", null ],
      [ "Enumerator", "namespacemembers_eval.html", null ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", "functions_func" ],
        [ "Variables", "functions_vars.html", null ],
        [ "Typedefs", "functions_type.html", null ],
        [ "Related Functions", "functions_rela.html", null ]
      ] ]
    ] ],
    [ "Files", null, [
      [ "File List", "files.html", "files" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"allocators_8h_source.html",
"classrapidjson_1_1_generic_object.html#af5d1661531777782d90249fe5ee748cb",
"classrapidjson_1_1_generic_value.html#ac51a3b3046aaa12aa1d88ac876a28cec",
"group___r_a_p_i_d_j_s_o_n___c_o_n_f_i_g.html#ga6a2b1695c13e77ae425e3cbac980ccb5",
"structrapidjson_1_1_base_reader_handler.html#a2932a8ecbb1997dda305f4dbef32ab0d"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';