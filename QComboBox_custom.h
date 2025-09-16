/* -------------------------------------------------------------------------- */
/*      REEMaker 5 -- Grégory WENTZEL (c) 2023   Code sous licence GPL3       */
/* -------------------------------------------------------------------------- */

#ifndef customQComboBox_H
#define customQComboBox_H
#include <QComboBox>
#include <QWidget>
#define NOGDI // Exclut les définitions GDI incluant DrawText
#include <windows.h>

class QComboBox_custom : public QComboBox
{
  public:
    QComboBox_custom(QWidget* parent = nullptr) { this->setParent(parent); }
    virtual void showPopup()
    {
        QComboBox::showPopup();
        RECT FenetrePrincipale;
        GetWindowRect((HWND)this->winId(), &FenetrePrincipale);
        QWidget* popup = this->findChild<QFrame*>();
        MoveWindow((HWND)popup->winId(),
                   FenetrePrincipale.left,
                   FenetrePrincipale.top,
                   popup->width(),
                   popup->height(),
                   true);
        /* -- Le fonctionnement de base est le suivant : -- */
        //        popup->move(popup->x(), popup->y() /* + popup->height()*/);
    }
};
#endif // customQComboBox_H
