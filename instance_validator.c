#include "instance_validator.h"
#include "schema_validator.h"
#include "utils.h"

int 
json_compare_num(double num1, double num2, int strict){
	if(strict == 1)
		return (num1 > num2) - (num1 < num2);
	if(strict == 0)
		return (num1 >= num2) - (num1 <= num2);
	return -1;
}


struct json_object*
copy_object(struct json_object* obj){
	// parse the object and produce a new object
	const char *json_string = json_object_to_json_string(obj);
	return json_tokener_parse(json_string); 
}

struct lh_entry* 
json_get_keyword_entry(const lh_table *table, const char * instance_key){
	struct lh_entry * entry;
	lh_foreach(table,entry){
		const char * key = (const char*) entry->k;
		struct json_object* entry_object = (struct json_object*) entry->v;
		if(strcmp(key,instance_key) == 0){
			return entry;
		}
		if(json_object_get_type(entry_object) == json_type_object){
			struct lh_entry * temp_entry;
			if((temp_entry = json_get_keyword_entry((const lh_table *)json_object_get_object(entry_object),instance_key)) != NULL)
				return temp_entry;
		}
	}
	return NULL;
}

static inline int
json_handle_multipleOf_keyword(double instance_value, double schema_value){
	double op = instance_value / schema_value;
	if(op - (int)op != 0)
		return json_multipleOf_error;
	return 1;
}

static inline int
json_handle_maximum_keyword(struct lh_entry* excl_entry, int instance_value, int schema_value ){
	int excl = (excl_entry != NULL && json_object_get_boolean((json_object*)excl_entry->v))?1:0;
	int return_error = (excl ==1)?json_exclusiveMaximum_error:json_maximum_error; // which error to return in case of an error
	return (json_compare_num(instance_value, schema_value, excl)-excl>=0)?1:return_error;
}

static inline int
json_handle_minimum_keyword(struct lh_entry* excl_entry, int instance_value, int schema_value ){
	int excl = (excl_entry != NULL && json_object_get_boolean((json_object*)excl_entry->v))?1:0;
	int return_error = (excl ==1)?json_exclusiveMinimum_error:json_minimum_error; // which error to return in case of an error
	return (json_compare_num(instance_value, schema_value, excl)-excl>=0)?1:return_error;
}

int 
json_validate_numeric_keywords(struct jsonschema_object instance_object){
	int res = 1;
	json_object *instance_entry = instance_object.instance;
	json_object *schema_entry = instance_object.instance_schema;
	struct lh_table *key_table = json_object_get_object(schema_entry); /* the schema of the key we are validating */
	struct lh_entry *keyword_entry;
	lh_foreach(key_table,keyword_entry){
		const char *keyword = (const char *)keyword_entry->k;
		int keyword_id = json_get_keyword_id(keyword); /* get the keyword id */
		struct json_object *schema_value = (struct json_object *)keyword_entry->v;

		/* check if its a valid type */
		if(json_object_get_type(instance_entry) != json_type_int && json_object_get_type(instance_entry) != json_type_double){
			json_add_error(json_invalid_value_error,instance_object.key,instance_object.object_pos,NULL);
			return 0 ;
		}
		json_printf_colored(ANSI_COLOR_BLUE,"\t KEYWORD %s %d ",keyword, instance_object.object_pos);
		/* validate the instance against all present keywords */
		switch(keyword_id){
		case json_keyword_multipleOf:
			res = json_handle_multipleOf_keyword(json_object_get_double(instance_entry),json_object_get_double(schema_value));
			if (res != 1)  
				json_add_error(res,instance_object.key,instance_object.object_pos,NULL);
		break;
		case json_keyword_maximum: /* exclusiveMaximum tells wether its a strict comparison or not */
			res =json_handle_maximum_keyword(lh_table_lookup_entry(key_table,"exclusiveMaximum"), json_object_get_int(instance_entry), json_object_get_int(schema_value));
			if (res != 1)  
				json_add_error(res,instance_object.key,instance_object.object_pos,NULL);
		break;
		case json_keyword_minimum: /* same for exclusiveMinimum */
			res = json_handle_minimum_keyword(lh_table_lookup_entry(key_table,"exclusiveMinimum"), json_object_get_int(instance_entry), json_object_get_int(schema_value));
			if (res != 1)  
				json_add_error(res,instance_object.key,instance_object.object_pos,NULL);
		break;
		case -1: // no valid keyword found
			json_printf_colored(ANSI_COLOR_RED,"unknown keyword %s", keyword);
			json_add_error(json_invalid_value_error,instance_object.key,instance_object.object_pos,NULL);
		break;
		}
	}
	json_printf_colored(ANSI_COLOR_GREEN,"ALL OK");
	if(res != 1)
		return 0;
	return 1; 
}

static inline int
json_handle_pattern_keyword(const char *pattern_value, const char *instance_value){
	regex_t pattern;
	memset(&pattern, 0, sizeof(regex_t));
	/* compile the pattern */
	if(regcomp(&pattern, pattern_value, REG_EXTENDED | REG_NOSUB) != 0){
		return json_pattern_error;
	}
	/* check if the instance is a valid regex against our pattern */
	if(regexec(&pattern,instance_value, 0, NULL, 0) != 0){
		return json_pattern_error;
	}
	return 1;
}
int 
json_validate_string_keywords(struct jsonschema_object instance_object){
	int res = 1;
	json_object * instance_entry = instance_object.instance;
	json_object * schema_entry = instance_object.instance_schema;
	struct lh_table * key_table = json_object_get_object(schema_entry); /* the schema of the key we are validating */
	struct lh_entry * keyword_entry;

	lh_foreach(key_table,keyword_entry){
		const char * keyword = (const char *)keyword_entry->k;
		int keyword_id = json_get_keyword_id(keyword); // get the id
		struct json_object * schema_value = (struct json_object *)keyword_entry->v;
		json_printf_colored(ANSI_COLOR_BLUE, "\t KEYWORD %s %d",keyword, instance_object.object_pos);
		if(json_object_get_type(instance_entry) != json_type_string){
			json_add_error(json_invalid_value_error,instance_object.key,instance_object.object_pos,NULL);
			return 0 ;
		}
		switch(keyword_id) { 
		case json_keyword_maxLength:
			res = (json_compare_num(strlen(json_object_get_string(instance_entry)), json_object_get_int(schema_value),1) > 0)?json_maxLength_error:1;
			if (res != 1)  
				json_add_error(res,instance_object.key,instance_object.object_pos,NULL);
		break;
		case json_keyword_minLength:
			res = (json_compare_num(strlen(json_object_get_string(instance_entry)), json_object_get_int(schema_value),1) < 0)?json_minLength_error:1;
			if (res != 1)  
				json_add_error(res,instance_object.key,instance_object.object_pos,NULL);
		break;
		case json_keyword_pattern:
			res = json_handle_pattern_keyword(json_object_get_string(schema_value), json_object_get_string(instance_entry));
			if (res != 1)  
				json_add_error(res,instance_object.key,instance_object.object_pos,NULL);
		break;
		case -1: // no valid keyword found
			json_printf_colored(ANSI_COLOR_RED,"unknown keyword %s", keyword);
			json_add_error(json_invalid_value_error,instance_object.key,instance_object.object_pos,NULL);
		break;
		}
	}
	json_printf_colored(ANSI_COLOR_GREEN,"ALL OK");
	if(res != 1)
		return 0;
	return 1; 
}

static inline int 
json_handle_additionalItems_keyword(struct json_object * items,struct json_object * instance_entry, struct json_object * schema_value){
	/* we need validation only when additionalItems is false and items is an array */
	if(json_object_is_type(schema_value, json_type_boolean) && !json_object_get_boolean(schema_value) 
				&& json_object_is_type(items, json_type_array)){
		return (json_compare_num(json_object_array_length(items), json_object_array_length(instance_entry), 1) < 0)?json_additionalItems_error:0;
	}
	return 1;
}

int 
json_validate_array_keywords(struct jsonschema_object instance_object){ 
	int res = 1;
	json_object * instance_entry = instance_object.instance;
	json_object * schema_entry = instance_object.instance_schema;
	struct lh_table * key_table = json_object_get_object(schema_entry); /* the schema of the key we are validating */
	struct lh_entry * keyword_entry;

	lh_foreach(key_table,keyword_entry){
		const char * keyword = (const char *)keyword_entry->k;
		int keyword_id = json_get_keyword_id(keyword); // get the id
		struct json_object * schema_value = (struct json_object *)keyword_entry->v;
		if(json_object_get_type(instance_entry) != json_type_array){
			json_add_error(json_invalid_value_error,instance_object.key,instance_object.object_pos,NULL);
			return 0 ;
		}
		json_printf_colored(ANSI_COLOR_BLUE, "\t KEYWORD %s %d",keyword, instance_object.object_pos);
		switch(keyword_id){
		case json_keyword_additionalItems:
			res = json_handle_additionalItems_keyword((struct json_object *)json_get_keyword_entry(key_table,"items")->v, instance_entry, schema_value);
			if (res != 1)  
				json_add_error(res,instance_object.key,instance_object.object_pos,NULL);
		break;
		case json_keyword_maxItems:
			res = json_compare_num(json_object_array_length(instance_entry), json_object_get_int(schema_value), 1)>0?json_maxItems_error:0;
			if (res != 1)  
				json_add_error(res,instance_object.key,instance_object.object_pos,NULL);
		break;
		case json_keyword_minItems:
			res = json_compare_num(json_object_array_length(instance_entry), json_object_get_int(schema_value), 1)<0?json_minItems_error:0;
			if (res != 1)  
				json_add_error(res,instance_object.key,instance_object.object_pos,NULL);
		break;
		case json_keyword_uniqueItems:
			res = (json_object_get_boolean(schema_value) && json_validate_array_items_uniqueness(instance_entry,-1) != 1)?json_uniqueItems_error:1;
			if (res != 1)  
				json_add_error(res,instance_object.key,instance_object.object_pos,NULL);
		break;
		case -1: // no valid keyword found
			json_printf_colored(ANSI_COLOR_RED,"unknown keyword %s", keyword);
			json_add_error(json_invalid_value_error,instance_object.key,instance_object.object_pos,NULL);
		break; 
		}
	}
	json_printf_colored(ANSI_COLOR_GREEN,"ALL OK");
	if(res != 1)
		return 0;
	return 1; 
}

static inline int 
json_handle_required_keyword(struct json_object *schema_value, struct lh_table *instance_table){
	int i;
	for (i=0; i< json_object_array_length(schema_value); i++){
		const char* required_key = json_object_get_string(json_object_array_get_idx(schema_value, i));
		if(lh_table_lookup_entry(instance_table,required_key) == NULL){
			return json_required_error;
		}
	}
	return 1;
}

int 
json_handle_additionalProperties_keyword(struct json_object *schema_value, struct lh_table *instance_table, struct lh_table *key_table){
	int num_founds = 0 ; // keep track of valid properties
	if(json_object_get_type(schema_value) == json_type_boolean && json_object_get_boolean(schema_value) == 0){
		/* additionalProperties depends on the properties and patternProperties keywords */
		struct json_object *properties = (struct json_object *)lh_table_lookup_entry(key_table,"properties")->v;
		struct json_object *patternProperties = (struct json_object *)lh_table_lookup_entry(key_table,"patternProperties")->v;
		//loop through the instance keys and see if we have a match
		struct lh_entry *instance_temp_entry ;

		lh_foreach(instance_table,instance_temp_entry){
			if(properties != NULL) {
				struct lh_table* properties_table = json_object_get_object(properties);
				if(lh_table_lookup_entry(properties_table,(char *)instance_temp_entry->k) != NULL){
					num_founds++;
					continue; /* dont seach in patternProperties */
				}
			}
			//search in patternProperties
			if(patternProperties != NULL) {
				regex_t pattern;
				memset(&pattern, 0, sizeof(regex_t));
				struct lh_entry* temp_entry;
				lh_foreach(json_object_get_object(patternProperties),temp_entry){
					/* check if its a valid regex*/ 
					if(regcomp(&pattern, (char *)temp_entry->k,REG_EXTENDED | REG_NOSUB) != 0){
						json_printf_colored(ANSI_COLOR_RED,"failed compiling the regex.");
						return json_additionalProperties_error;
					}
					if(regexec(&pattern,(char *)instance_temp_entry->k,0,NULL,0) == 0){
						/* add this key to be validated later. */
						if(properties != NULL)
							json_object_object_add(properties,(char *)instance_temp_entry->k,(json_object*)temp_entry->v);
						else
							/* add it to the parent schema */
							lh_table_insert(key_table,(char *)instance_temp_entry->k,(json_object*)temp_entry->v);
						num_founds++;
						break;
					}
				}
			}
		}
		/* if there are keys left, then they are invalid */
		if(lh_table_length(instance_table) != num_founds){
			return json_additionalProperties_error;
		}
	}
	return 1;
}

int 
json_handle_dependencies_keyword(struct jsonschema_object instance_object, struct json_object *schema_value, struct lh_table *key_table){
	struct json_object *instance = instance_object.instance;
	struct lh_table *schema_table = json_object_get_object(schema_value);
	struct lh_table *instance_table = json_object_get_object(instance);
	struct lh_entry *value;

	lh_foreach(schema_table, value){
		/* if the value type is an array, its a property dependency */
		if(json_object_is_type((struct json_object*)value->v, json_type_array)){
		/* check if the property exist in the instance */
		if(lh_table_lookup_entry(instance_table,(char *)value->k) != NULL){
			/* all items in the key array must be present in the instance */
			int i;
			for(i=0 ; i< json_object_array_length((struct json_object *)value->v); i++){
			const char *dependency = json_object_get_string(json_object_array_get_idx((struct json_object *)value->v,i));
			/* if the dependency is not present, return failure */
			if(lh_table_lookup_entry(instance_table, dependency) == NULL){
				return json_dependencies_error;
			}
			}
		}
		}else if(json_object_is_type((struct json_object*)value->v, json_type_object)){ /* its a schema dependency */
		struct lh_table *dependencies_table = json_object_get_object((struct json_object*)value->v);
		struct lh_entry *dependency;
		lh_foreach(dependencies_table, dependency){
			struct lh_entry *tmp;
			/* if the keyword is properties or patternProperties, check that all the keys are already present in the instance before validating them */
			if(strcmp(dependency->k, "properties") == 0){
				lh_foreach(json_object_get_object((struct json_object *)dependency->v), tmp){
					if(json_object_object_get_ex(instance_object.instance,tmp->k, NULL) != 1)
						return json_dependencies_error;
				}
			}else if(strcmp(dependency->k, "patternProperties") == 0){
				lh_foreach(json_object_get_object((struct json_object *)dependency->v), tmp){
					/* compile the pattern */
					regex_t pattern;
					memset(&pattern, 0, sizeof(regex_t));
					if(regcomp(&pattern, dependency->k, REG_EXTENDED | REG_NOSUB) != 0){
						return json_dependencies_error;
					}
					/* check if the instance is a valid regex against our pattern */
					struct lh_entry *inst_tmp;
					lh_foreach(json_object_get_object(instance_object.instance), inst_tmp){
						if(regexec(&pattern,inst_tmp->k, 0, NULL, 0) == 0){
							break;
						}
					}
				}
				return json_dependencies_error;
			}
			/* if the key is present in the key table add its content there, otherwise create a new key in the key_table */
			struct lh_entry *key = lh_table_lookup_entry(key_table, (char *)dependency->k);
			if(key != NULL){
				lh_foreach(json_object_get_object((struct json_object *)dependency->v), tmp){
					/* check that the key is not already present */
					if(json_object_object_get_ex((struct json_object *)key->v,tmp->k, NULL) != 1){
						json_object_object_add((struct json_object *)key->v,tmp->k,(struct json_object *)tmp->v);
					}
				}
			}
			else
				lh_table_insert(key_table, dependency->k, dependency->v);
		}
		/* temporarly remove this keyword so we dont enter in an endless loop */
		struct lh_entry *dependencies = lh_table_lookup_entry(key_table,"dependencies");
		dependencies->prev->next = dependencies->next;
		/* validate the object */
		int res = json_validate_object_instance(instance_object);
		/* return the schema to the original state */
		dependencies->prev->next = dependencies;
		lh_foreach(dependencies_table, dependency){
			struct lh_entry *key = lh_table_lookup_entry(key_table, (char *) dependency->k);
			if(key != NULL){
				/* get the first item in the dependencies table */
				struct lh_entry *head = lh_table_lookup_entry(json_object_get_object((struct json_object *)key->v), json_object_get_object((struct json_object *)dependency->v)->head->k);
				/* in the key in the schema, remove the dependencies */
				head->prev->next = NULL;
			}
		}
		if(res != 1){
			return json_dependencies_error;
		}
		}else{ /* unknown type, this shouldnt happen if the schema file is validated */
			return json_dependencies_error;
		}
	}
	return 1;
}

int 
json_validate_object_keywords(struct jsonschema_object instance_object){ 
	int res = 1;
	json_object *instance_entry = instance_object.instance;
	json_object *schema_entry = instance_object.instance_schema;
	struct lh_table *key_table = json_object_get_object(schema_entry); /* the schema of the key we are validating */
	struct lh_entry *keyword_entry;
	struct lh_table *instance_table = json_object_get_object(instance_entry) ;

	lh_foreach(key_table,keyword_entry){
		const char *keyword = (const char *)keyword_entry->k;
		struct json_object *schema_value = (struct json_object *)keyword_entry->v;
		int keyword_id = json_get_keyword_id(keyword); // get the id
		if(json_object_get_type(instance_entry) != json_type_object){
			json_add_error(json_invalid_value_error,instance_object.key,instance_object.object_pos,NULL);
			return 0 ;
		}
		json_printf_colored(ANSI_COLOR_BLUE, "\t KEYWORD %s %d",keyword, instance_object.object_pos);
		switch(keyword_id){
		case json_keyword_maxProperties:
			res = json_compare_num(lh_table_length(instance_table), json_object_get_int(schema_value), 1)>0?json_maxProperties_error:1;
			if (res != 1)  
				json_add_error(res,instance_object.key,instance_object.object_pos,NULL);
		break;
		case json_keyword_minProperties:
			res = json_compare_num(lh_table_length(instance_table), json_object_get_int(schema_value), 1)<0?json_maxProperties_error:1;
			if (res != 1)  
				json_add_error(res,instance_object.key,instance_object.object_pos,NULL);
		break;
		case json_keyword_required:
			res = json_handle_required_keyword(schema_value, instance_table);
			if (res != 1)  
				json_add_error(res,instance_object.key,instance_object.object_pos,NULL);
		break;
		case json_keyword_additionalProperties:
			res = json_handle_additionalProperties_keyword(schema_value, instance_table, key_table);
			if (res != 1)  
				json_add_error(res,instance_object.key,instance_object.object_pos,NULL);
		break;
		case json_keyword_dependencies:
			res = json_handle_dependencies_keyword(instance_object,schema_value, key_table);
			if (res != 1)  
				json_add_error(res,instance_object.key,instance_object.object_pos,NULL);
		break;
		case -1: // no valid keyword found
			json_printf_colored(ANSI_COLOR_RED,"unknown keyword %s", keyword);
			json_add_error(json_invalid_value_error,instance_object.key,instance_object.object_pos,NULL);
		break;
		}
	}
	json_printf_colored(ANSI_COLOR_GREEN,"ALL OK");
	if(res != 1)
		return 0;
	return 1; 
}

int 
inline json_handle_enum_keyword(struct json_object *schema_value, struct json_object *instance_entry){
	int i;
	for(i = 0; i< json_object_array_length(schema_value); i++){
		const char * enum_val = json_object_to_json_string(json_object_array_get_idx(schema_value,i));
		if(strcmp(json_object_to_json_string(instance_entry),enum_val) == 0){
			return 1;
		}
	}
	return json_enum_error;
}

/**
 * @brief used by allOf, anyOf, oneOf and not keywords.
 *          the idea is to retrieve a schema object from these keywords, put it in the place of the keyword, validate the object and re-insert the keyword again
 */
static int
json_get_instance_schema_from_Of_keywords(struct jsonschema_object instance_object, struct lh_entry *keyword_entry, struct json_object *temp_schema){
	struct lh_table *temp_schema_table = json_object_get_object(temp_schema);
	/* link the retrieved schema to the instance schema to be validated */
	keyword_entry->prev->next = temp_schema_table->head;
	temp_schema_table->tail->next = keyword_entry->next;
	struct jsonschema_object temp_object;
	temp_object.key = instance_object.key;
	temp_object.instance = instance_object.instance;
	temp_object.instance_schema = instance_object.instance_schema;
	temp_object.object_pos = instance_object.object_pos;
	int key_type = json_get_key_type(instance_object.instance_schema);
	/* do the validation */
	int tmp_res = json_validate_anytype_keywords(temp_object);
	if(tmp_res < 0){
		/* re-insert the original keyword */  
		keyword_entry->prev->next = keyword_entry;
		return 0; 
	}
	tmp_res = json_validate_instance_keywords(temp_object,key_type);
	if(tmp_res < 0){
		/* re-insert the original keyword */
		keyword_entry->prev->next = keyword_entry;
		return 0; 
	}
	return 1;
}

static inline int
json_handle_allOf_keyword(struct jsonschema_object instance_object, struct lh_entry *keyword_entry, json_object *schema_value){
	int i;
	for(i = 0; i< json_object_array_length(schema_value); i++){
		json_printf_colored(ANSI_COLOR_CYAN,"Validating allOf schema #%d",i);
		int res = json_get_instance_schema_from_Of_keywords(instance_object, keyword_entry, json_object_array_get_idx(schema_value, i));
		if(res !=1 )
			return json_allOf_error;
	}
	return 1;
}

static inline int
json_handle_anyOf_keyword(struct jsonschema_object instance_object, struct lh_entry *keyword_entry, json_object *schema_value){
	int i;
	for(i = 0; i< json_object_array_length(schema_value); i++){
		json_printf_colored(ANSI_COLOR_CYAN,"Validating anyOf schema #%d",i);
		int res = json_get_instance_schema_from_Of_keywords(instance_object, keyword_entry, json_object_array_get_idx(schema_value, i));
		if(res == 1)
			return res;
	}
	return json_anyOf_error;
}

static inline int
json_handle_oneOf_keyword(struct jsonschema_object instance_object, struct lh_entry *keyword_entry, json_object *schema_value){
	int i, num_found = 0;
	for(i = 0; i< json_object_array_length(schema_value); i++){
		json_printf_colored(ANSI_COLOR_CYAN,"Validating oneOf schema #%d",i);
		int res = json_get_instance_schema_from_Of_keywords(instance_object, keyword_entry, json_object_array_get_idx(schema_value, i));
		if(res == 1)
			num_found++;
	}
	if(num_found != 1)
		return json_oneOf_error;
	return 1;
}

static inline int
json_handle_not_keyword(struct jsonschema_object instance_object, struct lh_entry* keyword_entry, struct json_object *schema_value){
	int res = json_get_instance_schema_from_Of_keywords(instance_object, keyword_entry, schema_value);
	return (res == 1 ? json_not_error : 1);
}

int 
json_validate_anytype_keywords(struct jsonschema_object instance_object){ 
	int res = 1;
	json_object *instance_entry = instance_object.instance;
	json_object *schema_entry = instance_object.instance_schema;
	struct lh_table *key_table = json_object_get_object(schema_entry); /* the schema of the key we are validating */
	struct lh_entry *keyword_entry;
	struct lh_entry *tmp_keyword_entry;

	lh_foreach_safe(key_table,keyword_entry,tmp_keyword_entry){
		const char * keyword = (const char *)keyword_entry->k;
		int keyword_id = json_get_keyword_id(keyword); // get the id
		struct json_object * schema_value = (struct json_object *)keyword_entry->v;
		switch(keyword_id){
		case json_keyword_enum:
			res = json_handle_enum_keyword(schema_value, instance_entry);
			if (res != 1)  
				json_add_error(res,instance_object.key,instance_object.object_pos,NULL);
		break;
		case json_keyword_allOf:
			res = json_handle_allOf_keyword(instance_object, keyword_entry, schema_value);
			if (res != 1)  
				json_add_error(res,instance_object.key,instance_object.object_pos,NULL);
		break;
		case json_keyword_anyOf:
			res = json_handle_anyOf_keyword(instance_object, keyword_entry, schema_value); 
			if (res != 1)  
				json_add_error(res,instance_object.key,instance_object.object_pos,NULL);
		break;
		case json_keyword_oneOf:
			res = json_handle_oneOf_keyword(instance_object, keyword_entry, schema_value); 
			if (res != 1)  
				json_add_error(res,instance_object.key,instance_object.object_pos,NULL);
		break;
		case json_keyword_not:
			res = json_handle_not_keyword(instance_object, keyword_entry, schema_value); 
			if (res != 1)
				json_add_error(res,instance_object.key,instance_object.object_pos,NULL);
		break;
		case -1: // no valid keyword found
			json_printf_colored(ANSI_COLOR_RED,"unknown keyword %s", keyword);
			json_add_error(json_invalid_value_error,instance_object.key,instance_object.object_pos,NULL);
		break;
		}
	
	}
	if(res != 1)
		return 0;
	return 1;
}

// /* NOT IMPLEMENTED YET */
// int 
// json_validate_metakeywords(const struct lh_entry* instance_entry,const struct lh_entry* schema_entry){ 
//     struct lh_table * key_table = json_object_get_object(schema_entry); //contains all keywords of the key from the schema
//     struct lh_entry * keyword_entry;
//     lh_foreach(key_table,keyword_entry){
//         const char * keyword = (const char *)keyword_entry->k;
//         struct json_object * schema_value = (struct json_object *)keyword_entry->v;
//         //struct json_object * instance_value = (struct json_object *)instance_entry->v;
//         if(json_object_get_type(instance_entry) != json_type_object)
//             return 0;
//         
//         if(strcmp(keyword,"enum") == 0){
//             int i;
//             for(i = 0; i< json_object_array_length(schema_value); i++){
//                 const char * enum_val = json_object_to_json_string(json_object_array_get_idx(schema_value,i));
//                 if(strcmp(json_object_to_json_string(instance_entry),enum_val) == 0)
//                     return 1;
//             }
//             return 0;
//         }else if(strcmp(keyword,"allOf") == 0){
//             struct lh_table* instance_table = json_object_get_object(instance_entry);
//             if(lh_table_length(instance_table) > json_object_get_int(schema_value))
//                 return 0;
//         }else if(strcmp(keyword,"anyOf") == 0){
//             // to be done
//         }else if(strcmp(keyword,"oneOf") == 0){
//             // to be done
//         }else if(strcmp(keyword,"not") == 0){
//             // to be done
//             }
//     }
//     return 1;
// }

int 
json_validate_instance_keywords(struct jsonschema_object instance_object,int key_type){
	//number = 7, double = 2 and int = 3 have the same validation
	if(key_type == 7 || key_type == 2)
		key_type = 3;
	switch(key_type){
	case json_type_int:
		return json_validate_numeric_keywords(instance_object);
	case json_type_object:
		return json_validate_object_keywords(instance_object);
	case json_type_array:
		return json_validate_array_keywords(instance_object);
	case json_type_string:
		return json_validate_string_keywords(instance_object);
	}
	return 0;
}

int
json_validate_array_instance(struct jsonschema_object instance_object){
	//validate the array itself.
	int tmp_res = json_validate_array_keywords(instance_object);
	//if the array has the items keyword and its type is object, we need it to validate array items
	struct json_object *items_schema = (json_object *)lh_table_lookup_entry(json_object_get_object(instance_object.instance_schema),"items")->v;
	if(items_schema != NULL && json_object_get_type(items_schema) == json_type_object){
		//validate array elements using the items schema
		int i;
		for(i=0; i< json_object_array_length(instance_object.instance); i++){
			json_object* obj = json_object_array_get_idx(instance_object.instance,i);
			struct json_object* items_schema_copy = copy_object(items_schema);
			struct jsonschema_object tmp_instance_object;
			tmp_instance_object.key = "instance object";
			tmp_instance_object.object_pos = i+1; /* i starts from 0 */
			tmp_instance_object.instance = obj;
			tmp_instance_object.instance_schema = items_schema_copy;
			tmp_res = json_validate_object_instance(tmp_instance_object ) && tmp_res;
		}
	}
	return tmp_res;
}

int
json_validate_object_instance(struct jsonschema_object instance_object){
	//an instance can be either an object or an array of objects.
	//if its an array of objects, validate each one.
	struct lh_table *instance_table = json_object_get_object(instance_object.instance);
	struct lh_table *schema_table = json_object_get_object(instance_object.instance_schema);
	struct lh_entry *instance_entry; 
	//the object itself first
	int tmp_res = 1;
	tmp_res = json_validate_object_keywords(instance_object);
	// elements 
	lh_foreach(instance_table,instance_entry){
		char *instance_key = (char*)instance_entry->k;
		struct json_object *instance_value = (struct json_object *)instance_entry->v;
		//lookup the key in the schema 
		struct lh_entry *schema_entry = lh_table_lookup_entry(schema_table, instance_key);
		//lookup the key in the schema 
		if(schema_entry == NULL){
			//look for the properties keyword
			struct lh_entry* properties_entry;
			properties_entry = lh_table_lookup_entry(schema_table,"properties");
			//look for the key inside the properties object
			if(properties_entry != NULL){
				schema_entry = lh_table_lookup_entry(json_object_get_object((json_object*) properties_entry->v),instance_key);
			}
			if(schema_entry == NULL){
				/* 
				* "patternProperties", "anyOf" ,"allOf", "not" and "oneOf" make it hard to check if a key is there or not
				* if they are not present, report the key is not found and exit, otherwise move on. all keys will be validated later and non-valid keys will be spotted
				*/
				if( (lh_table_lookup_entry(schema_table, "patternProperties") == NULL && lh_table_lookup_entry(schema_table, "anyOf") == NULL) &&
				(lh_table_lookup_entry(schema_table, "allOf") == NULL && lh_table_lookup_entry(schema_table, "not") == NULL) &&
				(lh_table_lookup_entry(schema_table, "oneOf") == NULL) ){
					json_printf_colored(ANSI_COLOR_RED,"--key not found in the schema");
					return 0;
				}else{
					continue;
				}
			}
		}
		// prepare the object for validation
		struct json_object *schema_value = (struct json_object *)schema_entry->v;
		instance_object.key = instance_key;
		instance_object.instance = instance_value;
		instance_object.instance_schema = schema_value;
		json_printf_colored(ANSI_COLOR_CYAN,"***KEY: %s %d",instance_key, instance_object.object_pos);
		/* if its an object, call this function again */
		if(json_object_get_type(instance_value) == json_type_object)
			tmp_res = json_validate_object_instance(instance_object) && tmp_res;
		else{ /* validate the key */
			// get the type of the key
			int key_type = json_get_key_type(schema_value);
			//validate keywords that apply to any type
			tmp_res = json_validate_anytype_keywords(instance_object) && tmp_res;
			tmp_res = json_validate_instance_keywords(instance_object,key_type) && tmp_res;
		}
	}
	return tmp_res;
}

int
json_validate_instance(struct json_object *instance, struct json_object *schema){
	if (!instance || !schema)
		return 0;

	/* validate the schema first */
	json_printf_colored(ANSI_COLOR_YELLOW,"Validating JSON schema...");
	if(json_validate_schema(schema) == 1) {
		json_printf_colored(ANSI_COLOR_YELLOW,"Validating JSON file...");
		struct jsonschema_object instance_object;
		instance_object.instance = instance;
		instance_object.instance_schema = schema;
		instance_object.object_pos = 0; //root element
		/* a json file can either contain an object or an array of objects */
		if(json_object_get_type(instance) == json_type_array){
			instance_object.key = "root array";
			json_validate_array_instance(instance_object);
		}else{
			instance_object.key = "root object";
			json_validate_object_instance(instance_object);
		}
		// retrieve errors
		struct json_error *errors = malloc((256 * sizeof(struct json_error))) ;
		int num_errors;
		errors = json_add_error(0,NULL,-1,&num_errors);
		if(num_errors > 0){
			json_printf_colored(ANSI_COLOR_RED,"\nInvalid JSON file! %d error(s) found",num_errors);
			int i;
			for(i=0; i< num_errors; i++){
				json_printf_colored(ANSI_COLOR_RED,"%s(object #%d):  %s",errors[i].key,errors[i].obj_pos, errors[i].error_message);
			}
			return 0;
		}else{
			json_printf_colored(ANSI_COLOR_GREEN,"\n Valid JSON file!");
			return 1;
		}
	}else{
		json_printf_colored(ANSI_COLOR_RED,"\n Invalid JSON schema");
		return 0;
	}

}

int
json_validate_instance_from_file(const char *instance_path, const char * schema_path){
	/* get the schema and instance objects */
	json_object *schema = json_object_from_file(schema_path);
	json_object *instance = json_object_from_file(instance_path);

	if(schema == NULL) {
		json_printf_colored("could not load schema from file.",ANSI_COLOR_RED);
		return 0;
	}
	if(instance == NULL) {
		json_printf_colored("could not load instance from file.",ANSI_COLOR_RED);
		return 0;
	}
	return json_validate_instance(instance, schema);
}
