#ifndef _CLOG_H_
#define _CLOG_H_

#ifdef __cplusplus
extern "C" {
#endif

enum CLogLevel {
   CLOG_LEVEL_ERROR = 1,
   CLOG_LEVEL_WARN,
   CLOG_LEVEL_INFO,
   CLOG_LEVEL_DEBUG
};

extern CLogLevel logLevel;
#define LOGSTREAM getLogStream()

#define LOG_ERROR(format, ARG...) do { \
    if(logLevel>=CLOG_LEVEL_ERROR) \
    log_message(CLOG_LEVEL_ERROR,__LINE__,__FILE__,format_log_message(format, ##ARG)); \
} while(0)

#define LOG_WARN(format, ARG...) do { \
    if(logLevel>=CLOG_LEVEL_WARN) \
    log_message(CLOG_LEVEL_WARN,__LINE__,__FILE__,format_log_message(format, ##ARG)); \
} while(0)

#define LOG_INFO(format, ARG...) do { \
    if(logLevel>=CLOG_LEVEL_INFO) \
    log_message(CLOG_LEVEL_INFO,__LINE__,__FILE__,format_log_message(format, ##ARG)); \
} while(0)

#define LOG_DEBUG(format, ARG...) do { \
    if(logLevel==CLOG_LEVEL_DEBUG) \
    log_message(CLOG_LEVEL_DEBUG,__LINE__,__FILE__,format_log_message(format, ##ARG)); \
} while(0)

void log_message(CLogLevel curLevel, int line,const char* funcName,
    const char* message);

const char* format_log_message(const char* format,...);

bool log_init(CLogLevel level, const char* log_filename);

#ifdef __cplusplus
}
#endif

#endif /*_CLOG_H_*/
