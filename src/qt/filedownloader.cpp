#include "filedownloader.h"

#include <QtWidgets>
#include <QDebug>

#include "ui_authenticationdialog.h"

FileDownloader::FileDownloader(const std::vector<QUrl> &urlList, const QString &jobPath, const bool visible, QWidget *parent)
    : QDialog(parent)
    , urlList_(urlList)
    , jobPath_(jobPath)
    , visible_(visible)
    , itemIndex_(0)
    , itemNetworkReply_(Q_NULLPTR)
    , itemFile_(Q_NULLPTR)
    , httpRequestAborted_(false)
{
    if (true != visible_)
        this->move(-1000, -1000);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Download"));

    connect(&networkAccessManager_, &QNetworkAccessManager::authenticationRequired,
            this, &FileDownloader::slotAuthenticationRequired);
#ifndef QT_NO_SSL
    connect(&networkAccessManager_, &QNetworkAccessManager::sslErrors,
            this, &FileDownloader::sslErrors);
#endif

    if(jobPath_.isEmpty())
        jobPath_ += QDir::currentPath() + "UpdateDownload";
    jobPath_ = QDir::cleanPath(QDir::toNativeSeparators(jobPath_));
    qDebug() << jobPath_;

    totalProgressBar_.setMinimum(0);
    totalProgressBar_.setValue(0);
    itemProgressBar_.setMinimum(0);
    itemProgressBar_.setValue(0);

    auto cancelBtn = buttonBox_.addButton(QDialogButtonBox::Cancel);
    cancelBtn->setText(tr("Cancel"));
    rootVBoxLayout_.addWidget(&totalLabel_);
    rootVBoxLayout_.addWidget(&totalProgressBar_);
    rootVBoxLayout_.addWidget(&itemLabel_);
    rootVBoxLayout_.addWidget(&itemProgressBar_);
    rootVBoxLayout_.addWidget(&buttonBox_);
    this->setLayout(&rootVBoxLayout_);

    totalProgressBar_.setRange(0, static_cast<int>(urlList_.size()));

    connect(cancelBtn, &QAbstractButton::clicked, this, &FileDownloader::cancelDownload);

    jobStartTimer_.start(1, this);
}

void FileDownloader::resizeEvent(QResizeEvent *event)
{
    rootVBoxLayout_.setGeometry(QRect(QPoint(0,0),event->size()));
}
void FileDownloader::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == jobStartTimer_.timerId())
    {
        jobStartTimer_.stop();

        if(1 > urlList_.size())
        {
            if (visible_)
                QMessageBox::critical(this, tr("Error."), tr("No Download Target Information."), QMessageBox::Close, QMessageBox::Close);
            this->reject();
            return;
        }

        QDir removeRecursively(jobPath_);
        if(removeRecursively.exists())
        {
            if (!visible_ || QMessageBox::Yes == QMessageBox::warning(this, tr("Remove all files."),
                                      tr("Removes all files in the update folder.\n(%1)").arg(jobPath_),
                                      QMessageBox::Yes|QMessageBox::No, QMessageBox::No))
            {
                if (!removeRecursively.removeRecursively()) {
                    if (visible_)
                        QMessageBox::critical(this, tr("Error."), tr("Failed to erase file.\n(%1)").arg(jobPath_), QMessageBox::Close, QMessageBox::Close);
                    this->reject();
                    return;
                }
            }
        }

        downloadFile();
    }
}

void FileDownloader::networkReplyProgress(qint64 bytesRead, qint64 totalBytes)
{
    itemProgressBar_.setMaximum(totalBytes);
    itemProgressBar_.setValue(bytesRead);
}

void FileDownloader::startRequest(const QUrl &requestedUrl)
{
    itemUrl_ = requestedUrl;

    itemNetworkReply_ = networkAccessManager_.get(QNetworkRequest(itemUrl_));
    connect(itemNetworkReply_, &QNetworkReply::finished, this, &FileDownloader::httpFinished);
    connect(itemNetworkReply_, &QIODevice::readyRead, this, &FileDownloader::httpReadyRead);

    connect(itemNetworkReply_, &QNetworkReply::downloadProgress, this, &FileDownloader::networkReplyProgress);

    itemLabel_.setText(tr("Downloading %1...").arg(itemUrl_.toString()));
}

bool FileDownloader::downloadFile()
{
    totalLabel_.setText(tr("Total: %1/%2").arg(itemIndex_).arg(static_cast<int>(urlList_.size())));
    totalProgressBar_.setValue(itemIndex_);

    if(urlList_.size() <= itemIndex_){
        this->accept();
        return true;
    }

    const QUrl& url = urlList_[itemIndex_];
    QString fileName = url.fileName();
    if (!url.isValid() || fileName.isEmpty()) {
        if (visible_)
            QMessageBox::critical(this, tr("Error"),
                                 tr("Invalid url: %1").arg(url.toString()));
        this->reject();
        return false;
    }

    fileName.prepend(jobPath_ + '/');
    fileName = QDir::cleanPath(QDir::toNativeSeparators(fileName));
    qDebug() << fileName;

    if (QFile::exists(fileName)) {
        // 180727 기준 동일한 파일명이 다른경로로 있으면 발생할 수 있겠다.
        if (!visible_ || QMessageBox::No
                == QMessageBox::critical(this, tr("Overwrite Existing itemFile_"),
                                         tr("There already exists a itemFile_ called %1 in "
                                            "the current directory. Overwrite?").arg(fileName),
                                         QMessageBox::Yes|QMessageBox::No, QMessageBox::No)){
            this->reject();
            return false;
        }
        QFile::remove(fileName);
    }

    itemFile_ = openFileForWrite(fileName);
    if (!itemFile_){
        this->reject();
        return false;
    }

    // schedule the request
    startRequest(url);
    return true;
}

QFile *FileDownloader::openFileForWrite(const QString &fileName)
{
    QFileInfo fileInfo(fileName);
    QDir dir;
    dir.mkpath(fileInfo.path());

    QScopedPointer<QFile> file(new QFile(fileName));
    if (!file->open(QIODevice::WriteOnly)) {
        if (visible_)
            QMessageBox::critical(this, tr("Error"),
                                 tr("Unable to save the file %1: %2.")
                                 .arg(QDir::toNativeSeparators(fileName),
                                      file->errorString()));
        return Q_NULLPTR;
    }
    return file.take();
}

void FileDownloader::cancelDownload()
{
    if(QMessageBox::No == QMessageBox::warning(this, tr("Cancel Download"), tr("Are you sure you want to cancel the download?"), QMessageBox::Yes|QMessageBox::No, QMessageBox::No))
        return;

    httpRequestAborted_ = true;
    itemNetworkReply_->abort();
    this->reject();
}

void FileDownloader::httpFinished()
{
    QFileInfo fi;
    if (itemFile_) {
        fi.setFile(itemFile_->fileName());
        itemFile_->close();
        delete itemFile_;
        itemFile_ = Q_NULLPTR;
    }

    if (httpRequestAborted_) {
        itemNetworkReply_->deleteLater();
        itemNetworkReply_ = Q_NULLPTR;
        this->reject();
        return;
    }

    if (itemNetworkReply_->error()) {
        QFile::remove(fi.absoluteFilePath());
        if (visible_)
            QMessageBox::critical(this, tr("Error."), tr("Download failed:\n%1.").arg(itemNetworkReply_->errorString()), QMessageBox::Close);
        itemNetworkReply_->deleteLater();
        itemNetworkReply_ = Q_NULLPTR;
        this->reject();
        return;
    }

    const QVariant redirectionTarget = itemNetworkReply_->attribute(QNetworkRequest::RedirectionTargetAttribute);

    itemNetworkReply_->deleteLater();
    itemNetworkReply_ = Q_NULLPTR;

    if (!redirectionTarget.isNull()) {
        const QUrl redirectedUrl = itemUrl_.resolved(redirectionTarget.toUrl());
        itemFile_ = openFileForWrite(fi.absoluteFilePath());
        if (!itemFile_) {
            this->reject();
            return;
        }
        startRequest(redirectedUrl);
        return;
    }

    itemIndex_++;
    downloadFile();
}

void FileDownloader::httpReadyRead()
{
    // this slot gets called every time the QNetworkReply has new data.
    // We read all of its new data and write it into the itemFile_.
    // That way we use less RAM than when reading it at the finished()
    // signal of the QNetworkReply
    if (itemFile_)
        itemFile_->write(itemNetworkReply_->readAll());
}

void FileDownloader::slotAuthenticationRequired(QNetworkReply*,QAuthenticator *authenticator)
{
    QDialog authenticationDialog;
    Ui::Dialog ui;
    ui.setupUi(&authenticationDialog);
    authenticationDialog.adjustSize();
    ui.siteDescription->setText(tr("%1 at %2").arg(authenticator->realm(), itemUrl_.host()));

    // Did the itemUrl_ have information? Fill the UI
    // This is only relevant if the itemUrl_-supplied credentials were wrong
    ui.userEdit->setText(itemUrl_.userName());
    ui.passwordEdit->setText(itemUrl_.password());

    if (authenticationDialog.exec() == QDialog::Accepted) {
        authenticator->setUser(ui.userEdit->text());
        authenticator->setPassword(ui.passwordEdit->text());
    }
}

#ifndef QT_NO_SSL
void FileDownloader::sslErrors(QNetworkReply*,const QList<QSslError> &errors)
{
    QString errorString;
    for (QSslError error : errors) {
        if (!errorString.isEmpty())
            errorString += '\n';
        errorString += error.errorString();
    }

    if (!visible_ || QMessageBox::warning(this, tr("SSL Errors"),
                             tr("One or more SSL errors has occurred:\n%1").arg(errorString),
                             QMessageBox::Ignore | QMessageBox::Abort) == QMessageBox::Ignore) {
        itemNetworkReply_->ignoreSslErrors();
    }
}
#endif

bool FileDownloader::clean()
{
    QDir removeRecursively(jobPath_);
    if(removeRecursively.exists())
    {
        if(!removeRecursively.removeRecursively())
        {
            if (visible_)
                QMessageBox::critical(this, tr("Error."), tr("Failed to erase file.\n(%1)").arg(jobPath_), QMessageBox::Close, QMessageBox::Close);
            return false;
        }
    }
    return true;
}
