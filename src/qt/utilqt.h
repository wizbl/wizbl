#ifndef UTIL_H
#define UTIL_H

#include <QtGlobal>
#include <QObject>

class QPaintEvent;
class QWidget;

class util : public QObject
{
    Q_OBJECT
private:
    util():QObject(){}

public:
    //경로 설정 및 가져오기
    static bool setCurrentPath(const QString& path);
    static QString currentPath();
    //main Argument 복사 및 가져오기
    static void copyArgument(int argc, char *argv[]);
    static QString copyArgument();
    //Process Id
    static ulong currentProcessId();
    static bool isProcessExist(ulong processID);
    static QString processName(ulong processID);
    //현재 os 이름 얻기
    static QString currentOsName();
    //log 남기기 (qInstallMessageHandler(util::logToFile);)
    static void logToFile(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    //공용 스타일 시트
    static QString getStyleSheet();
    //타이틀 스타일 적용
    static void paintEvent(QPaintEvent *event, QWidget *qthis);

private:
    static QString currentPath_;
    static QString copyArgument_;
};

#endif // UTIL_H
