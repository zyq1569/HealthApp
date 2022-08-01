#include "HCurlNetwork.h"


HCurlNetwork::HCurlNetwork(QObject *parent)
    : QObject(parent)
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

    emit transfer->progress(static_cast<qint64>(downloadTotal), static_cast<qint64>(downloadNow),
                            static_cast<qint64>(uploadTotal), static_cast<qint64>(uploadNow));

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
