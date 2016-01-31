// Schema Validator example

// The example validates JSON text from stdin with a JSON schema specified in the argument.

#include "rapidjson/error/en.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/schema.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: schemavalidator schema.json < input.json\n");
        return EXIT_FAILURE;
    }

    // Read a JSON schema from file into Document
    Document d;
    char buffer[4096];

    {
        FILE *fp = fopen(argv[1], "r");
        if (!fp) {
            printf("Schema file '%s' not found\n", argv[1]);
            return -1;
        }
        FileReadStream fs(fp, buffer, sizeof(buffer));
        d.ParseStream(fs);
        if (d.HasParseError()) {
            fprintf(stderr, "Schema file '%s' is not a valid JSON\n", argv[1]);
            fprintf(stderr, "Error(offset %u): %s\n",
                static_cast<unsigned>(d.GetErrorOffset()),
                GetParseError_En(d.GetParseError()));
            fclose(fp);
            return EXIT_FAILURE;
        }
        fclose(fp);
    }
    
    // Then convert the Document into SchemaDocument
    SchemaDocument sd(d);

    // Use reader to parse the JSON in stdin, and forward SAX events to validator
    SchemaValidator validator(sd);
    Reader reader;
    FileReadStream is(stdin, buffer, sizeof(buffer));
    if (!reader.Parse(is, validator) && reader.GetParseErrorCode() != kParseErrorTermination) {
        // Schema validator error would cause kParseErrorTermination, which will handle it in next step.
        fprintf(stderr, "Input is not a valid JSON\n");
        fprintf(stderr, "Error(offset %u): %s\n",
            static_cast<unsigned>(reader.GetErrorOffset()),
            GetParseError_En(reader.GetParseErrorCode()));
    }

    // Check the validation result
    if (validator.IsValid()) {
        printf("Input JSON is valid.\n");
        return EXIT_SUCCESS;
    }
    else {
        printf("Input JSON is invalid.\n");
        StringBuffer sb;
        validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
        fprintf(stderr, "Invalid schema: %s\n", sb.GetString());
        fprintf(stderr, "Invalid keyword: %s\n", validator.GetInvalidSchemaKeyword());
        sb.Clear();
        validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
        fprintf(stderr, "Invalid document: %s\n", sb.GetString());
        return EXIT_FAILURE;
    }
}
