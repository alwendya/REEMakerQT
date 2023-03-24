/* -------------------------------------------------------------------------- */
/*      REEMaker 5 -- Grégory WENTZEL (c) 2023   Code sous licence GPL3       */
/* -------------------------------------------------------------------------- */

#include "customHeader.h"
/** customHeader:
    Créer un bloc servant de header à la liste

    @param QWidget *parent
    @return Aucun
*/
customHeader::customHeader(QWidget* parent) : QWidget{parent} {
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    QLabel* LabelObligatoire = new QLabel("Obl.");
    LabelObligatoire->setAlignment(Qt::AlignCenter);
    LabelObligatoire->setFixedWidth(32);
    layout->addWidget(LabelObligatoire);
    QLabel* LabelTexte = new QLabel("Question");
    LabelTexte->setFixedWidth(350);
    layout->addWidget(LabelTexte);
    QLabel* LabelAide = new QLabel("Aide");
    LabelAide->setAlignment(Qt::AlignCenter);
    LabelAide->setFixedWidth(32);
    layout->addWidget(LabelAide);
    QLabel* LabelSaisie = new QLabel("Champ à renseigner");
    LabelSaisie->setAlignment(Qt::AlignCenter);
    layout->addWidget(LabelSaisie);
}
