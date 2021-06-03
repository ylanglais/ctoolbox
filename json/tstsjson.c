
#define c_BS			'\b'
#define c_FF			'\f'
#define c_LF			'\n'
#define c_CR			'\r'
#define c_HT			'\t'

static char ctrl_NULL =  0;
static char ctrl_SOH  =  1;
static char ctrl_STX  =  2;
static char ctrl_ETX  =  3;
static char ctrl_OET  =  4;
static char ctrl_ENQ  =  5;
static char ctrl_ACK  =  6;
static char ctrl_BEL  =  7;
static char ctrl_BS   =  8;
static char ctrl_HT   =  9;
static char ctrl_LF   = 10;
static char ctrl_VT   = 11;
static char ctrl_FF   = 12;
static char ctrl_CR   = 13;
static char ctrl_SO   = 14;
static char ctrl_SI   = 15;
static char ctrl_DLE  = 16;
static char ctrl_DC1  = 17;
static char ctrl_DC2  = 18;
static char ctrl_DC3  = 19;
static char ctrl_DC4  = 20;
static char crlt_NAK  = 21;
static char ctrl_SYN  = 22;
static char ctrl_ETB  = 23;
static char ctrl_CAN  = 24;
static char ctrl_EM   = 25;
static char ctrl_SUB  = 26;
static char ctrl_ESC  = 27;
static char ctrl_FS   = 28;
static char ctrl_GS   = 29;
static char ctrl_RS   = 30;
static char ctrl_US   = 31;

#define ctrl_CTRLCHAR  "["ctrl_SOH##"|"##ctrl_STX##"|"##ctrl_ETX##"|"##ctrl_OET##"|"##ctrl_ENQ##"|"##ctrl_ACK##"|"##ctrl_BEL##"|"##ctrl_BS##"|"##ctrl_HT##"|"##ctrl_LF##"|"##ctrl_VT##"|"##ctrl_FF##"|"##ctrl_CR##"|"##ctrl_SO##"|"##ctrl_SI##"|"##ctrl_DLE##"|"##ctrl_DC1##"|"##ctrl_DC2##"|"##ctrl_DC3##"|"##ctrl_DC4##"|"##crlt_NAK##"|"##ctrl_SYN##"|"##ctrl_ETB##"|"##ctrl_CAN##"|"##ctrl_EM##"|"##ctrl_SUB##"|"##ctrl_ESC##"|"##ctrl_FS##"|"##ctrl_GS##"|"##ctrl_RS##"|"##ctrl_US"]"

#define ctrl_RCTRLCHAR  "["ctrl_SOH##"|"##ctrl_STX##"|"##ctrl_ETX##"|"##ctrl_OET##"|"##ctrl_ENQ##"|"##ctrl_ACK##"|"##ctrl_BEL##"|"##ctrl_VT##"|"##ctrl_SO##"|"##ctrl_SI##"|"##ctrl_DLE##"|"##ctrl_DC1##"|"##ctrl_DC2##"|"##ctrl_DC3##"|"##ctrl_DC4##"|"##crlt_NAK##"|"##ctrl_SYN##"|"##ctrl_ETB##"|"##ctrl_CAN##"|"##ctrl_EM##"|"##ctrl_SUB##"|"##ctrl_ESC##"|"##ctrl_FS##"|"##ctrl_GS##"|"##ctrl_RS##"|"##ctrl_US"]"

#define json_NULL		"null"
#define json_TRUE		"true"
#define json_FALSE		"false"

#define json_SEP		"[\n\t ]"
#define json_SEPS		json_SEP"{1,}"

#define json_E 			"[eE][+\\-]?"
#define json_DIGIT 		"[0-9]"
#define json_DIGITS		json_DIGIT"{1,}"
#define json_19DIGIT	"[1-9]"
#define json_19DIGITS	json_19DIGIT"{1,}"
#define json_HEXDIGIT	"[0-9a-fA-F]"
#define json_HEXDIGITS	json_HEXDIGIT"{1,}"
#define json_EXP		json_E""json_DIGITS
#define	json_FRAC		"\\."json_DIGITS
#define json_INT		"[+-]?"json_DIGITS
#define json_NUMBER		json_INT"("json_FRAC")?""("json_EXP")?"
//#define json_CHAR		"[^"
//#define json_CHAR		"([[:ascii:]]|[^[:ascii:]])"
#define json_CHAR		"([[:ascii:]]|[^\\x00-\\x7F])"
#define json_CHARS		json_CHAR"{1,}"
#define json_STRING		"[\"][^\"]*[\"]|['][^']*[']"

#define json_ARRAY		"\\[[^]]*\\]"
#define json_OBJECT		"\\{[^}]*\\}"

#define json_VALUE		"("json_STRING"|"json_NUMBER"|"json_OBJECT"|"json_ARRAY"|"json_NULL"|"json_FALSE"|"json_NULL")"

#define json_ELEMENT	json_VALUE
#define json_ELEMENTS	json_ELEMENT"(,"json_ELEMENT"){1,}"
#define json_PAIR		json_STRING""json_SEP?":"json_SEP?""json_VALUE
#define json_MEMBER		json_PAIR""json_SEP?"(,"json_SEP?""json_PAIR"){0,}"

#include <stdlib.h>
#include <wchar.h> 
#include <stdio.h>
#include <string.h>
#include <tbx/re.h>
#include <tbx/mem.h>

static char tests_json[] = "\
{\"menu\": {\n\
  \"id\": \"file\",\n\
  \"value\": \"File\",\n\
  \"popup\": {\n\
    \"menuitem\": [\n\
      {\"value\": \"New\",   \"onclick\": \"CreateNewDoc()\"},\n\
      {\"value\": \"Open\",  \"onclick\": \"OpenDoc()\"},\n\
      {\"value\": \"Close\", \"onclick\": \"CloseDoc()\"}\n\
    ]\n\
  }\n\
}}";

static char tests_SEPS[]      = "a_space b_tab\tLF\nd_2spaces  END_w/_4_matches";
static char tests_E[]         = "e1e12E+33 E-543 END_w/_6_matches";
static char tests_DIGITS[]    = "012345 12 99 102 1100 END_w/_6_matches";
static char tests_19DIGITS[]  = "0123 0001 123 END_w/_4_matches";
static char tests_HEXDIGITS[] = "Aa09fEKe03b END_w_8_matches";

static char tests_EXP[]       = "e12 E-111 e+3 -e 12 END_w_3_matches";
static char tests_FRAC[]      = "012.123 .235 11.6999 1. END_w_3_matches";
static char tests_INT[]       = "111212 +1223789 -0123456 END_w_4_matches";
static char tests_NUMBER[]    = "123456 023.222 .123 END_w_3_matches";
static char tests_CHAR[]      = "Aa09fEéà03b фыва i END_w_28_matches";
static char tests_CHARS[]     = "Aa09fEKe03b END_w_2_matches";
static char tests_STRING[]    = " \"\" ' '  \"ser azer aze\" ' \" END_w_3_matches";
static char tests_ARRAY[]     = "['a': \"b\", 'b' :  \"c\"] [] [ ] [ END_w_3_matches";
static char tests_OBJECT[]    = " {} { ['a': \"b\", 'b' :  \"c\"], []} {a: 'b'} { } END_w_4_matches";

char *
json_parse_string(char **s) {
	char *p, sep, *r;
	p = *s;

	if (*p == '"') sep = '"';
	else if (*p == '\'') sep = '\'';
	else return NULL;

	
	for (p++; *p != 0 && *p != sep; p++) {
		if (*p == '\\') {
			if (!p[1]) return NULL;
			if (p[1] == '"' || p[1] == '\\' || 
				p[1] == '/' || p[1] == 'b' || 
				p[1] == 'f' || p[1] == 'n' || 
				p[1] == 'r' || p[1] == 't') {
				p++;
			} else if (p[1] == 'u') {
				int i;
				for (i == 1; i < 4; i++) {
					if (p[i] == 0) return NULL;
					if (!((p[i] >= '0' && p[i] <= '9') || 
						  (p[i] >= 'A' && p[i] <= 'F') || 
					      (p[i] >= 'a' && p[i] <= 'f') )) return NULL;
				}
				p += 4;
			} else {
				return NULL; 
			}
		} 
	}
	if (*p != sep) return NULL;
	
	if (!(r = mem_zmalloc((size_t) (p - *s)))) return NULL;
	memcpy(r, *s + 1, p - *s - 1);
	*s = ++p;
	return r; 

}

char *
sep_clean(char *buff) {
	int i;
	pre_t r;
	printf("json_SEPS: \"%s\"\n", json_SEPS);
	r = re_new(buff, json_SEPS, reEXTENDED|reNEWLINE);
	i = re_replace_all(r, " ");
	printf("replaced %d separators\n", i);
	return re_buffer(r);
}

int
match_print(pre_t r, void *data, prematch_t m) {
	char *p;
	printf(" -> '%s'\n", p = re_substr(r, m->subs[0].so, m->subs[0].eo));
	free(p);
}

#define TEST(x) \
	printf("Test %s with \"%s\" (regexp: \"%s\"):\n", #x, tests_##x, json_##x);\
	if ((r = re_new(tests_##x, json_##x, reEXTENDED|reUTF8))) {\
		int i;\
		i = re_find_foreach(r, NULL, (re_match_processor_f) match_print); \
		printf("--> %d match(s)\n\n", i);\
		r = re_destroy(r);\
	}

int
json_parse(char *b) {
	char *p;
	char *q;
	
	for (p = b; *p && *p != '{' && (*p == ' ' || *p == '\t' || *p == '\n'); p++) ;
	if (*p != '{') return 0;
	return 0;
}	


static char string_tests = {
	"'Azeraz aze éi Aa09fEéà03b фыва i'",
	"\"Azeraz aze éi Aa09fEéà03b фыва i\nAzeraz aze éi Aa09fEéà03b фыва i",
	"\"azeraz erazea \\nzaerzear\"",
	"\" \\uD834\\uDD1E \"",
	NULL
}

int 
main(void) {
	char b1[] = " toto	titi\n tutu  	 tata\n";
	char *b, *b2, *r;
	pre_t r;

	b = mem_strdup(b1);
	printf("b: \"%s\"\n", b);
	printf("b: \"%s\"\n", b2 = sep_clean(b)); 	

	mem_free(b);
	mem_free(b2);

	TEST(SEPS);
	TEST(E);
	TEST(DIGITS);
	TEST(19DIGITS);
	TEST(HEXDIGITS);
	TEST(EXP);
	TEST(FRAC);
	TEST(INT);
	TEST(CHAR);
	TEST(CHARS);
	TEST(STRING);
	TEST(ARRAY);
	TEST(OBJECT);


	for (i = 0; strings_tests[i]; i++) {
		char *r;
		printf("string: %s\n", strings_test[i])
		r = parse_string(string_tests[i]);
		if (!r)
	


#if 0
#endif
	return 0;
}




