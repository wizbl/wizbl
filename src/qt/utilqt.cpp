#include "utilqt.h"

#if defined(Q_OS_WIN)
#   include <windows.h>
#   include <stdio.h>
#   include <tchar.h>
#   include <psapi.h>
#elif defined(Q_OS_LINUX)
#elif defined(Q_OS_MACOS)
#endif

#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QTime>
#include <QMessageBox>

#include <QMainWindow>
#include <QPainter>
#include <QStyleOptionTitleBar>
#include <QStyle>

QString util::currentPath_ = "";
QString util::copyArgument_ = "";

ulong util::currentProcessId()
{
    ulong ret = 0;
#if defined(Q_OS_WIN)
    ret = GetCurrentProcessId();
#elif defined(Q_OS_LINUX)
#elif defined(Q_OS_MACOS)
#endif
    return ret;
}

bool util::isProcessExist(ulong processID)
{
    bool ret = true;//구현 안되있으면 어디 경고라도 뜨게 하자.
#if defined(Q_OS_WIN)
    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID );
    qDebug() << processID << ":" << hProcess;
    ret = nullptr != hProcess;
    CloseHandle( hProcess );
#elif defined(Q_OS_LINUX)
#elif defined(Q_OS_MACOS)
#endif
    return ret;
}

QString util::processName(ulong processID)
{
#if defined(Q_OS_WIN)
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    qDebug() << processID << ":" << hProc;
    if(nullptr == hProc)
        return "";

    char exeName[(MAX_PATH*4)+1] = {0};
    //if(!GetProcessImageFileName(hProc, exeName, sizeof(exeName)/sizeof(*exeName)))
    //    return "";

    //QFileInfo fi(QString::fromWCharArray(exeName));
    QFileInfo filePath(QString(exeName));
    //qDebug() << filePath.path();
    //return filePath.path();
    return "";
#elif defined(Q_OS_LINUX)
    return "";
#elif defined(Q_OS_MACOS)
#else
    return "";
#endif
    return "";
}

QString util::currentOsName()
{
    QString ret = "unknown";
#if defined(Q_OS_WIN)
    ret = "windows";
#elif defined(Q_OS_LINUX)
    ret = "linux";
#elif defined(Q_OS_MACOS)
    ret = "maxos";
#endif
    return ret;
}

bool util::setCurrentPath(const QString& path)
{
    QFileInfo fi(path);
    const QString old = QDir::currentPath();
    const bool ret = QDir::setCurrent(fi.absolutePath());
    qDebug() << ret;
    currentPath_ = QDir::currentPath();
    qDebug() << currentPath_;
    QDir::setCurrent(old);
    return ret;
}

QString util::currentPath()
{
    if(currentPath_.isEmpty())
        return QDir::currentPath();
    return currentPath_;
}

void util::copyArgument(int argc, char *argv[])
{
    copyArgument_.clear();
    for(int i = 0; i < argc; i++)
    {
        copyArgument_ += "\"";
        copyArgument_ += argv[i];
        copyArgument_ += "\" ";
        qDebug() << argv[i];
    }
    qDebug() << copyArgument_;
}

QString util::copyArgument()
{
    return copyArgument_;
}

void util::logToFile(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
#if defined(Q_OS_WIN)
    QFile file("C:/log/log.txt");
    //#elif defined(Q_OS_LINUX)
    //#elif defined(Q_OS_MACOS)
#else
    QFile file("./log/log.txt");
#endif

    if(!file.open(QIODevice::Append | QIODevice::Text)) {
        return;
    }
    QTextStream out(&file);

    //QString curtime = QDateTime::currentDateTime().toString(" yyMMdd hh:mm:ss ");
    //context.file, context.line, context.function
    QString curtime = QDateTime::currentDateTime().toString(" yyMMdd hh:mm:ss %1(%2)%3: ").arg(context.file).arg(context.line).arg(context.function);

    out << currentProcessId() << curtime;

    switch (type) {
    case QtDebugMsg:
        out << "[Debug]\t" << msg << "\n";
        break;
    case QtInfoMsg:
        out << "[Info]\t" << msg << "\n";
        break;
    case QtWarningMsg:
        out << "[Warning]\t" << msg << "\n";
        break;
    case QtCriticalMsg:
        out << "[Critical]\t" << msg << context.function << context.line << "\n";
        break;
    case QtFatalMsg:
        out << "[Fatal]\t" << msg << "\n";
        abort();
    }
    file.close();
}

QString util::getStyleSheet()
{
    return 
//공용
"*"
"{"
//"	background-color: rgb(62,62,66);"
"	background-color: rgb(62,62,66);"
"	selection-background-color: rgb(82,90,250);"
"	font: 10pt \"Gulim\";"
"	color:rgb(255,255,255);"
"}"
//버튼"
"QPushButton"
"{"
"	background-color: rgb(82,90,250);"
"	border-radius: 5px;"
"	min-height: 24px;"
"   min-width: 160px;"
//"   margin-right: 13px;"
"}"
"QAbstractButton:hover"
"{"
"	background-color: rgb(112,120,255);"
"}"
"QAbstractButton:pressed"
"{"
"	background-color: rgb(50,60,220);"
"}"
"QPushButton:checked"
"{"
"	background-color: rgb(82,90,250);"
"	border: 3px solid rgb(82,90,250);"
"}"
"QToolButton:checked"
"{"
"	background-color: rgb(82,90,250);"
"	border: 3px solid rgb(82,90,250);"
"}"
"QPushButton:disabled,.QAbstractButton:disabled,QLineEdit:disabled,"
"QComboBox:disabled,QAbstractSpinBox:disabled"
"{"
"	background-color: rgb(80,80,90);"
"	color:rgb(120,120,135);"
"}"
"QToolBar > QToolButton"
"{"
"	background-color: rgb(62,62,66);"
"	selection-background-color: rgb(82,90,250);"
"	min-height: 33px;"
"   min-width: 90px;"
"}"
"QToolBar"
"{"
"   border: 1px solid rgb(50,50,60);"
"   border-top: 0px solid rgb(50,50,60);"
"}"
//버튼 예외
"QPushButton#showRequestButton,QPushButton#removeRequestButton,QPushButton#selectFileButton,"
"QPushButton#okButton,QPushButton#cancelButton,QPushButton#closeButton,QPushButton#btnClearTrafficGraph"
"QDialogButtonBox,QAbstractButton"
"{"
"    min-width : 80px;"
"}"
"QPushButton#addressBookButton_SM,QPushButton#pasteButton_SM,QPushButton#copySignatureButton_SM,"
"QPushButton#addressBookButton_VM,QPushButton#ellipsisButton,QPushButton#labelWalletStatus,"
"QPushButton#labelTransactionsStatus,QPushButton#addressBookButton,QPushButton#pasteButton,QPushButton#deleteButton,"
"QPushButton#warningIcon"
"{"
"    min-width : 30px;"
"}"
"QPushButton#fontSmallerButton,QPushButton#fontBiggerButton,QPushButton#clearButton"
"{"
"    min-width : 24px;"
"}"
"QPushButton#promptIcon"
"{"
"    min-width : 16px;"
"}"
"QPushButton#labelWalletStatus,QPushButton#labelTransactionsStatus,QPushButton#warningIcon"
"{"
"    background: transparent;"
"}"
"QPushButton#labelWalletStatus:pressed,QPushButton#labelTransactionsStatus:pressed"
"{"
"	background-color: rgb(50,60,220);"
"}"
////버튼 예외,간격 삭제
//"#exportButton,#addressBookButton_VM,#pasteButton_SM,#copySignatureButton_SM,"
//"{"
//"   margin-right: 0px;"
//"}"
//프레임
"QFrame"
"{"
"	border-radius: 5px;"
//"    padding: 10px;"
//"	border: 1px solid rgb(50,50,60);"
//"	background-color: rgb(40,40,50);"
"}"
//프레임 예외
"QFrame#frame"
"{"
"    padding-lef: 0px;"
"    padding-right: 0px;"
"}"
//라벨
"QLabel"
"{"
//"	border: 1px solid rgba(50,50,60,0);"
"    min-height: 20px;"
"    background: transparent;"
"}"
//라인
"Line"
"{"
"	border: 2px solid rgb(50,50,60);"
"}"
//테이블
"QTableView"
"{"
"	border-radius: 5px;"
"	background-color: rgb(40,40,50);"
"	selection-background-color: rgb(53,53,108);"
"	border: 1px solid rgb(50,50,60);"
"}"
"QHeaderView::section:first"
"{"
"	border-top-left-radius: 5px;"
"}"
"QHeaderView::section:last"
"{"
"	border-top-right-radius: 5px;"
"}"
"QHeaderView::section"
"{"
"	background-color: rgb(40,40,50);"
"	border: 1px solid rgb(62,62,66);"
"   min-height: 24px;"
"   text-align: center;"
"   qproperty-defaultAlignment: AlignHCenter;"
//"	border-top-left-radius: 5px;"
//"	border-top-right-radius: 5px;"
//"	font: 10pt \"나눔스퀘어\";"
"}"
"QAbstractItemView"
"{"
"	border-bottom-left-radius: 5px;"
"	border-bottom-right-radius: 5px;"
"}"
//체크박스
"QCheckBox"
"{"
"    background: transparent;"
"    min-height : 16px;"
"}"
"QCheckBox:disabled"
"{"
"	color: gray;"
"}"
//에디트
"QLineEdit,QTextEdit,#messageIn_SM,#messageIn_VM"
"{"
"	border-radius: 5px;"
"	background-color: rgb(242,242,242);"
"	border: 1px solid rgb(255,255,255);"
"	color:rgb(0,0,0);"
"   min-height: 18px;"
"	font: 9pt \"Gulim\";"
"}"
//에디트:스핀박스
"QAbstractSpinBox"
"{"
"	border-radius: 5px;"
"	background-color: rgb(242,242,242);"
"	border: 1px solid rgb(255,255,255);"
"	color:rgb(0,0,0);"
"   min-height: 18px;"
"}"
//그룹박스
"QGroupBox"
"{"
"    background: transparent;"
"    padding: 10px;"
"    padding-top: 22px;"
"    border: 1px solid rgb(50,50,60);"
"}"
//탭
"QTabWidget::pane {" /* The tab widget frame */
"    border: 0px solid rgb(50,50,60);"
"}"
"QTabWidget::tab-bar {"
"    left: 5px;" /* move to the right by 5px */
"}"
/* Style the tab using the tab sub-control. Note that it reads QTabBar _not_ QTabWidget */
"QTabBar::tab {"
"    background-color: rgb(62,62,66);"
"    border: 2px solid rgb(50,50,60);"
"    border-top-left-radius: 5px;"
"    border-top-right-radius: 5px;"
"    min-width: 8ex;"
"    padding: 3px;"
"    padding-left: 10px;"
"    padding-right: 10px;"
"	color: rgb(255,255,255);"
"}"
"QTabBar::tab:selected,QTabBar::tab:hover {"
//"    background-color: rgb(40,40,50);"
"	 background-color: rgb(82,90,250);"
"}"
"QTabBar::tab:selected {"
"    border-color: rgb(50,50,60);"
"}"
"QTabBar::tab:!selected {"
"    background-color: rgb(62,62,66);"
"    margin-top: 2px; /* make non-selected tabs look smaller */"
"}"
"#tabMain,#tabWallet,#tabNetwork,#tabWindow,#tabDisplay,"
"#tabSignMessage,#tabVerifyMessage {"
"	background-color: rgb(40,40,50);"
"    border: 0px solid rgb(50,50,60);"
"    border-radius: 5px;"
"}"
//툴팁
"QToolTip {"
"    border: 2px solid rgb(50,50,60);"
"    border-radius: 0px;"
"    padding: 5px;"
"	 background-color: rgb(40,40,50);"
"    color: rgb(255,255,255);"
"}"
//스크롤바
"QScrollBar:vertical {"
"    border-radius: 5px;"
"    background: rgb(40,40,50);"
"    border: 0px solid #999999;"
"    width:10px;"
"    margin: 0px 0px 0px 0px;"
"}"
"QScrollBar::sub-page:vertical {"
"    border-top-radius: 5px;"
"    background: rgb(40,40,50);"
"}"
"QScrollBar::add-page:vertical {"
"    border-bottom-radius: 5px;"
"    background: rgb(40,40,50);"
"}"
"QScrollBar::handle:vertical {"
"    border-radius: 5px;"
"    background: rgb(60,60,70);"
"    min-height: 0px;"
"}"
"QScrollBar::add-line:vertical {"
//"    border-radius: 5px;"
"    background: rgb(40,40,50);"
"    height: 0px;"
"    subcontrol-position: bottom;"
"    subcontrol-origin: margin;"
"}"
"QScrollBar::sub-line:vertical {"
//"    border-radius: 5px;"
"    background: rgb(40,40,50);"
"    height: 0 px;"
"    subcontrol-position: top;"
"    subcontrol-origin: margin;"
"}"
//콤보박스
"QComboBox"
"{"
"   border-radius: 5px;"
"	background-color: rgb(242,242,242);"
"	border: 1px solid rgb(255,255,255);"
"	color:rgb(0,0,0);"
"   min-height: 18px;"
"}"
"QComboBox QAbstractItemView"
"{"
"   border-radius: 5px;"
"	background-color: rgb(242,242,242);"
"   selection-color: blue;"
"   selection-background-color: gray;"
"	border: 1px solid rgb(255,255,255);"
"	color:rgb(0,0,0);"
"}"
"QComboBox::drop-down"
"{"
//"     subcontrol-origin: padding;"
//"     subcontrol-position: top right;"
//"     background: rgb(40,40,50);"
///"     image: url(:/icons/synced);"
///"     width: 20px;"
///"     color: rgb(255,255,255);"
///"     border-left-width: 0px;"
///"     border-left-color: rgb(50,50,60);"
///"     border-left-style: solid; /* just a single line */"
///"     border-top-right-radius: 5px; /* same radius as the QComboBox */"
///"     border-bottom-right-radius: 5px;"
//"     padding-left: 10px;"
"}"
//메뉴바
"QMenu"
"{"
"	background-color: rgb(40,40,50);"
"   selection-background-color: rgb(112,120,255);"
"	border: 1px solid rgb(50,50,60);"
"}"
"QMenu::item:disabled"
"{"
"	color: gray;"
"}"
"QMenuBar {"
"    background-color: rgb(62,62,66);"
"	border: 1px solid rgb(50,50,60);"
"}"
"QMenuBar::item {"
"    background-color: rgb(62,62,66);"
"}"
"QMenuBar::item:hover {"
"    background-color: rgb(112,120,255);"
"}"
"QMenuBar::item:selected {"
"    background-color: rgb(112,120,255);"
"}"
"QMenuBar::item:pressed {"
"    background-color: rgb(112,120,255);"
"}"
//프로그래스 바
"QProgressBar"//#progressBar"
"{"
"   border-radius: 5px;"
"   background: transparent;"
"   border: 1px solid rgb(50,50,60);"
//"   margin-top: 4px"
"   min-height: 18px;"
//"   height: 15px;"
"}"
"QProgressBar::chunk"
"{"
"    background-color: rgb(82,90,250);"
"    width: 20px;"
"}"
//영역
"QStackedWidget"
"{"
"    border: 0px solid rgb(40,40,50);"
"    border-radius: 0px;"
"    background-color: rgb(40,40,50);"
"}"
"#SendCoinsEntry, QFrame#SendCoins, QFrame#SendCoins_UnauthenticatedPaymentRequest, QFrame#SendCoins_AuthenticatedPaymentRequest, #contentWidget"
"{"
"    border-radius: 5px;"
"    border: 1px solid rgb(40,40,50);"
"    background-color: rgb(40,40,50);"
"}"
"#scrollAreaWidgetContents"
"{"
"	border-radius: 5px;"
//"    padding: 10px;"
//"	border: 1px solid rgb(50,50,60);"
//"	background-color: rgb(62,62,66);"
"}"
//
"#helpMessage,#aboutMessage,#scrollAreaWidgetContents"
"{"
"	border: 0px solid rgb(50,50,60);"
"	background-color: rgb(62,62,66);"
"}"

//"*::title"
//"{"
//"    background-color: rgb(62,62,66);"
//"    color: rgb(255,255,255);"
//"    height: 30px;"
////"    font-weight: bold;"
////"    color: #000000;"
////"    background: #ffffff;"
//"}"
;
}

void util::paintEvent(QPaintEvent *event,QWidget *qthis)
{
    //Q_UNUSED(event)
    //
    //QPainter p(qthis);
    //QStyle* style = qthis->style();
    //QRect active_area = qthis->rect();
    //int titlebar_height = 0;
    //
    //// Titlebar.
    //QStyleOptionTitleBar t_opt;
    //t_opt.initFrom(qthis);
    //
    //titlebar_height = style->pixelMetric(QStyle::PM_TitleBarHeight,&t_opt,qthis);
    //
    //const int iconSize = 16;
    //t_opt.icon = QPixmap(":/icons/wizblcoin");
    //t_opt.rect = QRect(0,0,qthis->width(),titlebar_height);
    //t_opt.titleBarState = qthis->windowState();
    //t_opt.text = t_opt.fontMetrics.elidedText(qthis->windowTitle(),Qt::ElideRight,t_opt.rect.width());
    //style->drawComplexControl(QStyle::CC_TitleBar,&t_opt,&p,qthis);
    //style->drawItemText(&p,QRect(t_opt.rect).adjusted(8+iconSize+5,0,0,0),Qt::AlignVCenter | Qt::AlignLeft,t_opt.palette,true,t_opt.text,QPalette::Text);
    //style->drawItemPixmap(&p,QRect(t_opt.rect).adjusted(8,0,0,0),Qt::AlignVCenter | Qt::AlignLeft,t_opt.icon.pixmap(iconSize,iconSize));
    //style->drawItemPixmap(&p,QRect(t_opt.rect).adjusted(0,0,-5,0),Qt ::AlignRight | Qt ::AlignVCenter,style->standardIcon(QStyle::SP_DialogCloseButton).pixmap(19,19)); 
    //
    //
    //// Background widget.
    //active_area.setTopLeft(QPoint(0,titlebar_height));
    //qthis->setContentsMargins(0,titlebar_height,0,0);
    //
    //QStyleOption w_opt;
    //w_opt.initFrom(qthis);
    //w_opt.rect = active_area;
    //style->drawPrimitive(QStyle::PE_Widget,&w_opt,&p,qthis);
}
