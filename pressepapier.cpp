#include "pressepapier.h"

///
/// \brief PressePapier::PressePapier   Initialisation du presse-papier
///
PressePapier::PressePapier()
{

}
///
/// \brief PressePapier::AjoutPressePapier  Ajout d'un item au presse-papier
/// \param Item
///
void PressePapier::AjoutPressePapier(Bloc::ItemDefinition Item)
{
    vecPressePaPier.append(Item);
}
///
/// \brief PressePapier::Taille Retourne le nombre d'élément contenu dans le presse-papier
/// \return
///
qint16 PressePapier::Taille()
{
    return vecPressePaPier.count();
}

///
/// \brief PressePapier::Clear  Efface tout les éléments du presse-papier
///
void PressePapier::Clear()
{
    vecPressePaPier.clear();
}

///
/// \brief PressePapier::ItemAt     Retourne un item défini par son index contenu dans le presse-papier
/// \param index
/// \return
///
Bloc::ItemDefinition PressePapier::ItemAt(qint16 index)
{
    if (index < 0 )
        return Bloc::ItemDefinition();//Empty bloc
    if (index > (vecPressePaPier.count()-1))
        return Bloc::ItemDefinition();//Empty bloc
    return vecPressePaPier[index];

}

///
/// \brief PressePapier::Liste      Retourne le contenu complet du presse-papier
/// \return
///
QVector<Bloc::ItemDefinition> PressePapier::Liste()
{
    return vecPressePaPier;
}
