#include "hmanagethread.h"

///------------------------------------------------
HManageThread::HManageThread()
{
    for (int i=0; i<Max_thread_size; i++)
    {
        worker[i] = new HThreadObject;
        worker[i]->moveToThread(&workerThread[i]);
        connect(&workerThread[i], &QThread::finished, worker[i], &QObject::deleteLater);
        connect(this, &HManageThread::operate, worker[i], &HThreadObject::work);
        connect(worker[i], &HThreadObject::notifyResult, this, &HManageThread::handleResults);
        workerThread[i].start();
    }
}

HManageThread::~HManageThread()
{
    for (int i=0; i<Max_thread_size; i++)
    {
        workerThread[i].quit();
        workerThread[i].wait();
    }
}

void HManageThread::start(QList<HttpInfo> httpInfo)
{
    int size = httpInfo.size();
    m_total = size;
    m_remainder = size;
    m_fileinfo = "Total "+ QString("%1").arg(m_total) + " files save ok!";
    int averg = size/Max_thread_size;
    int mod = size%Max_thread_size;
    int index = 0;
    for (int i=0; i<Max_thread_size; i++)
    {
        number[i] = averg;
        if(mod>0)
        {
            number[i]++;
            mod --;
        }
        QList<HttpInfo> temp;
        int max = averg*(i+1);
        if (max > size)
        {
            max = size;
        }
        for (; number[i]>0&&index<size; number[i]--)
        {
            temp.push_back(httpInfo[index++]);
        }
        worker[i]->setInput(temp);
    }
    QUrl url = httpInfo[0].url;
    QString host = url.host();
    QString  port = ":"+QString("%1").arg(url.port());

    static QProgressDialog progressDialog(NULL);
    static bool setok = false;
    if (!setok)
    {
        setok = true;
        Qt::WindowFlags flags = progressDialog.windowFlags();
        flags |= Qt::WindowStaysOnTopHint;
        progressDialog.setWindowFlags(flags);
        progressDialog.setModal(true);
        progressDialog.setRange(1, 100);
        progressDialog.setValue(100);
        progressDialog.setMinimumDuration(0);
        progressDialog.setWindowTitle("Down Files...");
        //progressDialog.setWindowFlags();
        progressDialog.setMinimumSize(QSize(300, 75));
        //progressDialog.setLabelText("Downloading "+host+port);
        progressDialog.setCancelButton(0);
        connect(this, &HManageThread::ProgressInfo, &progressDialog, &QProgressDialog::setLabelText);
        connect(this, &HManageThread::finished, &progressDialog, &QProgressDialog::close);
    }
    progressDialog.setLabelText("Start Downloading...");
    progressDialog.show();

    emit operate();
}


void HManageThread::cancelDownload()
{

}

void HManageThread::handleResults(const int &msg)
{
    m_remainder--;
    emit ProgressInfo(tr("Downloading %1 / %2   ...").arg(m_total-m_remainder).arg(m_total));
    if (m_remainder <1)
    {
        emit finished();
    }
}
