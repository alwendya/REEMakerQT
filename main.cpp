#include "mainwindow.h"
#include <QApplication>

///
/// \brief qMain    Point d'entrée de l'executable, prends en compte le lancement en thème lumineux ou sombre
/// \param argc
/// \param argv
/// \return
///
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QPoint pos = QCursor::pos();
    bool Found = false;

    bool EnNoir = false;
    if (QFileInfo::exists(QCoreApplication::applicationDirPath() + "/REEMAKER.ini"))
        if (QFileInfo(QCoreApplication::applicationDirPath() + "/REEMAKER.ini").size() > 0)
        {

            QSettings settings(QCoreApplication::applicationDirPath() + "/REEMAKER.ini", QSettings::IniFormat);
            EnNoir = settings.value("REGLAGES/ThemeSombre").toBool();
        }

    if (EnNoir)
    {
        qDebug() << "Mode sombre";
        QApplication::setStyle(new DarkStyle);
        FramelessWindow framelessWindow;
        MainWindow *mainWindow = new MainWindow;
        framelessWindow.setContent(mainWindow);
        framelessWindow.setWindowTitle(mainWindow->windowTitle());
        framelessWindow.setWindowIcon(QIcon(":/REEMaker16"));
        framelessWindow.setGeometry(mainWindow->geometry());
        for (int i = 0; i < QGuiApplication::screens().count();++i)
        {
            QScreen *screen = QGuiApplication::screens().at(i);
            QRect screenRect = screen->geometry();
            if (screenRect.contains(pos)) {
                Found = true;
                qDebug() << "Centrage fenêtre principale sur l'écran '" << screen->name() << "' index n°" << i;
                framelessWindow.setGeometry(
                            QStyle::alignedRect(
                                Qt::LeftToRight,
                                Qt::AlignCenter,
                                framelessWindow.size(),
                                QGuiApplication::screens().at(i)->geometry()
                                )
                            );
                break;
            }
        }
        if (!Found)
            framelessWindow.setGeometry(
                        QStyle::alignedRect(
                            Qt::LeftToRight,
                            Qt::AlignCenter,
                            framelessWindow.size(),
                            QGuiApplication::primaryScreen()->geometry()
                            )
                        );
        framelessWindow.show();
        {
            bool bVisible = IsWindowVisible((HWND)framelessWindow.winId());
            SetWindowPos((HWND)framelessWindow.winId(), HWND_TOP, 0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW |
                         (bVisible ? SWP_NOACTIVATE : 0));
            SetForegroundWindow((HWND)framelessWindow.winId());
        }
        return a.exec();
    }
    else
    {
        qDebug() << "Mode clair";
        a.setStyle("Fusion");
        MainWindow w;
        for (int i = 0; i < QGuiApplication::screens().count();++i)
        {
            QScreen *screen = QGuiApplication::screens().at(i);
            QRect screenRect = screen->geometry();
            if (screenRect.contains(pos)) {
                Found = true;
                qDebug() << "Centrage fenêtre principale sur l'écran '" << screen->name() << "' index n°" << i;
                w.setGeometry(
                            QStyle::alignedRect(
                                Qt::LeftToRight,
                                Qt::AlignCenter,
                                w.size(),
                                QGuiApplication::screens().at(i)->geometry()
                                )
                            );
                break;
            }
        }
        if (!Found)
            w.setGeometry(
                        QStyle::alignedRect(
                            Qt::LeftToRight,
                            Qt::AlignCenter,
                            w.size(),
                            QGuiApplication::primaryScreen()->geometry()
                            )
                        );
        qDebug() << "Chargé";
        w.setWindowState( (w.windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
        w.activateWindow(); // for Windows
        w.show();
        {
            bool bVisible = IsWindowVisible((HWND)w.winId());
                SetWindowPos((HWND)w.winId(), HWND_TOP, 0, 0, 0, 0,
                                SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW |
                                (bVisible ? SWP_NOACTIVATE : 0));
                    SetForegroundWindow((HWND)w.winId());
        }
        qDebug() << "Déchargé";
        return a.exec();
    }
    return a.exec();
}
