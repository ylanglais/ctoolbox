int is_utf8(const char * string) {
    if(!string) return 0;

    const unsigned char * bytes = (const unsigned char *) string;

    while (*bytes) {
        if (*bytes == 0x09 || *bytes == 0x0A || *bytes == 0x0D || (0x20 <= *bytes && *bytes <= 0x7E)) {
            // ASCII  use *bytes <= 0x7F to allow ASCII control characters:
            bytes += 1;
            continue;
        }
        if ((0xC2 <= bytes[0] && bytes[0] <= 0xDF) && (0x80 <= bytes[1] && bytes[1] <= 0xBF)) {
           // non-overlong 2-byte
			bytes += 2;
            continue;
        }

        if ((// excluding overlongs
                bytes[0] == 0xE0 &&
                (0xA0 <= bytes[1] && bytes[1] <= 0xBF) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF)
            ) ||
            (// straight 3-byte
                ((0xE1 <= bytes[0] && bytes[0] <= 0xEC) ||
                    bytes[0] == 0xEE ||
                    bytes[0] == 0xEF) &&
                (0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF)
            ) ||
            (// excluding surrogates
                bytes[0] == 0xED &&
                (0x80 <= bytes[1] && bytes[1] <= 0x9F) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF)
            )
        ) {
            bytes += 3;
            continue;
        }

        if( (// planes 1-3
                bytes[0] == 0xF0 &&
                (0x90 <= bytes[1] && bytes[1] <= 0xBF) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
                (0x80 <= bytes[3] && bytes[3] <= 0xBF)
            ) ||
            (// planes 4-15
                (0xF1 <= bytes[0] && bytes[0] <= 0xF3) &&
                (0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
                (0x80 <= bytes[3] && bytes[3] <= 0xBF)
            ) ||
            (// plane 16
                bytes[0] == 0xF4 &&
                (0x80 <= bytes[1] && bytes[1] <= 0x8F) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
                (0x80 <= bytes[3] && bytes[3] <= 0xBF)
            )
        ) {
            bytes += 4;
            continue;
        }
        return 0;
    }
    return 1;
}


// Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

#define UTF8_ACCEPT 0
#define UTF8_REJECT 1

static const uint8_t utf8d[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
  8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
  0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
  0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
  0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
  1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
  1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
  1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
};

uint32_t inline
decode(uint32_t* state, uint32_t* codep, uint32_t byte) {
	uint32_t type = utf8d[byte];

	*codep = (*state != UTF8_ACCEPT) ?  (byte & 0x3fu) | (*codep << 6) : (0xff >> type) & (byte);
	*state = utf8d[256 + *state*16 + type];

	return *state;
}

uint32_t
validate_utf8(uint32_t * state, char *str, size_t len) {
    size_t i;
    uint32_t type;

    for (i = 0; i < len; i++) {
        // We don't care about the codepoint, so this is
        // a simplified version of the decode function.
        type = utf8d[(uint8_t) str[i]];
        *state = utf8d[256 + (*state) * 16 + type];

        if (*state == UTF8_REJECT)
            break;
    }

    return *state;
}

#include <stdio.h>
#include <string.h>
/* UTF-8 : BYTE_BITS*/
/* B0_BYTE : 0XXXXXXX */
/* B1_BYTE : 10XXXXXX */
/* B2_BYTE : 110XXXXX */
/* B3_BYTE : 1110XXXX */
/* B4_BYTE : 11110XXX */
/* B5_BYTE : 111110XX */
/* B6_BYTE : 1111110X */

#define B0_BYTE 0x00
#define B1_BYTE 0x80
#define B2_BYTE 0xC0
#define B3_BYTE 0xE0
#define B4_BYTE 0xF0
#define B5_BYTE 0xF8
#define B6_BYTE 0xFC

/* Please tune this as per number of lines input */
#define MAX_UTF8_STR 10

/* 600 is used because 6byteX100chars */
#define MAX_UTF8_CHR 600

void func_find_utf8(char *ptr_to_str);
void print_non_ascii(int bytes, char *pbyte);
char strbuf[MAX_UTF8_STR][MAX_UTF8_CHR];

int
main(int ac, char *av[]) {
    int i = 0;
    char no_newln_str[MAX_UTF8_CHR];
    i = 0;
    printf("\n\nYou can enter utf-8 string or Q/q to QUIT\n\n");

    while (i < MAX_UTF8_STR) {
        fgets(strbuf[i], MAX_UTF8_CHR, stdin);
        if (!strlen(strbuf[i]))
            break;

        if ((strbuf[i][0] == 'Q') || (strbuf[i][0] == 'q'))
            break;

        strcpy(no_newln_str, strbuf[i]);
        no_newln_str[strlen(no_newln_str) - 1] = 0;
        func_find_utf8(no_newln_str);
        ++i;
    }
    return 1;
}

void
func_find_utf8(char *ptr_to_str) {
    int found_non_ascii;
    char *pbyte;
    pbyte = ptr_to_str;
    found_non_ascii = 0;

    while (*pbyte) {
        if ((*pbyte & B1_BYTE) == B0_BYTE) {
            pbyte++;
            continue;
        } else {
            found_non_ascii = 1;
            if ((*pbyte & B7_BYTE) == B6_BYTE) {
                print_non_ascii(6, pbyte);
                pbyte += 6;
                continue;
            }

            if ((*pbyte & B6_BYTE) == B5_BYTE) {
                print_non_ascii(5, pbyte);
                pbyte += 5;
                continue;
            }

            if ((*pbyte & B5_BYTE) == B4_BYTE) {
                print_non_ascii(4, pbyte);
                pbyte += 4;
                continue;
            }

            if ((*pbyte & B4_BYTE) == B3_BYTE) {
                print_non_ascii(3, pbyte);
                pbyte += 3;
                continue;
            }

            if ((*pbyte & B3_BYTE) == B2_BYTE) {
                print_non_ascii(2, pbyte);
                pbyte += 2;
                continue;
            }
        }
    }
    if (found_non_ascii) printf(" These are Non Ascci chars\n");

}

void
print_non_ascii(int bytes, char *pbyte) {
    char store[6];
    int i;
    memset(store, 0, 6);
    memcpy(store, pbyte, bytes);

    i = 0;
    while (i < bytes) printf("%c", store[i++]);
    printf("%c", ' ');

    fflush(stdout);
}
