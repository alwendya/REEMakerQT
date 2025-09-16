/* -------------------------------------------------------------------------- */
/*      REEMaker 5 -- Grégory WENTZEL (c) 2023   Code sous licence GPL3       */
/* -------------------------------------------------------------------------- */

#ifndef BLOCEDITEUR_H
#define BLOCEDITEUR_H

#include "bloc.h"
#include "pressepapier.h"
#include <QApplication>
#include <QCheckBox>
#include <QComboBox_custom.h>
// #include <QComboBox>
#include <QDateTime>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QProgressDialog>
#include <QPushButton>
#include <QSpacerItem>
#include <QSpinBox>
#include <QStyleOption>
#include <QTextEdit>
#include <QValidator>
#include <QWidget>
// #include <mainwindow.h>
#define NOGDI // Exclut les définitions GDI incluant DrawText
#include <windows.h>

// Connect showPopup to specific sub

/** BlocEditeur:
    Class du bloc Editeur pour le module d'édition des pages de gardes
*/
class BlocEditeur : public QWidget
{
    Q_OBJECT

  public:
    explicit BlocEditeur(PressePapier*, QWidget* parent = nullptr, Bloc::ItemDefinition mIDEF = Bloc::ItemDefinition(), bool Sombre = false);
    explicit BlocEditeur()
    {
        mPressePapier = new PressePapier();
        mItemDef = new Bloc::ItemDefinition();
    }
    ~BlocEditeur() { qDebug() << "Contrôle " << mItemDef->NomControle << " déchargé"; }
    Bloc::ItemDefinition RetourneDonnee();
    bool PeuplerListe(QListWidget*, QVector<Bloc::ItemDefinition>&, qint64);
    qint64 IndexAPartirNomControle(QListWidget*, QString);
    bool AjoutFinListe(QListWidget*, Bloc::ItemDefinition&);
    QString GetActionQString(Bloc::TypeAction typeAction);

  private slots:
    qint64 RetourneIndexLibre();
    void ClicBoutonSupprimer();
    void ClicBoutonDeplaceHaut();
    void ClicBoutonDeplacePremier();
    void ClicBoutonDeplaceBas();
    void ClicBoutonDeplaceFin();
    void ClicBoutonPressePapier();
    void ClicCheckBoxPrincipale1(bool);
    void ClicCheckBoxAccent1(bool);
    void ClicCheckBoxPrincipaleRemplissage(bool);
    void ClicCheckBoxAccentRemplissage(bool);

  private:
    struct MonitorRects
    {
        QVector<RECT> rcMonitors;
        QVector<QString> qvName;

        static BOOL CALLBACK MonitorEnum(HMONITOR hMon, HDC hdc, LPRECT lprcMonitor, LPARAM pData)
        {
            Q_UNUSED(hdc);
            MonitorRects* pThis = reinterpret_cast<MonitorRects*>(pData);
            pThis->rcMonitors.push_back(*lprcMonitor);
            MONITORINFOEXW miex;
            miex.cbSize = sizeof(MONITORINFOEXW); // set cbSize member
            miex.rcMonitor = { 0, 0, 0, 0 };
            miex.rcWork = { 0, 0, 0, 0 };
            if (GetMonitorInfoW(hMon, &miex)) {
                pThis->qvName.push_back(QString::fromWCharArray(miex.szDevice));
            };
            return TRUE;
        }

        MonitorRects() { EnumDisplayMonitors(0, 0, MonitorEnum, (LPARAM)this); }
    };
    class KeyPressEater : public QObject
    {
      protected:
        bool eventFilter(QObject* obj, QEvent* event) override
        {
            if (event->type() == QEvent::KeyPress) {
                QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

                qDebug("Ate key press %d", keyEvent->key());
                return true;
            } else {
                // standard event processing
                return QObject::eventFilter(obj, event);
            }
        }
    };
    KeyPressEater* keyPressEater = new KeyPressEater();
    void Update_mItemDef();
    Bloc::ItemDefinition* mItemDef;
    PressePapier* mPressePapier;

    // Contrôle double DebutX;
    QDoubleSpinBox* CtrlDebutX = new QDoubleSpinBox();
    // Contrôle double DebutY;
    QDoubleSpinBox* CtrlDebutY = new QDoubleSpinBox();
    // Contrôle double DebutX2;
    QDoubleSpinBox* CtrlDebutX2 = new QDoubleSpinBox();
    // Contrôle double DebutY2;
    QDoubleSpinBox* CtrlDebutY2 = new QDoubleSpinBox();
    // Contrôle double FinX;
    QDoubleSpinBox* CtrlFinX = new QDoubleSpinBox();
    // Contrôle double FinY;
    QDoubleSpinBox* CtrlFinY = new QDoubleSpinBox();
    // Contrôle double Largeur;
    QDoubleSpinBox* CtrlLargeur = new QDoubleSpinBox();
    // Contrôle double Hauteur;
    QDoubleSpinBox* CtrlHauteur = new QDoubleSpinBox();
    // Contrôle QString Couleur;
    QLineEdit* CtrlCouleur = new QLineEdit();
    // Contrôle double Epaisseur;
    QDoubleSpinBox* CtrlEpaisseur = new QDoubleSpinBox();
    // Contrôle qint16 NombreColonne;
    QSpinBox* CtrlNombreColonne = new QSpinBox();
    // Contrôle qint16 NombreLigne;
    QSpinBox* CtrlNombreLigne = new QSpinBox();
    // Contrôle QString CouleurRemplissage;
    QLineEdit* CtrlCouleurRemplissage = new QLineEdit();
    // Contrôle AlignementHorizontale Alignement_Horizontale;
    QComboBox_custom* CtrlAlignementHorizontale = new QComboBox_custom(this);
    // Contrôle AlignementVerticale Alignement_Verticale;
    QComboBox_custom* CtrlAlignementVerticale = new QComboBox_custom(this);
    // Contrôle double TaillePolice;
    QDoubleSpinBox* CtrlTaillePolice = new QDoubleSpinBox();
    // Contrôle qint16 ValeurSplit;
    QSpinBox* CtrlValeurSplit = new QSpinBox();
    // Contrôle bool CheckboxDefaut;
    QCheckBox* CtrlCheckboxDefaut = new QCheckBox();
    // Contrôle bool Gras;
    QCheckBox* CtrlGras = new QCheckBox();
    // Contrôle bool Italique;
    QCheckBox* CtrlItalique = new QCheckBox();
    // Contrôle bool GrasEtItalique;
    QCheckBox* CtrlGrasEtItalique = new QCheckBox();
    // Contrôle bool Monospace;
    QCheckBox* CtrlMonospace = new QCheckBox();
    // Contrôle QString Texte;
    QLineEdit* CtrlTexte = new QLineEdit();
    // Contrôle QString TexteMultiligne;
    QTextEdit* CtrlTexteMultiligne = new QTextEdit();
    // Contrôle bool Obligatoire;
    QCheckBox* CtrlObligatoire = new QCheckBox();
    // Contrôle bool Majuscule;
    QCheckBox* CtrlMajuscule = new QCheckBox();
    // Contrôle bool Minuscule;
    QCheckBox* CtrlMinuscule = new QCheckBox();
    // Contrôle bool Chiffre;
    QCheckBox* CtrlChiffre = new QCheckBox();
    // Contrôle qint16 LongueurMaximale;
    QSpinBox* CtrlLongueurMaximale = new QSpinBox();
    // Contrôle QString Question;
    QLineEdit* CtrlQuestion = new QLineEdit();
    // Contrôle QString QuestionAide;
    QLineEdit* CtrlQuestionAide = new QLineEdit();
    // Contrôle QString CheminImage;
    QLineEdit* CtrlCheminImage = new QLineEdit();
    // Contrôle QString Commentaire;
    QTextEdit* CtrlCommentaire = new QTextEdit();
    // Contrôle bool UtiliseCouleurPrincipale;
    QCheckBox* CtrlUtiliseCouleurPrincipale = new QCheckBox();
    // Contrôle bool UtiliseCouleurAccent;
    QCheckBox* CtrlUtiliseCouleurAccent = new QCheckBox();
    // Contrôle bool UtiliseCouleurPrincipale2;
    QCheckBox* CtrlUtiliseCouleurPrincipaleRemplissage = new QCheckBox();
    // Contrôle bool UtiliseCouleurAccent2;
    QCheckBox* CtrlUtiliseCouleurAccentRemplissage = new QCheckBox();
    // Contrôle QPushButton BoutonSupprimer;
    QPushButton* CtrlBoutonSupprimer = new QPushButton();
    // Contrôle QPushButton BoutonDeplaceHaut;
    QPushButton* CtrlBoutonDeplaceHaut = new QPushButton();
    // Contrôle QPushButton BoutonDeplacePremier;
    QPushButton* CtrlBoutonDeplacePremier = new QPushButton();
    // Contrôle QPushButton BoutonDeplaceBas;
    QPushButton* CtrlBoutonDeplaceBas = new QPushButton();
    // Contrôle QPushButton BoutonDeplaceFin;
    QPushButton* CtrlBoutonDeplaceFin = new QPushButton();
    // Contrôle QPushButton BoutonPressePapier;
    QPushButton* CtrlBoutonPressePapier = new QPushButton();
    // Contrôle QString Texte;
    QLineEdit* CtrlNomVariable = new QLineEdit();

    QSpacerItem* mSpacer;
};

#endif // BLOCEDITEUR_H
