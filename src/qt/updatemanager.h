#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QObject>
#include <QString>
#include <vector>
#include <QDomDocument>
#include <QSharedMemory>

class UpdateManager : public QObject
{
    Q_OBJECT
private:
    UpdateManager():QObject(){}

public:
    static bool updateClean();
    static bool updateInstallation(const QString &parentProcessId, const QString &reStart, const QString &targetPath, const QString &files);
    static bool versionCheck(const QString &currentRule, const QString &currentVersion);

    static int updateCheckIntervalSecond();

private:
    static bool versionDownload(const QString &updateVersion, const std::vector<QString> &fileList);
    static bool versionUpdate(const QString &workPath, const std::vector<QString> &fileList);
    static bool LoadXmlFile(QDomDocument& doc, const QString &path);
    static QString createProcessActiveCheck();
    static bool existProcessActiveCheck(const QString& processActiveCheckKey);
private:
    static int updateCheckIntervalSecond_;
    static QSharedMemory processActiveCheck_;
};

#endif // UPDATEMANAGER_H
