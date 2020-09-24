#ifndef _CZM__LOG_H_
#define _CZM__LOG_H_

#ifdef __cplusplus
extern "C" {
#endif /* ifndef _cplusplus */

#include <stddef.h>
void start_split (char c);
void czm_loop_dots(int seconds);
int czm_log_print(const char *fmt, ...);
int czm_log_err(const char *fmt, ...);
int czm_dump(char *label, char *buf, size_t buf_len);
void czm_print_time();

#define __CZM_FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)
/** ##__VA_ARGS__ 是 gcc 写法，不支持 C99 */
#define czm_print(fmt, ...) czm_log_print("%s:%d:%s:" fmt, __CZM_FILENAME__, __LINE__, __func__, ##__VA_ARGS__)
#define czm_err(fmt, ...) czm_log_err("%s:%d:%s:" fmt, __CZM_FILENAME__, __LINE__, __func__, ##__VA_ARGS__)

#if (CZM_DEBUG_LEVEL==2)
extern int _czm_debug_in_loop_  __attribute__((unused));
extern int _czm_debug_loop_counter_  __attribute__((unused));
#define CZM_LOOP_FLAG(n) _czm_debug_in_loop_=(n)
#else
#define CZM_LOOP_FLAG(n)
#endif

#include <time.h>
struct CZM_PROG {
	char dir[512];
	char name[48];
	time_t start_time;
	int (*uptime)();
	int (*show_threads)();
};
extern struct CZM_PROG *czm_prog;

#ifdef __cplusplus
}
#endif /* ifndef _cplusplus */

#endif /* ifndef _CZM_LOG_H_ */
