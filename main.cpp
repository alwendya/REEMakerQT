/* -------------------------------------------------------------------------- */
/*      REEMaker 5 -- Grégory WENTZEL (c) 2023   Code sous licence GPL3       */
/* -------------------------------------------------------------------------- */

#include "mainwindow.h"
#include <QApplication>

bool
CentrageHWND(HWND Fenetre)
{
    MainWindow::MonitorRects monitors;
    int NumEcran = 0;
    RECT FenetreAvecSouris;
    SetRect(&FenetreAvecSouris, 0, 0, 0, 0);
    foreach (auto var, monitors.rcMonitors) {
        POINT Souris;
        GetCursorPos(&Souris);
        RECT InterRect;
        RECT rSouris;
        SetRect(&rSouris, Souris.x, Souris.y, Souris.x + 10, Souris.y + 10);
        if (IntersectRect(&InterRect, &var, &rSouris) == TRUE) {
            qDebug().nospace().noquote()
              << QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss : ")
              << QString("Centrage : Souris dans l'écran n°%1 [%2]").arg(QString::number(NumEcran), monitors.qvName[NumEcran]);
            SetRect(&FenetreAvecSouris, var.left, var.top, var.right, var.bottom);
        }
        NumEcran++;
    }
    if ((FenetreAvecSouris.right - FenetreAvecSouris.left) > 0) { // On peut centrer
        RECT FenetreActuel;
        GetWindowRect(Fenetre, &FenetreActuel);
        MoveWindow(Fenetre,
                   /*Left*/ FenetreAvecSouris.left + ((FenetreAvecSouris.right - FenetreAvecSouris.left) /*WidthEcran par2*/ / 2) -
                     ((FenetreActuel.right - FenetreActuel.left) /*WidthFenetre*/ / 2),
                   /*top*/ FenetreAvecSouris.top + ((FenetreAvecSouris.bottom - FenetreAvecSouris.top) /*HeightEcran par2*/ / 2) -
                     ((FenetreActuel.bottom - FenetreActuel.top) /*HeightFenetre*/ / 2),
                   /*width*/ FenetreActuel.right - FenetreActuel.left,
                   /*height*/ FenetreActuel.bottom - FenetreActuel.top,
                   TRUE);
    }
    return true;
}

/** main:
    Point d'entrée de l'executable, prends en compte le lancement en thème
   lumineux ou sombre

    @param int argc
    @param char *argv[]
    @return int Code de retour
*/
int
main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    if (QFileInfo::exists(QCoreApplication::applicationDirPath() + "/REEMAKER.ini"))
        if (QFileInfo(QCoreApplication::applicationDirPath() + "/REEMAKER.ini").size() > 0) {
            QSettings settings(QCoreApplication::applicationDirPath() + "/REEMAKER.ini", QSettings::IniFormat);
        }
    a.setStyle("Fusion");
    MainWindow w;

    CentrageHWND((HWND)w.winId());
    w.setWindowState((w.windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    w.activateWindow(); // for Windows
    w.show();
    {
        bool bVisible = IsWindowVisible((HWND)w.winId());
        SetWindowPos((HWND)w.winId(), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | (bVisible ? SWP_NOACTIVATE : 0));
        SetForegroundWindow((HWND)w.winId());
    }
    return a.exec();
}

/* -- Si pas sous Windows -- */
//    QPoint pos = QCursor::pos();
//    bool Found = false;
//        for (int i = 0; i < QGuiApplication::screens().count(); ++i) {
//            QScreen* screen  = QGuiApplication::screens().at(i);
//            QRect screenRect = screen->geometry();
//            if (screenRect.contains(pos)) {
//                Found = true;
//                qDebug() << "Centrage fenêtre principale sur l'écran '" << screen->name() << "' index n°" << i;
//                w.setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, w.size(),
//                QGuiApplication::screens().at(i)->geometry())); break;
//            }
//        }
//        if (!Found)
//            w.setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, w.size(), QGuiApplication::primaryScreen()->geometry()));
