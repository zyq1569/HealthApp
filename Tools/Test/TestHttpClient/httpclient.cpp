#include "httpclient.h"

#include "ui_authenticationdialog.h"


#include <QNetworkReply>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QDesktopServices>
#include <QAuthenticator>

ProgressDialog::ProgressDialog(const QUrl &url, QWidget *parent):QProgressDialog(parent)
{
    setWindowTitle(tr("Download Progress"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setLabelText(tr("Downloading %1.").arg(url.toDisplayString()));
    setMinimum(0);
    setValue(0);
    setMinimumDuration(0);
    setMinimumSize(QSize(400, 75));
}

ProgressDialog::~ProgressDialog()
{

}

void ProgressDialog::networkReplyProgress(qint64 bytesRead, qint64 totalBytes)
{
    setMaximum(totalBytes);
    setValue(bytesRead);
}


//-----------------------------------------HttpClient------------------------------------------------------------
HttpClient::HttpClient(QObject *parent, QString dir) : QObject(parent),m_httpRequestAborted(false)
{
    m_file = nullptr;
    if (dir != "")
    {
        setDwonloadDir(dir);
    }
    else
    {
        m_downDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)+"\\";
    }
    m_currentfiletype = DownFileType::other;
}

void HttpClient::setDwonloadDir(QString dir)
{
    m_downDir = dir;
    if (m_downDir.isEmpty() || !QFileInfo(m_downDir).isDir())
    {
        m_downDir = QDir::currentPath();
    }
}
void HttpClient::startRequest(const QUrl &requestedUrl)
{
    m_url = requestedUrl;
    m_httpRequestAborted = false;

    m_networkreply = m_networkmanager.get(QNetworkRequest(m_url));
    connect(m_networkreply, &QIODevice::readyRead, this, &HttpClient::httpReadyRead);
    connect(m_networkreply, &QNetworkReply::finished, this, &HttpClient::httpFinished);

    //    ProgressDialog *progressDialog = new ProgressDialog(m_url, NULL);
    //    progressDialog->setAttribute(Qt::WA_DeleteOnClose);
    //    connect(progressDialog, &QProgressDialog::canceled, this, &HttpClient::cancelDownload);
    //    connect(m_networkreply, &QNetworkReply::downloadProgress, progressDialog, &ProgressDialog::networkReplyProgress);
    //    connect(m_networkreply, &QNetworkReply::finished, progressDialog, &ProgressDialog::hide);
    //    progressDialog->show();


}

void HttpClient::getStudyDBinfo(QUrl url,QString start,QString end,QString page,QString limit)
{
    ///http://127.0.0.1:8080/healthsystem/ris/stduyimage/?start=19700101&end=20191230&page=1&limit=10
    //QString startDate = ui->m_startDate->text();
    //QString endDate = ui->m_endDate->text();
    //QString mod = ui->m_StudyModality->currentText();
    //QUrl url = ui->m_URL->text();
    QString newUrlStr = url.toString() + "/healthsystem/ris/stduyimage/?start=";
    newUrlStr += start+"&end="+end+"&page="+page+"&limit="+limit;
    m_currentfiletype = DownFileType::json;

    const QUrl newUrl = newUrlStr;
    if (!newUrl.isValid())
    {
        QMessageBox::information(NULL, tr("Error"),tr("Invalid URL: %1: %2").arg(newUrlStr, newUrl.errorString()));
        return;
    }
    downFileFromWeb(newUrl,"studyDb.json",m_downDir);
}

void HttpClient::downFileFromWeb(QUrl httpUrl, QString savefilename, QString downDir)
{
    QString fileName = savefilename;
    QString downloadDirectory = downDir;
    bool useDirectory = !downloadDirectory.isEmpty() && QFileInfo(downloadDirectory).isDir();
    if (useDirectory)
    {
        fileName.prepend(downloadDirectory + '/');
    }
    if (QFile::exists(fileName))
    {
        if (QMessageBox::question(NULL, tr("Overwrite Existing File"),
                                  tr("There already exists a file called %1%2."
                                     " Overwrite?").arg(fileName,
                                                        useDirectory ? QString() : QStringLiteral(" in the current directory")),
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
        {
            return;
        }
        QFile::remove(fileName);
    }
    if (!openFileForWrite(fileName))
    {
        return;
    }

    // schedule the request
    startRequest(httpUrl);
}

void HttpClient::getStudyImageFile(QUrl url,QString studyuid,QString seruid, QString imguid)
{
    /// -------------------------------------------------------------------------
    ///http://127.0.0.1:8080/WADO?
    ///studyuid=1.2.826.1.1.3680043.2.461.20090916105245.168977.200909160196
    ///&seriesuid=1.2.840.113619.2.55.3.604688119.969.1252951290.810.4
    ///&sopinstanceuid=1.2.840.113619.2.55.3.604688119.969.1252951290.968.37
    /// -------------------------------------------------------------------------
    ///http://127.0.0.1:8080/WADO?
    ///studyuid=1.2.826.1.1.3680043.2.461.20090916105245.168977.200909160196&type=json
    ///-------------------------------------------------------------------------
    m_currentfiletype = DownFileType::other;
    if (studyuid != "")
    {
        if (seruid != "" && imguid != "")
        {
            QString strURL = url.toString()+"/WADO?studyuid="+studyuid+"&seriesuid="+seruid+"&sopinstanceuid="+imguid;
            url = strURL;
            m_currentfiletype = DownFileType::dcm;
        }
        else
        {
            QString strURL = url.toString()+"/WADO?studyuid="+studyuid+"&type=json";
            url = QUrl(strURL);
            m_currentfiletype = DownFileType::json;
        }
    }
    m_url = url;
    const QString urlSpec = m_url.toString().trimmed();
    if (urlSpec.isEmpty())
    {
        return;
    }

    const QUrl newUrl = QUrl::fromUserInput(urlSpec);
    if (!newUrl.isValid())
    {
        QMessageBox::information(NULL, tr("Error"),tr("Invalid URL: %1: %2").arg(urlSpec, newUrl.errorString()));
        return;
    }

    QString fileName = newUrl.fileName();
    switch (m_currentfiletype)
    {
    case DownFileType::dcm :
        fileName = imguid+".dcm";
        break;
    case DownFileType::json:
        fileName = studyuid+".json";
        break;
    default:
        break;
    }
    if (fileName.isEmpty())
    {
        fileName = "temp.tmp";
    }

    downFileFromWeb(newUrl,fileName,m_downDir);

}

void HttpClient::cancelDownload()
{
    m_httpRequestAborted = true;
    m_networkreply->abort();
}

void HttpClient::httpFinished()
{
    QFileInfo fileinfo;
    if (m_file)
    {
        fileinfo.setFile(m_file->fileName());
        m_file->close();
        m_file->reset();
        delete  m_file;
        m_file = NULL;
    }

    if (m_httpRequestAborted)
    {
        m_networkreply->deleteLater();
        m_networkreply = nullptr;
        return;
    }

    if (m_networkreply->error())
    {
        QFile::remove(fileinfo.absoluteFilePath());
        m_networkreply->deleteLater();
        m_networkreply = nullptr;
        return;
    }

    const QVariant redirectionTarget = m_networkreply->attribute(QNetworkRequest::RedirectionTargetAttribute);

    m_networkreply->deleteLater();
    m_networkreply = nullptr;

    if (!redirectionTarget.isNull())
    {
        const QUrl redirectedUrl = m_url.resolved(redirectionTarget.toUrl());
        if (QMessageBox::question(NULL, tr("Redirect"),tr("Redirect to %1 ?").arg(redirectedUrl.toString()),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
        {
            QFile::remove(fileinfo.absoluteFilePath());
            return;
        }
        if (!openFileForWrite(fileinfo.absoluteFilePath()))
        {
            QFile::remove(fileinfo.absoluteFilePath());
            return;
        }
        startRequest(redirectedUrl);
        return;
    }

    /// test ///QDesktopServices::openUrl(QUrl::fromLocalFile(fileinfo.absoluteFilePath()));

}

bool HttpClient::openFileForWrite(const QString &fileName)
{
    if (m_file)
    {
        delete m_file;
        m_file = nullptr;
    }
    m_file = new QFile(fileName);
    if (!m_file->open(QIODevice::WriteOnly))
    {
        QMessageBox::information(NULL, tr("Error"),tr("Unable to save the file %1: %2.").arg(
                                     QDir::toNativeSeparators(fileName),m_file->errorString()));
        m_file->close();
        delete m_file;
        m_file = nullptr;
        return false;
    }
    return true;
}

void HttpClient::httpReadyRead()
{
    // this slot gets called every time the QNetworkReply has new data.
    // We read all of its new data and write it into the file.
    // That way we use less RAM than when reading it at the finished()
    // signal of the QNetworkReply
    if (m_file)
    {
        if (m_currentfiletype == DownFileType::json)
        {
            m_currentDownData = m_networkreply->readAll();
            m_file->write(m_currentDownData);
        }
        else
        {
            m_file->write(m_networkreply->readAll());
        }
    }
}

void HttpClient::slotAuthenticationRequired(QNetworkReply *, QAuthenticator *authenticator)
{
    QDialog authenticationDialog;
    Ui::Dialog ui;
    ui.setupUi(&authenticationDialog);
    authenticationDialog.adjustSize();
    ui.siteDescription->setText(tr("%1 at %2").arg(authenticator->realm(), m_url.host()));

    // Did the URL have information? Fill the UI
    // This is only relevant if the URL-supplied credentials were wrong
    ui.userEdit->setText(m_url.userName());
    ui.passwordEdit->setText(m_url.password());

    if (authenticationDialog.exec() == QDialog::Accepted)
    {
        authenticator->setUser(ui.userEdit->text());
        authenticator->setPassword(ui.passwordEdit->text());
    }
}

#ifndef QT_NO_SSL
void HttpClient::sslErrors(QNetworkReply *, const QList<QSslError> &errors)
{
    QString errorString;
    for (const QSslError &error : errors)
    {
        if (!errorString.isEmpty())
        {
            errorString += '\n';
        }
        errorString += error.errorString();
    }

    if (QMessageBox::warning(NULL, tr("SSL Errors"),
                             tr("One or more SSL errors has occurred:\n%1").arg(errorString),
                             QMessageBox::Ignore | QMessageBox::Abort) == QMessageBox::Ignore)
    {
        m_networkreply->ignoreSslErrors();
    }
}
#endif
