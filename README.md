# jsonschema-c
JSON schema validation for json-c library.
jsonschema-c is a schema validation based on the json-c library and the V4 draft of json schema standard.
The library is composed of two components: schema validator and instance validator.
*Schema validator is used to verify that the provided schema is a valid V4 schema.
*Instance validator is used to validate a JSON object against a given schema.

Currently the library is in the testing phase.

### Dependencies:
* C compiler
* libtool
* json-c

### Building process:
```sh 
	$ libtoolize
	$ aclocal
	$ automake --add-missing
	$ autoconf
	$ ./configure
	$ make
	$ make install (you may need sudo make install)
```
#### Building tests:
```sh
	$ make check
```
