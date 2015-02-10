#include "clog.h"

#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define TIME_BUF_SIZE 1024
#define FORMAT_BUF_SIZE 4096

#define THREADED

#ifdef THREADED
#include <pthread.h>
#include <syscall.h>
#include <stdlib.h>

static pthread_key_t time_buffer;
static pthread_key_t format_msg_buffer;

void freeBuffer(void* p){
    if(p) free(p);
}

void prepareTSDKeys() {
    pthread_key_create (&time_buffer, freeBuffer);
    pthread_key_create (&format_msg_buffer, freeBuffer);
}

char* getTSData(pthread_key_t key,int size){
    char* p = (char*)pthread_getspecific(key);
    if(p == 0){
        int res;
        p = (char*)calloc(1,size);
        res = pthread_setspecific(key, p);
        if(res != 0){
            fprintf(stderr,"Failed to set TSD key: %d",res);
        }
    }
    return p;
}

char* get_time_buffer(){
    return getTSData(time_buffer,TIME_BUF_SIZE);
}

char* get_format_log_buffer(){  
    return getTSData(format_msg_buffer,FORMAT_BUF_SIZE);
}
#else
char* get_time_buffer(){
    static char buf[TIME_BUF_SIZE];
    return buf;    
}

char* get_format_log_buffer(){
    static char buf[FORMAT_BUF_SIZE];
    return buf;
}
#endif

CLogLevel logLevel = CLOG_LEVEL_INFO;

static FILE* logStream = NULL;
FILE* getLogStream(){
    if(logStream == 0)
        logStream = stderr;
    return logStream;
}

static const char* time_now(char* now_str){
    struct timeval tv;
    struct tm lt;
    time_t now = 0;
    
    gettimeofday(&tv,0);
    now = tv.tv_sec;
    localtime_r(&now, &lt);

    strftime(now_str, TIME_BUF_SIZE, "%Y-%m-%d %H:%M:%S", &lt);

    return now_str;
}

void log_message(CLogLevel curLevel,int line,const char* funcName,
    const char* message)
{
    static const char* dbgLevelStr[]={"LOG_INVALID","LOG_ERROR","LOG_WARN",
            "LOG_INFO","LOG_DEBUG"};

    static pid_t pid=0;
    if(pid==0)pid=getpid();

#ifndef THREADED
    fprintf(LOGSTREAM, "%s [%d][%s][%s:%d] %s\n", time_now(get_time_buffer()),pid,
            dbgLevelStr[curLevel],funcName,line,message);
#else
    fprintf(LOGSTREAM, "%s [%d:%d][%s][%s:%d] %s\n", time_now(get_time_buffer()),pid,
            (int) syscall(__NR_gettid),
            dbgLevelStr[curLevel],funcName,line,message);      
#endif
    fflush(LOGSTREAM);
}

const char* format_log_message(const char* format,...)
{
    va_list va;
    char* buf=get_format_log_buffer();
    if(!buf)
        return "format_log_message: Unable to allocate memory buffer";
    
    va_start(va,format);
    vsnprintf(buf, FORMAT_BUF_SIZE-1,format,va);
    va_end(va); 
    return buf;
}

void set_debug_level(CLogLevel level)
{
    if(level==0){
        // disable logging (unit tests do this)
        logLevel=(CLogLevel)0;
        return;
    }
    if(level<CLOG_LEVEL_ERROR) level=CLOG_LEVEL_ERROR;
    if(level>CLOG_LEVEL_DEBUG) level=CLOG_LEVEL_DEBUG;
    logLevel=level;
}

bool log_init(CLogLevel level, const char* log_filename) {
    set_debug_level(level);

#ifdef THREADED
    prepareTSDKeys();
#endif

    logStream = fopen(log_filename, "a+");
    if (logStream == NULL) {
        fprintf(stderr,"Failed to open log file:%s", strerror(errno));
    }
}

