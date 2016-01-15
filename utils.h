#ifndef __UTILS_H__
#define __UTILS_H__

/**
 * @file utils.h
 * @author Amine Aouled Hamed
 * @date 04 Nov. 2015
 * @brief Utility functions for the schema validator
 * 
 * @see http://json-schema.org/latest/json-schema-validation.html
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <json-c/json.h>
#include <json-c/linkhash.h>

//terminal colors
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

/* Maximum number of error messages to be reported */
#define MAX_ERR_MSGS 32
/**
 * @brief contains error elements
 */
struct 
json_error {
    char * key; //key that got the error
    int obj_pos; // object the key belongs too
    char * error_message;
    int error_code;
}; 

enum 
json_error_values {
    json_invalid_value_error = -1000,
    json_multipleOf_error ,
    json_maximum_error,
    json_minimum_error,
    json_exclusiveMinimum_error,
    json_exclusiveMaximum_error,
    json_maxLength_error,
    json_minLength_error,
    json_pattern_error,
    json_additionalItems_error,
    json_items_error,
    json_minItems_error,
    json_maxItems_error,
    json_uniqueItems_error,
    json_maxProperties_error,
    json_minProperties_error,
    json_required_error,
    json_properties_error,
    json_patternProperties_error,
    json_additionalProperties_error,
    json_dependencies_error,
    json_enum_error,
    json_type_error,
    json_allOf_error,
    json_anyOf_error,
    json_oneOf_error,
    json_not_error,
    json_definitions_error,
    json_format_error
};

char* json_error_messages[MAX_ERR_MSGS];

/**
 * @brief Prints a colored message in the terminal.
 */
void
json_printf_colored(char* color,char * message, ...);

/**
 * @brief   adds an error code to the list of errors and return the new size of the list.
 *          If the last argument is not NULL, it will be filled with the list.
 *          If the first 3 argument are 0 and NULL respectively, they will be ignored and no errors will be added.
 */
struct json_error *
json_add_error(int error_value, char* key,int obj_pos,int * num_errors);

/**
 * @brief Sorts items of a JSON array. copied from test1.c
 */
int 
json_sort_fn (const void *j1, const void *j2);

#endif