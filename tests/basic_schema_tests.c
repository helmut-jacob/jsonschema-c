#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include <schema_validator.h>
#include <instance_validator.h>

#ifdef TEST_FORMATTED
#define json_object_to_json_string(obj) json_object_to_json_string_ext(obj,sflags)
#else
/* no special define */
#endif

int main(int argc, char *argv[])
{
	struct json_object *js;

	/* validate a minimal schema -> always valid */
	js = json_tokener_parse(
		"{"
		"\"$schema\":\"http://json-schema.org/draft-04/schema#\""
		"}");

	if (!json_validate_schema(js)) {
		printf("Minimal schema failed to validate\n");
		return 1;
	}
	json_object_put(js);

	/* unknown keywords need to be skipped */
	js = json_tokener_parse(
		"{"
		"\"$schema\":\"http://json-schema.org/draft-04/schema#\","
		"\"unknown_keywork\":\"\""
		"}");

	if (!json_validate_schema(js)) {
		printf("Schema with unknown keywords failed to validate\n");
		return 1;
	}
	json_object_put(js);

	/* type:object is valid */
	js = json_tokener_parse(
		"{"
		"\"$schema\":\"http://json-schema.org/draft-04/schema#\","
		"\"type\":\"object\""
		"}");

	if (!json_validate_schema(js)) {
		printf("Schema with type object failed to validate\n");
		return 1;
	}
	json_object_put(js);

	/* type:invalid */
	js = json_tokener_parse(
		"{"
		"\"$schema\":\"http://json-schema.org/draft-04/schema#\","
		"\"type\":\"invalid\""
		"}");

	if (json_validate_schema(js)) {
		printf("Schema with invalid type was validated\n");
		return 1;
	}
	json_object_put(js);

	return 0;
}
