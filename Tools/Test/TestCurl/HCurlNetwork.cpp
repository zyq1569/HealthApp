#include "HCurlNetwork.h"

#include "CurlMulti.h"
#include <limits>
#include <memory>
#include <QThreadStorage>
#include <QTimer>
#include <QSocketNotifier>

HCurlNetwork::HCurlNetwork(QObject *parent): QObject(parent)
{
    handle_ = curl_easy_init();
    Q_ASSERT(handle_ != nullptr);

    set(CURLOPT_PRIVATE, this);
    set(CURLOPT_XFERINFOFUNCTION, staticCurlXferInfoFunction);
    set(CURLOPT_XFERINFODATA, this);
    set(CURLOPT_NOPROGRESS, long(0));
}

HCurlNetwork::~HCurlNetwork()
{
    removeFromMulti();

    if (handle_)
    {
        curl_easy_cleanup(handle_);
    }

    if (curlHttpHeaders_)
    {
        curl_slist_free_all(curlHttpHeaders_);
        curlHttpHeaders_ = nullptr;
    }
}

void HCurlNetwork::deleteLater()
{
    removeFromMulti();
    QObject::deleteLater();
}

void HCurlNetwork::perform()
{
    if (isRunning())
    {
        return;
    }

    rebuildCurlHttpHeaders();

    if (preferredMulti_)
    {
        runningOnMulti_ = preferredMulti_;
    }
    else
    {
        runningOnMulti_ = CurlMulti::threadInstance();
    }

    runningOnMulti_->addTransfer(this);
}

void HCurlNetwork::abort()
{
    if (!isRunning())
    {
        return;
    }

    removeFromMulti();

    emit aborted();
}

void HCurlNetwork::removeFromMulti()
{
    if (runningOnMulti_ != nullptr)
    {
        runningOnMulti_->removeTransfer(this);
        runningOnMulti_ = nullptr;
    }
}

void HCurlNetwork::onCurlMessage(CURLMsg *message)
{
    if (message->msg == CURLMSG_DONE)
    {
        removeFromMulti();
        lastResult_ = message->data.result;
        emit done(lastResult_);
    }
}

void HCurlNetwork::rebuildCurlHttpHeaders()
{
    if (!httpHeadersWereSet_)
    {
        return;
    }

    if (curlHttpHeaders_)
    {
        curl_slist_free_all(curlHttpHeaders_);
        curlHttpHeaders_ = nullptr;
    }

    for (auto it = httpHeaders_.begin(); it != httpHeaders_.end(); ++it)
    {
        const QString &header = it.key();
        const QByteArray &value = it.value();

        QByteArray headerString = header.toUtf8();
        headerString += ": ";
        headerString += value;
        headerString.append(char(0));

        curlHttpHeaders_ = curl_slist_append(curlHttpHeaders_, headerString.constData());
    }

    set(CURLOPT_HTTPHEADER, curlHttpHeaders_);
}

void HCurlNetwork::setReadFunction(const HCurlNetwork::DataFunction &function)
{
    readFunction_ = function;
    if (readFunction_)
    {
        set(CURLOPT_READFUNCTION, staticCurlReadFunction);
        set(CURLOPT_READDATA, this);
    }
    else
    {
        set(CURLOPT_READFUNCTION, nullptr);
        set(CURLOPT_READDATA, nullptr);
    }
}

void HCurlNetwork::setWriteFunction(const HCurlNetwork::DataFunction &function)
{
    writeFunction_ = function;
    if (writeFunction_)
    {
        set(CURLOPT_WRITEFUNCTION, staticCurlWriteFunction);
        set(CURLOPT_WRITEDATA, this);
    }
    else
    {
        set(CURLOPT_WRITEFUNCTION, nullptr);
        set(CURLOPT_WRITEDATA, nullptr);
    }
}

void HCurlNetwork::setHeaderFunction(const HCurlNetwork::DataFunction &function)
{
    headerFunction_ = function;
    if (headerFunction_)
    {
        set(CURLOPT_HEADERFUNCTION, staticCurlHeaderFunction);
        set(CURLOPT_HEADERDATA, this);
    }
    else
    {
        set(CURLOPT_HEADERFUNCTION, nullptr);
        set(CURLOPT_HEADERDATA, nullptr);
    }
}

void HCurlNetwork::setSeekFunction(const HCurlNetwork::SeekFunction &function)
{
    seekFunction_ = function;
    if (seekFunction_)
    {
        set(CURLOPT_SEEKFUNCTION, staticCurlSeekFunction);
        set(CURLOPT_SEEKDATA, this);
    }
    else
    {
        set(CURLOPT_SEEKFUNCTION, nullptr);
        set(CURLOPT_SEEKDATA, nullptr);
    }
}

size_t HCurlNetwork::staticCurlWriteFunction(char *data, size_t size, size_t nitems, void *easyPtr)
{
    HCurlNetwork *easy = static_cast<HCurlNetwork*>(easyPtr);
    Q_ASSERT(easy != nullptr);

    if (easy->writeFunction_)
    {
        return easy->writeFunction_(data, size*nitems);
    }
    else
    {
        return  size*nitems;
    }
}

size_t HCurlNetwork::staticCurlHeaderFunction(char *data, size_t size, size_t nitems, void *easyPtr)
{
    HCurlNetwork *easy = static_cast<HCurlNetwork*>(easyPtr);
    Q_ASSERT(easy != nullptr);

    if (easy->headerFunction_)
    {
        return easy->headerFunction_(data, size*nitems);
    }
    else
    {
        return  size*nitems;
    }
}

int HCurlNetwork::staticCurlSeekFunction(void *easyPtr, curl_off_t offset, int origin)
{
    HCurlNetwork *easy = static_cast<HCurlNetwork*>(easyPtr);
    Q_ASSERT(easy != nullptr);

    if (easy->seekFunction_)
    {
        return easy->seekFunction_(static_cast<qint64>(offset), origin);
    }
    else
    {
        return CURL_SEEKFUNC_CANTSEEK;
    }
}

size_t HCurlNetwork::staticCurlReadFunction(char *buffer, size_t size, size_t nitems, void *easyPtr)
{
    HCurlNetwork *transfer = static_cast<HCurlNetwork*>(easyPtr);
    Q_ASSERT(transfer != nullptr);

    if (transfer->readFunction_)
    {
        return transfer->readFunction_(buffer, size*nitems);
    }
    else
    {
        return size*nitems;
    }
}

int HCurlNetwork::staticCurlXferInfoFunction(void *easyPtr, curl_off_t downloadTotal, curl_off_t downloadNow, curl_off_t uploadTotal, curl_off_t uploadNow)
{
    HCurlNetwork *transfer = static_cast<HCurlNetwork*>(easyPtr);
    Q_ASSERT(transfer != nullptr);

    emit transfer->progress(static_cast<qint64>(downloadTotal), static_cast<qint64>(downloadNow),  static_cast<qint64>(uploadTotal), static_cast<qint64>(uploadNow));

    return 0;
}

void HCurlNetwork::removeHttpHeader(const QString &header)
{
    httpHeaders_.remove(header);
    httpHeadersWereSet_ = true;
}

QByteArray HCurlNetwork::httpHeaderRaw(const QString &header) const
{
    return httpHeaders_[header];
}

void HCurlNetwork::setHttpHeaderRaw(const QString &header, const QByteArray &encodedValue)
{
    httpHeaders_[header] = encodedValue;
    httpHeadersWereSet_ = true;
}

bool HCurlNetwork::set(CURLoption option, const QString &parameter)
{
    return set(option, parameter.toUtf8().constData());
}

bool HCurlNetwork::set(CURLoption option, const QUrl &parameter)
{
    return set(option, parameter.toEncoded().constData());
}

void HCurlNetwork::setHttpHeader(const QString &header, const QString &value)
{
    setHttpHeaderRaw(header, QUrl::toPercentEncoding(value));
}

QString HCurlNetwork::httpHeader(const QString &header) const
{
    return QUrl::fromPercentEncoding(httpHeaders_[header]);
}

bool HCurlNetwork::hasHttpHeader(const QString &header) const
{
    return httpHeaders_.contains(header);
}



/////---------------------------------------------------------------------------------------------------------------------------
struct CurlMultiSocket
{
    curl_socket_t socketDescriptor = CURL_SOCKET_BAD;
    QSocketNotifier *readNotifier = nullptr;
    QSocketNotifier *writeNotifier = nullptr;
    QSocketNotifier *errorNotifier = nullptr;
};

CurlMulti::CurlMulti(QObject *parent) : QObject(parent), timer_(new QTimer(this))
{
    handle_ = curl_multi_init();
    Q_ASSERT(handle_ != nullptr);

    curl_multi_setopt(handle_, CURLMOPT_SOCKETFUNCTION, staticCurlSocketFunction);
    curl_multi_setopt(handle_, CURLMOPT_SOCKETDATA, this);
    curl_multi_setopt(handle_, CURLMOPT_TIMERFUNCTION, staticCurlTimerFunction);
    curl_multi_setopt(handle_, CURLMOPT_TIMERDATA, this);

    timer_->setSingleShot(true);
    connect(timer_, &QTimer::timeout, this, &CurlMulti::curlMultiTimeout);
}

CurlMulti::~CurlMulti()
{
    while (!transfers_.empty())
    {
        (*transfers_.begin())->abort();
    }

    if (handle_)
    {
        curl_multi_cleanup(handle_);
    }
}

CurlMulti *CurlMulti::threadInstance()
{
    static QThreadStorage<std::shared_ptr<CurlMulti>> instances;
    if (!instances.hasLocalData())
    {
        instances.setLocalData(std::make_shared<CurlMulti>());
    }
    return instances.localData().get();
}

void CurlMulti::addTransfer(HCurlNetwork *transfer)
{
    transfers_ << transfer;
    curl_multi_add_handle(handle_, transfer->handle());
}

void CurlMulti::removeTransfer(HCurlNetwork *transfer)
{
    if (transfers_.contains(transfer))
    {
        curl_multi_remove_handle(handle_, transfer->handle());
        transfers_.remove(transfer);
    }
}

int CurlMulti::curlSocketFunction(CURL *easyHandle, curl_socket_t socketDescriptor, int action, CurlMultiSocket *socket)
{
    Q_UNUSED(easyHandle);
    if (!socket)
    {
        if (action == CURL_POLL_REMOVE || action == CURL_POLL_NONE)
        {
            return 0;
        }

        socket = new CurlMultiSocket;
        socket->socketDescriptor = socketDescriptor;
        curl_multi_assign(handle_, socketDescriptor, socket);
    }

    if (action == CURL_POLL_REMOVE)
    {
        curl_multi_assign(handle_, socketDescriptor, nullptr);

        // Note: deleteLater will NOT work here since there are
        //       situations where curl subscribes same sockect descriptor
        //       until events processing is done and actual delete happen.
        //       This causes QSocketNotifier not to register notifications again.
        if (socket->readNotifier)
        {
            delete socket->readNotifier;
        }
        if (socket->writeNotifier)
        {
            delete socket->writeNotifier;
        }
        if (socket->errorNotifier)
        {
            delete socket->errorNotifier;
        }
        delete socket;
        return 0;
    }

    if (action == CURL_POLL_IN || action == CURL_POLL_INOUT)
    {
        if (!socket->readNotifier)
        {
            socket->readNotifier = new QSocketNotifier(socket->socketDescriptor, QSocketNotifier::Read);
            connect(socket->readNotifier, &QSocketNotifier::activated, this, &CurlMulti::socketReadyRead);
        }
        socket->readNotifier->setEnabled(true);
    }
    else
    {
        if (socket->readNotifier)
        {
            socket->readNotifier->setEnabled(false);
        }
    }

    if (action == CURL_POLL_OUT || action == CURL_POLL_INOUT)
    {
        if (!socket->writeNotifier)
        {
            socket->writeNotifier = new QSocketNotifier(socket->socketDescriptor, QSocketNotifier::Write);
            connect(socket->writeNotifier, &QSocketNotifier::activated, this, &CurlMulti::socketReadyWrite);
        }
        socket->writeNotifier->setEnabled(true);
    }
    else
    {
        if (socket->writeNotifier)
        {
            socket->writeNotifier->setEnabled(false);
        }
    }

    return 0;
}

int CurlMulti::curlTimerFunction(int timeoutMsec)
{
    if (timeoutMsec >= 0)
    {
        timer_->start(timeoutMsec);
    }
    else
    {
        timer_->stop();
    }

    return 0;
}

void CurlMulti::curlMultiTimeout()
{
    curlSocketAction(CURL_SOCKET_TIMEOUT, 0);
}

void CurlMulti::socketReadyRead(int socketDescriptor)
{
    curlSocketAction(socketDescriptor, CURL_CSELECT_IN);
}

void CurlMulti::socketReadyWrite(int socketDescriptor)
{
    curlSocketAction(socketDescriptor, CURL_CSELECT_OUT);
}

void CurlMulti::socketException(int socketDescriptor)
{
    curlSocketAction(socketDescriptor, CURL_CSELECT_ERR);
}

void CurlMulti::curlSocketAction(curl_socket_t socketDescriptor, int eventsBitmask)
{
    int runningHandles;
    CURLMcode rc = curl_multi_socket_action(handle_, socketDescriptor, eventsBitmask, &runningHandles);
    if (rc != 0)
    {
        // TODO: Handle global curl errors
    }

    int messagesLeft = 0;
    do
    {
        CURLMsg *message = curl_multi_info_read(handle_, &messagesLeft);

        if (message == nullptr)
        {
            break;
        }

        if (message->easy_handle == nullptr)
        {
            continue;
        }

        HCurlNetwork *transfer = nullptr;
        curl_easy_getinfo(message->easy_handle, CURLINFO_PRIVATE, &transfer);

        if (transfer == nullptr)
        {
            continue;
        }

        transfer->onCurlMessage(message);
    }
    while (messagesLeft);
}



int CurlMulti::staticCurlSocketFunction(CURL *easyHandle, curl_socket_t socketDescriptor, int what, void *userp, void *sockp)
{
    Q_UNUSED(easyHandle);
    CurlMulti *multi = static_cast<CurlMulti*>(userp);
    Q_ASSERT(multi != nullptr);

    return multi->curlSocketFunction(easyHandle, socketDescriptor, what, static_cast<CurlMultiSocket*>(sockp));
}

int CurlMulti::staticCurlTimerFunction(CURLM *multiHandle, long timeoutMs, void *userp)
{
    Q_UNUSED(multiHandle);
    CurlMulti *multi = static_cast<CurlMulti*>(userp);
    Q_ASSERT(multi != nullptr);

    int intTimeoutMs;

    if (timeoutMs >= std::numeric_limits<int>::max())
    {
        intTimeoutMs = std::numeric_limits<int>::max();
    }
    else if (timeoutMs >= 0)
    {
        intTimeoutMs = static_cast<int>(timeoutMs);
    }
    else
    {
        intTimeoutMs = -1;
    }

    return multi->curlTimerFunction(intTimeoutMs);
}
