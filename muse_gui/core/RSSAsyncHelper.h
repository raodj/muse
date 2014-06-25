#ifndef RSSASYNCHELPER_H
#define RSSASYNCHELPER_H

#include <QThread>
#include <functional>

template<typename RetVal>
class RSSAsyncHelper : public QThread {
    //Q_OBJECT
typedef std::function<RetVal(void)> MethodCall;
public:

    RSSAsyncHelper();
    RSSAsyncHelper(MethodCall method);
    void run();

    RetVal getResult();

private:
    RetVal result;
    MethodCall method;

};

#include "RSSAsyncHelper.ipp"

#endif // RSSASYNCHELPER_H
