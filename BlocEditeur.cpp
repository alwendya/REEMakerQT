/* -------------------------------------------------------------------------- */
/*      REEMaker 5 -- Grégory WENTZEL (c) 2023   Code sous licence GPL3       */
/* -------------------------------------------------------------------------- */

#include "BlocEditeur.h"

/** BlocEditeur:
    Créer un bloc en prenant en entrée les paramètres suivants

    @param PressePapier* Pointeur vers le presse papier
    @param QWidget* QObject du parent du contrôle
    @param Bloc::ItemDefinition ItemDefinition pour peupler le contrôle
    @param bool Sombre oui ou non
    @return retour
*/
BlocEditeur::BlocEditeur(PressePapier* lPressePapier, QWidget* parent, Bloc::ItemDefinition mIDEF, bool Sombre)
  : QWidget{ parent }
{
    mPressePapier = lPressePapier;
    mItemDef      = new Bloc::ItemDefinition(mIDEF);
    /* -- Gestion des couleurs foncés -- */
    this->setObjectName(mItemDef->NomControle);
    QString qsStyleSheet             = "background-color: rgb(225, 198, 10);color: rgb(0, 0, 0);";
    QString qsStyleSheetPressePapier = "background-color: rgb(127, 255, 0);color: rgb(0, 0, 0);";
    QString qsStyleSheetSupprimer    = "background-color: rgb(222, 20, 10);color: rgb(0, 0, 0);";
    /* -- Gestion des statusTip des contrôles -- */
#define SetStatusAndTooltip(a, b)                                                                                                                    \
    {                                                                                                                                                \
        a->setStatusTip(b);                                                                                                                          \
        a->setToolTip(b);                                                                                                                            \
    };
    SetStatusAndTooltip(CtrlDebutX, "Point de départ sur l'axe des X");
    SetStatusAndTooltip(CtrlDebutY, "Point de départ sur l'axe des Y");
    SetStatusAndTooltip(CtrlDebutX2, "Point de départ n°2 sur l'axe des X (2nde case à cocher)");
    SetStatusAndTooltip(CtrlDebutY2, "Point de départ n°2 sur l'axe des Y (2nde case à cocher)");
    SetStatusAndTooltip(CtrlFinX, "Point d'arrivée sur l'axe des X");
    SetStatusAndTooltip(CtrlFinY, "Point d'arrivée sur l'axe des Y");
    SetStatusAndTooltip(CtrlLargeur, "Largeur du rectangle");
    SetStatusAndTooltip(CtrlHauteur, "Hauteur du rectangle");
    SetStatusAndTooltip(CtrlCouleur, "Couleur du trait de dessin de la forme en hexadecimal format HTML (ex. #FF00FF)");
    SetStatusAndTooltip(CtrlEpaisseur, "Valeur de l'epaisseur du trait (Si non renseigné, la valeur apr défaut est de 1.0 point)");
    SetStatusAndTooltip(CtrlNombreColonne, "Nombre de colonne pour la grille");
    SetStatusAndTooltip(CtrlNombreLigne, "Nombre de ligne pour la grille");
    SetStatusAndTooltip(CtrlCouleurRemplissage, "Couleur du remplissage de la forme en hexadecimal format HTML (ex. #FF00FF)");
    SetStatusAndTooltip(CtrlAlignementHorizontale, "Alignement du texte en largeur");
    SetStatusAndTooltip(CtrlAlignementVerticale, "Alignement du texte en hauteur");
    SetStatusAndTooltip(CtrlTaillePolice, "La taille de la police en point (Si non renseignée, la taille par défaut est de 10.0 point)");
    SetStatusAndTooltip(CtrlValeurSplit, "Coupe la largeur de la zone de texte selon cette valeur et place un caractère par emplacement");
    SetStatusAndTooltip(CtrlCheckboxDefaut, "Etat par défaut de la case à cocher");
    SetStatusAndTooltip(CtrlGras, "Afficher le texte en gras");
    SetStatusAndTooltip(CtrlItalique, "Afficher le texte en italique");
    SetStatusAndTooltip(CtrlGrasEtItalique, "Afficher le texte en gras et en italique");
    SetStatusAndTooltip(CtrlMonospace, "Afficher le texte en mode Monospace (tout les caractères font la même largeurs)");
    SetStatusAndTooltip(CtrlTexte, "Texte à faire apparaitre sur une ligne (Réponse par défaut en cas de question)");
    SetStatusAndTooltip(CtrlTexteMultiligne, "Texte à faire apparaitre sur plusieurs lignes");
    SetStatusAndTooltip(
      CtrlObligatoire,
      "Un avertissement apparait à côté de la question si coché, l'utilisateur ne pourra pas finaliser le foliotage sir non renseigné");
    SetStatusAndTooltip(CtrlMajuscule, "Le texte sera en majuscule uniquement");
    SetStatusAndTooltip(CtrlMinuscule, "Le texte sera en minuscule uniquement");
    SetStatusAndTooltip(CtrlChiffre, "Le texte contiendra des chiffres uniquement");
    SetStatusAndTooltip(CtrlLongueurMaximale,
                        "Valeur maximale admissible de la réponse en caractère (Si non renseignée, la valeur sera de 4000 caractère)");
    SetStatusAndTooltip(CtrlQuestion, "Question qui sera demandée via REEMaker");
    SetStatusAndTooltip(CtrlQuestionAide, "Aide contextuelle qui sera affichée dans REEMaker");
    SetStatusAndTooltip(
      CtrlCheminImage,
      "Chemin (entre les guillemets) de l'emplacement de l'image dans le dossier Img_resource (le séparateur de dossier est le caractère slash /)");
    SetStatusAndTooltip(CtrlCommentaire, "Commentaire qui sera écrit adns le document");
    SetStatusAndTooltip(CtrlUtiliseCouleurPrincipale,
                        "Si cochée, la couleur utilisée sera celle définie dans REEMaker dans 'Couleur du tampon tranche i'");
    SetStatusAndTooltip(CtrlUtiliseCouleurAccent,
                        "Si cochée, la couleur utilisée sera celle définie dans REEMaker dans 'Couleur accentuation tranche i'");
    SetStatusAndTooltip(CtrlUtiliseCouleurPrincipaleRemplissage,
                        "Si cochée, la couleur de remplissage utilisée sera celle définie dans REEMaker dans 'Couleur du tampon tranche i'");
    SetStatusAndTooltip(CtrlUtiliseCouleurAccentRemplissage,
                        "Si cochée, la couleur de remplissage utilisée sera celle définie dans REEMaker dans 'Couleur accentuation tranche i'");
    SetStatusAndTooltip(CtrlBoutonSupprimer, "Supprime le bloc actuel");
    SetStatusAndTooltip(CtrlBoutonDeplaceHaut, "Déplace ce bloc vers le haut");
    SetStatusAndTooltip(CtrlBoutonDeplacePremier, "Déplace ce bloc au début");
    SetStatusAndTooltip(CtrlBoutonDeplaceBas, "Déplace ce bloc vers le bas");
    SetStatusAndTooltip(CtrlBoutonDeplaceFin, "Déplace ce bloc tout en ba");
    SetStatusAndTooltip(CtrlBoutonPressePapier, "Déplace ce bloc à la fin");
    QFont mFont = CtrlTexte->font();
    mFont.setFamily("Roboto Mono");
    CtrlTexte->setFont(mFont);
    CtrlQuestion->setFont(mFont);
    CtrlQuestionAide->setFont(mFont);
    CtrlCommentaire->setFont(mFont);
    CtrlTexteMultiligne->setFont(mFont);
    /* -- Début population des blocs -- */
    qint16 Row              = 0;
    QGridLayout* MainLayout = new QGridLayout(this);
    MainLayout->setContentsMargins(2, 2, 2, 2);
    mSpacer = new QSpacerItem(100, 10, QSizePolicy::Expanding, QSizePolicy::Preferred);
    {
        // TITRE
        QLabel* LabelIcon = new QLabel();
        LabelIcon->setText("");
        LabelIcon->setMargin(0);
        if (Sombre)
            LabelIcon->setStyleSheet("background-color: rgba(255, 255, 255,40);");
        QLabel* LabelTitre = new QLabel(GetActionQString(mItemDef->TypeAction));
        LabelTitre->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        {
            auto mFont = LabelTitre->font();
            mFont.setBold(true);
            LabelTitre->setFont(mFont);
        }
        if (mItemDef->TypeAction == Bloc::TypeAction::DESSINELIGNE)
            LabelIcon->setPixmap(QPixmap(":/Ligne"));
        if (mItemDef->TypeAction == Bloc::TypeAction::DESSINERECTANGLEVIDE)
            LabelIcon->setPixmap(QPixmap(":/CarreVide"));
        if (mItemDef->TypeAction == Bloc::TypeAction::DESSINERECTANGLEGRILLE)
            LabelIcon->setPixmap(QPixmap(":/RectangleGrille"));
        if (mItemDef->TypeAction == Bloc::TypeAction::DESSINERECTANGLEREMPLIS)
            LabelIcon->setPixmap(QPixmap(":/CarreRemplis"));
        if (mItemDef->TypeAction == Bloc::TypeAction::DESSINETEXTE)
            LabelIcon->setPixmap(QPixmap(":/Texte"));
        if (mItemDef->TypeAction == Bloc::TypeAction::DESSINETEXTEMULTILIGNE)
            LabelIcon->setPixmap(QPixmap(":/Texte"));
        if (mItemDef->TypeAction == Bloc::TypeAction::DESSINETEXTEQUESTION)
            LabelIcon->setPixmap(QPixmap(":/SaisieTexte"));
        if (mItemDef->TypeAction == Bloc::TypeAction::DESSINETEXTEMULTILIGNEQUESTION)
            LabelIcon->setPixmap(QPixmap(":/SaisieTexte"));
        if (mItemDef->TypeAction == Bloc::TypeAction::DESSINECHECKBOX)
            LabelIcon->setPixmap(QPixmap(":/CheckBox"));
        if (mItemDef->TypeAction == Bloc::TypeAction::DESSINECHECKBOXQUESTION)
            LabelIcon->setPixmap(QPixmap(":/CheckBox"));
        if (mItemDef->TypeAction == Bloc::TypeAction::DESSINEMULTICHECKBOXQUESTION)
            LabelIcon->setPixmap(QPixmap(":/MultiCheckbox"));
        if (mItemDef->TypeAction == Bloc::TypeAction::INSEREIMAGE)
            LabelIcon->setPixmap(QPixmap(":/AjoutImage"));
        if (mItemDef->TypeAction == Bloc::TypeAction::PAGESUIVANTE)
            LabelIcon->setPixmap(QPixmap(":/Separateur"));
        if (mItemDef->TypeAction == Bloc::TypeAction::COMMENTAIRE)
            LabelIcon->setPixmap(QPixmap(":/Commentaire"));
        MainLayout->addWidget(LabelIcon, Row, 0);
        LabelIcon->setMinimumWidth(26);
        LabelIcon->setSizePolicy(QSizePolicy ::Minimum, QSizePolicy ::Preferred);
        LabelIcon->setAlignment(Qt::AlignCenter);
        MainLayout->addWidget(LabelTitre, Row, 1, 1, -1);

        MainLayout->addWidget(CtrlBoutonPressePapier, Row, 7);
        CtrlBoutonPressePapier->setText("Presse-Papier");
        CtrlBoutonPressePapier->setIcon(QIcon(":/Coller"));
        connect(CtrlBoutonPressePapier, SIGNAL(clicked()), this, SLOT(ClicBoutonPressePapier()));
        CtrlBoutonPressePapier->setStyleSheet(qsStyleSheetPressePapier);

        MainLayout->addWidget(CtrlBoutonSupprimer, Row, 8);
        CtrlBoutonSupprimer->setText("Supprimer");
        CtrlBoutonSupprimer->setIcon(QIcon(":/Supprime"));
        connect(CtrlBoutonSupprimer, SIGNAL(clicked()), this, SLOT(ClicBoutonSupprimer()));
        CtrlBoutonSupprimer->setStyleSheet(qsStyleSheetSupprimer);
        MainLayout->addItem(mSpacer, Row, 9);
    }
    CtrlAlignementHorizontale->setVisible(false);
    CtrlAlignementVerticale->setVisible(false);
    CtrlEpaisseur->setRange(0, 100);
    CtrlNombreColonne->setRange(0, 100);
    CtrlNombreLigne->setRange(0, 100);
    CtrlTaillePolice->setRange(0, 100);
    CtrlValeurSplit->setRange(0, 100);
    CtrlLongueurMaximale->setRange(0, 4096);
    CtrlDebutX->setRange(0, 9999);
    CtrlDebutY->setRange(0, 9999);
    CtrlDebutX2->setRange(0, 9999);
    CtrlDebutY2->setRange(0, 9999);
    CtrlFinX->setRange(0, 9999);
    CtrlFinY->setRange(0, 9999);
    CtrlLargeur->setRange(0, 9999);
    CtrlHauteur->setRange(0, 9999);
    if (mItemDef->TypeAction == Bloc::TypeAction::DESSINELIGNE) {
        Row++;
        MainLayout->addWidget(new QLabel("Debut X"), Row, 1);
        MainLayout->addWidget(CtrlDebutX, Row, 2);
        MainLayout->addWidget(new QLabel("Debut Y"), Row, 3);
        MainLayout->addWidget(CtrlDebutY, Row, 4);
        MainLayout->addWidget(new QLabel("Fin X"), Row, 5);
        MainLayout->addWidget(CtrlFinX, Row, 6);
        MainLayout->addWidget(new QLabel("Fin Y"), Row, 7);
        MainLayout->addWidget(CtrlFinY, Row, 8);
        CtrlDebutX->setValue(mItemDef->DebutX);
        CtrlDebutY->setValue(mItemDef->DebutY);
        CtrlFinX->setValue(mItemDef->FinX);
        CtrlFinY->setValue(mItemDef->FinY);
        Row++;
        MainLayout->addWidget(new QLabel("Couleur de trait\n[HTML #RRGGBB]"), Row, 1);
        CtrlCouleur->setText(mItemDef->Couleur);
        CtrlCouleur->setMaxLength(7);
        if (mItemDef->Couleur == "{ACC 1}") {
            CtrlCouleur->setEnabled(false);
            CtrlUtiliseCouleurPrincipale->setChecked(true);
            CtrlUtiliseCouleurAccent->setChecked(false);
        }
        if (mItemDef->Couleur == "{ACC 2}") {
            CtrlCouleur->setEnabled(false);
            CtrlUtiliseCouleurPrincipale->setChecked(false);
            CtrlUtiliseCouleurAccent->setChecked(true);
        }
        MainLayout->addWidget(CtrlCouleur, Row, 2);
        CtrlUtiliseCouleurPrincipale->setText("Utiliser la couleur d'accentuation 1");
        MainLayout->addWidget(CtrlUtiliseCouleurPrincipale, Row, 3, 1, 2);
        CtrlUtiliseCouleurPrincipale->setLayoutDirection(Qt::RightToLeft);
        CtrlUtiliseCouleurAccent->setText("Utiliser la couleur d'accentuation 2");
        MainLayout->addWidget(CtrlUtiliseCouleurAccent, Row, 5, 1, 2);
        CtrlUtiliseCouleurAccent->setLayoutDirection(Qt::RightToLeft);
        MainLayout->addWidget(new QLabel("Epaisseur\n0=Epaisseur par défaut (1)"), Row, 7);
        MainLayout->addWidget(CtrlEpaisseur, Row, 8);
        CtrlEpaisseur->setValue(mItemDef->Epaisseur);

        connect(CtrlUtiliseCouleurPrincipale, SIGNAL(toggled(bool)), this, SLOT(ClicCheckBoxPrincipale1(bool)));
        connect(CtrlUtiliseCouleurAccent, SIGNAL(toggled(bool)), this, SLOT(ClicCheckBoxAccent1(bool)));
    }
    if (mItemDef->TypeAction == Bloc::TypeAction::DESSINERECTANGLEVIDE) {
        Row++;
        MainLayout->addWidget(new QLabel("Debut X"), Row, 1);
        MainLayout->addWidget(CtrlDebutX, Row, 2);
        MainLayout->addWidget(new QLabel("Debut Y"), Row, 3);
        MainLayout->addWidget(CtrlDebutY, Row, 4);
        MainLayout->addWidget(new QLabel("Largeur"), Row, 5);
        MainLayout->addWidget(CtrlLargeur, Row, 6);
        MainLayout->addWidget(new QLabel("Hauteur"), Row, 7);
        MainLayout->addWidget(CtrlHauteur, Row, 8);
        CtrlDebutX->setValue(mItemDef->DebutX);
        CtrlDebutY->setValue(mItemDef->DebutY);
        CtrlLargeur->setValue(mItemDef->Largeur);
        CtrlHauteur->setValue(mItemDef->Hauteur);
        Row++;
        MainLayout->addWidget(new QLabel("Couleur de trait\n[HTML #RRGGBB]"), Row, 1);
        CtrlCouleur->setText(mItemDef->Couleur);
        CtrlCouleur->setMaxLength(7);
        if (mItemDef->Couleur == "{ACC 1}") {
            CtrlCouleur->setEnabled(false);
            CtrlUtiliseCouleurPrincipale->setChecked(true);
            CtrlUtiliseCouleurAccent->setChecked(false);
        }
        if (mItemDef->Couleur == "{ACC 2}") {
            CtrlCouleur->setEnabled(false);
            CtrlUtiliseCouleurPrincipale->setChecked(false);
            CtrlUtiliseCouleurAccent->setChecked(true);
        }
        MainLayout->addWidget(CtrlCouleur, Row, 2);
        CtrlUtiliseCouleurPrincipale->setText("Utiliser la couleur d'accentuation 1");
        MainLayout->addWidget(CtrlUtiliseCouleurPrincipale, Row, 3, 1, 2);
        CtrlUtiliseCouleurPrincipale->setLayoutDirection(Qt::RightToLeft);
        CtrlUtiliseCouleurAccent->setText("Utiliser la couleur d'accentuation 2");
        MainLayout->addWidget(CtrlUtiliseCouleurAccent, Row, 5, 1, 2);
        CtrlUtiliseCouleurAccent->setLayoutDirection(Qt::RightToLeft);
        MainLayout->addWidget(new QLabel("Epaisseur\n0=Epaisseur par défaut (1)"), Row, 7);
        MainLayout->addWidget(CtrlEpaisseur, Row, 8);
        CtrlEpaisseur->setValue(mItemDef->Epaisseur);

        connect(CtrlUtiliseCouleurPrincipale, SIGNAL(toggled(bool)), this, SLOT(ClicCheckBoxPrincipale1(bool)));
        connect(CtrlUtiliseCouleurAccent, SIGNAL(toggled(bool)), this, SLOT(ClicCheckBoxAccent1(bool)));
    }
    if (mItemDef->TypeAction == Bloc::TypeAction::DESSINERECTANGLEGRILLE) {
        Row++;
        MainLayout->addWidget(new QLabel("Debut X"), Row, 1);
        MainLayout->addWidget(CtrlDebutX, Row, 2);
        MainLayout->addWidget(new QLabel("Debut Y"), Row, 3);
        MainLayout->addWidget(CtrlDebutY, Row, 4);
        MainLayout->addWidget(new QLabel("Largeur"), Row, 5);
        MainLayout->addWidget(CtrlLargeur, Row, 6);
        MainLayout->addWidget(new QLabel("Hauteur"), Row, 7);
        MainLayout->addWidget(CtrlHauteur, Row, 8);
        CtrlDebutX->setValue(mItemDef->DebutX);
        CtrlDebutY->setValue(mItemDef->DebutY);
        CtrlLargeur->setValue(mItemDef->Largeur);
        CtrlHauteur->setValue(mItemDef->Hauteur);
        Row++;
        MainLayout->addWidget(new QLabel("Couleur de trait\n[HTML #RRGGBB]"), Row, 1);
        CtrlCouleur->setText(mItemDef->Couleur);
        CtrlCouleur->setMaxLength(7);
        if (mItemDef->Couleur == "{ACC 1}") {
            CtrlCouleur->setEnabled(false);
            CtrlUtiliseCouleurPrincipale->setChecked(true);
            CtrlUtiliseCouleurAccent->setChecked(false);
        }
        if (mItemDef->Couleur == "{ACC 2}") {
            CtrlCouleur->setEnabled(false);
            CtrlUtiliseCouleurPrincipale->setChecked(false);
            CtrlUtiliseCouleurAccent->setChecked(true);
        }
        MainLayout->addWidget(CtrlCouleur, Row, 2);
        CtrlUtiliseCouleurPrincipale->setText("Utiliser la couleur d'accentuation 1");
        MainLayout->addWidget(CtrlUtiliseCouleurPrincipale, Row, 3, 1, 2);
        CtrlUtiliseCouleurPrincipale->setLayoutDirection(Qt::RightToLeft);
        CtrlUtiliseCouleurAccent->setText("Utiliser la couleur d'accentuation 2");
        MainLayout->addWidget(CtrlUtiliseCouleurAccent, Row, 5, 1, 2);
        CtrlUtiliseCouleurAccent->setLayoutDirection(Qt::RightToLeft);
        MainLayout->addWidget(new QLabel("Epaisseur\n0=Epaisseur par défaut (1)"), Row, 7);
        MainLayout->addWidget(CtrlEpaisseur, Row, 8);
        CtrlEpaisseur->setValue(mItemDef->Epaisseur);
        Row++;
        MainLayout->addWidget(new QLabel("Nombre de colonne"), Row, 1);
        MainLayout->addWidget(CtrlNombreColonne, Row, 2);
        MainLayout->addWidget(new QLabel("Nombre de ligne"), Row, 3);
        MainLayout->addWidget(CtrlNombreLigne, Row, 4);
        CtrlNombreColonne->setValue(mItemDef->NombreColonne);
        CtrlNombreLigne->setValue(mItemDef->NombreLigne);

        connect(CtrlUtiliseCouleurPrincipale, SIGNAL(toggled(bool)), this, SLOT(ClicCheckBoxPrincipale1(bool)));
        connect(CtrlUtiliseCouleurAccent, SIGNAL(toggled(bool)), this, SLOT(ClicCheckBoxAccent1(bool)));
    }
    if (mItemDef->TypeAction == Bloc::TypeAction::DESSINERECTANGLEREMPLIS) {
        Row++;
        MainLayout->addWidget(new QLabel("Debut X"), Row, 1);
        MainLayout->addWidget(CtrlDebutX, Row, 2);
        MainLayout->addWidget(new QLabel("Debut Y"), Row, 3);
        MainLayout->addWidget(CtrlDebutY, Row, 4);
        MainLayout->addWidget(new QLabel("Largeur"), Row, 5);
        MainLayout->addWidget(CtrlLargeur, Row, 6);
        MainLayout->addWidget(new QLabel("Hauteur"), Row, 7);
        MainLayout->addWidget(CtrlHauteur, Row, 8);
        CtrlDebutX->setValue(mItemDef->DebutX);
        CtrlDebutY->setValue(mItemDef->DebutY);
        CtrlLargeur->setValue(mItemDef->Largeur);
        CtrlHauteur->setValue(mItemDef->Hauteur);
        Row++;
        MainLayout->addWidget(new QLabel("Couleur de trait\n[HTML #RRGGBB]"), Row, 1);
        CtrlCouleur->setText(mItemDef->Couleur);
        CtrlCouleur->setMaxLength(7);
        if (mItemDef->Couleur == "{ACC 1}") {
            CtrlCouleur->setEnabled(false);
            CtrlUtiliseCouleurPrincipale->setChecked(true);
            CtrlUtiliseCouleurAccent->setChecked(false);
        }
        if (mItemDef->Couleur == "{ACC 2}") {
            CtrlCouleur->setEnabled(false);
            CtrlUtiliseCouleurPrincipale->setChecked(false);
            CtrlUtiliseCouleurAccent->setChecked(true);
        }
        MainLayout->addWidget(CtrlCouleur, Row, 2);
        CtrlUtiliseCouleurPrincipale->setText("Utiliser la couleur d'accentuation 1");
        MainLayout->addWidget(CtrlUtiliseCouleurPrincipale, Row, 3, 1, 2);
        CtrlUtiliseCouleurPrincipale->setLayoutDirection(Qt::RightToLeft);
        CtrlUtiliseCouleurAccent->setText("Utiliser la couleur d'accentuation 2");
        MainLayout->addWidget(CtrlUtiliseCouleurAccent, Row, 5, 1, 2);
        CtrlUtiliseCouleurAccent->setLayoutDirection(Qt::RightToLeft);
        MainLayout->addWidget(new QLabel("Epaisseur\n0=Epaisseur par défaut (1)"), Row, 7);
        MainLayout->addWidget(CtrlEpaisseur, Row, 8);
        CtrlEpaisseur->setValue(mItemDef->Epaisseur);
        connect(CtrlUtiliseCouleurPrincipale, SIGNAL(toggled(bool)), this, SLOT(ClicCheckBoxPrincipale1(bool)));
        connect(CtrlUtiliseCouleurAccent, SIGNAL(toggled(bool)), this, SLOT(ClicCheckBoxAccent1(bool)));
        Row++;
        MainLayout->addWidget(new QLabel("Couleur de remplissage\n[#RRGGBB]"), Row, 1);
        CtrlCouleurRemplissage->setText(mItemDef->CouleurRemplissage);
        CtrlCouleurRemplissage->setMaxLength(7);
        if (mItemDef->CouleurRemplissage == "{ACC 1}") {
            CtrlCouleurRemplissage->setEnabled(false);
            CtrlUtiliseCouleurPrincipaleRemplissage->setChecked(true);
            CtrlUtiliseCouleurAccentRemplissage->setChecked(false);
        }
        if (mItemDef->CouleurRemplissage == "{ACC 2}") {
            CtrlCouleurRemplissage->setEnabled(false);
            CtrlUtiliseCouleurPrincipaleRemplissage->setChecked(false);
            CtrlUtiliseCouleurAccentRemplissage->setChecked(true);
        }
        MainLayout->addWidget(CtrlCouleurRemplissage, Row, 2);
        CtrlUtiliseCouleurPrincipaleRemplissage->setText("Utiliser la couleur d'accentuation 1");
        MainLayout->addWidget(CtrlUtiliseCouleurPrincipaleRemplissage, Row, 3, 1, 2);
        CtrlUtiliseCouleurPrincipaleRemplissage->setLayoutDirection(Qt::RightToLeft);
        CtrlUtiliseCouleurAccentRemplissage->setText("Utiliser la couleur d'accentuation 2");
        MainLayout->addWidget(CtrlUtiliseCouleurAccentRemplissage, Row, 5, 1, 2);
        CtrlUtiliseCouleurAccentRemplissage->setLayoutDirection(Qt::RightToLeft);
        connect(CtrlUtiliseCouleurPrincipaleRemplissage, SIGNAL(toggled(bool)), this, SLOT(ClicCheckBoxPrincipaleRemplissage(bool)));
        connect(CtrlUtiliseCouleurAccentRemplissage, SIGNAL(toggled(bool)), this, SLOT(ClicCheckBoxAccentRemplissage(bool)));
    }
    if (mItemDef->TypeAction == Bloc::TypeAction::DESSINETEXTE) {
        CtrlAlignementHorizontale->setVisible(true);
        CtrlAlignementVerticale->setVisible(true);

        Row++;
        MainLayout->addWidget(new QLabel("Debut X"), Row, 1);
        MainLayout->addWidget(CtrlDebutX, Row, 2);
        MainLayout->addWidget(new QLabel("Debut Y"), Row, 3);
        MainLayout->addWidget(CtrlDebutY, Row, 4);
        MainLayout->addWidget(new QLabel("Largeur"), Row, 5);
        MainLayout->addWidget(CtrlLargeur, Row, 6);
        MainLayout->addWidget(new QLabel("Hauteur"), Row, 7);
        MainLayout->addWidget(CtrlHauteur, Row, 8);
        CtrlDebutX->setValue(mItemDef->DebutX);
        CtrlDebutY->setValue(mItemDef->DebutY);
        CtrlLargeur->setValue(mItemDef->Largeur);
        CtrlHauteur->setValue(mItemDef->Hauteur);

        Row++;
        MainLayout->addWidget(new QLabel("Couleur du texte\n[HTML #RRGGBB]"), Row, 1);
        CtrlCouleur->setText(mItemDef->Couleur);
        CtrlCouleur->setMaxLength(7);
        if (mItemDef->Couleur == "{ACC 1}") {
            CtrlCouleur->setEnabled(false);
            CtrlUtiliseCouleurPrincipale->setChecked(true);
            CtrlUtiliseCouleurAccent->setChecked(false);
        }
        if (mItemDef->Couleur == "{ACC 2}") {
            CtrlCouleur->setEnabled(false);
            CtrlUtiliseCouleurPrincipale->setChecked(false);
            CtrlUtiliseCouleurAccent->setChecked(true);
        }
        MainLayout->addWidget(CtrlCouleur, Row, 2);

        CtrlUtiliseCouleurPrincipale->setText("Utiliser la couleur d'accentuation 1");
        MainLayout->addWidget(CtrlUtiliseCouleurPrincipale, Row, 3, 1, 2);
        CtrlUtiliseCouleurPrincipale->setLayoutDirection(Qt::RightToLeft);
        CtrlUtiliseCouleurAccent->setText("Utiliser la couleur d'accentuation 2");
        MainLayout->addWidget(CtrlUtiliseCouleurAccent, Row, 5, 1, 2);

        CtrlUtiliseCouleurAccent->setLayoutDirection(Qt::RightToLeft);
        connect(CtrlUtiliseCouleurPrincipale, SIGNAL(toggled(bool)), this, SLOT(ClicCheckBoxPrincipale1(bool)));
        connect(CtrlUtiliseCouleurAccent, SIGNAL(toggled(bool)), this, SLOT(ClicCheckBoxAccent1(bool)));

        Row++;
        MainLayout->addWidget(new QLabel("Alignement largeur"), Row, 1);
        MainLayout->addWidget(CtrlAlignementHorizontale, Row, 2);
        CtrlAlignementHorizontale->addItems({ "Gauche", "Milieu", "Droite" });
        if (mItemDef->Alignement_Horizontale == Bloc::AlignementHorizontale::Gauche)
            CtrlAlignementHorizontale->setCurrentIndex(0);
        if (mItemDef->Alignement_Horizontale == Bloc::AlignementHorizontale::Milieu)
            CtrlAlignementHorizontale->setCurrentIndex(1);
        if (mItemDef->Alignement_Horizontale == Bloc::AlignementHorizontale::Droite)
            CtrlAlignementHorizontale->setCurrentIndex(2);
        MainLayout->addWidget(new QLabel("Alignement hauteur"), Row, 3);
        MainLayout->addWidget(CtrlAlignementVerticale, Row, 4);
        CtrlAlignementVerticale->addItems({ "Haut", "Centre", "Bas" });
        if (mItemDef->Alignement_Verticale == Bloc::AlignementVerticale::Haut)
            CtrlAlignementVerticale->setCurrentIndex(0);
        if (mItemDef->Alignement_Verticale == Bloc::AlignementVerticale::Centre)
            CtrlAlignementVerticale->setCurrentIndex(1);
        if (mItemDef->Alignement_Verticale == Bloc::AlignementVerticale::Bas)
            CtrlAlignementVerticale->setCurrentIndex(2);

        MainLayout->addWidget(new QLabel("Taille police\n0 = Taille par défaut"), Row, 5);
        MainLayout->addWidget(CtrlTaillePolice, Row, 6);
        CtrlTaillePolice->setValue(mItemDef->TaillePolice);

        MainLayout->addWidget(new QLabel("Splitting\n0 = pas de splitting"), Row, 7);
        MainLayout->addWidget(CtrlValeurSplit, Row, 8);
        CtrlValeurSplit->setValue(mItemDef->ValeurSplit);

        Row++;
        CtrlGras->setLayoutDirection(Qt::RightToLeft);
        CtrlGras->setText("Gras");
        MainLayout->addWidget(CtrlGras, Row, 1, 1, 2);

        CtrlItalique->setLayoutDirection(Qt::RightToLeft);
        CtrlItalique->setText("Italique");
        MainLayout->addWidget(CtrlItalique, Row, 3, 1, 2);

        CtrlGrasEtItalique->setLayoutDirection(Qt::RightToLeft);
        CtrlGrasEtItalique->setText("Gras et italique");
        MainLayout->addWidget(CtrlGrasEtItalique, Row, 5, 1, 2);

        CtrlMonospace->setLayoutDirection(Qt::RightToLeft);
        CtrlMonospace->setText("Monospace");
        MainLayout->addWidget(CtrlMonospace, Row, 7, 1, 2);
        CtrlGras->setChecked(mItemDef->Gras);
        CtrlItalique->setChecked(mItemDef->Italique);
        CtrlGrasEtItalique->setChecked(mItemDef->GrasEtItalique);
        CtrlMonospace->setChecked(mItemDef->Monospace);

        Row++;
        MainLayout->addWidget(new QLabel("Texte"), Row, 1);
        MainLayout->addWidget(CtrlTexte, Row, 2, 1, 7);
        CtrlTexte->setText(mItemDef->Texte);
    }
    if (mItemDef->TypeAction == Bloc::TypeAction::DESSINETEXTEMULTILIGNE) {
        CtrlAlignementHorizontale->setVisible(true);
        CtrlAlignementVerticale->setVisible(true);
        Row++;
        MainLayout->addWidget(new QLabel("Debut X"), Row, 1);
        MainLayout->addWidget(CtrlDebutX, Row, 2);
        MainLayout->addWidget(new QLabel("Debut Y"), Row, 3);
        MainLayout->addWidget(CtrlDebutY, Row, 4);
        MainLayout->addWidget(new QLabel("Largeur"), Row, 5);
        MainLayout->addWidget(CtrlLargeur, Row, 6);
        MainLayout->addWidget(new QLabel("Hauteur"), Row, 7);
        MainLayout->addWidget(CtrlHauteur, Row, 8);
        CtrlDebutX->setValue(mItemDef->DebutX);
        CtrlDebutY->setValue(mItemDef->DebutY);
        CtrlLargeur->setValue(mItemDef->Largeur);
        CtrlHauteur->setValue(mItemDef->Hauteur);

        Row++;
        MainLayout->addWidget(new QLabel("Couleur de trait\n[HTML #RRGGBB]"), Row, 1);
        CtrlCouleur->setText(mItemDef->Couleur);
        CtrlCouleur->setMaxLength(7);
        if (mItemDef->Couleur == "{ACC 1}") {
            CtrlCouleur->setEnabled(false);
            CtrlUtiliseCouleurPrincipale->setChecked(true);
            CtrlUtiliseCouleurAccent->setChecked(false);
        }
        if (mItemDef->Couleur == "{ACC 2}") {
            CtrlCouleur->setEnabled(false);
            CtrlUtiliseCouleurPrincipale->setChecked(false);
            CtrlUtiliseCouleurAccent->setChecked(true);
        }
        MainLayout->addWidget(CtrlCouleur, Row, 2);

        CtrlUtiliseCouleurPrincipale->setText("Utiliser la couleur d'accentuation 1");
        MainLayout->addWidget(CtrlUtiliseCouleurPrincipale, Row, 3, 1, 2);
        CtrlUtiliseCouleurPrincipale->setLayoutDirection(Qt::RightToLeft);
        CtrlUtiliseCouleurAccent->setText("Utiliser la couleur d'accentuation 2");
        MainLayout->addWidget(CtrlUtiliseCouleurAccent, Row, 5, 1, 2);

        CtrlUtiliseCouleurAccent->setLayoutDirection(Qt::RightToLeft);
        connect(CtrlUtiliseCouleurPrincipale, SIGNAL(toggled(bool)), this, SLOT(ClicCheckBoxPrincipale1(bool)));
        connect(CtrlUtiliseCouleurAccent, SIGNAL(toggled(bool)), this, SLOT(ClicCheckBoxAccent1(bool)));

        Row++;
        MainLayout->addWidget(new QLabel("Alignement largeur"), Row, 1);
        MainLayout->addWidget(CtrlAlignementHorizontale, Row, 2);
        CtrlAlignementHorizontale->addItems({ "Gauche", "Milieu", "Droite" });
        if (mItemDef->Alignement_Horizontale == Bloc::AlignementHorizontale::Gauche)
            CtrlAlignementHorizontale->setCurrentIndex(0);
        if (mItemDef->Alignement_Horizontale == Bloc::AlignementHorizontale::Milieu)
            CtrlAlignementHorizontale->setCurrentIndex(1);
        if (mItemDef->Alignement_Horizontale == Bloc::AlignementHorizontale::Droite)
            CtrlAlignementHorizontale->setCurrentIndex(2);

        MainLayout->addWidget(new QLabel("Alignement hauteur"), Row, 3);
        MainLayout->addWidget(CtrlAlignementVerticale, Row, 4);
        CtrlAlignementVerticale->addItems({ "Haut", "Centre", "Bas" });
        if (mItemDef->Alignement_Verticale == Bloc::AlignementVerticale::Haut)
            CtrlAlignementVerticale->setCurrentIndex(0);
        if (mItemDef->Alignement_Verticale == Bloc::AlignementVerticale::Centre)
            CtrlAlignementVerticale->setCurrentIndex(1);
        if (mItemDef->Alignement_Verticale == Bloc::AlignementVerticale::Bas)
            CtrlAlignementVerticale->setCurrentIndex(2);

        MainLayout->addWidget(new QLabel("Taille police\n0 = Taille par défaut"), Row, 5);
        MainLayout->addWidget(CtrlTaillePolice, Row, 6);
        CtrlTaillePolice->setValue(mItemDef->TaillePolice);

        Row++;
        CtrlGras->setLayoutDirection(Qt::RightToLeft);
        CtrlGras->setText("Gras");
        MainLayout->addWidget(CtrlGras, Row, 1, 1, 2);

        CtrlItalique->setLayoutDirection(Qt::RightToLeft);
        CtrlItalique->setText("Italique");
        MainLayout->addWidget(CtrlItalique, Row, 3, 1, 2);

        CtrlGrasEtItalique->setLayoutDirection(Qt::RightToLeft);
        CtrlGrasEtItalique->setText("Gras et italique");
        MainLayout->addWidget(CtrlGrasEtItalique, Row, 5, 1, 2);

        CtrlMonospace->setLayoutDirection(Qt::RightToLeft);
        CtrlMonospace->setText("Monospace");
        MainLayout->addWidget(CtrlMonospace, Row, 7, 1, 2);

        CtrlGras->setChecked(mItemDef->Gras);
        CtrlItalique->setChecked(mItemDef->Italique);
        CtrlGrasEtItalique->setChecked(mItemDef->GrasEtItalique);
        CtrlMonospace->setChecked(mItemDef->Monospace);

        Row++;
        MainLayout->addWidget(new QLabel("Texte"), Row, 1);
        MainLayout->addWidget(CtrlTexteMultiligne, Row, 2, 1, 7);
        CtrlTexteMultiligne->setText(mItemDef->TexteMultiligne);
    }
    if (mItemDef->TypeAction == Bloc::TypeAction::DESSINETEXTEQUESTION) {
        CtrlAlignementHorizontale->setVisible(true);
        CtrlAlignementVerticale->setVisible(true);
        Row++;
        MainLayout->addWidget(new QLabel("Debut X"), Row, 1);
        MainLayout->addWidget(CtrlDebutX, Row, 2);
        MainLayout->addWidget(new QLabel("Debut Y"), Row, 3);
        MainLayout->addWidget(CtrlDebutY, Row, 4);
        MainLayout->addWidget(new QLabel("Largeur"), Row, 5);
        MainLayout->addWidget(CtrlLargeur, Row, 6);
        MainLayout->addWidget(new QLabel("Hauteur"), Row, 7);
        MainLayout->addWidget(CtrlHauteur, Row, 8);
        CtrlDebutX->setValue(mItemDef->DebutX);
        CtrlDebutY->setValue(mItemDef->DebutY);
        CtrlLargeur->setValue(mItemDef->Largeur);
        CtrlHauteur->setValue(mItemDef->Hauteur);

        Row++;
        MainLayout->addWidget(new QLabel("Couleur de trait\n[HTML #RRGGBB]"), Row, 1);
        CtrlCouleur->setText(mItemDef->Couleur);
        CtrlCouleur->setMaxLength(7);
        if (mItemDef->Couleur == "{ACC 1}") {
            CtrlCouleur->setEnabled(false);
            CtrlUtiliseCouleurPrincipale->setChecked(true);
            CtrlUtiliseCouleurAccent->setChecked(false);
        }
        if (mItemDef->Couleur == "{ACC 2}") {
            CtrlCouleur->setEnabled(false);
            CtrlUtiliseCouleurPrincipale->setChecked(false);
            CtrlUtiliseCouleurAccent->setChecked(true);
        }
        MainLayout->addWidget(CtrlCouleur, Row, 2);

        CtrlUtiliseCouleurPrincipale->setText("Utiliser la couleur d'accentuation 1");
        MainLayout->addWidget(CtrlUtiliseCouleurPrincipale, Row, 3, 1, 2);
        CtrlUtiliseCouleurPrincipale->setLayoutDirection(Qt::RightToLeft);
        CtrlUtiliseCouleurAccent->setText("Utiliser la couleur d'accentuation 2");
        MainLayout->addWidget(CtrlUtiliseCouleurAccent, Row, 5, 1, 2);

        CtrlUtiliseCouleurAccent->setLayoutDirection(Qt::RightToLeft);
        connect(CtrlUtiliseCouleurPrincipale, SIGNAL(toggled(bool)), this, SLOT(ClicCheckBoxPrincipale1(bool)));
        connect(CtrlUtiliseCouleurAccent, SIGNAL(toggled(bool)), this, SLOT(ClicCheckBoxAccent1(bool)));

        Row++;
        MainLayout->addWidget(new QLabel("Alignement largeur"), Row, 1);
        MainLayout->addWidget(CtrlAlignementHorizontale, Row, 2);
        CtrlAlignementHorizontale->addItems({ "Gauche", "Milieu", "Droite" });
        if (mItemDef->Alignement_Horizontale == Bloc::AlignementHorizontale::Gauche)
            CtrlAlignementHorizontale->setCurrentIndex(0);
        if (mItemDef->Alignement_Horizontale == Bloc::AlignementHorizontale::Milieu)
            CtrlAlignementHorizontale->setCurrentIndex(1);
        if (mItemDef->Alignement_Horizontale == Bloc::AlignementHorizontale::Droite)
            CtrlAlignementHorizontale->setCurrentIndex(2);

        MainLayout->addWidget(new QLabel("Alignement hauteur"), Row, 3);
        MainLayout->addWidget(CtrlAlignementVerticale, Row, 4);
        CtrlAlignementVerticale->addItems({ "Haut", "Centre", "Bas" });
        if (mItemDef->Alignement_Verticale == Bloc::AlignementVerticale::Haut)
            CtrlAlignementVerticale->setCurrentIndex(0);
        if (mItemDef->Alignement_Verticale == Bloc::AlignementVerticale::Centre)
            CtrlAlignementVerticale->setCurrentIndex(1);
        if (mItemDef->Alignement_Verticale == Bloc::AlignementVerticale::Bas)
            CtrlAlignementVerticale->setCurrentIndex(2);

        MainLayout->addWidget(new QLabel("Taille police\n0 = Taille par défaut"), Row, 5);
        MainLayout->addWidget(CtrlTaillePolice, Row, 6);
        CtrlTaillePolice->setValue(mItemDef->TaillePolice);

        MainLayout->addWidget(new QLabel("Splitting\n0 = pas de splitting"), Row, 7);
        MainLayout->addWidget(CtrlValeurSplit, Row, 8);

        Row++;
        CtrlGras->setLayoutDirection(Qt::RightToLeft);
        CtrlGras->setText("Gras");
        MainLayout->addWidget(CtrlGras, Row, 1, 1, 2);

        CtrlItalique->setLayoutDirection(Qt::RightToLeft);
        CtrlItalique->setText("Italique");
        MainLayout->addWidget(CtrlItalique, Row, 3, 1, 2);

        CtrlGrasEtItalique->setLayoutDirection(Qt::RightToLeft);
        CtrlGrasEtItalique->setText("Gras et italique");
        MainLayout->addWidget(CtrlGrasEtItalique, Row, 5, 1, 2);

        CtrlMonospace->setLayoutDirection(Qt::RightToLeft);
        CtrlMonospace->setText("Monospace");
        MainLayout->addWidget(CtrlMonospace, Row, 7, 1, 2);

        CtrlGras->setChecked(mItemDef->Gras);
        CtrlItalique->setChecked(mItemDef->Italique);
        CtrlGrasEtItalique->setChecked(mItemDef->GrasEtItalique);
        CtrlMonospace->setChecked(mItemDef->Monospace);

        Row++;
        CtrlObligatoire->setLayoutDirection(Qt::RightToLeft);
        CtrlObligatoire->setText("Obligatoire");
        MainLayout->addWidget(CtrlObligatoire, Row, 1, 1, 2);

        CtrlMinuscule->setLayoutDirection(Qt::RightToLeft);
        CtrlMinuscule->setText("Minuscule");
        MainLayout->addWidget(CtrlMinuscule, Row, 3, 1, 2);

        CtrlMajuscule->setLayoutDirection(Qt::RightToLeft);
        CtrlMajuscule->setText("Majuscule");
        MainLayout->addWidget(CtrlMajuscule, Row, 5, 1, 2);

        CtrlChiffre->setLayoutDirection(Qt::RightToLeft);
        CtrlChiffre->setText("Chiffre");
        MainLayout->addWidget(CtrlChiffre, Row, 7, 1, 2);

        CtrlObligatoire->setChecked(mItemDef->Obligatoire);
        CtrlMinuscule->setChecked(mItemDef->Minuscule);
        CtrlMajuscule->setChecked(mItemDef->Majuscule);
        CtrlChiffre->setChecked(mItemDef->Chiffre);

        Row++;
        MainLayout->addWidget(new QLabel("Longueur maximale\n0=illimité"), Row, 1);
        MainLayout->addWidget(CtrlLongueurMaximale, Row, 2);
        CtrlLongueurMaximale->setValue(mItemDef->LongueurMaximale);

        Row++;
        MainLayout->addWidget(new QLabel("Question"), Row, 1);
        MainLayout->addWidget(CtrlQuestion, Row, 2, 1, 7);

        Row++;
        MainLayout->addWidget(new QLabel("Question par défaut"), Row, 1);
        MainLayout->addWidget(CtrlTexte, Row, 2, 1, 7);

        Row++;
        MainLayout->addWidget(new QLabel("Aide"), Row, 1);
        MainLayout->addWidget(CtrlQuestionAide, Row, 2, 1, 7);

        CtrlQuestion->setText(mItemDef->Question);
        CtrlTexte->setText(mItemDef->QuestionDefaut);
        CtrlQuestionAide->setText(mItemDef->QuestionAide);
    }
    if (mItemDef->TypeAction == Bloc::TypeAction::DESSINETEXTEMULTILIGNEQUESTION) {
        CtrlAlignementHorizontale->setVisible(true);
        CtrlAlignementVerticale->setVisible(true);
        Row++;
        MainLayout->addWidget(new QLabel("Debut X"), Row, 1);
        MainLayout->addWidget(CtrlDebutX, Row, 2);
        MainLayout->addWidget(new QLabel("Debut Y"), Row, 3);
        MainLayout->addWidget(CtrlDebutY, Row, 4);
        MainLayout->addWidget(new QLabel("Largeur"), Row, 5);
        MainLayout->addWidget(CtrlLargeur, Row, 6);
        MainLayout->addWidget(new QLabel("Hauteur"), Row, 7);
        MainLayout->addWidget(CtrlHauteur, Row, 8);
        CtrlDebutX->setValue(mItemDef->DebutX);
        CtrlDebutY->setValue(mItemDef->DebutY);
        CtrlLargeur->setValue(mItemDef->Largeur);
        CtrlHauteur->setValue(mItemDef->Hauteur);

        Row++;
        MainLayout->addWidget(new QLabel("Couleur de trait\n[HTML #RRGGBB]"), Row, 1);
        CtrlCouleur->setText(mItemDef->Couleur);
        CtrlCouleur->setMaxLength(7);
        if (mItemDef->Couleur == "{ACC 1}") {
            CtrlCouleur->setEnabled(false);
            CtrlUtiliseCouleurPrincipale->setChecked(true);
            CtrlUtiliseCouleurAccent->setChecked(false);
        }
        if (mItemDef->Couleur == "{ACC 2}") {
            CtrlCouleur->setEnabled(false);
            CtrlUtiliseCouleurPrincipale->setChecked(false);
            CtrlUtiliseCouleurAccent->setChecked(true);
        }
        MainLayout->addWidget(CtrlCouleur, Row, 2);

        CtrlUtiliseCouleurPrincipale->setText("Utiliser la couleur d'accentuation 1");
        MainLayout->addWidget(CtrlUtiliseCouleurPrincipale, Row, 3, 1, 2);
        CtrlUtiliseCouleurPrincipale->setLayoutDirection(Qt::RightToLeft);
        CtrlUtiliseCouleurAccent->setText("Utiliser la couleur d'accentuation 2");
        MainLayout->addWidget(CtrlUtiliseCouleurAccent, Row, 5, 1, 2);

        CtrlUtiliseCouleurAccent->setLayoutDirection(Qt::RightToLeft);
        connect(CtrlUtiliseCouleurPrincipale, SIGNAL(toggled(bool)), this, SLOT(ClicCheckBoxPrincipale1(bool)));
        connect(CtrlUtiliseCouleurAccent, SIGNAL(toggled(bool)), this, SLOT(ClicCheckBoxAccent1(bool)));

        Row++;
        MainLayout->addWidget(new QLabel("Alignement largeur"), Row, 1);
        MainLayout->addWidget(CtrlAlignementHorizontale, Row, 2);
        CtrlAlignementHorizontale->addItems({ "Gauche", "Milieu", "Droite" });
        if (mItemDef->Alignement_Horizontale == Bloc::AlignementHorizontale::Gauche)
            CtrlAlignementHorizontale->setCurrentIndex(0);
        if (mItemDef->Alignement_Horizontale == Bloc::AlignementHorizontale::Milieu)
            CtrlAlignementHorizontale->setCurrentIndex(1);
        if (mItemDef->Alignement_Horizontale == Bloc::AlignementHorizontale::Droite)
            CtrlAlignementHorizontale->setCurrentIndex(2);

        MainLayout->addWidget(new QLabel("Alignement hauteur"), Row, 3);
        MainLayout->addWidget(CtrlAlignementVerticale, Row, 4);
        CtrlAlignementVerticale->addItems({ "Haut", "Centre", "Bas" });
        if (mItemDef->Alignement_Verticale == Bloc::AlignementVerticale::Haut)
            CtrlAlignementVerticale->setCurrentIndex(0);
        if (mItemDef->Alignement_Verticale == Bloc::AlignementVerticale::Centre)
            CtrlAlignementVerticale->setCurrentIndex(1);
        if (mItemDef->Alignement_Verticale == Bloc::AlignementVerticale::Bas)
            CtrlAlignementVerticale->setCurrentIndex(2);

        MainLayout->addWidget(new QLabel("Taille police\n0 = Taille par défaut"), Row, 5);
        MainLayout->addWidget(CtrlTaillePolice, Row, 6);
        CtrlTaillePolice->setValue(mItemDef->TaillePolice);

        MainLayout->addWidget(new QLabel("Longueur maximale\n0=illimité"), Row, 7);
        MainLayout->addWidget(CtrlLongueurMaximale, Row, 8);
        CtrlLongueurMaximale->setValue(mItemDef->LongueurMaximale);

        Row++;
        CtrlGras->setLayoutDirection(Qt::RightToLeft);
        CtrlGras->setText("Gras");
        MainLayout->addWidget(CtrlGras, Row, 1, 1, 2);

        CtrlItalique->setLayoutDirection(Qt::RightToLeft);
        CtrlItalique->setText("Italique");
        MainLayout->addWidget(CtrlItalique, Row, 3, 1, 2);

        CtrlGrasEtItalique->setLayoutDirection(Qt::RightToLeft);
        CtrlGrasEtItalique->setText("Gras et italique");
        MainLayout->addWidget(CtrlGrasEtItalique, Row, 5, 1, 2);

        CtrlMonospace->setLayoutDirection(Qt::RightToLeft);
        CtrlMonospace->setText("Monospace");
        MainLayout->addWidget(CtrlMonospace, Row, 7, 1, 2);

        CtrlGras->setChecked(mItemDef->Gras);
        CtrlItalique->setChecked(mItemDef->Italique);
        CtrlGrasEtItalique->setChecked(mItemDef->GrasEtItalique);
        CtrlMonospace->setChecked(mItemDef->Monospace);

        Row++;
        CtrlObligatoire->setLayoutDirection(Qt::RightToLeft);
        CtrlObligatoire->setText("Obligatoire");
        MainLayout->addWidget(CtrlObligatoire, Row, 1, 1, 2);

        CtrlMinuscule->setLayoutDirection(Qt::RightToLeft);
        CtrlMinuscule->setText("Minuscule");
        MainLayout->addWidget(CtrlMinuscule, Row, 3, 1, 2);

        CtrlMajuscule->setLayoutDirection(Qt::RightToLeft);
        CtrlMajuscule->setText("Majuscule");
        MainLayout->addWidget(CtrlMajuscule, Row, 5, 1, 2);

        CtrlChiffre->setLayoutDirection(Qt::RightToLeft);
        CtrlChiffre->setText("Chiffre");
        MainLayout->addWidget(CtrlChiffre, Row, 7, 1, 2);

        CtrlObligatoire->setChecked(mItemDef->Obligatoire);
        CtrlMinuscule->setChecked(mItemDef->Minuscule);
        CtrlMajuscule->setChecked(mItemDef->Majuscule);
        CtrlChiffre->setChecked(mItemDef->Chiffre);

        Row++;
        MainLayout->addWidget(new QLabel("Question"), Row, 1);
        MainLayout->addWidget(CtrlQuestion, Row, 2, 1, 7);

        Row++;
        MainLayout->addWidget(new QLabel("Question par défaut"), Row, 1);
        MainLayout->addWidget(CtrlTexteMultiligne, Row, 2, 1, 7);

        Row++;
        MainLayout->addWidget(new QLabel("Aide"), Row, 1);
        MainLayout->addWidget(CtrlQuestionAide, Row, 2, 1, 7);

        CtrlQuestion->setText(mItemDef->Question);
        CtrlTexteMultiligne->setText(mItemDef->QuestionDefaut);
        CtrlQuestionAide->setText(mItemDef->QuestionAide);
    }
    if (mItemDef->TypeAction == Bloc::TypeAction::DESSINECHECKBOX) {
        Row++;
        MainLayout->addWidget(new QLabel("Debut X"), Row, 1);
        MainLayout->addWidget(CtrlDebutX, Row, 2);
        MainLayout->addWidget(new QLabel("Debut Y"), Row, 3);
        MainLayout->addWidget(CtrlDebutY, Row, 4);
        MainLayout->addWidget(new QLabel("Largeur"), Row, 5);
        MainLayout->addWidget(CtrlLargeur, Row, 6);
        MainLayout->addWidget(new QLabel("Hauteur"), Row, 7);
        MainLayout->addWidget(CtrlHauteur, Row, 8);
        CtrlDebutX->setValue(mItemDef->DebutX);
        CtrlDebutY->setValue(mItemDef->DebutY);
        CtrlLargeur->setValue(mItemDef->Largeur);
        CtrlHauteur->setValue(mItemDef->Hauteur);

        Row++;
        MainLayout->addWidget(new QLabel("Couleur de trait\n[HTML #RRGGBB]"), Row, 1);
        CtrlCouleur->setText(mItemDef->Couleur);
        CtrlCouleur->setMaxLength(7);
        if (mItemDef->Couleur == "{ACC 1}") {
            CtrlCouleur->setEnabled(false);
            CtrlUtiliseCouleurPrincipale->setChecked(true);
            CtrlUtiliseCouleurAccent->setChecked(false);
        }
        if (mItemDef->Couleur == "{ACC 2}") {
            CtrlCouleur->setEnabled(false);
            CtrlUtiliseCouleurPrincipale->setChecked(false);
            CtrlUtiliseCouleurAccent->setChecked(true);
        }
        MainLayout->addWidget(CtrlCouleur, Row, 2);

        CtrlUtiliseCouleurPrincipale->setText("Utiliser la couleur d'accentuation 1");
        MainLayout->addWidget(CtrlUtiliseCouleurPrincipale, Row, 3, 1, 2);
        CtrlUtiliseCouleurPrincipale->setLayoutDirection(Qt::RightToLeft);
        CtrlUtiliseCouleurAccent->setText("Utiliser la couleur d'accentuation 2");
        MainLayout->addWidget(CtrlUtiliseCouleurAccent, Row, 5, 1, 2);

        CtrlUtiliseCouleurAccent->setLayoutDirection(Qt::RightToLeft);
        connect(CtrlUtiliseCouleurPrincipale, SIGNAL(toggled(bool)), this, SLOT(ClicCheckBoxPrincipale1(bool)));
        connect(CtrlUtiliseCouleurAccent, SIGNAL(toggled(bool)), this, SLOT(ClicCheckBoxAccent1(bool)));
        MainLayout->addWidget(new QLabel("Epaisseur\n0=Epaisseur par défaut (1)"), Row, 7);
        MainLayout->addWidget(CtrlEpaisseur, Row, 8);
        CtrlEpaisseur->setValue(mItemDef->Epaisseur);
    }
    if (mItemDef->TypeAction == Bloc::TypeAction::DESSINECHECKBOXQUESTION) {
        Row++;
        MainLayout->addWidget(new QLabel("Debut X"), Row, 1);
        MainLayout->addWidget(CtrlDebutX, Row, 2);
        MainLayout->addWidget(new QLabel("Debut Y"), Row, 3);
        MainLayout->addWidget(CtrlDebutY, Row, 4);
        MainLayout->addWidget(new QLabel("Largeur"), Row, 5);
        MainLayout->addWidget(CtrlLargeur, Row, 6);
        MainLayout->addWidget(new QLabel("Hauteur"), Row, 7);
        MainLayout->addWidget(CtrlHauteur, Row, 8);
        CtrlDebutX->setValue(mItemDef->DebutX);
        CtrlDebutY->setValue(mItemDef->DebutY);
        CtrlLargeur->setValue(mItemDef->Largeur);
        CtrlHauteur->setValue(mItemDef->Hauteur);

        Row++;
        MainLayout->addWidget(new QLabel("Couleur de trait\n[HTML #RRGGBB]"), Row, 1);
        CtrlCouleur->setText(mItemDef->Couleur);
        CtrlCouleur->setMaxLength(7);
        if (mItemDef->Couleur == "{ACC 1}") {
            CtrlCouleur->setEnabled(false);
            CtrlUtiliseCouleurPrincipale->setChecked(true);
            CtrlUtiliseCouleurAccent->setChecked(false);
        }
        if (mItemDef->Couleur == "{ACC 2}") {
            CtrlCouleur->setEnabled(false);
            CtrlUtiliseCouleurPrincipale->setChecked(false);
            CtrlUtiliseCouleurAccent->setChecked(true);
        }
        MainLayout->addWidget(CtrlCouleur, Row, 2);

        CtrlUtiliseCouleurPrincipale->setText("Utiliser la couleur d'accentuation 1");
        MainLayout->addWidget(CtrlUtiliseCouleurPrincipale, Row, 3, 1, 2);
        CtrlUtiliseCouleurPrincipale->setLayoutDirection(Qt::RightToLeft);
        CtrlUtiliseCouleurAccent->setText("Utiliser la couleur d'accentuation 2");
        MainLayout->addWidget(CtrlUtiliseCouleurAccent, Row, 5, 1, 2);

        CtrlUtiliseCouleurAccent->setLayoutDirection(Qt::RightToLeft);
        connect(CtrlUtiliseCouleurPrincipale, SIGNAL(toggled(bool)), this, SLOT(ClicCheckBoxPrincipale1(bool)));
        connect(CtrlUtiliseCouleurAccent, SIGNAL(toggled(bool)), this, SLOT(ClicCheckBoxAccent1(bool)));
        MainLayout->addWidget(new QLabel("Epaisseur\n0=Epaisseur par défaut (1)"), Row, 7);
        MainLayout->addWidget(CtrlEpaisseur, Row, 8);
        CtrlEpaisseur->setValue(mItemDef->Epaisseur);

        Row++;
        CtrlObligatoire->setLayoutDirection(Qt::RightToLeft);
        CtrlObligatoire->setText("Obligatoire");
        MainLayout->addWidget(CtrlObligatoire, Row, 1, 1, 2);
        CtrlObligatoire->setChecked(mItemDef->Obligatoire);

        Row++;
        MainLayout->addWidget(new QLabel("Question"), Row, 1);
        MainLayout->addWidget(CtrlQuestion, Row, 2, 1, 7);

        Row++;
        CtrlCheckboxDefaut->setLayoutDirection(Qt::RightToLeft);
        CtrlCheckboxDefaut->setText("Etat de la checkbox");
        MainLayout->addWidget(CtrlCheckboxDefaut, Row, 2);

        Row++;
        MainLayout->addWidget(new QLabel("Aide"), Row, 1);
        MainLayout->addWidget(CtrlQuestionAide, Row, 2, 1, 7);

        CtrlQuestion->setText(mItemDef->Question);
        CtrlCheckboxDefaut->setChecked(mItemDef->CheckboxDefaut);
        CtrlQuestionAide->setText(mItemDef->QuestionAide);
    }
    if (mItemDef->TypeAction == Bloc::TypeAction::DESSINEMULTICHECKBOXQUESTION) {
        Row++;
        MainLayout->addWidget(new QLabel("Debut X\nCheckbox 1"), Row, 1);
        MainLayout->addWidget(CtrlDebutX, Row, 2);
        MainLayout->addWidget(new QLabel("Debut Y\nCheckbox 1"), Row, 3);
        MainLayout->addWidget(CtrlDebutY, Row, 4);
        MainLayout->addWidget(new QLabel("Largeur"), Row, 5);
        MainLayout->addWidget(CtrlLargeur, Row, 6);
        MainLayout->addWidget(new QLabel("Hauteur"), Row, 7);
        MainLayout->addWidget(CtrlHauteur, Row, 8);
        CtrlDebutX->setValue(mItemDef->DebutX);
        CtrlDebutY->setValue(mItemDef->DebutY);
        CtrlLargeur->setValue(mItemDef->Largeur);
        CtrlHauteur->setValue(mItemDef->Hauteur);

        Row++;
        MainLayout->addWidget(new QLabel("Debut X\nCheckbox 2"), Row, 1);
        MainLayout->addWidget(CtrlDebutX2, Row, 2);
        MainLayout->addWidget(new QLabel("Debut Y\nCheckbox 2"), Row, 3);
        MainLayout->addWidget(CtrlDebutY2, Row, 4);
        CtrlDebutX2->setValue(mItemDef->DebutX2);
        CtrlDebutY2->setValue(mItemDef->DebutY2);

        Row++;
        MainLayout->addWidget(new QLabel("Couleur de trait\n[HTML #RRGGBB]"), Row, 1);
        CtrlCouleur->setText(mItemDef->Couleur);
        CtrlCouleur->setMaxLength(7);
        if (mItemDef->Couleur == "{ACC 1}") {
            CtrlCouleur->setEnabled(false);
            CtrlUtiliseCouleurPrincipale->setChecked(true);
            CtrlUtiliseCouleurAccent->setChecked(false);
        }
        if (mItemDef->Couleur == "{ACC 2}") {
            CtrlCouleur->setEnabled(false);
            CtrlUtiliseCouleurPrincipale->setChecked(false);
            CtrlUtiliseCouleurAccent->setChecked(true);
        }
        MainLayout->addWidget(CtrlCouleur, Row, 2);

        CtrlUtiliseCouleurPrincipale->setText("Utiliser la couleur d'accentuation 1");
        MainLayout->addWidget(CtrlUtiliseCouleurPrincipale, Row, 3, 1, 2);
        CtrlUtiliseCouleurPrincipale->setLayoutDirection(Qt::RightToLeft);
        CtrlUtiliseCouleurAccent->setText("Utiliser la couleur d'accentuation 2");
        MainLayout->addWidget(CtrlUtiliseCouleurAccent, Row, 5, 1, 2);

        CtrlUtiliseCouleurAccent->setLayoutDirection(Qt::RightToLeft);
        connect(CtrlUtiliseCouleurPrincipale, SIGNAL(toggled(bool)), this, SLOT(ClicCheckBoxPrincipale1(bool)));
        connect(CtrlUtiliseCouleurAccent, SIGNAL(toggled(bool)), this, SLOT(ClicCheckBoxAccent1(bool)));
        MainLayout->addWidget(new QLabel("Epaisseur\n0=Epaisseur par défaut (1)"), Row, 7);
        MainLayout->addWidget(CtrlEpaisseur, Row, 8);

        Row++;
        CtrlObligatoire->setLayoutDirection(Qt::RightToLeft);
        CtrlObligatoire->setText("Obligatoire");
        MainLayout->addWidget(CtrlObligatoire, Row, 1, 1, 2);
        CtrlObligatoire->setChecked(mItemDef->Obligatoire);

        Row++;
        MainLayout->addWidget(new QLabel("Question"), Row, 1);
        MainLayout->addWidget(CtrlQuestion, Row, 2, 1, 7);

        Row++;
        CtrlCheckboxDefaut->setLayoutDirection(Qt::RightToLeft);
        CtrlCheckboxDefaut->setText("Etat de la checkbox");
        MainLayout->addWidget(CtrlCheckboxDefaut, Row, 2);

        Row++;
        MainLayout->addWidget(new QLabel("Aide"), Row, 1);
        MainLayout->addWidget(CtrlQuestionAide, Row, 2, 1, 7);

        CtrlQuestion->setText(mItemDef->Question);
        CtrlCheckboxDefaut->setChecked(mItemDef->CheckboxDefaut);
        CtrlQuestionAide->setText(mItemDef->QuestionAide);
    }
    if (mItemDef->TypeAction == Bloc::TypeAction::INSEREIMAGE) {
        Row++;
        MainLayout->addWidget(new QLabel("Debut X"), Row, 1);
        MainLayout->addWidget(CtrlDebutX, Row, 2);
        MainLayout->addWidget(new QLabel("Debut Y"), Row, 3);
        MainLayout->addWidget(CtrlDebutY, Row, 4);
        MainLayout->addWidget(new QLabel("Largeur"), Row, 5);
        MainLayout->addWidget(CtrlLargeur, Row, 6);
        MainLayout->addWidget(new QLabel("Hauteur"), Row, 7);
        MainLayout->addWidget(CtrlHauteur, Row, 8);
        CtrlDebutX->setValue(mItemDef->DebutX);
        CtrlDebutY->setValue(mItemDef->DebutY);
        CtrlLargeur->setValue(mItemDef->Largeur);
        CtrlHauteur->setValue(mItemDef->Hauteur);

        Row++;
        MainLayout->addWidget(new QLabel("Chemin de l'image"), Row, 1);
        MainLayout->addWidget(CtrlCheminImage, Row, 2, 1, 7);
        CtrlCheminImage->setText(mItemDef->CheminImage);
    }
    if (mItemDef->TypeAction == Bloc::TypeAction::COMMENTAIRE) {
        Row++;
        MainLayout->addWidget(new QLabel("Commentaire"), Row, 1);
        MainLayout->addWidget(CtrlCommentaire, Row, 2, 1, 7);
        CtrlCommentaire->setText(mItemDef->Commentaire);
    }

    // Bouton
    {
        Row++;

        MainLayout->addWidget(CtrlBoutonDeplaceHaut, Row, 1, 1, 2);
        CtrlBoutonDeplaceHaut->setText("Déplacer vers le haut");
        CtrlBoutonDeplaceHaut->setIcon(QIcon(":/Monte"));
        connect(CtrlBoutonDeplaceHaut, SIGNAL(clicked()), this, SLOT(ClicBoutonDeplaceHaut()));
        CtrlBoutonDeplaceHaut->setStyleSheet(qsStyleSheet);

        MainLayout->addWidget(CtrlBoutonDeplacePremier, Row, 3, 1, 2);
        CtrlBoutonDeplacePremier->setText("Déplacer tout en haut");
        CtrlBoutonDeplacePremier->setIcon(QIcon(":/Haut"));
        connect(CtrlBoutonDeplacePremier, SIGNAL(clicked()), this, SLOT(ClicBoutonDeplacePremier()));
        CtrlBoutonDeplacePremier->setStyleSheet(qsStyleSheet);

        MainLayout->addWidget(CtrlBoutonDeplaceBas, Row, 5, 1, 2);
        CtrlBoutonDeplaceBas->setText("Déplacer vers le bas");
        CtrlBoutonDeplaceBas->setIcon(QIcon(":/Descent"));
        connect(CtrlBoutonDeplaceBas, SIGNAL(clicked()), this, SLOT(ClicBoutonDeplaceBas()));
        CtrlBoutonDeplaceBas->setStyleSheet(qsStyleSheet);

        MainLayout->addWidget(CtrlBoutonDeplaceFin, Row, 7, 1, 2);
        CtrlBoutonDeplaceFin->setText("Déplacer tout en bas");
        CtrlBoutonDeplaceFin->setIcon(QIcon(":/Bas"));
        connect(CtrlBoutonDeplaceFin, SIGNAL(clicked()), this, SLOT(ClicBoutonDeplaceFin()));
        CtrlBoutonDeplaceFin->setStyleSheet(qsStyleSheet);

        {
            QSizePolicy policy0 = CtrlBoutonDeplaceHaut->sizePolicy();
            policy0.setHorizontalStretch(1);
            CtrlBoutonDeplaceHaut->setSizePolicy(policy0);
        }
        {
            QSizePolicy policy0 = CtrlBoutonDeplacePremier->sizePolicy();
            policy0.setHorizontalStretch(1);
            CtrlBoutonDeplacePremier->setSizePolicy(policy0);
        }
        {
            QSizePolicy policy0 = CtrlBoutonDeplaceBas->sizePolicy();
            policy0.setHorizontalStretch(1);
            CtrlBoutonDeplaceBas->setSizePolicy(policy0);
        }
        {
            QSizePolicy policy0 = CtrlBoutonDeplaceFin->sizePolicy();
            policy0.setHorizontalStretch(1);
            CtrlBoutonDeplaceFin->setSizePolicy(policy0);
        }
    }
    this->setLayout(MainLayout);
}

/** RetourneDonnee:
    Retourne les valeurs du contrôle

    @return ItemDefinition avec las valeurs du contrôle
*/
Bloc::ItemDefinition
BlocEditeur::RetourneDonnee()
{
    Update_mItemDef();
    Bloc::ItemDefinition Recopie(*mItemDef);
    return Recopie;
}

/** GetActionQString:
    Convertit en QString le type d'action

    @param Bloc::TypeAction typeAction
    @return QString
*/
QString
BlocEditeur::GetActionQString(Bloc::TypeAction typeAction)
{
    if (typeAction == Bloc::TypeAction::DESSINECHECKBOX)
        return "DESSINECHECKBOX";
    if (typeAction == Bloc::TypeAction::DESSINECHECKBOXQUESTION)
        return "DESSINECHECKBOXQUESTION";
    if (typeAction == Bloc::TypeAction::DESSINELIGNE)
        return "DESSINELIGNE";
    if (typeAction == Bloc::TypeAction::DESSINEMULTICHECKBOXQUESTION)
        return "DESSINEMULTICHECKBOXQUESTION";
    if (typeAction == Bloc::TypeAction::DESSINERECTANGLEGRILLE)
        return "DESSINERECTANGLEGRILLE";
    if (typeAction == Bloc::TypeAction::DESSINERECTANGLEREMPLIS)
        return "DESSINERECTANGLEREMPLIS";
    if (typeAction == Bloc::TypeAction::DESSINERECTANGLEVIDE)
        return "DESSINERECTANGLEVIDE";
    if (typeAction == Bloc::TypeAction::DESSINETEXTE)
        return "DESSINETEXTE";
    if (typeAction == Bloc::TypeAction::DESSINETEXTEMULTILIGNE)
        return "DESSINETEXTEMULTILIGNE";
    if (typeAction == Bloc::TypeAction::DESSINETEXTEMULTILIGNEQUESTION)
        return "DESSINETEXTEMULTILIGNEQUESTION";
    if (typeAction == Bloc::TypeAction::DESSINETEXTEQUESTION)
        return "DESSINETEXTEQUESTION";
    if (typeAction == Bloc::TypeAction::INSEREIMAGE)
        return "INSEREIMAGE";
    if (typeAction == Bloc::TypeAction::PAGESUIVANTE)
        return "PAGESUIVANTE";
    if (typeAction == Bloc::TypeAction::COMMENTAIRE)
        return "COMMENTAIRE";
    return "vide";
}

// qDebug() << "ClicBoutonPressePapier"<< mItemDef->IndexControle;
// qDebug() << "*0" << this->objectName();//BlocEditeur
// qDebug() << "*1" << this->parent()->objectName();//qt_scrollarea_viewport
// qDebug() << "*2" << this->parent()->parent()->objectName(); // EDIT_Liste
// qDebug() << "*3" <<
// this->parent()->parent()->parent()->objectName();//groupBox_2 qDebug() <<
// "*4" << this->parent()->parent()->parent()->parent()->objectName();//tab_6
// qDebug() << "*5" <<
// this->parent()->parent()->parent()->parent()->parent()->objectName();//qt_tabwidget_stackedwidget
// qDebug() << "*6" <<
// this->parent()->parent()->parent()->parent()->parent()->parent()->objectName();//tabWidget
// qDebug() << "*7" <<
// this->parent()->parent()->parent()->parent()->parent()->parent()->parent()->objectName();//centralwidget
// qDebug() << "*8" <<
// this->parent()->parent()->parent()->parent()->parent()->parent()->parent()->parent()->objectName();//MainWindow

/** ClicBoutonSupprimer:
    Supprime le bloc actuel

    @return aucun
*/
void
BlocEditeur::ClicBoutonSupprimer()
{
    QListWidget* mListWidget = qobject_cast<QListWidget*>(this->parent()->parent());
    auto NomBlocQuestion     = this->objectName();
    if (mListWidget->count() > 0)
        for (int i = 0; i < mListWidget->count(); ++i) {
            auto item       = mListWidget->item(i);
            auto itemWidget = qobject_cast<BlocEditeur*>(mListWidget->itemWidget(item));
            if (itemWidget->objectName() == NomBlocQuestion) // Trouvé
            {
                delete mListWidget->takeItem(mListWidget->row(item));
                break;
            }
        }
}

/** ClicBoutonDeplaceHaut:
    Déplace le bloc actuel vers le haut

    @return aucun
*/
void
BlocEditeur::ClicBoutonDeplaceHaut()
{
    QListWidget* mListWidget = qobject_cast<QListWidget*>(this->parent()->parent());
    auto NomBlocQuestion     = this->objectName();
    if (mListWidget->count() > 0)
        for (int i = 0; i < mListWidget->count(); ++i) {
            auto item               = mListWidget->item(i);
            auto itemWidget         = qobject_cast<BlocEditeur*>(mListWidget->itemWidget(item));
            auto itemWidgetAsWidget = mListWidget->itemWidget(item);
            if (itemWidget->objectName() == NomBlocQuestion) // Trouvé
            {
                if (i == 0)
                    return; // Je suis déjà en premier
                auto ItemCopie = mListWidget->takeItem(mListWidget->row(item));
                QCoreApplication::removePostedEvents(itemWidget, QEvent::DeferredDelete);
                mListWidget->insertItem(i - 1, ItemCopie);
                mListWidget->setItemWidget(ItemCopie, itemWidgetAsWidget);
                break;
            }
        }
}

/** ClicBoutonDeplacePremier:
    Déplace le bloc actuel au début de la liste

    @return aucun
*/
void
BlocEditeur::ClicBoutonDeplacePremier()
{
    QListWidget* mListWidget = qobject_cast<QListWidget*>(this->parent()->parent());
    auto NomBlocQuestion     = this->objectName();
    if (mListWidget->count() > 0)
        for (int i = 0; i < mListWidget->count(); ++i) {
            auto item               = mListWidget->item(i);
            auto itemWidget         = qobject_cast<BlocEditeur*>(mListWidget->itemWidget(item));
            auto itemWidgetAsWidget = mListWidget->itemWidget(item);
            if (itemWidget->objectName() == NomBlocQuestion) // Trouvé
            {
                if (i == 0)
                    return; // Je suis déjà en premier
                auto ItemCopie = mListWidget->takeItem(mListWidget->row(item));
                QCoreApplication::removePostedEvents(itemWidget, QEvent::DeferredDelete);
                mListWidget->insertItem(0, ItemCopie);
                mListWidget->setItemWidget(ItemCopie, itemWidgetAsWidget);
                break;
            }
        }
}

/** ClicBoutonDeplaceBas:
    Déplace le bloc actuel vers le bas

    @return aucun
*/
void
BlocEditeur::ClicBoutonDeplaceBas()
{
    QListWidget* mListWidget = qobject_cast<QListWidget*>(this->parent()->parent());
    auto NomBlocQuestion     = this->objectName();
    if (mListWidget->count() > 0)
        for (int i = 0; i < mListWidget->count(); ++i) {
            auto item               = mListWidget->item(i);
            auto itemWidget         = qobject_cast<BlocEditeur*>(mListWidget->itemWidget(item));
            auto itemWidgetAsWidget = mListWidget->itemWidget(item);
            if (itemWidget->objectName() == NomBlocQuestion) // Trouvé
            {
                if (i == (mListWidget->count() - 1))
                    return; // Je suis déjà en dernier
                auto ItemCopie = mListWidget->takeItem(mListWidget->row(item));
                QCoreApplication::removePostedEvents(itemWidget, QEvent::DeferredDelete);
                mListWidget->insertItem(i + 1, ItemCopie);
                mListWidget->setItemWidget(ItemCopie, itemWidgetAsWidget);
                break;
            }
        }
}

/** ClicBoutonDeplaceFin:
    Déplace le bloc actuel tout en bas

    @return aucun
*/
void
BlocEditeur::ClicBoutonDeplaceFin()
{
    QListWidget* mListWidget = qobject_cast<QListWidget*>(this->parent()->parent());
    auto NomBlocQuestion     = this->objectName();
    if (mListWidget->count() > 0)
        for (int i = 0; i < mListWidget->count(); ++i) {
            auto item               = mListWidget->item(i);
            auto itemWidget         = qobject_cast<BlocEditeur*>(mListWidget->itemWidget(item));
            auto itemWidgetAsWidget = mListWidget->itemWidget(item);
            if (itemWidget->objectName() == NomBlocQuestion) // Trouvé
            {
                if (i == (mListWidget->count() - 1))
                    return; // Je suis déjà en premier
                auto ItemCopie = mListWidget->takeItem(mListWidget->row(item));
                QCoreApplication::removePostedEvents(itemWidget, QEvent::DeferredDelete);
                mListWidget->addItem(ItemCopie);
                mListWidget->setItemWidget(ItemCopie, itemWidgetAsWidget);
                break;
            }
        }
}

/** ClicBoutonDepClicBoutonPressePapierlaceFin:
    Fait apparaitre le menu du presse-papier pour le bloc actuel

    @return aucun
*/
void
BlocEditeur::ClicBoutonPressePapier()
{
    Update_mItemDef();
    Bloc::ItemDefinition Recopie(*mItemDef);

    QMenu* menu = new QMenu(this);

    menu->addAction(QString("Presse papier [Bloc en mémoire : %1]").arg(QString::number(mPressePapier->Taille())));
    menu->addSeparator();
    menu->addAction(QIcon(":/Copier"), QString("Copier ce bloc"), this, [this, Recopie](bool) { mPressePapier->AjoutPressePapier(Recopie); });
    menu->addAction(QIcon(":/Couper"), QString("Couper ce bloc"), this, [this, Recopie](bool) {
        mPressePapier->AjoutPressePapier(Recopie);
        ClicBoutonSupprimer();
    });

    menu->addAction(QIcon(":/Coller"), QString("Coller le contenu du presse-papier avant ce bloc"), this, [this, Recopie](bool) {
        ///
        /// LAMBDA  COLLAGE DU PRESSE PAPIER AVANT
        ///
        QListWidget* mListWidget = qobject_cast<QListWidget*>(this->parent()->parent());
        int IndexInsere          = IndexAPartirNomControle(mListWidget, this->objectName());
        /// SAUVEGARDE
        QVector<Bloc::ItemDefinition> SavListe;
        for (int i = 0; i < mListWidget->count(); ++i) {
            BlocEditeur* mBlocquestion = qobject_cast<BlocEditeur*>(mListWidget->itemWidget(mListWidget->item(i)));
            SavListe.append(mBlocquestion->RetourneDonnee());
        }
        /// Insertion du Presse - Papier
        for (int i = 0; i < mPressePapier->Taille(); ++i)
            SavListe.insert(SavListe.begin() + IndexInsere + i, mPressePapier->ItemAt(i));
        /// On Efface et Redessine
        PeuplerListe(mListWidget, SavListe, IndexInsere);
    });

    menu->addAction(QIcon(":/Coller"), QString("Coller le contenu du presse-papier après ce bloc"), this, [this, Recopie](bool) {
        ///
        /// LAMBDA  COLLAGE DU PRESSE PAPIER APRES
        ///
        QListWidget* mListWidget = qobject_cast<QListWidget*>(this->parent()->parent());
        int IndexInsere          = IndexAPartirNomControle(mListWidget, this->objectName()) + 1;
        /// SAUVEGARDE
        QVector<Bloc::ItemDefinition> SavListe;
        for (int i = 0; i < mListWidget->count(); ++i) {
            BlocEditeur* mBlocquestion = qobject_cast<BlocEditeur*>(mListWidget->itemWidget(mListWidget->item(i)));
            SavListe.append(mBlocquestion->RetourneDonnee());
        }
        /// Insertion du Presse - Papier
        for (int i = 0; i < mPressePapier->Taille(); ++i)
            SavListe.insert(SavListe.begin() + IndexInsere + i, mPressePapier->ItemAt(i));
        /// On Efface et Redessine
        PeuplerListe(mListWidget, SavListe, IndexInsere);
    });

    menu->addAction(QIcon(":/Vider"), QString("Vider le presse-papier"), this, [&] { mPressePapier->Clear(); });
    menu->actions()[0]->setEnabled(false);
    if (mPressePapier->Liste().count() == 0) // On desactive le coller/vider
    {
        menu->actions()[4]->setEnabled(false);
        menu->actions()[5]->setEnabled(false);
        menu->actions()[6]->setEnabled(false);
    }
    menu->popup(this->mapTo(this, QCursor::pos()));
}

/** RetourneIndexLibre:
    Retourne le prochain numéro d'index libre pour les variables internes

    @return qint64
*/
qint64
BlocEditeur::RetourneIndexLibre()
{
    QListWidget* mListWidget = qobject_cast<QListWidget*>(this->parent()->parent());
    QVector<qint64> ListeIndex;
    if (mListWidget->count() > 0)
        for (int i = 0; i < mListWidget->count(); ++i) {
            auto itemWidget = qobject_cast<BlocEditeur*>(mListWidget->itemWidget(mListWidget->item(i)));
            ListeIndex.append(itemWidget->RetourneDonnee().IndexControle);
        }
    if (ListeIndex.count() == 0)
        return 0; // Liste vide

    std::sort(ListeIndex.begin(), ListeIndex.end());
    int next = 0;
    for (QVector<qint64>::iterator it = ListeIndex.begin(); it != ListeIndex.end(); ++it) {
        if (*it != next)
            return next;
        ++next;
    }
    return next;
}
/** ClicCheckBoxPrincipale1:
    Permet de basculer entre texte et checkbox pour la couleur principale

    @param bool Etat
    @return Aucun
*/
void
BlocEditeur::ClicCheckBoxPrincipale1(bool Etat)
{
    CtrlCouleur->setEnabled(!Etat);
    if (CtrlUtiliseCouleurAccent->isChecked() && Etat == true)
        CtrlUtiliseCouleurAccent->setChecked(!Etat);
    if (CtrlUtiliseCouleurAccent->isChecked())
        CtrlCouleur->setEnabled(false);
    if ((CtrlUtiliseCouleurAccent->isChecked() == false) && (CtrlUtiliseCouleurPrincipale->isChecked() == false) &&
        ((CtrlCouleur->text() == "{ACC 1}") || (CtrlCouleur->text() == "{ACC 2}")))
        CtrlCouleur->setText("#000000");
    if (CtrlUtiliseCouleurPrincipale->isChecked())
        CtrlCouleur->setText("{ACC 1}");
    if (CtrlUtiliseCouleurAccent->isChecked())
        CtrlCouleur->setText("{ACC 2}");
}

/** ClicCheckBoxAccent1:
    Permet de basculer entre texte et checkbox pour la couleur accentuée

    @param bool Etat
    @return Aucun
*/
void
BlocEditeur::ClicCheckBoxAccent1(bool Etat)
{
    CtrlCouleur->setEnabled(!Etat);
    if (CtrlUtiliseCouleurPrincipale->isChecked() && Etat == true)
        CtrlUtiliseCouleurPrincipale->setChecked(!Etat);
    if (CtrlUtiliseCouleurPrincipale->isChecked())
        CtrlCouleur->setEnabled(false);
    if ((CtrlUtiliseCouleurAccent->isChecked() == false) && (CtrlUtiliseCouleurPrincipale->isChecked() == false) &&
        ((CtrlCouleur->text() == "{ACC 1}") || (CtrlCouleur->text() == "{ACC 2}")))
        CtrlCouleur->setText("#000000");
    if (CtrlUtiliseCouleurPrincipale->isChecked())
        CtrlCouleur->setText("{ACC 1}");
    if (CtrlUtiliseCouleurAccent->isChecked())
        CtrlCouleur->setText("{ACC 2}");
}

/** ClicCheckBoxPrincipaleRemplissage:
    Permet de basculer entre texte et checkbox pour la couleur de remplissage

    @param bool Etat
    @return Aucun
*/
void
BlocEditeur::ClicCheckBoxPrincipaleRemplissage(bool Etat)
{
    CtrlCouleurRemplissage->setEnabled(!Etat);
    if (CtrlUtiliseCouleurAccentRemplissage->isChecked() && Etat == true)
        CtrlUtiliseCouleurAccentRemplissage->setChecked(!Etat);
    if (CtrlUtiliseCouleurAccentRemplissage->isChecked())
        CtrlCouleurRemplissage->setEnabled(false);
    if ((CtrlUtiliseCouleurAccentRemplissage->isChecked() == false) && (CtrlUtiliseCouleurPrincipaleRemplissage->isChecked() == false) &&
        ((CtrlCouleurRemplissage->text() == "{ACC 1}") || (CtrlCouleurRemplissage->text() == "{ACC 2}")))
        CtrlCouleurRemplissage->setText("#000000");
    if (CtrlUtiliseCouleurPrincipaleRemplissage->isChecked())
        CtrlCouleurRemplissage->setText("{ACC 1}");
    if (CtrlUtiliseCouleurAccentRemplissage->isChecked())
        CtrlCouleurRemplissage->setText("{ACC 2}");
}

/** ClicCheckBoxAccentRemplissage:
    Permet de basculer entre texte et checkbox pour la couleur accentuée de
   remplissage

    @param bool Etat
    @return Aucun
*/
void
BlocEditeur::ClicCheckBoxAccentRemplissage(bool Etat)
{
    CtrlCouleurRemplissage->setEnabled(!Etat);
    if (CtrlUtiliseCouleurPrincipaleRemplissage->isChecked() && Etat == true)
        CtrlUtiliseCouleurPrincipaleRemplissage->setChecked(!Etat);
    if (CtrlUtiliseCouleurPrincipaleRemplissage->isChecked())
        CtrlCouleurRemplissage->setEnabled(false);
    if ((CtrlUtiliseCouleurAccentRemplissage->isChecked() == false) && (CtrlUtiliseCouleurPrincipaleRemplissage->isChecked() == false) &&
        ((CtrlCouleurRemplissage->text() == "{ACC 1}") || (CtrlCouleurRemplissage->text() == "{ACC 2}")))
        CtrlCouleurRemplissage->setText("#000000");
    if (CtrlUtiliseCouleurPrincipaleRemplissage->isChecked())
        CtrlCouleurRemplissage->setText("{ACC 1}");
    if (CtrlUtiliseCouleurAccentRemplissage->isChecked())
        CtrlCouleurRemplissage->setText("{ACC 2}");
}

/** Update_mItemDef:
    Mise à jour du pointeur mItemDef avec les contrôles

    @return Aucun
*/
void
BlocEditeur::Update_mItemDef()
{
    mItemDef->DebutX             = CtrlDebutX->value();
    mItemDef->DebutY             = CtrlDebutY->value();
    mItemDef->DebutX2            = CtrlDebutX2->value();
    mItemDef->DebutY2            = CtrlDebutY2->value();
    mItemDef->FinX               = CtrlFinX->value();
    mItemDef->FinY               = CtrlFinY->value();
    mItemDef->Largeur            = CtrlLargeur->value();
    mItemDef->Hauteur            = CtrlHauteur->value();
    mItemDef->Couleur            = CtrlCouleur->text();
    mItemDef->Epaisseur          = CtrlEpaisseur->value();
    mItemDef->NombreColonne      = CtrlNombreColonne->value();
    mItemDef->NombreLigne        = CtrlNombreLigne->value();
    mItemDef->CouleurRemplissage = CtrlCouleurRemplissage->text();
    if (CtrlAlignementHorizontale->currentIndex() == 0)
        mItemDef->Alignement_Horizontale = Bloc::AlignementHorizontale::Gauche;
    if (CtrlAlignementHorizontale->currentIndex() == 1)
        mItemDef->Alignement_Horizontale = Bloc::AlignementHorizontale::Milieu;
    if (CtrlAlignementHorizontale->currentIndex() == 2)
        mItemDef->Alignement_Horizontale = Bloc::AlignementHorizontale::Droite;
    if (CtrlAlignementVerticale->currentIndex() == 0)
        mItemDef->Alignement_Verticale = Bloc::AlignementVerticale::Haut;
    if (CtrlAlignementVerticale->currentIndex() == 1)
        mItemDef->Alignement_Verticale = Bloc::AlignementVerticale::Centre;
    if (CtrlAlignementVerticale->currentIndex() == 2)
        mItemDef->Alignement_Verticale = Bloc::AlignementVerticale::Bas;
    mItemDef->TaillePolice             = CtrlTaillePolice->value();
    mItemDef->ValeurSplit              = CtrlValeurSplit->value();
    mItemDef->Gras                     = CtrlGras->isChecked();
    mItemDef->Italique                 = CtrlItalique->isChecked();
    mItemDef->GrasEtItalique           = CtrlGrasEtItalique->isChecked();
    mItemDef->Monospace                = CtrlMonospace->isChecked();
    mItemDef->Texte                    = CtrlTexte->text();
    mItemDef->TexteMultiligne          = CtrlTexteMultiligne->toPlainText();
    mItemDef->Obligatoire              = CtrlObligatoire->isChecked();
    mItemDef->Majuscule                = CtrlMajuscule->isChecked();
    mItemDef->Minuscule                = CtrlMinuscule->isChecked();
    mItemDef->Chiffre                  = CtrlChiffre->isChecked();
    mItemDef->LongueurMaximale         = CtrlLongueurMaximale->value();
    mItemDef->Question                 = CtrlQuestion->text();
    mItemDef->QuestionDefaut           = CtrlTexte->text();
    mItemDef->QuestionDefautMultiligne = CtrlTexteMultiligne->toPlainText();
    mItemDef->QuestionAide             = CtrlQuestionAide->text();
    mItemDef->CheminImage              = CtrlCheminImage->text();
    mItemDef->Commentaire              = CtrlCommentaire->toPlainText();
    mItemDef->CheckboxDefaut           = CtrlCheckboxDefaut->isChecked();
}

/** PeuplerListe:
    Peuple la liste avec le contenu du vecteur d'ItemDefinition et affiche à
   l'écran l'index choisi

    @param QListWidget* mListe
    @param QVector<Bloc::ItemDefinition>& mVecteur
    @param qint64 IndexAAfficher
    @return bool
*/
bool
BlocEditeur::PeuplerListe(QListWidget* mListe, QVector<Bloc::ItemDefinition>& mVecteur, qint64 IndexAAfficher)
{
    if (mListe == nullptr)
        return false;
    if (mVecteur.count() == 0)
        return false;

    QProgressDialog progress("Population de la liste...", "", 0, mVecteur.count(), this);
    if (mVecteur[0].ThemeSombre)
        progress.setStyle(new DarkStyle);
    progress.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    progress.setWindowModality(Qt::WindowModal);
    progress.setValue(0);
    progress.setAutoClose(false);
    progress.setCancelButton(0);
    progress.installEventFilter(keyPressEater);
    progress.show();
    {
        RECT FenetrePrincipale;
        GetWindowRect((HWND)mListe->winId(), &FenetrePrincipale);
        progress.setGeometry(FenetrePrincipale.left + (mListe->width() / 2) - (progress.width() / 2),
                             FenetrePrincipale.top + (mListe->height() / 2) - (progress.height() / 2),
                             progress.width(),
                             progress.height());
    }

    mListe->setUpdatesEnabled(false);
    mListe->clear();
    for (qsizetype VecIndex = 0; VecIndex < mVecteur.count(); ++VecIndex) {
        if (VecIndex % 10 == 0) {
            progress.setValue(VecIndex);
            QCoreApplication::processEvents();
        }
        Bloc::ItemDefinition StructBloc = mVecteur[VecIndex];
        StructBloc.IndexControle        = VecIndex;
        StructBloc.NomControle          = GetActionQString(StructBloc.TypeAction) + " " + QString::number(VecIndex);
        if (AjoutFinListe(mListe, StructBloc) == false) {
            progress.close();
            return false;
        }
    }
    mListe->setCurrentRow(IndexAAfficher);
    mListe->setUpdatesEnabled(true);

    progress.close();
    return true;
}

/** AjoutFinListe:
    Ajoute un bloc à la fin de la liste

    @param QListWidget* mListe
    @param Bloc::ItemDefinition& mBloc
    @return bool
*/
bool
BlocEditeur::AjoutFinListe(QListWidget* mListe, Bloc::ItemDefinition& mBloc)
{
    if (mListe == nullptr)
        return false;
    BlocEditeur* mBlocEditeur = new BlocEditeur(mPressePapier, this, mBloc, mBloc.ThemeSombre);
    QListWidgetItem* item;
    item = new QListWidgetItem(mListe);
    mListe->addItem(item);
    item->setSizeHint(mBlocEditeur->minimumSizeHint());
    mListe->setItemWidget(item, mBlocEditeur);
    return true;
}

/** IndexAPartirNomControle:
    Retourne l'index selon le nom du contrôle

    @param QListWidget* mListe
    @param QString Nom
    @return qint64
*/
qint64
BlocEditeur::IndexAPartirNomControle(QListWidget* mListe, QString Nom)
{
    if (mListe == nullptr)
        return -1;
    if (Nom == "")
        return -1;
    int IndexInsere = 0;
    if (mListe->count() > 0)
        for (int i = 0; i < mListe->count(); ++i)
            if (qobject_cast<BlocEditeur*>(mListe->itemWidget(mListe->item(i)))->objectName() == Nom) // Trouvé
            {
                IndexInsere = i;
                break;
            }
    return IndexInsere;
}
