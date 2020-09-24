#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include "czm-log.h"
#include "common.h"
// #include "etcpark.h"
// #include "type_convert.h"

#if (CZM_DEBUG_LEVEL==2)
int _czm_debug_in_loop_  __attribute__((unused));
int _czm_debug_loop_counter_  __attribute__((unused));
#endif
#define fflush(stdout)

struct CZM_PROG *czm_prog;
extern int verbose_flag;
static int czm_initted = 0;

static int czm_prog_get_uptime()
{
	time_t t;
	t = time(NULL);
	return (int)t-czm_prog->start_time;
}

static int czm_prog_show_threads()
{
	DIR *proc_dir;
	char dirname[100];
	int count = 0;

	snprintf(dirname, sizeof dirname, "/proc/%d/task", getpid());
	proc_dir = opendir(dirname);

	if (!proc_dir)
		return -1;

	/* /proc available, iterate through tasks... */
	struct dirent *entry;
	while ((entry = readdir(proc_dir)) != NULL)
	{
		if(entry->d_name[0] == '.')
			continue;

		// int tid = atoi(entry->d_name);
		fputs(entry->d_name, stdout);
		fputc(',', stdout);
		count++;
	}
	printf("Total: %d\n", count);
	closedir(proc_dir);
	return 0;
}

int czm_init()
{
	if (czm_initted++) return 0;
	char *p;
#ifdef WIN32
    char result[MAX_PATH];
    unsigned found;
    GetModuleFileName(NULL, result, MAX_PATH);
	p = strrchr(result, '\\');
#else
    char result[PATH_MAX];
	int ret;
	ret = readlink("/proc/self/exe",result,sizeof(result)-1);
    if(ret ==-1) {
		return -1;
    }
	p = strrchr(result, '/');
#endif
	czm_prog = (struct CZM_PROG *)xmalloc(sizeof(struct CZM_PROG));

	memset(czm_prog->dir, 0, sizeof(czm_prog->dir));
	memset(czm_prog->name, 0, sizeof(czm_prog->name));
	if (p == NULL) {
		strncpy(czm_prog->name, result, sizeof(czm_prog->name));
	} else {
		*p = 0;
		strncpy(czm_prog->dir, result, sizeof(czm_prog->dir));
		strncpy(czm_prog->name, p+1, sizeof(czm_prog->name));
	}

	czm_prog->start_time = time(NULL);
	czm_prog->uptime = czm_prog_get_uptime;
	czm_prog->show_threads = czm_prog_show_threads;
	return 0;
}

int czm_deinit()
{
	if (czm_prog) {
		free(czm_prog);
		czm_prog = NULL;
	}
}

void myConstructor (void) __attribute__ ((constructor, no_instrument_function));
void myConstructor (void)
{
	czm_init();
	struct tm* ptm;
	char time_string[48];
	memset(time_string, 0, sizeof(time_string));
	ptm = localtime(&czm_prog->start_time);
	strftime(time_string, sizeof (time_string), "\e[33m%c\e[0m%n", ptm);

	fputs("\e[33m================================================================================\e[0m\n", stdout);
	fputs(time_string, stdout);
	fflush(stdout);
}

void myDestructor (void) __attribute__ ((destructor, no_instrument_function));
void myDestructor (void)
{
	struct tm* ptm;
	char time_string[48];
	memset(time_string, 0, sizeof(time_string));
	ptm = localtime(&czm_prog->start_time);
	strftime(time_string, sizeof (time_string), "\e[33m%c\e[0m%n", ptm);

	fputs(time_string, stdout);
	fputs("\e[33m================================================================================\e[0m\n", stdout);
	fflush(stdout);
	czm_deinit();
}

// PTHREAD_MUTEX_INITIALIZER 全0，作为静态全局变量，czm_log_lock不初始化应该也可以。
static pthread_mutex_t czm_log_lock = PTHREAD_MUTEX_INITIALIZER;

void start_split (char c)
{
	if (!verbose_flag) return;

	time_t t;
	struct tm* ptm;
	char time_string[48];

	pthread_mutex_lock(&czm_log_lock);
	memset(time_string, 0, sizeof(time_string));
	t = time(NULL);
	ptm = localtime(&t);
	strftime(time_string, sizeof (time_string), "\e[33m%c\e[0m%n", ptm);

	if (c == '=') {
		fputs("\e[33m================================================================================\e[0m\n", stdout);
	} else if (c == '-') {
		fputs("\e[33m--------------------------------------------------------------------------------\e[0m\n", stdout);
	} else if (c == '*') {
		fputs("\e[33m********************************************************************************\e[0m\n", stdout);
	}
	fputs(time_string, stdout);
	pthread_mutex_unlock(&czm_log_lock);
	fflush(stdout);
}

void czm_print_time()
{
	// printf("%llu", (unsigned long long)time(0));
	// putchar(':');

	struct tm *tmp;
	char outTimeStr[24];
	struct timespec ts;
	int bytes;

	clock_gettime(CLOCK_REALTIME, &ts);
	tmp = localtime(&ts.tv_sec);

	memset(outTimeStr, 0, sizeof(outTimeStr));
	if (tmp != NULL) {
		bytes = strftime(outTimeStr, sizeof(outTimeStr), "%m-%d %H:%M:%S", tmp);
		if (bytes > 0) {
			snprintf(outTimeStr+bytes, sizeof(outTimeStr)-bytes, ".%03ld ", ts.tv_nsec/1000000);
			fputs(outTimeStr, stdout);
		}
	}
}

void czm_loop_dots(int seconds)
{
	static int _czm_x, _czm_abc;

	if (time(0)%seconds) {
		_czm_x = 1;
	} else {
		_czm_x = 0;
	}
	if (_czm_x) {
		if (!_czm_abc++) {
			putchar('.');
			fflush(stdout);
		}
	} else {
		_czm_abc=0;
	}
}

#ifndef __USE_GNU
#define __USE_GNU
#endif
int czm_log_print(const char *fmt, ...)
{
	if (!verbose_flag) return 0;

	pthread_mutex_lock(&czm_log_lock);
	czm_print_time();
	int ret;
	va_list args;
	va_start(args, fmt);
	ret = vprintf(fmt, args);
	va_end(args);
	pthread_mutex_unlock(&czm_log_lock);
	fflush(stdout);

	return ret;
}

int czm_dump(char *label, char *buf, size_t buf_len)
{
	if (!verbose_flag) return 0;

	pthread_mutex_lock(&czm_log_lock);
	int _czm_i, _czm_j, _czm_k, _czm_col, _czm_row, _czm_len;
	unsigned char *_czm_p = (unsigned char *)buf;
	int lineLen;

	czm_print_time();
	_czm_len = buf_len; _czm_col = _czm_len % 16; _czm_row = _czm_len / 16;
	printf("\e[0;93mDUMP %s, sz=%d:\e[0m\n", label, _czm_len);
	for((_czm_i)=0; (_czm_i)<(_czm_row); (_czm_i)+=(1)) {
		printf("\e[0;96m%02x0: ", _czm_i);
		lineLen = 0;
		for((_czm_j)=0; (_czm_j)<(16); (_czm_j)+=(2))
			lineLen += printf("%02x%02x ", _czm_p[_czm_i*16+_czm_j], _czm_p[_czm_i*16+_czm_j+1]);
		printf("%*s", 40-lineLen, "");
		for((_czm_k)=0; (_czm_k)<(16); (_czm_k)+=(1))
			putchar((_czm_p[_czm_i*16+_czm_k]>31&&_czm_p[_czm_i*16+_czm_k]<127)?_czm_p[_czm_i*16+_czm_k]:'.');
		puts("\e[0m");
	}
	if (!_czm_col) { pthread_mutex_unlock(&czm_log_lock); return 0; }
	lineLen = 0;
	printf("\e[0;96m%02x0: ", _czm_i);
	for((_czm_j)=0; (_czm_j)<(_czm_col-1); (_czm_j)+=(2))
		lineLen += printf("%02x%02x ", _czm_p[_czm_row*16+_czm_j], _czm_p[_czm_row*16+_czm_j+1]);
	if (_czm_col%2)
		lineLen += printf("%02x   ", _czm_p[_czm_len-1]);
	printf("%*s", 40-lineLen, "");
	for((_czm_k)=0; (_czm_k)<(_czm_col); (_czm_k)+=(1))
		putchar((_czm_p[_czm_row*16+_czm_k]>31&&_czm_p[_czm_row*16+_czm_k]<127)?_czm_p[_czm_row*16+_czm_k]:'.');
	puts("\e[0m");
	pthread_mutex_unlock(&czm_log_lock);
	fflush(stdout);

	return 0;
}

int czm_log_err(const char *fmt, ...)
{
	// if (!verbose_flag) return 0;

	pthread_mutex_lock(&czm_log_lock);
	fputs("\e[31m", stdout);
	czm_print_time();
	int ret;
	va_list args;
	va_start(args, fmt);
	ret = vprintf(fmt, args);
	va_end(args);
	fputs("\e[0m", stdout);
	pthread_mutex_unlock(&czm_log_lock);
	fflush(stdout);

	return ret;
}

int czm_color_log(const char *color, const char *fmt, ...)
{
	if (!verbose_flag) return 0;

	pthread_mutex_lock(&czm_log_lock);
	fputs(color, stdout);
	czm_print_time();
	int ret;
	va_list args;
	va_start(args, fmt);
	ret = vprintf(fmt, args);
	va_end(args);
	fputs("\e[0m", stdout);
	pthread_mutex_unlock(&czm_log_lock);
	fflush(stdout);

	return ret;
}

// void start_new_log_file()
// {
// #if !KEEP_ERR_LOG_ONLY
//     return;
// #endif
//     FILE *fp = NULL;
//     char *log_fname = "/tmp/etcpark-running.log";
//
//     verbose_flag = 1;
//
//     fp = fopen(log_fname, "w");
//     if (fp) {
//         fclose(fp);
//         freopen(log_fname, "a+", stdout);
//     }
//     return;
// }
//
// void end_new_log_file()
// {
// #if !KEEP_ERR_LOG_ONLY
//     return;
// #endif
//     FILE *fp = NULL;
//     char *line = NULL;
//     size_t len = 0, nread;
//     char *p = NULL;
//     int ret = 0;
//     char fname1[32];
//     static int log_err_id = 0;
//     char *log_fname = "/tmp/etcpark-running.log";
//
//     fflush(stdout); // 此时stdout已经等同于log_fname的文件流
//
//     fp = fopen(log_fname, "rb");
//     if (fp == NULL) goto end;
//
//     while ((nread = getline(&line, &len, fp)) != -1) {
//         if (strstr(line, "U16:statusCode") == NULL)
//             continue;
//         p = strrchr(line, ' ');
//         ret = strtol(p, NULL, 16);
//         if (ret) {
//             if (!resp_code_error_flag) {
//                 snprintf(fname1, sizeof(fname1), "logs/etcpark-err%d-%X.log", (log_err_id++)%6, ret);
//                 // rename(log_fname, fname1);
//                 copyfile(fname1, log_fname);
//             }
//         } else {
//             snprintf(fname1, sizeof(fname1), "logs/etcpark-success.log");
//             copyfile(fname1, log_fname);
//             // remove(log_fname);
//         }
//         break;
//     }
//     free(line);
//     fclose(fp);
//
// end:
// #ifndef WIN32
//     freopen("/dev/tty", "w", stdout);
// #else
//     freopen("CON", "w", stdout);
// #endif
//
//     verbose_flag = 0;
// }
//
