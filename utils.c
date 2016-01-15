#include "utils.h"


char* 
json_error_messages[] = {  "Invalid value",
                            "multipleOf: Value must be multiple of this keyword",
                            "maximum: Value must be strictly smaller than this keyword",
                            "minimum: Value must be strictly greater than this keyword",
                            "exclusiveMinimum: Value must be equal to or greater than this keyword",
                            "exclusiveMaximum: Value must be equal to or smaller than this keyword",
                            "maxLength: Value length must be equal to or smaller than this keyword",
                            "minLength: Value length must be equal to or greater than this keyword",
                            "pattern: Value must match the pattern",
                            "additionalItems: Number of items must be equal to or smaller than size of Items keyword",
                            "items: Number of items must be equal to or smaller than size of Items keyword",
                            "minItems: Array size must be equal to or greater than this keyword",
                            "maxItems: Array size must be equal to or smaller than this keyword",
                            "uniqueItems: Items must be unique",
                            "maxProperties: Number of properties must be equal to or smaller than this keyword",
                            "minProperties: Number of properties must be equal to or greater than this keyword",
                            "required: Items in this keyword array are required",
                            "properties: All properties must be in either properites or patternProperties or both",
                            "patternProperties: All properties must be in either properites or patternProperties or both",
                            "additionalProperties: All properties must be in either properites or patternProperties or both",
                            "dependencies: TO BE DONE",
                            "enum: Value must be one of the enumeration.",
                            "type: type must be one of the seven primitive types of json-schema",
                            "allOf: The instance must be valid against all of the schemas in this keyword",
                            "anyOf: Instance must be valid against at least one the schemas in this keyword",
                            "oneOf: Instance must be valid against exactly one of the schemas in this keyword",
                            "not: Instance must not be valid against the schema in this keyword",
                            "format: TO BE DONE"
};

void
json_printf_colored(char* color, char *message, ...) {
	printf("%s",color);
	va_list vl;
	va_start(vl,message);
	vprintf (message,vl);
	va_end(vl);
	printf ("\n" ANSI_COLOR_RESET);
}

struct json_error *
json_add_error(int error_value,char* key, int obj_pos,int * size){
	static struct json_error errors[256];
	static int i = 0;
	//if we want to add a new error
	if(error_value < 0 && key != NULL && obj_pos >= 0){
		errors[i].key = key;
		errors[i].obj_pos = obj_pos;
		errors[i].error_message = json_error_messages[1000+error_value];
		errors[i].error_code = error_value;
		i++;
	}
	if(size != NULL){
		*size = i;
	}
	return errors;
}
int 
json_sort_fn (const void *j1, const void *j2){
	json_object * const *jso1, * const *jso2;
	char* i1;
	char* i2;

	jso1 = (json_object* const*)j1;
	jso2 = (json_object* const*)j2;
	if (!*jso1 && !*jso2)
		return 0;
	if (!*jso1)
		return -1;
	if (!*jso2)
		return 1;

	i1 = (char *)json_object_get_string(*jso1);
	i2 = (char *)json_object_get_string(*jso2);
	return strcmp(i1,i2);
}