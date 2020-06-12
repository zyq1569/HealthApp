#define DefaultLogFileName "HServerManageUI"

#include "mainwindow.h"
#include <QApplication>

#include "easylogging++.h"

INITIALIZE_EASYLOGGINGPP

class LogHandler : public el::LogDispatchCallback {
public:
    void handle(const el::LogDispatchData* data) {
        // NEVER LOG ANYTHING HERE! NOT HAPPY WITH MULTI_THREADING
        ELPP_COUT << "Test this " << data << std::endl;
    }
};

class HtmlHandler : public el::LogDispatchCallback {
public:
    HtmlHandler() {
        el::Loggers::getLogger("html");
    }
    void handle(const el::LogDispatchData* data) {
        // NEVER LOG ANYTHING HERE! NOT HAPPY WITH MULTI_THREADING
        ELPP_COUT << "<b>" << data->logMessage()->message() << "</b>" << std::endl;
    }
};

int main(int argc, char *argv[])
{
    START_EASYLOGGINGPP(argc, argv);

    el::Loggers::removeFlag(el::LoggingFlag::NewLineForContainer);
    el::Helpers::installLogDispatchCallback<LogHandler>("LogHandler");
    el::Helpers::installLogDispatchCallback<HtmlHandler>("HtmlHandler");
    LOG(INFO) << "First log";
    LogHandler* logHandler = el::Helpers::logDispatchCallback<LogHandler>("LogHandler");
    logHandler->setEnabled(false);

    LOG(INFO) << "Second log";

    QApplication a(argc, argv);
    HMainWindow w;
    w.show();

    return a.exec();
}
