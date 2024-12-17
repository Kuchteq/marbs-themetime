#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include "lib/sunriset.h"
#include "lib/locationprobe.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

#define TIMEZONE "/etc/timezone"
#define ZONETAB "/usr/share/zoneinfo/zone.tab"
#define THEMESET_SCRIPT "themeset"
#define NELEMS(array) (sizeof(array) / sizeof(array[0]))

extern long timezone;

// Also includes timezone conversion
long toseconds_riset(double riset)
{
	int h = (int)floor(riset);
	int m = (int)(60 * (riset - floor(riset)));
	return h * 3600 + m * 60 - timezone;
}
long toseconds_localtime(struct tm *tm)
{
	return tm->tm_hour * 3600 + tm->tm_min * 60;
}
void themeset(char *theme)
{
	pid_t pid = fork();

	if (pid < 0) {
		exit(EXIT_FAILURE);
	} else if (pid == 0) {
		execlp(THEMESET_SCRIPT, THEMESET_SCRIPT, theme, NULL);
		perror("can't run " THEMESET_SCRIPT);
		exit(EXIT_FAILURE);
	} else {
		wait(NULL);
	}
}

void calc_riset(struct tm *tm, int *rise_s, int *set_s)
{
	int year, month, day;
	double lon = 0.0, lat;
	double rise, set;

	year = 1900 + tm->tm_year;
	month = 1 + tm->tm_mon;
	day = tm->tm_mday;
	probe(&lat, &lon);
	sun_rise_set(year, month, day, lon, lat, &rise, &set);
	*rise_s = toseconds_riset(rise);
	*set_s = toseconds_riset(set);
}

// code from: https://stackoverflow.com/questions/32152276/real-time-aware-sleep-call
void blank_sighandler(int signal)
{
}

void isleep(int seconds)
{
	timer_t timerid;
	struct sigevent sev = { .sigev_notify = SIGEV_SIGNAL,
				.sigev_signo = SIGALRM,
				.sigev_value = (union sigval){
					.sival_ptr = &timerid } };
	signal(SIGALRM, blank_sighandler);
	timer_create(CLOCK_REALTIME, &sev, &timerid);

	struct itimerspec its = { .it_interval = { 0, 0 } };
	clock_gettime(CLOCK_REALTIME, &its.it_value);
	its.it_value.tv_sec += seconds;
	timer_settime(timerid, TIMER_ABSTIME, &its, NULL);
	pause();
	signal(SIGALRM, SIG_IGN);
}

int main()
{
	time_t now;
	struct tm *tm;
	int rise, set;

	now = time(NULL);
	tm = localtime(&now);
	calc_riset(tm, &rise, &set);

	// Daemon loop
	while (1) {
		now = time(NULL);
		tm = localtime(&now);
		int tm_s = toseconds_localtime(tm);

		if (tm_s >= rise && tm_s < set) {
			themeset("light");
			isleep(set - tm_s);
			continue;
		}

		themeset("dark");
		if (tm_s < rise) {
			// Before sunrise
			isleep(rise - tm_s);
		} else {
			// After sunset: Sleep until next day's sunrise
			now = time(NULL) + 86400;
			tm = localtime(&now);
			calc_riset(tm, &rise, &set);
			isleep(rise + 86400 - tm_s);
		}
	}
}
