#ifndef __INSTANCE_VALIDATOR_H__
#define __INSTANCE_VALIDATOR_H__

/**
 * @file instance_validator.h
 * @author Amine Aouled Hamed
 * @date 12 Nov. 2015
 * @brief Used to validate an instance against a v4 schema
 * 
 * @see http://json-schema.org/latest/json-schema-validation.html
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <json-c/json.h>

/**
 * @brief internal structure to manipulate object easily
 */
struct
jsonschema_object {
    char * key;
    int object_pos; /* if the root element is an array of objects, this holds the position of the object in the array (the main object to validate) */
    json_object* instance;
    json_object* instance_schema;
};

/**
 * @brief Returns the keyword object from an lh_entry object if it is found, otherwise NULL
 */
struct lh_entry *
json_get_keyword_entry(const struct lh_table* entry, const char* key);

/**
 * @brief Validates a numeric instance
 */
int 
json_validate_numeric_keywords(struct jsonschema_object instance_object);

/**
 * @brief Validates a string instance
 */
int 
json_validate_string_keywords(struct jsonschema_object instance_object);

/**
 * @brief Validates an array instance
 */
int 
json_validate_array_keywords(struct jsonschema_object instance_object);

/**
 * @brief Validates an object instance
 */
int 
json_validate_object_keywords(struct jsonschema_object instance_object);

/**
 * @brief Validates keywords that apply to any type(enum,type,etc...)
 */
int 
json_validate_anytype_keywords(struct jsonschema_object instance_object);

/**
 * @brief Validates meta keywords (name,description, format etc...)
 */
// int 
// json_validate_metakeywords(const struct json_object* instance_entry,const struct json_object* schema_entry);

/**
 * @brief Validates all keywords in an instance
 */
int 
json_validate_instance_keywords(struct jsonschema_object instance_object,const int key_type);

/**
 * @brief Validates an array instance
 */
int
json_validate_array_instance(struct jsonschema_object instance_object);

/**
 * @brief Validates an object instance
 */
int
json_validate_object_instance(struct jsonschema_object instance_object);

/**
 * @brief Validate a JSON object against a JSON schema loading both from files
 */
int
json_validate_instance_from_file(const char *instance_path, const char * schema_path);

/**
 * @brief Validate a JSON object against a JSON schema
 */
int
json_validate_instance(struct json_object* jso, struct json_object* schema);

#endif
