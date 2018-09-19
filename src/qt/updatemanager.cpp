#include "updatemanager.h"

#include "filedownloader.h"
#include "utilqt.h"
#include <QtXml>
#include <QCoreApplication>
#include <QFileInfo>
#include <QProcess>
#include <QMessageBox>
#include <QTime>
#include "../wizbl_package/define_wizbl.h"

const QString updateSiteAddress = UPDATE_SITE_ADDRESS;
const QString versionManagementFile = UPDATE_VERSION_MANAGEMENT_FILE;
int UpdateManager::updateCheckIntervalSecond_ = DEFAULT_UPDATE_CHECK_INTERVAL_SECOND;
QSharedMemory UpdateManager::processActiveCheck_;
const QString pathCheck = "/Check/";
const QString pathUpdate = "/Update/";

bool UpdateManager::LoadXmlFile(QDomDocument& doc, const QString &path)
{
    QFile load_xml_file(path);
    if (!load_xml_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << tr("Failed to read file: %1:%2").arg(path, load_xml_file.errorString());
        return false;
    }

    // load the xml file content
    doc.setContent(&load_xml_file);
    load_xml_file.close();
    return true;
}

bool UpdateManager::versionCheck(const QString &currentRule, const QString &currentVersion)
{
    qDebug() << "{currentRule}" << currentRule;
    qDebug() << "{currentVersion}" << currentVersion;
    const QString workPath = util::currentPath() + pathCheck;
    FileDownloader fdl({QUrl(updateSiteAddress + versionManagementFile)}, workPath, false);
    fdl.clean();
    const int ret = fdl.exec();
    if(QDialog::Accepted != ret)
    {
        qDebug() << tr("FileDownloader::Rejected");
        return false;
    }

    QDomDocument doc;
    if(!LoadXmlFile(doc, (workPath + versionManagementFile)))
        return false;

    const QDomNodeList updataRuleNodeList = doc.firstChildElement(TEXT_wizbl "coin-qt").childNodes();
    for(int iLIndex = 0; iLIndex < updataRuleNodeList.size(); iLIndex++)
    {
        const QDomNode ruleNode = updataRuleNodeList.item(iLIndex);
        if(0 == ruleNode.nodeName().compare("update") && 0 == ruleNode.toElement().attribute("rule").compare(currentRule))
        {
            updateCheckIntervalSecond_ = ruleNode.firstChildElement("checkIntervalSecond").text().toInt();
            if (0 == updateCheckIntervalSecond_)
                updateCheckIntervalSecond_ = DEFAULT_UPDATE_CHECK_INTERVAL_SECOND;

            const QString updateVersion = ruleNode.firstChildElement("version").text();
            if (currentVersion == updateVersion) {
                QDir removeRecursively;
                removeRecursively.setPath(util::currentPath() + pathCheck);
                removeRecursively.removeRecursively();
                removeRecursively.setPath(util::currentPath() + pathUpdate);
                removeRecursively.removeRecursively();
                return true;
            }

            const QDomElement filesElement = ruleNode.firstChildElement("files");
            const int fileCount = filesElement.attribute("count").toInt();
            if(1 > fileCount){
                qDebug() << tr("Version management file corruption.(File count 0)");
                return false;
            }

            std::vector<QString> fileList;
            for(int iFIndex = 0; iFIndex < fileCount; iFIndex++)
            {
                const QString fileText = filesElement.firstChildElement(QString("file%1").arg(iFIndex)).text();
                if(fileText.isEmpty()){
                    qDebug() << tr("Version management file corruption.(The file name is empty.[%1/%2])").arg(iFIndex).arg(fileCount);
                    return false;
                }
                fileList.emplace_back(fileText);
            }

            if (QMessageBox::question(Q_NULLPTR, tr("Wallet Update"),
                                      tr("Perform a wallet update.(%1 -> %2)").arg(currentVersion, updateVersion),
                                      QMessageBox::Yes, QMessageBox::Yes) != QMessageBox::Yes) {//사용자가 esc 키 하는것은 막지 않음.
                updateCheckIntervalSecond_ = 60;//하던 작업 마무리할 시간은 주자.
                qDebug() << tr("User cancels the update.");
                return false;
            }
            return versionDownload(updateVersion, fileList);
        }
    }

    qDebug() << tr("Version management file corruption.");
    return false;
}

bool UpdateManager::versionDownload(const QString &updateVersion, const std::vector<QString> &fileList)
{
    if (1 > fileList.size()) {
        qDebug() << tr("No job target.");
        return false;
    }

    std::vector<QUrl> urlList;
    for (auto& itr : fileList) {
        urlList.emplace_back(QString("%1%2/%3/%4").arg(updateSiteAddress, updateVersion, util::currentOsName(), itr));
    }

    const QString workPath = util::currentPath() + pathUpdate;
    FileDownloader fdl(urlList, workPath, true);
    fdl.clean();
    const int ret = fdl.exec();
    if (QDialog::Accepted != ret) {
        qDebug() << tr("FileDownloader::Rejected");
        return false;
    }

    return versionUpdate(workPath, fileList);
}

bool UpdateManager::versionUpdate(const QString &workPath, const std::vector<QString> &fileList)
{
    if (1 > fileList.size()) {
        qDebug() << tr("No job target.");
        return false;
    }
    QString updateProgram = workPath + "/" + fileList[0];
    updateProgram = QDir::cleanPath(QDir::toNativeSeparators(updateProgram));

    QString updateRestart = "-updaterestart=" + util::copyArgument().replace("\"", "!!");
    QString updateArguments = "-updatefiles=\"";
    for (auto& itr : fileList)
    {
        updateArguments += itr;
        updateArguments += ";";
    }
    updateArguments += "\"";

    QStringList arguments;
    arguments << "-updaterun=1";
    arguments << updateArguments;
    arguments << updateRestart;
    arguments << QString("-updateparentpid=%1").arg(createProcessActiveCheck());
    arguments << QString("-updatefolder=%1").arg(util::currentPath());

    if (!QProcess::startDetached(updateProgram, arguments)) {
        qDebug() << tr("An error executing the %1 program.").arg(updateProgram);
        return false;
    }
    qApp->quit();//지갑의 정상 종료 흐름을 사용하기 위해서.
    return true;
}

bool UpdateManager::updateInstallation(const QString &parentProcessId, const QString &reStart, const QString &targetPath, const QString &files)
{
    qDebug() << "{parentProcessId}" << parentProcessId;
    qDebug() << "{reStart}" << reStart;
    qDebug() << "{targetPath}" << targetPath;
    qDebug() << "{files}" << files;

    //정상적으로 설치할 파일정보가 넘어왔는지 확인한다.
    QStringList fileList = QString(files).replace("\"", "").split(";");
    if (1 > fileList.size()) {
        QMessageBox::warning(Q_NULLPTR, tr("error."), tr("There are no files to update."), QMessageBox::Close);
        exit(0);//updateInstallation은 지갑이 초기화 되기 전이기 때문에 exit(0); 로 종료한다.
        return false;
    }

    ////이전 버전 프로그램의 종료를 확인한다. (필요한 시점에 안먹혀서 아래 파일 복사 에러로 대체함.)
    //QTime time;
    //time.start();
    //while(existProcessActiveCheck(parentProcessId))
    //{
    //    qDebug() << "while(existProcessActiveCheck(parentProcessId))";
    //    if (60 * 1000 < time.elapsed()) {
    //        if (QMessageBox::question(Q_NULLPTR, tr("End confirmation."),
    //                                  tr("Confirm termination of [%1].").arg(parentProcessId),
    //                                  QMessageBox::Retry | QMessageBox::Abort) != QMessageBox::Retry) {
    //            exit(0);//updateInstallation은 지갑이 초기화 되기 전이기 때문에 exit(0); 로 종료한다.
    //            return false;
    //        }
    //        time.start();
    //    }
    //    QThread::msleep(100);
    //}

    QTime time;
    time.start();
    for(auto itr : fileList)
    {
        qDebug() << itr;
        if(itr.isEmpty())
            continue;
        QString fileName = util::currentPath() + "/" + itr;
        QString newName = targetPath + "/" + itr;
        bool ok = true;
        do{
            QFile::remove(newName);
            ok = QFile::copy(fileName, newName);
            qDebug() << ok << fileName << newName;
            if (true != ok && 60 * 1000 < time.elapsed()) {
                if (QMessageBox::warning(Q_NULLPTR, tr("Copy failed."),
                        tr("Failed to copy %1 files.\nConfirm termination and click Retry.").arg(itr),
                        QMessageBox::Retry | QMessageBox::Abort) != QMessageBox::Retry) {
                    exit(0);//updateInstallation은 지갑이 초기화 되기 전이기 때문에 exit(0); 로 종료한다.
                    return false;
                }
                time.start();
            }
            QThread::msleep(100);
        }while(true != ok);
    }

    QString reStarter = QString(reStart).replace("!!", "\"");//targetPath + "/" + fileList.at(0) + " " + reStart;
    if (!QProcess::startDetached(reStarter)) {
        qDebug() << tr("An error executing the %1 program.").arg(reStarter);
        QMessageBox::warning(Q_NULLPTR, tr("error."), tr("Restart failed after update."), QMessageBox::Close);
        exit(0);//updateInstallation은 지갑이 초기화 되기 전이기 때문에 exit(0); 로 종료한다.
        return false;
    }
    exit(0);//updateInstallation은 지갑이 초기화 되기 전이기 때문에 exit(0); 로 종료한다.
    return true;
}

int UpdateManager::updateCheckIntervalSecond()
{
    return updateCheckIntervalSecond_;
}

QString UpdateManager::createProcessActiveCheck()
{
    while (Q_NULLPTR == processActiveCheck_.data())
    {
        processActiveCheck_.setKey(QDateTime::currentDateTime().toString("%1 hh:mm:ss:zzz").arg(TEXT_wizbl "coin-qt"));
        if(processActiveCheck_.create(512,QSharedMemory::ReadWrite))
            break;
        QThread::msleep(10);
    } 
    
    return processActiveCheck_.key();
}

bool UpdateManager::existProcessActiveCheck(const QString& processActiveCheckKey)
{
    QSharedMemory processActiveCheck;
    processActiveCheck.setKey(processActiveCheckKey);
    bool ret = processActiveCheck.create(512, QSharedMemory::ReadWrite);
    qDebug() << processActiveCheckKey << !ret;
    return !ret;
}

bool UpdateManager::updateClean()
{
    QTime time;
    time.start();
    bool exists = false;
    QDir removeRecursively;
    do {
        exists = false;
        removeRecursively.setPath(util::currentPath() + pathCheck);
        removeRecursively.removeRecursively();
        exists |= removeRecursively.exists();
        removeRecursively.setPath(util::currentPath() + pathUpdate);
        removeRecursively.removeRecursively();
        exists |= removeRecursively.exists();

        if (true == exists && 60 * 1000 < time.elapsed()) {
            if (QMessageBox::warning(Q_NULLPTR, tr("Wait."),
                    tr("Wait for the updater to exit."),
                    QMessageBox::Retry | QMessageBox::Abort) != QMessageBox::Retry) {
                break;
            }
            time.start();
        }
        QThread::msleep(100);
    } while (exists);

    return !exists;
}
