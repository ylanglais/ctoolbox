#include <ctype.h>

void
hexdump(char *p, size_t size) {
	int i, j;
	char hex[256], asc[256], tmp[1000];
	char *buff;

	buff = (char *) p;

	if (!p) return;
	for (i = 0; i < size; i += 16) {
		hex[0] = asc[0] = '\0';
		for (j = 0; j < 16 && i + j < size; j++) {
			if (j == 8) {
				strcat(hex, "- ");
				strcat(asc, " - ");
			}
			sprintf(tmp, "%02x ", (unsigned char) buff[i+j]);
			strcat(hex, tmp);
			sprintf(tmp, "%c", isprint(buff[i + j]) ? buff[i + j] : '.');
			strcat(asc, tmp);
		}
		printf("%-50s | %-19s", hex, asc);
	}
}
