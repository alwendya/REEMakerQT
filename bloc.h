#ifndef BLOC_H
#define BLOC_H

#include <QObject>

class Bloc
{
public:
    Bloc();
    enum AlignementHorizontale : int {Gauche = 0, Milieu = 2, Droite = 4};
    enum AlignementVerticale : int {Haut = 1, Centre = 3, Bas = 5};
    enum TypeAction : int
    {
        DESSINELIGNE, DESSINERECTANGLEVIDE, DESSINERECTANGLEGRILLE, DESSINERECTANGLEREMPLIS, DESSINETEXTE, DESSINETEXTEMULTILIGNE, DESSINETEXTEQUESTION,
        DESSINETEXTEMULTILIGNEQUESTION, DESSINECHECKBOX, DESSINECHECKBOXQUESTION, DESSINEMULTICHECKBOXQUESTION, INSEREIMAGE, PAGESUIVANTE, COMMENTAIRE
    };
    struct ItemDefinition
    {
        QString NomControle;
        qint64 IndexControle;
        TypeAction TypeAction;
        double DebutX;
        double DebutY;
        double DebutX2;
        double DebutY2;
        double FinX;
        double FinY;
        double Largeur;
        double Hauteur;
        QString Couleur;
        double Epaisseur;
        qint16 NombreColonne;
        qint16 NombreLigne;
        QString CouleurRemplissage;
        AlignementHorizontale Alignement_Horizontale;
        AlignementVerticale Alignement_Verticale;
        double TaillePolice;
        qint16 ValeurSplit;
        bool Gras;
        bool Italique;
        bool GrasEtItalique;
        bool Monospace;
        QString Texte;
        QString TexteMultiligne;
        bool Obligatoire;
        bool Majuscule;
        bool Minuscule;
        bool Chiffre;
        bool CheckboxDefaut;
        qint16 LongueurMaximale;
        QString Question;
        QString QuestionDefaut;
        QString QuestionDefautMultiligne;
        QString QuestionAide;
        QString CheminImage;
        QString Commentaire;
        bool ThemeSombre;
    };
};

#endif // BLOC_H
