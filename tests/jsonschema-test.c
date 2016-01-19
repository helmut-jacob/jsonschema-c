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
        printf("Checking schema_validator!\n");
        if(argc > 2)
            json_validate_instance_from_file(argv[1],argv[2]);
        else 
            json_validate_instance_from_file("example_instance.txt","example_schema.txt");
	return 0;
}
