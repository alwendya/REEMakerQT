/* -------------------------------------------------------------------------- */
/*      REEMaker 5 -- Grégory WENTZEL (c) 2023   Code sous licence GPL3       */
/* -------------------------------------------------------------------------- */

#include "pressepapier.h"

/** PressePapier:
    Initialisation du presse-papier

    @return aucun
*/
PressePapier::PressePapier() {}

/** AjoutPressePapier:
    Ajout d'un item au presse-papier

    @param Item
    @return aucun
*/
void PressePapier::AjoutPressePapier(Bloc::ItemDefinition Item) {
    vecPressePaPier.append(Item);
}

/** Taille:
    Retourne le nombre d'élément contenu dans le presse-papier

    @return qint16
*/
qint16 PressePapier::Taille() {
    return vecPressePaPier.count();
}

/** Clear:
    Efface tout les éléments du presse-papier

    @return aucun
*/
void PressePapier::Clear() {
    vecPressePaPier.clear();
}

/** ItemAt:
    Retourne un item défini par son index contenu dans le presse-papier

    @param Index
    @return aucun
*/
Bloc::ItemDefinition PressePapier::ItemAt(qint16 index) {
    if (index < 0)
        return Bloc::ItemDefinition();  // Empty bloc
    if (index > (vecPressePaPier.count() - 1))
        return Bloc::ItemDefinition();  // Empty bloc
    return vecPressePaPier[index];
}

/** Liste:
    Retourne le contenu complet du presse-papier

    @return QVector<Bloc::ItemDefinition>
*/
QVector<Bloc::ItemDefinition> PressePapier::Liste() {
    return vecPressePaPier;
}
