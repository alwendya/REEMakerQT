#ifndef PRESSEPAPIER_H
#define PRESSEPAPIER_H

#include <QWidget>
#include <QDebug>
#include "bloc.h"

class PressePapier : public QWidget
{
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

#endif // PRESSEPAPIER_H
