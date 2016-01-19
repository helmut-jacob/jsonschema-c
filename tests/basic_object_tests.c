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
	struct json_object *o;

	/* simple schema -> any object */
	js = json_tokener_parse(
		"{"
		"\"$schema\":\"http://json-schema.org/draft-04/schema#\","
		"\"type\":\"object\""
		"}");

	/* validate empty object */
	o = json_tokener_parse("{}");
	if (!json_validate_instance(o, js)) {
		printf("Simple object failed to validate\n");
		return 1;
	}
	json_object_put(o);

	/* validate simple int against object schema */
	o = json_tokener_parse("1");
	if (json_validate_instance(o, js)) {
		printf("integer validated against simple object schema\n");
		return 1;
	}
	json_object_put(o);

	json_object_put(js);

	return 0;
}
