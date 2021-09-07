#ifndef LOG_H
#define LOG_H

#include <stdio.h>

#define roprint(x) printf(x)

#define COL_DEFAULT "\033[39m"
#define COL_RED     "\033[31m"
#define COL_GREEN   "\033[32m"
#define COL_ORANGE  "\033[33m"
#define COL_BLUE    "\033[34m"
#define COL_MAGENTA "\033[35m"
#define COL_CYAN    "\033[36m"
#define COL_GREY    "\033[37m"

#define OK_STRING COL_GREEN     "[OK   ]  " COL_DEFAULT
#define ERR_STRING COL_ORANGE   "[WARN ]  " COL_DEFAULT
#define INFO_STRING COL_CYAN    "[INFO ]  " COL_DEFAULT
#define PANIC_STRING COL_RED    "[PANIC]  " COL_DEFAULT

#define LINE_STRING log(__FILE__);        \
        log(" : ");           \
        log(__func__);        \
        log(" l. ");          \
        log(__LINE__);        \
        log(" -> ");

#define CRITICAL_ERROR() { exit(1); }

#define LOG_OK(...)    {roprint(OK_STRING);    printf(COL_BLUE "%s:%d %s -> " COL_DEFAULT, __FILE__, __LINE__, __func__); printf(__VA_ARGS__); roprint("\n");}
#define LOG_ERR(...)   {roprint(ERR_STRING);   printf(COL_BLUE "%s:%d %s -> " COL_DEFAULT, __FILE__, __LINE__, __func__); printf(__VA_ARGS__); roprint("\n");}
#define LOG_INFO(...)  {roprint(INFO_STRING);  printf(COL_BLUE "%s:%d %s -> " COL_DEFAULT, __FILE__, __LINE__, __func__); printf(__VA_ARGS__); roprint("\n");}
#define LOG_PANIC(...) {roprint(PANIC_STRING); printf(COL_BLUE "%s:%d %s -> " COL_DEFAULT, __FILE__, __LINE__, __func__); printf(__VA_ARGS__); roprint("\n");}

#define PANIC(...) {LOG_PANIC(__VA_ARGS__); CRITICAL_ERROR();}
#define PANIC_IF(COND, ...) {if(COND) PANIC(__VA_ARGS__)}

#define ASSERT(C, __TRUE, __FALSE, ...) {if(C){ LOG_OK(__TRUE, ##__VA_ARGS__); } else { PANIC(__FALSE, ##__VA_ARGS__);}}
#define CHECK(C, __TRUE, __FALSE, ...) {if(C){ LOG_OK(__TRUE, ##__VA_ARGS__); } else { LOG_ERR(__FALSE, ##__VA_ARGS__);}}

#define qASSERT(C) {if(C){ LOG_OK(#C); } else { LOG_PANIC(#C); CRITICAL_ERROR();}}
#define qCHECK(C) {if(C){ LOG_OK(#C); } else { LOG_ERR(#C);}}

#endif