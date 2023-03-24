#ifndef PRESSEPAPIER_H
#define PRESSEPAPIER_H

#include <QDebug>
#include <QWidget>
#include "bloc.h"

/** PressePapier:
    Classe Presse papier
*/
class PressePapier : public QWidget {
    Q_OBJECT

   public:
    PressePapier();
    void AjoutPressePapier(Bloc::ItemDefinition);
    qint16 Taille();
    void Clear();
    QVector<Bloc::ItemDefinition> Liste();
    Bloc::ItemDefinition ItemAt(qint16);

   private:
    QVector<Bloc::ItemDefinition> vecPressePaPier;
};

#endif  // PRESSEPAPIER_H
