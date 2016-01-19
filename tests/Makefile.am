TESTS=
TESTS+= check_jsonschema_c
TESTS+= basic_schema_tests
TESTS+= basic_object_tests

check_PROGRAMS = 
check_PROGRAMS += $(TESTS:.test=)
check_PROGRAMS += check_jsonschema_c
check_PROGRAMS += basic_schema_tests
check_PROGRAMS += basic_object_tests

check_jsonschema_c_SOURCES = jsonschema-test.c
check_jsonschema_c_CFLAGS = -I../ -I/usr/local/include/json-c
check_jsonschema_c_LDADD = $(top_builddir)/libjsonschema-c.la

basic_schema_tests_SOURCES = basic_schema_tests.c
basic_schema_tests_CFLAGS = -I../ -I/usr/local/include/json-c
basic_schema_tests_LDADD = $(top_builddir)/libjsonschema-c.la

basic_object_tests_SOURCES = basic_object_tests.c
basic_object_tests_CFLAGS = -I../ -I/usr/local/include/json-c
basic_object_tests_LDADD = $(top_builddir)/libjsonschema-c.la


EXTRA_DIST=
EXTRA_DIST += $(TESTS)


distclean-local:
	-rm -rf $(testsubdir)
