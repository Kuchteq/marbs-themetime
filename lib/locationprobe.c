#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tzalias.h"

#define TIMEZONE "/etc/timezone"
#define ZONETAB  "/usr/share/zoneinfo/zone.tab"
#define NELEMS(array) (sizeof(array) / sizeof(array[0]))
static void chomp(char *str)
{
	size_t len;

	if (!str)
		return;

	len = strlen(str) - 1;
	while (len && str[len] == '\n')
		str[len--] = 0;
}

static int alias(char *tz, size_t len)
{
	size_t i;

	for (i = 0; i < NELEMS(tzalias); i++) {
		if (strcmp(tzalias[i].name, tz))
			continue;

		strncpy(tz, tzalias[i].alias, len);
		return 1;
	}

	return 0;
}

int probe(double *lat, double *lon)
{
	int found = 0, again = 0;
	FILE *fp;
	char *ptr, tz[42], buf[80];

	ptr = getenv("TZ");
	if (!ptr) {
		fp = fopen(TIMEZONE, "r");
		if (!fp)
			return 0;

		if (fgets(tz, sizeof(tz), fp))
			chomp(tz);
		fclose(fp);
	} else {
		strncpy(tz, ptr, sizeof(tz));
		tz[sizeof(tz) - 1] = 0;
	}

retry:
	fp = fopen(ZONETAB, "r");
	if (!fp)
		return 0;

	while ((fgets(buf, sizeof(buf), fp))) {
		ptr = strstr(buf, tz);
		if (!ptr)
			continue;
		*ptr = 0;
		ptr = buf;
		while (*ptr != ' ' && *ptr != '	')
			ptr++;

		if (sscanf(ptr, "%lf%lf", lat, lon) == 2) {
                        // TODO fix timezone infos that include seconds
			*lat /= 100.0;
			*lon /= 100.0;
			found = 1;
		}
		break;
	}
	fclose(fp);

	if (!found) {
		if (!again) {
			again++;
			if (alias(tz, sizeof(tz)))
				goto retry;
		}
	}

	return found;
}
