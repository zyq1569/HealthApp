#ifndef _LOGGING_
#define _LOGGING_

#include <QString>


namespace udg {
    void beginLogging();
    /**
     * Returns the path where the log should be outputted to.
     * @return Log file path
     */
    QString getLogFilePath();
    /**
     * Location of log.conf file.
     * 
     * This function tries to locate the logging configuration file, trying
     * different fallback alternatives.
     * 
     * @return Path where log.conf is expected to be found
     */
    QString getLogConfFilePath();

    void debugLog(const QString &msg, const QString &file, int line, const QString &function);
    void infoLog(const QString &msg, const QString &file, int line, const QString &function);
    void warnLog(const QString &msg, const QString &file, int line, const QString &function);
    void errorLog(const QString &msg, const QString &file, int line, const QString &function);
    void fatalLog(const QString &msg, const QString &file, int line, const QString &function);
    void verboseLog(int vLevel, const QString &msg, const QString &file, int line, const QString &function);
    void traceLog(const QString &msg, const QString &file, int line, const QString &function);
}


//Taken from easylogging++.h
#if _MSC_VER  // Visual C++
    #define LOG_FUNC __FUNCSIG__
#elif __GNUC__  // GCC
    #define LOG_FUNC __PRETTY_FUNCTION__
#elif defined(__clang__) && (__clang__ == 1)  // Clang++
    #define LOG_FUNC __PRETTY_FUNCTION__
#elif __INTEL_COMPILER  // Intel C++
    #define LOG_FUNC __PRETTY_FUNCTION__
#else
    #if defined(__func__)
        #define LOG_FUNC __func__
    #else
        #define LOG_FUNC ""
    #endif
#endif


/// Macro per a missatges de debug. \TODO de moment fem servir aquesta variable de qmake i funciona bé, però podria ser més adequat troba la forma d'afegir
/// una variable pròpia, com per exemple DEBUG
#ifdef QT_NO_DEBUG
    #define DEBUG_LOG(msg) while (false)
#else
    #define DEBUG_LOG(msg) udg::debugLog(msg,__FILE__,__LINE__,LOG_FUNC)
#endif

#define INFO_LOG(msg) udg::infoLog(msg,__FILE__,__LINE__,LOG_FUNC)
#define WARN_LOG(msg) udg::warnLog(msg,__FILE__,__LINE__,LOG_FUNC)
#define ERROR_LOG(msg) udg::errorLog(msg,__FILE__,__LINE__,LOG_FUNC)
#define FATAL_LOG(msg) udg::fatalLog(msg,__FILE__,__LINE__,LOG_FUNC)
#define VERBOSE_LOG(vLevel, msg) udg::verboseLog(vLevel, msg,__FILE__,__LINE__,LOG_FUNC)
#define TRACE_LOG(msg) udg::traceLog(msg,__FILE__,__LINE__,LOG_FUNC)




#endif //_LOGGING_
