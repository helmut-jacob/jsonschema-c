#ifndef __JSONSCHEMA_VALIDATOR_H__
#define __JSONSCHEMA_VALIDATOR_H__

/**
 * @file schema_validator.h
 * @author Amine Aouled Hamed
 * @date 04 Nov. 2015
 * @brief Used to validate a schema against the V4 draft specification
 * 
 * @see http://json-schema.org/latest/json-schema-validation.html
 */

#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <ctype.h>
#include <json-c/json.h>
#include <json-c/linkhash.h>

enum json_keywords {
    /* numeric keywords */
    json_keyword_multipleOf,
    json_keyword_maximum,
    json_keyword_minimum,
    json_keyword_exclusiveMinimum,
    json_keyword_exclusiveMaximum,
    /* string keywords */
    json_keyword_maxLength,
    json_keyword_minLength,
    json_keyword_pattern,
    /* array keywords */
    json_keyword_additionalItems,
    json_keyword_items,
    json_keyword_minItems,
    json_keyword_maxItems,
    json_keyword_uniqueItems,
    /* object keywords */
    json_keyword_maxProperties,
    json_keyword_minProperties,
    json_keyword_required,
    json_keyword_properties,
    json_keyword_patternProperties,
    json_keyword_additionalProperties,
    json_keyword_dependencies,
    /* any type keywords */
    json_keyword_enum,
    json_keyword_type,
    json_keyword_allOf,
    json_keyword_anyOf,
    json_keyword_oneOf,
    json_keyword_not,
    json_keyword_definitions,
    json_keyword_title,
    json_keyword_description,
    json_keyword_default,
    json_keyword_format,
    json_keyword_schema
};

enum json_allowed_values {
    json_strictly_gt_0,
    json_gt_or_eqt_0,
    json_array_of_ojbects,
    json_array_of_unique_string_items_at_least_one_item,
    json_valid_regex,
    json_name_valid_regex,
    json_array_of_unique_items_at_least_one_item,
    json_value_in_an_array_of_values,
    json_value_matches_regex,
    json_value_is_object,
    json_no_rule = -1
};

enum json_format_keywords {
    json_format_date_time,
    json_format_email,
    json_format_hostname,
    json_format_ipv4,
    json_format_ipv6,
    json_format_uri,
    json_format_null,
};

/**
 * @brief mapping between json-c types and their string representation
 */
struct json_enum_to_string_map {
    int ikey;
    const char  *skey;
};
//all reserved keywords
extern char* json_keywords[];

/**
 * @brief Array contains all constraints of each keyword in the V4 spec
 *         {keyword,type,parent_type,dependencies,allowed values, children allowed values}
 *        allowed values = "-1" => no specific rules, just verify the value is of the required type
 *                         "0" => strictly greater than 0
 *                         "1" => greater than or equal to 0
 *                         "2" => array of objects
 *                         "3" => array of unique string items and at least one item is provided
 *                         "4" => a valid regex
 *                         "5" => name of the object should be a valid regex
 *                         "6" => array of unique items and at least one item is provided
 *                         "7" => value must be in a given array of values
 *                         "8" => value must match a given regex
 *                         "9" => value is an object
 */
extern char *json_keywords_constraints[][6];

/**
 * @brief array containing mapping between type values and their IDs in the json-c lib
 */
extern struct json_enum_to_string_map json_types_id[] ;

/**
 * @brief Use this array to check for case 8 in the allowed values
 */
extern char *json_values[] ;

/**
 * @brief regex to validate each format 
 */
extern char *json_format_regex[];

/**
 * @brief returns the id of a keyword from the keywords array
 */
int 
json_get_keyword_id(const char *keyword);

/**
 * @brief returns a json-c type from a schema object
 */
int 
json_get_key_type(json_object *obj);

/**
 * @brief Checks that array items are unique
 */
int 
json_validate_array_items_uniqueness(json_object *jobj, const int type);

/**
 * @brief Checks that array items are objects with valid JSON schema
 */
int
json_validate_array_items(json_object *jobj);

/**
 * @brief Checks if the given string is a reserved v4 keyword
 */
int 
json_is_keyword(const char *keyword);

/**
 * @brief Take a string for the keyword_constraints array and return all digits
 */
void
json_split_str(int keyword_position, int *allowed_types, int str_pos);

/**
 * @brief Validates a regular expression against the ECMA 262 spec
 */
int 
json_validate_regex(const char *regex);

/**
 * @brief Validates the parent's type of a keyword
 */
int
json_validate_parent_type(const char *type, const int keyword_position);

/**
 * @brief Validates the type of a keyword
 */
int 
json_validate_type(int type,int keyword_position);

/**
 * @brief Validates the dependencies of a keyword
 */
int 
json_check_dependencies(int keyword_position, lh_table *parent);

/**
 * @brief Validates the supplied value of a keyword
 */
int 
json_check_allowed_values(int keyword_position,struct json_object * value, struct lh_table* parent, int items_values);

/**
 * @brief Validates a keyword, by checking the provided keyword against the rules of the V4 draft. check schema_rules.txt file
 */
int
json_validate_keyword(struct lh_entry *keyword, struct lh_table *parent_type);

/**
 * @brief Validates a schema, by checking the provided schema against the rules of the V4 draft. check schema_rules.txt file
 */
int 
json_validate_schema(struct json_object* schema);

/**
 * @brief Validates a schema, by checking the provided schema against the rules of the V4 draft. check schema_rules.txt file
 */
int 
json_validate_schema_from_file(const char *filename);


#endif
