/* -------------------------------------------------------------------------- */
/*      REEMaker 5 -- Grégory WENTZEL (c) 2023   Code sous licence GPL3       */
/* -------------------------------------------------------------------------- */

#ifndef customHeader_H
#define customHeader_H

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>

/** customHeader:
    Class du header custom pour la liste des action (QListWidget)
*/
class customHeader : public QWidget {
    Q_OBJECT
   public:
    explicit customHeader(QWidget* parent = nullptr);

   signals:
};

#endif  // customHeader_H
