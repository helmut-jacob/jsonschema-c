#include "schema_validator.h"
#include "utils.h"

#define MAX_CONSTRAINTS 8 // maximum number of constraints when validating keyword values(or children values)
char* json_keywords_char[] = {[json_keyword_multipleOf] = "multipleOf",
                    [json_keyword_maximum] = "maximum",
                    [json_keyword_minimum] = "minimum",
                    [json_keyword_exclusiveMinimum] = "exclusiveMinimum",
                    [json_keyword_exclusiveMaximum] = "exclusiveMaximum",
                    [json_keyword_maxLength] = "maxLength",
                    [json_keyword_minLength] = "minLength",
                    [json_keyword_pattern] = "pattern",
                    [json_keyword_additionalItems] = "additionalItems",
                    [json_keyword_items] = "items",
                    [json_keyword_minItems] = "minItems",
                    [json_keyword_maxItems] = "maxItems",
                    [json_keyword_uniqueItems] = "uniqueItems",
                    [json_keyword_maxProperties] = "maxProperties",
                    [json_keyword_minProperties] = "minProperties",
                    [json_keyword_required] = "required",
                    [json_keyword_properties] = "properties",
                    [json_keyword_patternProperties] = "patternProperties",
                    [json_keyword_additionalProperties] = "additionalProperties",
                    [json_keyword_dependencies] = "dependencies",
                    [json_keyword_enum] = "enum",
                    [json_keyword_type] = "type",
                    [json_keyword_allOf] = "allOf",
                    [json_keyword_anyOf] = "anyOf",
                    [json_keyword_oneOf] = "oneOf",
                    [json_keyword_not] = "not",
                    [json_keyword_definitions] = "definitions",
                    [json_keyword_title] = "title",
                    [json_keyword_description] = "description",
                    [json_keyword_default] = "default",
                    [json_keyword_format] = "format",
                    [json_keyword_schema] = "$schema"
                };

char * json_keywords_constraints[][6] = {{"multipleOf","2 | 3","3 | 7", "-1","0","-1"},
                                    {"maximum","2 | 3","3 | 7", "-1","-1","-1"},
                                    {"minimum","2 | 3","3 | 7", "-1","-1","-1"},
                                    {"exclusiveMinimum","1","3 | 7", "2","-1","-1"},
                                    {"exclusiveMaximum","1","3 | 7", "1","-1","-1"},
                                    {"maxLength","3","6", "-1","1","-1"},
                                    {"minLength","3","6", "-1","1","-1"},
                                    {"pattern","6","6", "-1","4","-1"},
                                    {"additionalItems","1 | 4","5", "-1","-1","-1"},
                                    {"items","4 | 5","5", "-1","2","-1"},
                                    {"minItems","3","5", "-1","1","-1"},
                                    {"maxItems","3","5", "-1","1","-1"},
                                    {"uniqueItems","1","5", "-1","-1","-1"},
                                    {"maxProperties","3","4", "-1","1","-1"},
                                    {"minProperties","3","4", "-1","1","-1"},
                                    {"required","5","4", "-1","3","-1"},
                                    {"properties","4","4", "-1","-1","-1"},
                                    {"patternProperties","4","4", "-1","5","-1"},
                                    {"additionalProperties","1 | 4","4", "-1","-1","-1"},
                                    {"dependencies","4","4", "-1","-1","3 | 9"},
                                    {"enum","5","-1", "-1","6","-1"},
                                    {"type","6","-1", "-1","7","-1"},
                                    {"allOf","5","-1", "-1","2","-1"},
                                    {"anyOf","5","-1", "-1","2","-1"},
                                    {"oneOf","5","-1", "-1","2","-1"},
                                    {"not","4","-1", "-1","-1","-1"},
                                    {"definitions","4","-1", "-1","-1","-1"},
                                    {"title","6","-1", "-1","-1","-1"},
                                    {"description","6","-1", "-1","-1","-1"},
                                    {"default","-1","-1", "-1","-1","-1"},
                                    {"format","6","-1", "-1","7","-1"},
                                    {"$schema","6","-1", "-1","-1","-1"},// not handled yet, just add it to not get errors when it is present
                                };

struct json_enum_to_string_map json_types_id[] = {{.ikey = json_type_null, .skey = "null" },
                                                  {.ikey = json_type_boolean, .skey = "boolean"},
                                                  {.ikey = json_type_double, .skey = "double"},
                                                  {.ikey = json_type_int, .skey = "integer"},
                                                  {.ikey = json_type_object, .skey ="object"},
                                                  {.ikey = json_type_array, .skey ="array"},
                                                  {.ikey = json_type_string, .skey ="string"},
                                                  {.ikey = 7, .skey ="number"},
                                                  {0,NULL}
                                                }; 

char *json_values[] = {[json_format_date_time] = "date-time",
                        [json_format_email] = "email",
                        [json_format_hostname] = "hostname",
                        [json_format_ipv4] = "ipv4",
                        [json_format_ipv6] = "ipv6",
                        [json_format_uri] = "uri", //format keyword
                        "null","boolean","double","integer","number","object","array","string",// 7 primitive types of json-c + "number" is allowed in v4 draft
                        NULL
                    };

char *json_format_regex[] = {[json_format_date_time] =  "^([0-9]{4})-([0-9]{2})-([0-9]{2})"
                                                        "([Tt]([0-9]{2}):([0-9]{2}):([0-9]{2})(\\.[0-9]+)?)?"
							"([Tt]([0-9]{2}):([0-9]{2}):([0-9]{2})(\\.[0-9]+)?)?"
							"(([Zz]|([+-])([0-9]{2}):([0-9]{2})))?",
                             [json_format_email] = "^[A-Z0-9._%+\\-]+@[A-Z0-9.-]+\\.[A-Z]{2,10}$",
                             [json_format_hostname] = "(?:[^0-9][a-zA-Z0-9]+(?:(?:\\-|\\.)[a-zA-Z0-9]+)*)",
                             [json_format_ipv4] = "*",
                             [json_format_ipv6] = "*",
                             [json_format_uri] = "^([A-Z][-A-Z0-9+&@#/%=~_|]*)://"
						 "[-A-Za-z0-9+&@#/%?=~_|!:,.;]*[-A-Z0-9+&@#/%=~_|]$" //format keyword
};

int
json_get_keyword_id(const char *keyword){
	int i, size = sizeof(json_keywords_char)/sizeof(json_keywords_char[0]);
	for(i = 0; i< size; i++)
		if(strcmp(keyword, json_keywords_char[i]) == 0){
			return i;
		}
	return -1;
}
int 
json_get_key_type(json_object * obj){
	struct lh_entry *type_entry = lh_table_lookup_entry(json_object_get_object(obj),"type");
	//assume its an object(id=4) by default
	int key_type = json_type_object, i; // json-c types + "number" type

	//lookup the type in the schema key 
	if(type_entry != NULL){
		//get the type id from our private array
		for(i=0; json_types_id[i].skey; i++){
		if(strcmp(json_types_id[i].skey, json_object_get_string((json_object*)type_entry->v))==0){
			key_type = json_types_id[i].ikey;
			break;
		}
		}
	}
	return key_type;
}

int 
json_validate_array_items_uniqueness(json_object *jobj, const int type){
	if(json_object_get_type(jobj) != json_type_array)
	{
		json_printf_colored(ANSI_COLOR_RED,"type must be array.");
		return 0;
	}
	int arraylen = json_object_array_length(jobj); /*Getting the length of the array*/
	int i;
	//sort the array
	json_object_array_sort(jobj,json_sort_fn);
	//verify that every item is unique
	for (i=0; i< arraylen-1; i++){
		json_object * jvalue = json_object_array_get_idx(jobj, i);
		json_object * jvalue1 = json_object_array_get_idx(jobj, i+1);
		if(type != -1)
			if(json_object_get_type(jvalue) != type)
				return 0; // not valid type
		if(strcmp(json_object_to_json_string(jvalue),json_object_to_json_string(jvalue1)) == 0)
			return 0; //not unique
	}
	return 1;
}

int
json_validate_array_items(json_object *jobj) {
	if(json_object_get_type(jobj) != json_type_array)
	{
		json_printf_colored(ANSI_COLOR_RED,"type must be array.");
		return 0;
	}
	enum json_type type;

	int arraylen = json_object_array_length(jobj); /*Getting the length of the array*/
	int i;
	json_object * jvalue;

	//loop through array items
	for (i=0; i< arraylen; i++){
		jvalue = json_object_array_get_idx(jobj, i); /*Getting the array element at position i*/
		type = json_object_get_type(jvalue);
		if (type != json_type_object) {
			printf("array item #%d must be an object",i);
			return 0;
		}
	}
	return 1;
}

int 
json_is_keyword(const char* keyword){
	int array_size = sizeof(json_keywords_char)/sizeof(json_keywords_char[0]);
	int i;

	for(i=0 ; i < array_size ; i++){
		if(strcmp(keyword,json_keywords_char[i]) == 0){
			return i; // return the position of the keyword
		}
	}
	return -1; // not found
}

void
json_split_str(const int keyword_position, int *return_array, const int str_pos){
	//find the keyword type
	int i=0;
	char *p = json_keywords_constraints[keyword_position][str_pos];
	while (*p) { // While there are more characters to process...
		if (isdigit(*p)) { // Upon finding a digit, ...
			int val = strtol(p, &p, 10); // Read a number, ...
			return_array[i++] = val;
		}else if(*p == '-' || *p == '+') {
			int val = strtol(p, &p, 10); // Read a number, ...
			return_array[i++] = val;
		}
		else {
			p++;
		}
	}
}

int 
json_validate_regex(const char * regex){
	//pattern must conform the ECMA 262
	regex_t pattern;
	memset(&pattern, 0, sizeof(regex_t));
	int err;
	if((err = regcomp(&pattern, regex,REG_EXTENDED | REG_NOSUB))){
		char buffer[256];
		regerror(err,&pattern,buffer,sizeof(buffer)/sizeof(buffer[0]));
		char buff[256];
		sprintf(buff,"invalid pattern with error: %s", buffer);
		json_printf_colored(ANSI_COLOR_RED , buff);
		return 0;
	}
	return 1;
}

int
json_validate_parent_type(const char *type, const int keyword_position){
	//there should be at maximum two types for the parent
	int parent_type[2] = {8,8};
	int i;

	json_split_str(keyword_position,parent_type,2);
	//if a parent type validation is not required, return directly
	if(parent_type[0] == -1)
		return 1;
	//get parent type id
	for(i=0; json_types_id[i].skey; i++){
		if(strcmp(type,json_types_id[i].skey) == 0)
			if( json_types_id[i].ikey == parent_type[0] || json_types_id[i].ikey == parent_type[1])
				return 1;
	}
	return 0;
}

int 
json_validate_value_regex(const char *key, json_object *value){
	/* search for the regex in the regex array */
	char *regex="*";
	int i;
	for(i=0; json_format_regex[i]; i++)
		if(strcmp(json_values[i], key) == 0){
			regex = json_format_regex[i];
			break; 
		}
	/* compile the regex */
	//pattern must conform the ECMA 262
	regex_t pattern;
	memset(&pattern, 0, sizeof(regex_t));
	int err;
	if((err = regcomp(&pattern, regex,REG_EXTENDED | REG_NOSUB))){
		char buffer[256];
		regerror(err,&pattern,buffer,sizeof(buffer)/sizeof(buffer[0]));
		json_printf_colored(ANSI_COLOR_RED , "invalid pattern with error: %s", buffer);
		return 0;
	}
	return 1;
}

int 
json_validate_type(const int type, const int keyword_position){
	int allowed_types[8] = {8,8,8,8,8,8,8,8}; //max 7 types
	int i;
	json_split_str(keyword_position,allowed_types,1);
	for(i=0;i<8;i++){
		if(allowed_types[i] == -1)
			return 1;
		if(type == allowed_types[i]) {
			return 1;
		}else if(allowed_types[i] > 6)
			//w've reached the number of types
			break;
	}
	return 0;
}

int 
json_check_dependencies(const int keyword_position, lh_table* parent){
	int dep = atoi(json_keywords_constraints[keyword_position][3]); // position of the required keyword
	if(dep > -1){
		char * dep_keyword = json_keywords_constraints[dep][0] ; // the keyword our keyword depends on
		if (lh_table_lookup_entry(parent,dep_keyword) == NULL ){
		//need to be discussed
	//             //if we have the allOf, oneOf, anyOf or not keywords, they may contain our dependency.
	//             struct lh_entry* temp;
	//             if((temp = lh_table_lookup_entry(parent, "anyOf")) != NULL){
	//                 printf("ANYOF FOUND!\n");
	//                 if (lh_table_lookup_entry(json_object_get_object((json_object*)temp->v),dep_keyword) != NULL ) 
	//                     return 1;
	//             }
	//             if((temp = lh_table_lookup_entry(parent, "allOf")) != NULL){
	//                 printf("ALLOF FOUND! %s %d \n",(char*) temp->k,json_object_get_type((json_object*)temp->v));
	//                 if (lh_table_lookup_entry(json_object_get_object((json_object*)temp->v),dep_keyword) != NULL ) 
	//                     return 1;
	//             }
	//             if((temp = lh_table_lookup_entry(parent, "oneOf")) != NULL){
	//                 printf("ONEOF FOUND! \n");
	//                 if (lh_table_lookup_entry(json_object_get_object((json_object*)temp->v),dep_keyword) != NULL ) 
	//                     return 1;
	//             }
	//             if((temp = lh_table_lookup_entry(parent, "not")) != NULL){
	//                 printf("NOT FOUND! \n");
	//                 if (lh_table_lookup_entry(json_object_get_object((json_object*)temp->v),dep_keyword) != NULL ) 
	//                     return 1;
	//             }
		return 0;
		}
	}
	return 1;
}

int 
json_check_allowed_values(const int keyword_position,struct json_object * value, struct lh_table* parent, const int items_values){
	int allowed_values[MAX_CONSTRAINTS], i, res = 1;
	if(items_values == 1)
		json_split_str(keyword_position,(int *)&allowed_values,5); // get allowed children values ID
	else
		json_split_str(keyword_position,(int *)&allowed_values,4); // get allowed values ID

	const char *key = json_keywords_constraints[keyword_position][0];
	const int array_length = sizeof(json_values)/sizeof(json_values[0]);
	for(i=0; i< MAX_CONSTRAINTS; i++){
		switch(allowed_values[i]) {
		case json_strictly_gt_0:
			if(json_object_get_int(value) < 1){
				printf("value: %d\n",json_object_get_int(value));
				json_printf_colored(ANSI_COLOR_RED ,"value must be strictly greater than 0. ");
				res = 0;
			}
		break;
		case json_gt_or_eqt_0:
			if(json_object_get_int(value) < 0){
				printf("value: %d\n",json_object_get_int(value));
				json_printf_colored(ANSI_COLOR_RED ,"value must be greater than or equal to 0. ");
				res = 0;
			}
		break;
		case json_array_of_ojbects:
			if(json_object_get_type(value) == json_type_array)
				res = json_validate_array_items(value);
		break;
		case json_array_of_unique_string_items_at_least_one_item:
			if(json_object_get_type(value) == json_type_array){
				if(json_object_array_length(value) < 1){
					json_printf_colored(ANSI_COLOR_RED,"at least one item must be present in the array");
					res = 0;
				}
				res = json_validate_array_items_uniqueness(value,6);
			}
		break;
		case json_valid_regex:
			if(json_object_get_type(value) == json_type_string)
				res = json_validate_regex(json_object_get_string(value));
		break;
		case json_name_valid_regex:
			res = json_validate_regex(key);
		break;
		case json_array_of_unique_items_at_least_one_item:
			if(json_object_get_type(value) == json_type_array){
				if(json_object_array_length(value) < 1){
					json_printf_colored(ANSI_COLOR_RED,"at least one item must be present in the array");
					res = 0;
				}
				res = json_validate_array_items_uniqueness(value,-1); // no specific type is required
			}
		break;
		case json_value_in_an_array_of_values:
			if(json_object_get_type(value) == json_type_string)
				for(i=0; i<array_length; i++)
					if(strcmp(json_object_get_string(value),json_values[i]) == 0)
					return 1;
				res = 0;
		break;
		case json_value_matches_regex:
			res = json_validate_value_regex(key, value);
		case json_value_is_object:
		default:
			res = 1 ;
		}
		if(res == 1)
			break; //if one constraint succed, return success
	}
	return res;
}

int
json_validate_keyword(struct lh_entry* keyword, struct lh_table* parent){
	char * keyword_name = (char*)keyword->k;
	int keyword_position; // position in the keywords array
	struct json_object * value = (struct json_object *)keyword->v;
	enum json_type keyword_type = json_object_get_type(value);

	//check if it is a keyword or not
	if((keyword_position = json_is_keyword(keyword_name)) != -1){
		char * parent_type_value = "object"; //assume its an object from the beginning
		struct lh_entry* parent_type = lh_table_lookup_entry(parent,"type");
		if(parent_type == NULL ) {
			json_printf_colored(ANSI_COLOR_YELLOW,"---parent type not found assuming its an object" );
		}else{
			struct json_object * parent_type_object = (struct json_object * ) parent_type->v;
			//if the type is an array with more than an element, exit with an error.
			if(json_object_get_type(parent_type_object) == json_type_array){
				if(json_object_array_length(parent_type_object)>1) {
					json_printf_colored(ANSI_COLOR_RED,"array type of more than one item is not allowed");
					return 0;
				}
				struct json_object * item_value= json_object_array_get_idx(parent_type_object,0);
				//item should be a string
				if(json_object_get_type(item_value) != json_type_string){
					json_printf_colored(ANSI_COLOR_RED,"type: array item should be a string");
					return 0;
				}
				parent_type_value = (char*) json_object_get_string(item_value);
			}else if(json_object_get_type(parent_type_object) == json_type_string) {
				parent_type_value = (char*) json_object_get_string(parent_type_object);
			}else{
				json_printf_colored(ANSI_COLOR_RED,"type: value type should be either a string or an array");
				return 0;
			}
			//check parent type is valid
			if(json_validate_parent_type(parent_type_value,keyword_position) != 1){
				json_printf_colored(ANSI_COLOR_RED,"Invalid parent type");
				return 0;
			}
		}
		//check type is valid
		if(json_validate_type(keyword_type,keyword_position) != 1){
			json_printf_colored(ANSI_COLOR_RED,"Invalid type");
			return 0;
		}
		if(json_check_dependencies(keyword_position,parent) != 1){
			json_printf_colored(ANSI_COLOR_RED,"dependencies missing");
			return 0;
		}
		/* validate values */
		if(json_check_allowed_values(keyword_position, (struct json_object*)keyword->v, parent, 0) != 1){
			json_printf_colored(ANSI_COLOR_RED,"bad values");
			return 0;
		}
		if(json_object_is_type((struct json_object*)keyword->v, json_type_object)){
			/* validate children values */
			struct lh_entry *child;
			lh_foreach(json_object_get_object((struct json_object*)keyword->v),child){
				if(json_check_allowed_values(keyword_position, (struct json_object*)child->v, json_object_get_object((struct json_object*)keyword->v), 1) != 1)
				{
					json_printf_colored(ANSI_COLOR_RED,"bad children values");
					return 0;
				}
			}
		}
	}
	return 1;
}

int 
json_validate_object(json_object *jobj, const int last_pos) {
	struct lh_table* object_table = json_object_get_object(jobj);
	struct lh_entry* entry = object_table->head; 
	enum json_type type;
	static int res = 1;
	int obj_pos = 1 + last_pos;
	while(entry != NULL) {        
		char * key = (char*)entry->k;
		struct json_object * value = (struct json_object *)entry->v;
		int result;

		printf("**key: %*s\n", (int) strlen(key)+(obj_pos*2),key);
		type = json_object_get_type(value);
		//validate the keyword of the object first
		result = json_validate_keyword(entry,object_table);
		if (result != 1) 
			res = res && 0; // schema is not valid, but keep checking to get all errors.
		if(type == json_type_object)
			json_validate_object(value,obj_pos);
		entry = entry->next;
	}
	return res;
}

int 
json_validate_schema(struct json_object *schema) {
	int res;
	if (!schema)
		return 0;

	res = json_validate_object(schema,0);
	if(res != 1) {
		json_printf_colored(ANSI_COLOR_RED,"Invalid schema!");
		return 0;
	}else {
		json_printf_colored(ANSI_COLOR_GREEN,"Valid schema!");
		return 1;
	}
}

int 
json_validate_schema_from_file(const char* filename) {
	int res;
	json_object *schema = json_object_from_file(filename);
	if(schema == NULL) {
		json_printf_colored("could not load schema from file.",ANSI_COLOR_RED);
		return 0;
	}
	return json_validate_schema(schema);
}
