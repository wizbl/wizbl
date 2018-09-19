#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QtNetwork>
#include <QUrl>
#include <vector>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QDialogButtonBox>
#include <QNetworkAccessManager>
#include <QDialog>
#include <QBasicTimer>

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

class FileDownloader : public QDialog
{
    Q_OBJECT

public:
    explicit FileDownloader(const std::vector<QUrl> &urlList, const QString &tempDirectory, const bool visible, QWidget *parent = Q_NULLPTR);

public:
    bool clean();

private Q_SLOTS:
    void cancelDownload();
    void httpFinished();
    void httpReadyRead();
    void slotAuthenticationRequired(QNetworkReply*,QAuthenticator *);
    void networkReplyProgress(qint64 bytesRead, qint64 totalBytes);
#ifndef QT_NO_SSL
    void sslErrors(QNetworkReply*,const QList<QSslError> &errors);
#endif

private:
    bool downloadFile();
    void startRequest(const QUrl &requestedUrl);
    QFile *openFileForWrite(const QString &fileName);

private:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void timerEvent(QTimerEvent *event) override;

private:
    std::vector<QUrl>   urlList_;               //다운받을 파일 리스트
    QString             jobPath_;               //다운받을 경로
    bool                visible_;               //화면에 표시하는지.

private :
    QNetworkAccessManager   networkAccessManager_;
    qint32                  itemIndex_;         //개별 어떤걸 진행하고 있나?
    QUrl                    itemUrl_;           //개별 진행하는 url
    QNetworkReply           *itemNetworkReply_; //개별 진행의 NetworkReply
    QFile                   *itemFile_;         //개별 진행의 파일 저장
    bool                    httpRequestAborted_;//중지 명령이 있었나?
    QBasicTimer             jobStartTimer_;     //작업 시작 타이머(ui발생후 진행하려고.)

private:
    QVBoxLayout         rootVBoxLayout_;        //ui layout
    QLabel              totalLabel_;            //전체 진행상태
    QProgressBar        totalProgressBar_;      //전체 진행상태
    QLabel              itemLabel_;             //개별 진행상태
    QProgressBar        itemProgressBar_;       //개별 진행상태
    QDialogButtonBox    buttonBox_;             //취소등.
};

#endif // FILEDOWNLOADER_H
