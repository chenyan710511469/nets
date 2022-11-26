/*
 * =====================================================================================
 *
 *       Filename:  Log.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  07/25/2020 10:54:03 AM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱(Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef UTILITIES_LOGSMANAGER_LOG_H
#define UTILITIES_LOGSMANAGER_LOG_H

namespace utilities {
namespace logsmanager {
enum LogLevel
{
    EMERG = 0,   /*  致命级：紧急事件消息，系统崩溃之前提示，表示系统不可用 */
    FATAL,
    ALERT,       /*  警戒级：报告消息，表示必须采取措施 */
    CRIT,        /*  临界级：临界条件，通常涉及严重的硬件或软件操作失败 */
    ERR,         /*  错误级：错误条件，驱动程序常用KERN_ERR来报告硬件错误 */
    WARNING,     /*  告警级：警告条件，对可能出现问题的情况进行警告 */
    NOTICE,      /*  注意级：正常但又重要的条件，用于提醒 */
    INFO,        /*  通知级：提示信息，如驱动程序启动时，打印硬件信息 */
    DEBUG,       /*  调试级：调试级别的信息 */
};  // enum LogLevel

enum LogSwitch
{
    NOT_OUTPUT = 0,
    OUTPUT_SINGLE = 1,
    OUTPUT_RANGE = 2
};

class Log
{
public:
    static bool initLog();

    static void emergency(const char *tag, const char *format, ...);
    static void emerg(const char *tag, const char *format, ...);

    static void fatal(const char *tag, const char *format, ...);
    static void f(const char *tag, const char *format, ...);

    static void alert(const char *tag, const char *format, ...);
    static void a(const char *tag, const char *format, ...);

    static void critical(const char *tag, const char *format, ...);
    static void crit(const char *tag, const char *format, ...);
    static void c(const char *tag, const char *format, ...);

    static void err(const char *tag, const char *format, ...);
    static void e(const char *tag, const char *format, ...);

    static void warning(const char *tag, const char *format, ...);
    static void w(const char *tag, const char *format, ...);

    static void notice(const char *tag, const char *format, ...);
    static void n(const char *tag, const char *format, ...);

    static void info(const char *tag, const char *format, ...);
    static void i(const char *tag, const char *format, ...);

    static void debug(const char *tag, const char *format, ...);
    static void d(const char *tag, const char *format, ...);

private:
    Log();
    virtual ~Log();

    LogSwitch       mLogSwitch;
    LogLevel        mLogLevelStart;
    LogLevel        mLogLevelEnd;
    LogLevel        mLogLevelSingle;
    std::string     mLogFullFile;

    static Log*     mLog;

};  // class Log
};  // namespace utilities
};  // namespace logsmanager

#define LOGF(const char *format, ...)  Log.f(LOGTAG, format, ...)
#define LOGE(const char *format, ...)  Log.e(LOGTAG, format, ...)
#define LOGW(const char *format, ...)  Log.w(LOGTAG, format, ...)
#define LOGN(const char *format, ...)  Log.n(LOGTAG, format, ...)
#define LOGI(const char *format, ...)  Log.i(LOGTAG, format, ...)
#define LOGD(const char *format, ...)  Log.d(LOGTAG, format, ...)

#endif  // UTILITIES_LOGSMANAGER_LOG_H
