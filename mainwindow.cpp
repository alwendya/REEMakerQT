/* -------------------------------------------------------------------------- */
/*      REEMaker 5 -- Grégory WENTZEL (c) 2023   Code sous licence GPL3       */
/* -------------------------------------------------------------------------- */

#include "mainwindow.h"
#include "ui_mainwindow.h"

/** MainWindow:
    Initialisation de la fenêtre principale

    @param QWidget* parent
    @return Aucun
*/
MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ModeAjoutPDG = false;
    /* -- Lecture donnée utilisateur -- */
    LectureINI();
    /* -- Mise en place des dossiers des programmes et des pages de gardes -- */
    MiseEnPlaceChemin();
    /* -- Mise en place des validators MAJUSCULE -- */
    MiseEnPlaceValidator();
    /* -- Mise en place de l'interface (A besoin de la lecture INI avant!!!) -- */
    MiseEnPlaceInterface();
    /* -- Contrôle de l'intégrité des fichiers -- */
    CheckIntegrite();
    /* -- Setting de la liste -- */
    MiseEnPlaceListInfo();
    Consigne("REEMaker a démarré avec succès");
    // Gestion multiInstance
    GestionMultiInstance();
    /* -- Démarrage de la recherche de mise à jour + création signal -- */
    if (!ui->OPT_Check_MAJ->isChecked()) { // Décocher, on peut chercher une MAJ
        connect(
          this,
          &MainWindow::LanceUneMAJ,
          this,
          [&](QString Version, QUrl Chemin) {
              if (MGBoxContinuerAnnuler("Version " + Version.mid(2) + " disponible !",
                                        "<b>Une mise à jour est disponible.<br>Voulez-vous la télécharger et l'installer ?</b>",
                                        QMessageBox::Icon::Question,
                                        "<i>L'application sera redémarrée pour procéder à la mise à jour</i>") == ReponseMGBOX::repond_Continuer)
                  DemarreLeTelechargement(Chemin);
          },
          Qt::QueuedConnection);
        QThread* thread = QThread::create([&] {
            QThread::msleep(10000);

            auto Reponse = DerniereVersion();
            if (Reponse == "Timeout") {
                this->setWindowTitle(this->windowTitle() + " [Erreur à la récupération de la dernière version disponible]");
                Consigne("Impossible d'atteindre le serveur de mise à jour (https://github.com/alwendya/REEMakerQT).");
            } else if (Reponse.startsWith("v.", Qt::CaseInsensitive)) {
                QString cReponse         = Reponse;
                QString cmVersion        = mVersion;
                qint16 VersionServeurINT = QString(cReponse.replace("v.", "").replace(".", "")).toInt();
                qint16 VersionLocal      = QString(cmVersion.replace(".", "")).toInt();
                Consigne(QString("Version interne : %1, Version disponible : %2, la mise à jour %3 nécessaire")
                           .arg(QString::number(VersionLocal),
                                QString::number(VersionServeurINT),
                                QString((VersionServeurINT > VersionLocal) ? "est" : "n'est pas")),
                         false,
                         false,
                         true);
                if (VersionServeurINT > VersionLocal) {
                    this->setWindowTitle(this->windowTitle() + " [Une mise à jour est disponible : " + Reponse.toUpper() + "]");
                    Consigne("Une mise à jour [" + Reponse + "] a été trouvée.");
                    emit LanceUneMAJ(Reponse, QUrl("https://github.com/alwendya/REEMakerQT/releases/download/" + Reponse + "/REEMaker.Update.7z"));
                } else
                    this->setWindowTitle(this->windowTitle() + " [Vous êtes à jour]");
            } else {
                qDebug().noquote().nospace() << "On devrait pas être la...";
            }
        });
        if (QFile::exists(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/update.REEMaker.7z")) {
            QFile::remove(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/update.REEMaker.7z");
            Consigne("Mise à jour effectuée avec succès !");
            QString Commentaire = "";
            if (QFile::exists(QCoreApplication::applicationDirPath() + "/Commentaire de mise à jour.txt")) // Fichier de MAJ existe
            {
                QFile file(QCoreApplication::applicationDirPath() + "/Commentaire de mise à jour.txt");
                if (file.open(QIODevice::ReadOnly)) {
                    QByteArray blob = file.readAll();
                    Commentaire     = "\n\n" + QString(blob);
                    file.close();
                }
                QFile::remove(QCoreApplication::applicationDirPath() + "/Commentaire de mise à jour.txt");
            }
            MGBoxContinuer("Information", "<b>Mise à jour effectué avec succès !</b>", QMessageBox::Icon::Information, "", Commentaire);
        }
        thread->start();
    }
    /* -- On fait le lien entre PDGHelper et MainWindow -- */
    QObject::connect(&mPDGHelper, &PDGHelper::EnvoiLogMessage, this, &MainWindow::RecoisLogMessage);
}

/** ~MainWindow:
    Déchargement de la fenetre principale

    @return Aucun
*/
MainWindow::~MainWindow()
{
    /* -- Suppression du dossier TEMP -- */
    QDir(CheminTemp).removeRecursively();
    /* -- Sauvegarde des données utilisateurs -- */
    SauveINI();
    /* -- Deletion de l'UI -- */
    delete ui;
}

/** on_OPT_Btn_ColTranche0_clicked:
    Permet de choisir la couleur principale pour la tranche 0

    @return Aucun
*/
void
MainWindow::on_OPT_Btn_ColTranche0_clicked()
{
    MACRO_COLOR(ui->OPT_Btn_ColTranche0, ui->OPT_BText_CoulTranche_0);
}

/** on_OPT_Btn_ColTranche1_clicked:
    Permet de choisir la couleur principale pour la tranche 1

    @return Aucun
*/
void
MainWindow::on_OPT_Btn_ColTranche1_clicked()
{
    MACRO_COLOR(ui->OPT_Btn_ColTranche1, ui->OPT_BText_CoulTranche_1);
}

/** on_OPT_Btn_ColTranche2_clicked:
    Permet de choisir la couleur principale pour la tranche 2

    @return Aucun
*/
void
MainWindow::on_OPT_Btn_ColTranche2_clicked()
{
    MACRO_COLOR(ui->OPT_Btn_ColTranche2, ui->OPT_BText_CoulTranche_2);
}

/** on_OPT_Btn_ColTranche3_clicked:
    Permet de choisir la couleur principale pour la tranche 3

    @return Aucun
*/
void
MainWindow::on_OPT_Btn_ColTranche3_clicked()
{
    MACRO_COLOR(ui->OPT_Btn_ColTranche3, ui->OPT_BText_CoulTranche_3);
}

/** on_OPT_Btn_ColTranche4_clicked:
    Permet de choisir la couleur principale pour la tranche 4

    @return Aucun
*/
void
MainWindow::on_OPT_Btn_ColTranche4_clicked()
{
    MACRO_COLOR(ui->OPT_Btn_ColTranche4, ui->OPT_BText_CoulTranche_4);
}

/** on_OPT_Btn_ColTranche5_clicked:
    Permet de choisir la couleur principale pour la tranche 5

    @return Aucun
*/
void
MainWindow::on_OPT_Btn_ColTranche5_clicked()
{
    MACRO_COLOR(ui->OPT_Btn_ColTranche5, ui->OPT_BText_CoulTranche_5);
}

/** on_OPT_Btn_ColTranche6_clicked:
    Permet de choisir la couleur principale pour la tranche 6

    @return Aucun
*/
void
MainWindow::on_OPT_Btn_ColTranche6_clicked()
{
    MACRO_COLOR(ui->OPT_Btn_ColTranche6, ui->OPT_BText_CoulTranche_6);
}

/** on_OPT_Btn_ColTranche7_clicked:
    Permet de choisir la couleur principale pour la tranche 7

    @return Aucun
*/
void
MainWindow::on_OPT_Btn_ColTranche7_clicked()
{
    MACRO_COLOR(ui->OPT_Btn_ColTranche7, ui->OPT_BText_CoulTranche_7);
}

/** on_OPT_Btn_ColTranche8_clicked:
    Permet de choisir la couleur principale pour la tranche 8

    @return Aucun
*/
void
MainWindow::on_OPT_Btn_ColTranche8_clicked()
{
    MACRO_COLOR(ui->OPT_Btn_ColTranche8, ui->OPT_BText_CoulTranche_8);
}

/** on_OPT_Btn_ColTranche9_clicked:
    Permet de choisir la couleur principale pour la tranche 9

    @return Aucun
*/
void
MainWindow::on_OPT_Btn_ColTranche9_clicked()
{
    MACRO_COLOR(ui->OPT_Btn_ColTranche9, ui->OPT_BText_CoulTranche_9);
}

/** on_OPT_Btn_ColAccent0_clicked:
    Permet de choisir la couleur d'accentuation pour la tranche 0

    @return Aucun
*/
void
MainWindow::on_OPT_Btn_ColAccent0_clicked()
{
    MACRO_COLOR(ui->OPT_Btn_ColAccent0, ui->OPT_BText_CoulAccent_0);
}

/** on_OPT_Btn_ColAccent1_clicked:
    Permet de choisir la couleur d'accentuation pour la tranche 1

    @return Aucun
*/
void
MainWindow::on_OPT_Btn_ColAccent1_clicked()
{
    MACRO_COLOR(ui->OPT_Btn_ColAccent1, ui->OPT_BText_CoulAccent_1);
}

/** on_OPT_Btn_ColAccent2_clicked:
    Permet de choisir la couleur d'accentuation pour la tranche 2

    @return Aucun
*/
void
MainWindow::on_OPT_Btn_ColAccent2_clicked()
{
    MACRO_COLOR(ui->OPT_Btn_ColAccent2, ui->OPT_BText_CoulAccent_2);
}

/** on_OPT_Btn_ColAccent3_clicked:
    Permet de choisir la couleur d'accentuation pour la tranche 3

    @return Aucun
*/
void
MainWindow::on_OPT_Btn_ColAccent3_clicked()
{
    MACRO_COLOR(ui->OPT_Btn_ColAccent3, ui->OPT_BText_CoulAccent_3);
}

/** on_OPT_Btn_ColAccent4_clicked:
    Permet de choisir la couleur d'accentuation pour la tranche 4

    @return Aucun
*/
void
MainWindow::on_OPT_Btn_ColAccent4_clicked()
{
    MACRO_COLOR(ui->OPT_Btn_ColAccent4, ui->OPT_BText_CoulAccent_4);
}

/** on_OPT_Btn_ColAccent5_clicked:
    Permet de choisir la couleur d'accentuation pour la tranche 5

    @return Aucun
*/
void
MainWindow::on_OPT_Btn_ColAccent5_clicked()
{
    MACRO_COLOR(ui->OPT_Btn_ColAccent5, ui->OPT_BText_CoulAccent_5);
}

/** on_OPT_Btn_ColAccent6_clicked:
    Permet de choisir la couleur d'accentuation pour la tranche 6

    @return Aucun
*/
void
MainWindow::on_OPT_Btn_ColAccent6_clicked()
{
    MACRO_COLOR(ui->OPT_Btn_ColAccent6, ui->OPT_BText_CoulAccent_6);
}

/** on_OPT_Btn_ColAccent7_clicked:
    Permet de choisir la couleur d'accentuation pour la tranche 7

    @return Aucun
*/
void
MainWindow::on_OPT_Btn_ColAccent7_clicked()
{
    MACRO_COLOR(ui->OPT_Btn_ColAccent7, ui->OPT_BText_CoulAccent_7);
}

/** on_OPT_Btn_ColAccent8_clicked:
    Permet de choisir la couleur d'accentuation pour la tranche 8

    @return Aucun
*/
void
MainWindow::on_OPT_Btn_ColAccent8_clicked()
{
    MACRO_COLOR(ui->OPT_Btn_ColAccent8, ui->OPT_BText_CoulAccent_8);
}

/** on_OPT_Btn_ColAccent9_clicked:
    Permet de choisir la couleur d'accentuation pour la tranche 9

    @return Aucun
*/
void
MainWindow::on_OPT_Btn_ColAccent9_clicked()
{
    MACRO_COLOR(ui->OPT_Btn_ColAccent9, ui->OPT_BText_CoulAccent_9);
}

/** on_Folioter_Btn_RechercherPDF_clicked:
    Sélection et ouverture du fichier PDF à folioter

    @return Aucun
*/
void
MainWindow::on_Folioter_Btn_RechercherPDF_clicked()
{
    PDFOuvert = QFileDialog::getOpenFileName(this, "Ouvrir un fichier PDF", "", "Fichier PDF (*.pdf)");
    if (PDFOuvert == "")
        return;
    ui->Folioter_Txt_RechercheProcedure->setText(PDFOuvert);
    ui->Folioter_Label_NombreFolioAnnuler->setText("Aucun folio annulé");
    qint16 NombrePage = PDFInfoNombrePage(PDFOuvert);
    Consigne("PDFInfo : Donné récupéré avec " + QString::number(NombrePage) + " pages");
    if (ui->OPT_Check_GhostScript->isChecked()) {
        auto CodeErreur = RepareGhostScript(PDFOuvert, NombrePage, false);
        switch (CodeErreur) {
            case ValeurRetour::AucunFichierEntree: {
                MGBoxContinuer("REEMaker - Erreur", "<b>Fichier d'entrée introuvable</b>", QMessageBox::Icon::Critical);
                Consigne("Erreur : Fichier d'entrée introuvable");
                ui->Folioter_Txt_RechercheProcedure->setText("");
                return;
            }
            case ValeurRetour::ErreurDeplacement: {
                MGBoxContinuer("REEMaker - Erreur", "<b>Erreur dans la sauvegarde du document PDF</b>", QMessageBox::Icon::Critical);
                Consigne("Erreur dans la sauvegarde du document PDF");
                ui->Folioter_Txt_RechercheProcedure->setText("");
                return;
            }
            case ValeurRetour::GhostScriptAbsent: {
                MGBoxContinuer(
                  "REEMaker - Erreur", "<b>Erreur dans les fichiers de Ghostscript, réinstaller REEMaker</b>", QMessageBox::Icon::Critical);
                Consigne("Erreur dans les fichiers de Ghostscript, réinstaller REEMaker");
                ui->Folioter_Txt_RechercheProcedure->setText("");
                return;
            }
            case ValeurRetour::ErreurDeGhostScript: {
                MGBoxContinuer("REEMaker - Attention",
                               "<b>Ghostscript n'a pu réparer le document.\nREEMaker va continuer avec le document PDF original.</b>",
                               QMessageBox::Icon::Information);
                Consigne("Ghostscript n'a pu réparer le document.\nREEMaker va continuer avec le document PDF original");
                break;
            }
            case ValeurRetour::Succes: // L'opération à réussi
                Consigne("GhostScript à terminé sans erreur");
                break;
            default: // L'opération à réussi
                Consigne("GhostScript à terminé sans erreur");
                break;
        }
        Consigne("Ouverture de '" + QFileInfo(PDFOuvert).fileName() + "' réussi.");
    }
    QProgressDialog progress("Ouverture du fichier PDF...", "", 0, 0, this);
    if (ThemeDark)
        progress.setStyle(new DarkStyle);
    progress.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMaximum(1);
    progress.setAutoClose(false);
    progress.setCancelButton(0);
    progress.setWindowIcon(this->windowIcon());
    progress.installEventFilter(keyPressEater);
    progress.show();
    {
        RECT FenetrePrincipale;
        GetWindowRect((HWND)this->winId(), &FenetrePrincipale);
        progress.setGeometry(FenetrePrincipale.left + (this->geometry().width() / 2) - (progress.width() / 2),
                             FenetrePrincipale.top + (this->geometry().height() / 2) - (progress.height() / 2),
                             progress.width(),
                             progress.height());
    }
    // CentreHWND((HWND)progress.winId());
    QCoreApplication::processEvents();
    try {
        vecMediaBox.clear();
        vecRotation.clear();
        vecFolioAnnuler.clear();
        PoDoFo::PdfMemDocument documentSource(PDFOuvert.toStdWString().c_str());
        NombrePages = documentSource.GetPageCount();
        progress.setMaximum(NombrePages);
        progress.setValue(0);
        for (int i = 0; i < NombrePages; i++) {
            progress.setValue(i);
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            PoDoFo::PdfPage* pPage = documentSource.GetPage(i);
            vecMediaBox.push_back(pPage->GetMediaBox());
            vecRotation.push_back(pPage->GetRotation());
            vecFolioAnnuler.push_back(false);
        }
        ui->Folioter_Group_RefTampon->setVisible(false);
        ui->Folioter_Group_Etendue->setVisible(false);
        ui->Folioter_Group_Annuler->setVisible(false);
        ui->Folioter_Group_ChoixTranche->setVisible(false);
        ui->Folioter_Btn_EtapeSuivante1->setEnabled(true);
        ui->Folioter_Btn_EtapeSuivante2->setEnabled(false);
    } catch (const PoDoFo::PdfError& e) {
        MiseEnPlaceListInfo();
        if (e.GetError() == PoDoFo::ePdfError_FileNotFound) {
            ui->FolioListeINFO->addItem("Information sur le fichier PDF :");
            ui->FolioListeINFO->addItem("Erreur : Le fichier est introuvable");
            Consigne("Erreur : Le fichier est introuvable");
        } else if (e.GetError() == PoDoFo::ePdfError_BrokenFile) {
            ui->FolioListeINFO->addItem("Information sur le fichier PDF :");
            ui->FolioListeINFO->addItem("Erreur : Le fichier est endommagé, essayez d'activer le contrôle "
                                        "par GhostScript dans les options");
            Consigne("Erreur : Le fichier est endommagé, essayez d'activer le "
                     "contrôle par GhostScript dans les options");
        } else if (e.GetError() == PoDoFo::ePdfError_NoPdfFile) {
            ui->FolioListeINFO->addItem("Information sur le fichier PDF :");
            ui->FolioListeINFO->addItem("Erreur : Le fichier n'est pas un fichier PDF");
            Consigne("Erreur : Le fichier n'est pas un fichier PDF");
        } else {
            ui->FolioListeINFO->addItem("Information sur le fichier PDF :");
            ui->FolioListeINFO->addItem(QString("Erreur à l'ouverture : %1").arg(QString::fromStdString(e.what())));
            Consigne(QString("Erreur à l'ouverture : %1").arg(QString::fromStdString(e.what())));
        }
    }
    ModeAjoutPDG = false;
    progress.close();
}

/** on_Folioter_Btn_EtapeSuivante1_clicked:
    Passage à l'étape suivante

    @return Aucun
*/
void
MainWindow::on_Folioter_Btn_EtapeSuivante1_clicked()
{
    ui->Folioter_Group_RefTampon->setVisible(true);
    ui->Folioter_Group_RefTampon->setEnabled(true);
    ui->Folioter_Btn_EtapeSuivante1->setEnabled(false);
    ui->Folioter_Btn_EtapeSuivante2->setEnabled(false);
    ui->Folioter_Txt_NomDuSite->setFocus();
    if ((ui->Folioter_Txt_NomDuSite->text().length() > 0) && (ui->Folioter_Txt_RefREE->text().length() > 0) &&
        (ui->Folioter_Txt_IndiceREE->text().length() > 0)) {
        ui->Folioter_Btn_EtapeSuivante2->setEnabled(true);
        ui->Folioter_Btn_EtapeSuivante2->setFocus();
    }
    ui->Folioter_Btn_RechercherPDF->setText("    Recommencer");
}

/** on_Folioter_Txt_NomDuSite_textChanged:
    Permet de faire apparaitre le bouton d'étape suivante

    @param const QString& arg1
    @return Aucun
*/
void
MainWindow::on_Folioter_Txt_NomDuSite_textChanged(const QString& arg1)
{
    Q_UNUSED(arg1)
    if ((ui->Folioter_Txt_NomDuSite->text() != "") && (ui->Folioter_Txt_RefREE->text()) != "" && (ui->Folioter_Txt_IndiceREE->text() != ""))
        ui->Folioter_Btn_EtapeSuivante2->setEnabled(true);
    else
        ui->Folioter_Btn_EtapeSuivante2->setEnabled(false);
}

/** on_Folioter_Txt_RefREE_textChanged:
    Permet de faire apparaitre le bouton d'étape suivante

    @param const QString& arg1
    @return Aucun
*/
void
MainWindow::on_Folioter_Txt_RefREE_textChanged(const QString& arg1)
{
    Q_UNUSED(arg1)
    if ((ui->Folioter_Txt_NomDuSite->text() != "") && (ui->Folioter_Txt_RefREE->text()) != "" && (ui->Folioter_Txt_IndiceREE->text() != ""))
        ui->Folioter_Btn_EtapeSuivante2->setEnabled(true);
    else
        ui->Folioter_Btn_EtapeSuivante2->setEnabled(false);
}

/** on_Folioter_Txt_IndiceREE_textChanged:
    Permet de faire apparaitre le bouton d'étape suivante

    @param const QString& arg1
    @return Aucun
*/
void
MainWindow::on_Folioter_Txt_IndiceREE_textChanged(const QString& arg1)
{
    Q_UNUSED(arg1)
    if ((ui->Folioter_Txt_NomDuSite->text() != "") && (ui->Folioter_Txt_RefREE->text()) != "" && (ui->Folioter_Txt_IndiceREE->text() != ""))
        ui->Folioter_Btn_EtapeSuivante2->setEnabled(true);
    else
        ui->Folioter_Btn_EtapeSuivante2->setEnabled(false);
}

/** on_Folioter_Btn_EtapeSuivante2_clicked:
    Passage à l'étape suivante

    @return Aucun
*/
void
MainWindow::on_Folioter_Btn_EtapeSuivante2_clicked()
{
    ui->Folioter_Spin_Partiel_Debut->setMinimum(1);
    ui->Folioter_Spin_Partiel_Debut->setMaximum(NombrePages);
    ui->Folioter_Spin_Partiel_Debut->setValue(1);
    ui->Folioter_Spin_Partiel_Fin->setMinimum(1);
    ui->Folioter_Spin_Partiel_Fin->setMaximum(NombrePages);
    ui->Folioter_Spin_Partiel_Fin->setValue(NombrePages);
    ui->Folioter_Spin_PremierePage->setMaximum(99999);

    ui->Folioter_Group_Etendue->setEnabled(true);
    ui->Folioter_Group_Etendue->setVisible(true);
    ui->Folioter_Group_RefTampon->setEnabled(false);
}

/** on_Folioter_Btn_EtapeSuivante3_clicked:
    Passage à l'étape suivante

    @return Aucun
*/
void
MainWindow::on_Folioter_Btn_EtapeSuivante3_clicked()
{
    ui->Folioter_Group_Etendue->setEnabled(false);
    ui->Folioter_Group_Annuler->setVisible(true);
    ui->Folioter_Group_Annuler->setEnabled(true);
}

/** on_Folioter_Btn_EtapeSuivante4_clicked:
    Passage à l'étape suivante

    @return Aucun
*/
void
MainWindow::on_Folioter_Btn_EtapeSuivante4_clicked()
{
    ui->Folioter_Group_ChoixTranche->setEnabled(true);
    ui->Folioter_Group_ChoixTranche->setVisible(true);
    ui->Folioter_Group_Annuler->setEnabled(false);
    {
        int NbCheck = 0;
        if (ui->Folioter_Check_Tranche0->isChecked())
            NbCheck++;
        if (ui->Folioter_Check_Tranche1->isChecked())
            NbCheck++;
        if (ui->Folioter_Check_Tranche2->isChecked())
            NbCheck++;
        if (ui->Folioter_Check_Tranche3->isChecked())
            NbCheck++;
        if (ui->Folioter_Check_Tranche4->isChecked())
            NbCheck++;
        if (ui->Folioter_Check_Tranche5->isChecked())
            NbCheck++;
        if (ui->Folioter_Check_Tranche6->isChecked())
            NbCheck++;
        if (ui->Folioter_Check_Tranche7->isChecked())
            NbCheck++;
        if (ui->Folioter_Check_Tranche8->isChecked())
            NbCheck++;
        if (ui->Folioter_Check_Tranche9->isChecked())
            NbCheck++;
        if (NbCheck > 0) {
            ui->Folioter_Btn_FoliotageAvecPDG->setEnabled(true);
            ui->Folioter_Btn_FoliotageSansPDG->setEnabled(true);
        } else {
            ui->Folioter_Btn_FoliotageAvecPDG->setEnabled(false);
            ui->Folioter_Btn_FoliotageSansPDG->setEnabled(false);
        }
    }
}

/** on_Folioter_Check_Tranche1_toggled:
    Permet de faire apparaitre / disparaitre le démarrage du foliotage

    @param bool checked
    @return Aucun
*/
void
MainWindow::on_Folioter_Check_Tranche1_toggled(bool checked)
{
    Q_UNUSED(checked);
    int NbCheck = 0;
    if (ui->Folioter_Check_Tranche0->isChecked())
        NbCheck++;
    if (ui->Folioter_Check_Tranche1->isChecked())
        NbCheck++;
    if (ui->Folioter_Check_Tranche2->isChecked())
        NbCheck++;
    if (ui->Folioter_Check_Tranche3->isChecked())
        NbCheck++;
    if (ui->Folioter_Check_Tranche4->isChecked())
        NbCheck++;
    if (ui->Folioter_Check_Tranche5->isChecked())
        NbCheck++;
    if (ui->Folioter_Check_Tranche6->isChecked())
        NbCheck++;
    if (ui->Folioter_Check_Tranche7->isChecked())
        NbCheck++;
    if (ui->Folioter_Check_Tranche8->isChecked())
        NbCheck++;
    if (ui->Folioter_Check_Tranche9->isChecked())
        NbCheck++;
    if (NbCheck > 0) {
        ui->Folioter_Btn_FoliotageAvecPDG->setEnabled(true);
        ui->Folioter_Btn_FoliotageSansPDG->setEnabled(true);
    } else {
        ui->Folioter_Btn_FoliotageAvecPDG->setEnabled(false);
        ui->Folioter_Btn_FoliotageSansPDG->setEnabled(false);
    }
    ui->Folioter_Texte_Tranche0->setEnabled(ui->Folioter_Check_Tranche0->isChecked());
    ui->Folioter_Texte_Tranche1->setEnabled(ui->Folioter_Check_Tranche1->isChecked());
    ui->Folioter_Texte_Tranche2->setEnabled(ui->Folioter_Check_Tranche2->isChecked());
    ui->Folioter_Texte_Tranche3->setEnabled(ui->Folioter_Check_Tranche3->isChecked());
    ui->Folioter_Texte_Tranche4->setEnabled(ui->Folioter_Check_Tranche4->isChecked());
    ui->Folioter_Texte_Tranche5->setEnabled(ui->Folioter_Check_Tranche5->isChecked());
    ui->Folioter_Texte_Tranche6->setEnabled(ui->Folioter_Check_Tranche6->isChecked());
    ui->Folioter_Texte_Tranche7->setEnabled(ui->Folioter_Check_Tranche7->isChecked());
    ui->Folioter_Texte_Tranche8->setEnabled(ui->Folioter_Check_Tranche8->isChecked());
    ui->Folioter_Texte_Tranche9->setEnabled(ui->Folioter_Check_Tranche9->isChecked());
}

/** on_Folioter_Check_Tranche3_toggled:
    Permet de faire apparaitre / disparaitre le démarrage du foliotage

    @param bool checked
    @return Aucun
*/
void
MainWindow::on_Folioter_Check_Tranche3_toggled(bool checked)
{
    on_Folioter_Check_Tranche1_toggled(checked);
}

/** on_Folioter_Check_Tranche5_toggled:
    Permet de faire apparaitre / disparaitre le démarrage du foliotage

    @param bool checked
    @return Aucun
*/
void
MainWindow::on_Folioter_Check_Tranche5_toggled(bool checked)
{
    on_Folioter_Check_Tranche1_toggled(checked);
}

/** on_Folioter_Check_Tranche2_toggled:
    Permet de faire apparaitre / disparaitre le démarrage du foliotage

    @param bool checked
    @return Aucun
*/
void
MainWindow::on_Folioter_Check_Tranche2_toggled(bool checked)
{
    on_Folioter_Check_Tranche1_toggled(checked);
}

/** on_Folioter_Check_Tranche4_toggled:
    Permet de faire apparaitre / disparaitre le démarrage du foliotage

    @param bool checked
    @return Aucun
*/
void
MainWindow::on_Folioter_Check_Tranche4_toggled(bool checked)
{
    on_Folioter_Check_Tranche1_toggled(checked);
}

/** on_Folioter_Check_Tranche6_toggled:
    Permet de faire apparaitre / disparaitre le démarrage du foliotage

    @param bool checked
    @return Aucun
*/
void
MainWindow::on_Folioter_Check_Tranche6_toggled(bool checked)
{
    on_Folioter_Check_Tranche1_toggled(checked);
}

/** on_Folioter_Check_Tranche0_toggled:
    Permet de faire apparaitre / disparaitre le démarrage du foliotage

    @param bool checked
    @return Aucun
*/
void
MainWindow::on_Folioter_Check_Tranche0_toggled(bool checked)
{
    on_Folioter_Check_Tranche1_toggled(checked);
}

/** on_Folioter_Check_Tranche9_toggled:
    Permet de faire apparaitre / disparaitre le démarrage du foliotage

    @param bool checked
    @return Aucun
*/
void
MainWindow::on_Folioter_Check_Tranche9_toggled(bool checked)
{
    on_Folioter_Check_Tranche1_toggled(checked);
}

/** on_Folioter_Check_Tranche8_toggled:
    Permet de faire apparaitre / disparaitre le démarrage du foliotage

    @param bool checked
    @return Aucun
*/
void
MainWindow::on_Folioter_Check_Tranche8_toggled(bool checked)
{
    on_Folioter_Check_Tranche1_toggled(checked);
}

/** on_Folioter_Check_Tranche7_toggled:
    Permet de faire apparaitre / disparaitre le démarrage du foliotage

    @param bool checked
    @return Aucun
*/
void
MainWindow::on_Folioter_Check_Tranche7_toggled(bool checked)
{
    on_Folioter_Check_Tranche1_toggled(checked);
}

/** on_Folioter_Btn_ChoixFolioAnnuler_clicked:
    Permet de faire apparaitre l'onglet de choix de folio à annuler

    @return Aucun
*/
void
MainWindow::on_Folioter_Btn_ChoixFolioAnnuler_clicked()
{
    ui->Annulation_Liste->setRowCount(0);
    ui->Annulation_LabelPrevisualisation->setPixmap(QPixmap());
    ui->tabWidget->setTabVisible(0, false);
    ui->tabWidget->setTabVisible(1, true);
    ui->tabWidget->setTabVisible(2, false);
    ui->tabWidget->setTabVisible(3, false);
    ui->tabWidget->setTabVisible(5, false);
    ui->tabWidget->setCurrentIndex(1);

    qint64 PageDebut = 1;
    qint64 PageFin   = NombrePages;
    if (ui->Folioter_Radio_FolioPartiel->isChecked()) {
        PageDebut = ui->Folioter_Spin_Partiel_Debut->value();
        PageFin   = ui->Folioter_Spin_Partiel_Fin->value();
    }
    auto procPPM            = new QProcess();
    QString PathOutputImage = QFileInfo(CheminTemp).filePath() + "/_64_";
    procPPM->setWorkingDirectory(QFileInfo(CheminPoppler).path());
    procPPM->setProgram(CheminPoppler);
    procPPM->setArguments(
      { "-f", QString::number(PageDebut), "-l", QString::number(PageFin), "-q", "-r", "10", "-scale-to", "64", "-png", PDFOuvert, PathOutputImage });
    QProgressDialog progress("Génération des miniatures...", "Annuler", 0, 0, this);
    progress.setStyle(new DarkStyle);
    progress.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    progress.setWindowModality(Qt::WindowModal);
    progress.setAutoClose(false);
    progress.setMaximum(PageFin - PageDebut + 1);
    progress.setWindowIcon(this->windowIcon());
    progress.installEventFilter(keyPressEater);
    progress.show();
    //    CentreHWND((HWND)progress.winId());
    {
        RECT FenetrePrincipale;
        GetWindowRect((HWND)this->winId(), &FenetrePrincipale);
        progress.setGeometry(FenetrePrincipale.left + (this->geometry().width() / 2) - (progress.width() / 2),
                             FenetrePrincipale.top + (this->geometry().height() / 2) - (progress.height() / 2),
                             progress.width(),
                             progress.height());
    }
    QCoreApplication::processEvents();
    procPPM->start();
    while (procPPM->state() != QProcess::NotRunning) {
        QThread::msleep(250);
        QCoreApplication::processEvents();
        QDir directory(QFileInfo(CheminTemp).filePath());
        QStringList images = directory.entryList(QStringList() << "_64_*.png", QDir::Files, QDir::Name | QDir::IgnoreCase);
        progress.setValue(images.count());
        if (progress.wasCanceled()) {
            procPPM->kill();
            procPPM->waitForFinished(-1);
            images = directory.entryList(QStringList() << "_64_*.png", QDir::Files, QDir::Name | QDir::IgnoreCase);
            foreach (QString filename, images) {
                directory.remove(filename);
            }
            ui->tabWidget->setTabVisible(0, true);
            ui->tabWidget->setTabVisible(1, false);
            ui->tabWidget->setTabVisible(2, true);
            ui->tabWidget->setTabVisible(3, true);
            ui->tabWidget->setTabVisible(5, true);
            ui->tabWidget->setCurrentIndex(0);
            progress.close();
            return;
        }
    }
    procPPM->close();
    progress.close();
    Consigne("Fin de génération des miniatures par Poppler");
    QDir directory(QFileInfo(CheminTemp).filePath());
    QStringList images = directory.entryList(QStringList() << "_64_*.png", QDir::Files, QDir::Name | QDir::IgnoreCase);
    vecFolioAnnuler.clear();
    vecFolioAnnuler.resize(vecMediaBox.count()); // On remet tout à false
    ui->Annulation_Liste->setRowCount(images.count());
    for (int var = 0; var < images.count(); ++var) {
        images[var]            = QFileInfo(CheminTemp).filePath() + "/" + images[var];
        QTableWidgetItem* item = new QTableWidgetItem;
        item->setIcon(QIcon(images[var]));
        item->setText("Page " + QFileInfo(images[var]).baseName().mid(5));
        ui->Annulation_Liste->setRowHeight(var, 74);
        ui->Annulation_Liste->setItem(var, 1, item);
        QCheckBox* mItem = new QCheckBox();
        mItem->setObjectName("QCheckbox" + QString::number(var));
        mItem->setChecked(false);
        mItem->setText("");
        ui->Annulation_Liste->setCellWidget(var, 0, mItem);
    }
}

/** on_Annulation_Btn_Valider_clicked:
    Validation des folios à annuler

    @return Aucun
*/
void
MainWindow::on_Annulation_Btn_Valider_clicked()
{
    ui->tabWidget->setTabVisible(0, true);
    ui->tabWidget->setTabVisible(1, false);
    ui->tabWidget->setTabVisible(2, true);
    ui->tabWidget->setTabVisible(3, true);
    ui->tabWidget->setTabVisible(5, true);
    ui->tabWidget->setCurrentIndex(0);
    QDir directory(QFileInfo(CheminTemp).filePath());
    QStringList images = directory.entryList(QStringList() << "_64_*.png"
                                                           << "_HR_*.png",
                                             QDir::Files,
                                             QDir::Name | QDir::IgnoreCase);
    foreach (QString filename, images) {
        directory.remove(filename);
    }
    int NombreAnnule = 0;
    for (int var = 0; var < ui->Annulation_Liste->rowCount(); ++var) {
        QCheckBox* mCheck = qobject_cast<QCheckBox*>(ui->Annulation_Liste->cellWidget(var, 0));
        vecFolioAnnuler[var + (ui->Folioter_Radio_FolioPartiel->isChecked() ? ui->Folioter_Spin_Partiel_Debut->value() - 1 : 0)] =
          mCheck->isChecked();
        if (mCheck->isChecked())
            NombreAnnule++;
    }
    ui->Folioter_Label_NombreFolioAnnuler->setText(
      QString("%1 folio%2 annulé%3").arg(QString::number(NombreAnnule), (NombreAnnule > 1) ? "s" : "", (NombreAnnule > 1) ? "s" : ""));
    ui->Annulation_Liste->setRowCount(0);
    ui->Annulation_LabelPrevisualisation->setPixmap(QPixmap());
    Consigne("Annulation de " + QString::number(NombreAnnule) + " folios");
}

/** on_Annulation_Btn_NePasAnnuler_clicked:
    Fermeture de la fenêtre de choix de folio ) annuler sans aucun choix

    @return Aucun
*/
void
MainWindow::on_Annulation_Btn_NePasAnnuler_clicked()
{
    ui->tabWidget->setTabVisible(0, true);
    ui->tabWidget->setTabVisible(1, false);
    ui->tabWidget->setTabVisible(2, true);
    ui->tabWidget->setTabVisible(3, true);
    ui->tabWidget->setTabVisible(5, true);
    ui->tabWidget->setCurrentIndex(0);
    QDir directory(QFileInfo(CheminTemp).filePath());
    QStringList images = directory.entryList(QStringList() << "_64_*.png"
                                                           << "_HR_*.png",
                                             QDir::Files,
                                             QDir::Name | QDir::IgnoreCase);
    foreach (QString filename, images) {
        directory.remove(filename);
    }
    ui->Annulation_Liste->setRowCount(0);
    ui->Annulation_LabelPrevisualisation->setPixmap(QPixmap());
    vecFolioAnnuler.clear();
    vecFolioAnnuler.resize(vecRotation.count()); // On remet tout à false
    ui->Folioter_Label_NombreFolioAnnuler->setText("Aucun folio annulé");
    Consigne("Aucun folio n'est annulé");
}

/** on_Annulation_Liste_cellClicked:
    Permet de charger une miniature de meilleur qualité quand une ligne est
   cliquée

    @param int row
    @param int column
    @return Aucun
*/
void
MainWindow::on_Annulation_Liste_cellClicked(int row, int column)
{
    Q_UNUSED(column);
    int NumeroPage          = ui->Annulation_Liste->item(row, /*column*/ 1)->text().mid(5).toInt();
    auto procPPM            = new QProcess();
    QString PathOutputImage = QFileInfo(CheminTemp).filePath() + "/_HR_";
    procPPM->setWorkingDirectory(QFileInfo(CheminPoppler).path());
    procPPM->setProgram(CheminPoppler); // QDir::toNativeSeparators
    procPPM->setArguments({
      "-f",
      QString::number(NumeroPage),
      "-l",
      QString::number(NumeroPage),
      "-q",
      "-r",
      "100",
      "-png",
      /*QDir::toNativeSeparators(*/ PDFOuvert /*)*/,
      /*QDir::toNativeSeparators(*/ PathOutputImage /*)*/
    });
    QProgressDialog progress("Préparation de l'image...", "Annuler", 0, 0, this);
    progress.setStyle(new DarkStyle);
    progress.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMaximum(0);
    progress.setAutoClose(false);
    progress.setCancelButton(0);
    progress.show();
    //    CentreHWND((HWND)progress.winId());
    {
        RECT FenetrePrincipale;
        GetWindowRect((HWND)this->winId(), &FenetrePrincipale);
        progress.setGeometry(FenetrePrincipale.left + (this->geometry().width() / 2) - (progress.width() / 2),
                             FenetrePrincipale.top + (this->geometry().height() / 2) - (progress.height() / 2),
                             progress.width(),
                             progress.height());
    }
    progress.setWindowIcon(this->windowIcon());
    progress.installEventFilter(keyPressEater);
    QCoreApplication::processEvents();

    procPPM->start();
    while (procPPM->state() != QProcess::NotRunning) {
        QThread::msleep(150);
        QCoreApplication::processEvents();
        if (progress.wasCanceled()) {
            procPPM->kill();
            procPPM->waitForFinished(-1);
            progress.close();
            return;
        }
    }
    QDir directory(QFileInfo(CheminTemp).filePath());
    QStringList images = directory.entryList(QStringList() << "_HR_*.png", QDir::Files, QDir::Name | QDir::IgnoreCase);
    if (images.count() > 0) {
        QPXpreview = QPixmap(QFileInfo(CheminTemp).filePath() + "/" + images[0]);
        ui->Annulation_LabelPrevisualisation->setPixmap(QPXpreview.scaled(ui->Annulation_LabelPrevisualisation->width() - 10,
                                                                          ui->Annulation_LabelPrevisualisation->height() - 10,
                                                                          Qt::KeepAspectRatio,
                                                                          Qt::SmoothTransformation));
        directory.remove(images[0]);
    }
}

/** on_Annulation_Bouton_Tout_clicked:
    Sélectionne la totalité des checkbox des folios annulables

    @return Aucun
*/
void
MainWindow::on_Annulation_Bouton_Tout_clicked()
{
    for (int var = 0; var < ui->Annulation_Liste->rowCount(); ++var) {
        QCheckBox* mCheck = qobject_cast<QCheckBox*>(ui->Annulation_Liste->cellWidget(var, 0));
        mCheck->setChecked(true);
    }
}

/** on_Annulation_Bouton_Rien_clicked:
    Désélectionne toutes les checkbox des folios annulables

    @return Aucun
*/
void
MainWindow::on_Annulation_Bouton_Rien_clicked()
{
    for (int var = 0; var < ui->Annulation_Liste->rowCount(); ++var) {
        QCheckBox* mCheck = qobject_cast<QCheckBox*>(ui->Annulation_Liste->cellWidget(var, 0));
        mCheck->setChecked(false);
    }
}

/** on_MainWindow_destroyed:
    Déchargement de la fenêtre principale

    @return Aucun
*/
void
MainWindow::on_MainWindow_destroyed()
{
    Consigne("MainWindow détruite");
}
/** on_Folioter_Radio_FolioTotal_toggled:
  Logique de choix entre foliotage total et partiel

  @param bool checked
  @return Aucun
*/
void
MainWindow::on_Folioter_Radio_FolioTotal_toggled(bool checked)
{
    Q_UNUSED(checked);
    ui->label_6->setEnabled(false);
    ui->Folioter_Spin_Partiel_Debut->setEnabled(false);
    ui->label_7->setEnabled(false);
    ui->Folioter_Spin_Partiel_Fin->setEnabled(false);
}

/** on_Folioter_Radio_FolioPartiel_toggled:
  Logique de choix entre foliotage total et partiel

  @param bool checked
  @return Aucun
*/
void
MainWindow::on_Folioter_Radio_FolioPartiel_toggled(bool checked)
{
    Q_UNUSED(checked);
    ui->label_6->setEnabled(true);
    ui->Folioter_Spin_Partiel_Debut->setEnabled(true);
    ui->label_7->setEnabled(true);
    ui->Folioter_Spin_Partiel_Fin->setEnabled(true);
}

/** on_Folioter_Btn_FoliotageSansPDG_clicked:
  Lance l'opération de foliotage avec les différents paramètres
  Cette fonction peut aussi être appelé avec le Flag AjoutPDG pour insérer la
page de garde

@return Aucun
*/
void
MainWindow::on_Folioter_Btn_FoliotageSansPDG_clicked()
{
    ///*  CONFIGURATION TAMPON
    //    /---------------------------------------\ 60
    //    |                               |margH  |
    //    | <margL>Site de fla...<margL>  Tr.2    |
    //    |                               |margH  |
    //    |---------------------------------------|h2  40
    //    |                         |     |margH  |
    //    | PNO REE DEL 003 VEC CHA | Indice A0   |
    //    |                         |     |margH  |
    //    |---------------------s1-100------------|h1  20
    //    |               |               |margH  |
    //    | Folio 10000   |      0D2320           |
    //    |               |               |margH  |
    //    \-----------s2-60-----------------------/
    //                                                150
    ///*
    QVector<QColor> CouleurTranche;
    CouleurTranche.append(ui->OPT_Btn_ColTranche0->getColor());
    CouleurTranche.append(ui->OPT_Btn_ColTranche1->getColor());
    CouleurTranche.append(ui->OPT_Btn_ColTranche2->getColor());
    CouleurTranche.append(ui->OPT_Btn_ColTranche3->getColor());
    CouleurTranche.append(ui->OPT_Btn_ColTranche4->getColor());
    CouleurTranche.append(ui->OPT_Btn_ColTranche5->getColor());
    CouleurTranche.append(ui->OPT_Btn_ColTranche6->getColor());
    CouleurTranche.append(ui->OPT_Btn_ColTranche7->getColor());
    CouleurTranche.append(ui->OPT_Btn_ColTranche8->getColor());
    CouleurTranche.append(ui->OPT_Btn_ColTranche9->getColor());
    QVector<QColor> CouleurAccentuation;
    CouleurAccentuation.append(ui->OPT_Btn_ColAccent0->getColor());
    CouleurAccentuation.append(ui->OPT_Btn_ColAccent1->getColor());
    CouleurAccentuation.append(ui->OPT_Btn_ColAccent2->getColor());
    CouleurAccentuation.append(ui->OPT_Btn_ColAccent3->getColor());
    CouleurAccentuation.append(ui->OPT_Btn_ColAccent4->getColor());
    CouleurAccentuation.append(ui->OPT_Btn_ColAccent5->getColor());
    CouleurAccentuation.append(ui->OPT_Btn_ColAccent6->getColor());
    CouleurAccentuation.append(ui->OPT_Btn_ColAccent7->getColor());
    CouleurAccentuation.append(ui->OPT_Btn_ColAccent8->getColor());
    CouleurAccentuation.append(ui->OPT_Btn_ColAccent9->getColor());
    QVector<QString> CodeTranche;
    CodeTranche.append(ui->Folioter_Texte_Tranche0->text());
    CodeTranche.append(ui->Folioter_Texte_Tranche1->text());
    CodeTranche.append(ui->Folioter_Texte_Tranche2->text());
    CodeTranche.append(ui->Folioter_Texte_Tranche3->text());
    CodeTranche.append(ui->Folioter_Texte_Tranche4->text());
    CodeTranche.append(ui->Folioter_Texte_Tranche5->text());
    CodeTranche.append(ui->Folioter_Texte_Tranche6->text());
    CodeTranche.append(ui->Folioter_Texte_Tranche7->text());
    CodeTranche.append(ui->Folioter_Texte_Tranche8->text());
    CodeTranche.append(ui->Folioter_Texte_Tranche9->text());
    QString PropositionNomFichier =
      QFileInfo(PDFOuvert).path() + "/REE_" + ui->Folioter_Txt_RefREE->text() + "[" + ui->Folioter_Txt_IndiceREE->text() + "].pdf";
    PDFASauver = QFileDialog::getSaveFileName(this, "Enregistrer le fichier PDF", PropositionNomFichier, "Fichier PDF (*.pdf)");
    if (PDFASauver == "") {
        return;
    }
    QProgressDialog progress("Création des tampons de la tranche" + QString::number(0) + "...", "", 0, 0, this);
    if (ThemeDark)
        progress.setStyle(new DarkStyle);
    progress.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMaximum(0);
    progress.setValue(0);
    progress.setAutoClose(false);
    progress.setCancelButton(0);
    progress.setWindowIcon(this->windowIcon());
    progress.installEventFilter(keyPressEater);
    progress.show();
    //    CentreHWND((HWND)progress.winId());
    {
        RECT FenetrePrincipale;
        GetWindowRect((HWND)this->winId(), &FenetrePrincipale);
        progress.setGeometry(FenetrePrincipale.left + (this->geometry().width() / 2) - (progress.width() / 2),
                             FenetrePrincipale.top + (this->geometry().height() / 2) - (progress.height() / 2),
                             progress.width(),
                             progress.height());
    }
    QCoreApplication::processEvents();
    progress.setValue(0);
    qint64 mStarting = 0;
    qint64 mEnding   = vecMediaBox.size();
    if (ui->Folioter_Radio_FolioPartiel->isChecked()) // Partiel
    {
        mStarting = ui->Folioter_Spin_Partiel_Debut->value() - 1;
        mEnding   = ui->Folioter_Spin_Partiel_Fin->value();
    }
    progress.setMinimum(mStarting);
    progress.setMaximum(mEnding);
    qint64 iTranche = 0;
    qint64 iPage    = 0;
    QMutex progressLock;
    QThread* thread = QThread::create([&] {
        constexpr double TamponLargeur   = 150.0;
        constexpr double TamponHauteur   = 60.0;
        constexpr double TamponMargH     = 7.0;
        constexpr double TamponMargL     = 4.0;
        constexpr double TamponH1        = 20.0;
        constexpr double TamponH2        = 40.0;
        constexpr double TamponS1        = 100.0;
        constexpr double TamponS2        = 60.0;
        constexpr double TamponEpaisseur = 1.0;
        constexpr double TamponPolice    = 8;
        PdfError exPDFError;

        Consigne("Démarrage de la génération des tampons...");
        for (int t = 0; t < 10; ++t) {
            progressLock.lock();
            iTranche = t;
            progressLock.unlock();
            // On test si la tranche est cochée, sinon on continue à la prochaine
            if ((t == 0) && (!ui->Folioter_Check_Tranche0->isChecked()))
                continue;
            if ((t == 1) && (!ui->Folioter_Check_Tranche1->isChecked()))
                continue;
            if ((t == 2) && (!ui->Folioter_Check_Tranche2->isChecked()))
                continue;
            if ((t == 3) && (!ui->Folioter_Check_Tranche3->isChecked()))
                continue;
            if ((t == 4) && (!ui->Folioter_Check_Tranche4->isChecked()))
                continue;
            if ((t == 5) && (!ui->Folioter_Check_Tranche5->isChecked()))
                continue;
            if ((t == 6) && (!ui->Folioter_Check_Tranche6->isChecked()))
                continue;
            if ((t == 7) && (!ui->Folioter_Check_Tranche7->isChecked()))
                continue;
            if ((t == 8) && (!ui->Folioter_Check_Tranche8->isChecked()))
                continue;
            if ((t == 9) && (!ui->Folioter_Check_Tranche9->isChecked()))
                continue;
            try {
                QString PDFSortieTranche =
                  QFileInfo(PDFASauver).path() + "/" + QFileInfo(PDFASauver).baseName() + "_TR" + QString::number(t) + ".pdf";
                PoDoFo::PdfMemDocument document(PDFOuvert.toStdWString().c_str());
                PoDoFo::PdfFont* pFont = document.CreateFontSubset("Roboto",
                                                                   true,
                                                                   false,
                                                                   false,
                                                                   PoDoFo::PdfEncodingFactory::GlobalWinAnsiEncodingInstance(),
                                                                   QDir::toNativeSeparators(TempTamponRoboto).toStdString().c_str());
                pFont->SetFontSize(TamponPolice);
                for (qint64 i = mStarting; i < mEnding; i++) {
                    progressLock.lock();
                    iPage = i;
                    progressLock.unlock();
                    PoDoFo::PdfPage* pPage = document.GetPage(i);
                    {
                        if (vecFolioAnnuler[i]) {
                            const double DualSpace   = 120.0;
                            const double SingleSpace = 60.0;

                            PoDoFo::PdfPainter painter;
                            PoDoFo::PdfRect rect(0, 0, vecMediaBox[i].GetWidth() - DualSpace, vecMediaBox[i].GetHeight() - DualSpace);

                            PoDoFo::PdfXObject xObj(rect, &document);
                            painter.SetPage(&xObj);
                            painter.SetStrokeWidth(2.0);
                            painter.SetColor(MACRO_COLORtoDOUBLE(CouleurTranche, t));
                            painter.SetFont(pFont);
                            pFont->SetFontSize(70.0);
                            PoDoFo::PdfString TexteFolioAnnulee(
                              reinterpret_cast<const PoDoFo::pdf_utf8*>(ui->OPT_Text_FolioAnnule->text().toUtf8().constData()));
                            if (vecRotation[i] == 0) {
                                painter.SetStrokingColor(MACRO_COLORtoDOUBLE(CouleurTranche, t));
                                painter.DrawLine(0.0, 0.0, 0.0, vecMediaBox[i].GetHeight() - DualSpace);
                                painter.Stroke();
                                painter.DrawLine(0.0, vecMediaBox[i].GetHeight() - DualSpace, vecMediaBox[i].GetWidth() - DualSpace, 0.0);
                                painter.Stroke();
                                painter.DrawMultiLineText(0.0,
                                                          0.0,
                                                          rect.GetWidth(),
                                                          rect.GetHeight(),
                                                          TexteFolioAnnulee,
                                                          PoDoFo::EPdfAlignment::ePdfAlignment_Center,
                                                          PoDoFo::EPdfVerticalAlignment::ePdfVerticalAlignment_Center); // OK
                            } else if (vecRotation[i] == 90) {
                                painter.SetTransformationMatrix(0.0, 1.0, -1.0, 0.0, (double)vecMediaBox[i].GetHeight() - DualSpace, 0.0);
                                painter.SetStrokingColor(MACRO_COLORtoDOUBLE(CouleurTranche, t));
                                painter.DrawLine(
                                  0.0, -(vecMediaBox[i].GetWidth() - vecMediaBox[i].GetHeight()), 0.0, vecMediaBox[i].GetHeight() - DualSpace);
                                painter.Stroke();
                                painter.DrawLine(vecMediaBox[i].GetHeight() - DualSpace,
                                                 -(vecMediaBox[i].GetWidth() - vecMediaBox[i].GetHeight()),
                                                 0.0,
                                                 vecMediaBox[i].GetHeight() - DualSpace);
                                painter.Stroke();
                                painter.DrawMultiLineText(0.0,
                                                          -(vecMediaBox[i].GetWidth() - vecMediaBox[i].GetHeight()),
                                                          rect.GetHeight(),
                                                          rect.GetWidth(),
                                                          TexteFolioAnnulee,
                                                          PoDoFo::EPdfAlignment::ePdfAlignment_Center,
                                                          PoDoFo::EPdfVerticalAlignment::ePdfVerticalAlignment_Center); // OK
                            } else if (vecRotation[i] == 180) {
                                painter.SetTransformationMatrix(-1.0,
                                                                0.0,
                                                                0.0,
                                                                -1.0,
                                                                (double)vecMediaBox[i].GetWidth() - DualSpace,
                                                                (double)vecMediaBox[i].GetHeight() - DualSpace);
                                painter.SetStrokingColor(MACRO_COLORtoDOUBLE(CouleurTranche, t));
                                painter.DrawLine(0.0, 0.0, 0.0, vecMediaBox[i].GetHeight() - DualSpace);
                                painter.Stroke();
                                painter.DrawLine(0.0, vecMediaBox[i].GetHeight() - DualSpace, vecMediaBox[i].GetWidth() - DualSpace, 0.0);
                                painter.Stroke();
                                painter.DrawMultiLineText(0.0,
                                                          0.0,
                                                          rect.GetWidth(),
                                                          rect.GetHeight(),
                                                          TexteFolioAnnulee,
                                                          PoDoFo::EPdfAlignment::ePdfAlignment_Center,
                                                          PoDoFo::EPdfVerticalAlignment::ePdfVerticalAlignment_Center); // OK
                            } else if (vecRotation[i] == 270) {
                                painter.SetTransformationMatrix(0.0, -1.0, 1.0, 0.0, 0.0, (double)vecMediaBox[i].GetWidth() - DualSpace);
                                painter.SetStrokingColor(MACRO_COLORtoDOUBLE(CouleurTranche, t));
                                painter.DrawLine(0.0 + vecMediaBox[i].GetWidth() - vecMediaBox[i].GetHeight(),
                                                 0.0,
                                                 0.0 + vecMediaBox[i].GetWidth() - vecMediaBox[i].GetHeight(),
                                                 vecMediaBox[i].GetWidth() - DualSpace);
                                painter.Stroke();
                                painter.DrawLine(0.0 + vecMediaBox[i].GetWidth() - vecMediaBox[i].GetHeight(),
                                                 vecMediaBox[i].GetWidth() - DualSpace,
                                                 vecMediaBox[i].GetWidth() - DualSpace,
                                                 0.0);
                                painter.Stroke();
                                painter.DrawMultiLineText(0.0 + vecMediaBox[i].GetWidth() - vecMediaBox[i].GetHeight(),
                                                          0.0,
                                                          vecMediaBox[i].GetHeight() - DualSpace,
                                                          vecMediaBox[i].GetWidth() - DualSpace,
                                                          TexteFolioAnnulee,
                                                          PoDoFo::EPdfAlignment::ePdfAlignment_Center,
                                                          PoDoFo::EPdfVerticalAlignment::ePdfVerticalAlignment_Center); // OK
                            }
                            rect.SetLeft(SingleSpace);
                            rect.SetBottom(SingleSpace);
                            PoDoFo::PdfAnnotation* pAnnotation = pPage->CreateAnnotation(PoDoFo::EPdfAnnotation::ePdfAnnotation_Stamp, rect);
                            pAnnotation->SetFlags(PoDoFo::ePdfAnnotationFlags_Print);
                            pAnnotation->SetTitle(PoDoFo::PdfString(QString("Annuler_p" + QString::number(i + 1)).toStdWString().c_str()));
                            pAnnotation->SetAppearanceStream(&xObj);
                            painter.FinishPage();
                            pFont->SetFontSize(TamponPolice);
                        }
                    } // Fin folio à annuler
                    { // Début des tampons
                        PoDoFo::PdfPainter painter;
                        PoDoFo::PdfRect rect(0, 0, TamponLargeur, TamponHauteur);

                        if (vecRotation[i] == 90)
                            rect = PoDoFo::PdfRect(0, 0, TamponHauteur, TamponLargeur);
                        else if (vecRotation[i] == 270)
                            rect = PoDoFo::PdfRect(0, 0, TamponHauteur, TamponLargeur);

                        PoDoFo::PdfXObject xObj(rect, &document);
                        painter.SetPage(&xObj);

                        ///
                        ///  MATRIX WORK
                        ///  SetTransformationMatrix(a, b, c, d, e, f)
                        ///  double a, b, c, d, e, f;
                        ///  double alpha = AngleRotation;
                        ///  ______________________ Pour 90    Pour 180    Pour 270
                        ///  a = cos (alpha);	0	   -1          0
                        ///  b = sin (alpha);	1	   0           -1
                        ///  c = -sin(alpha);	-1	   0           1
                        ///  d = cos (alpha);	0	   -1          0
                        ///  e = coord X Rotation 	ValHauteur ValLargeur  0
                        ///  f = coord Y Rotation   0          ValHauteur  ValLargeur
                        ///
                        ///  e et f = point de rotation
                        ///  https://en.wikipedia.org/wiki/Rotation_matrix#Common_rotations
                        ///

                        if (vecRotation[i] == 90)
                            painter.SetTransformationMatrix(0.0, 1.0, -1.0, 0.0, (double)TamponHauteur, 0.0);
                        else if (vecRotation[i] == 180)
                            painter.SetTransformationMatrix(-1.0, 0.0, 0.0, -1.0, (double)TamponLargeur, (double)TamponHauteur);
                        else if (vecRotation[i] == 270)
                            painter.SetTransformationMatrix(0.0, -1.0, 1.0, 0.0, 0.0, (double)TamponLargeur);

                        painter.SetStrokeWidth(TamponEpaisseur);
                        /* -- Fond du tampon en blanc avec bord de couleur rouge -- */
                        painter.SetStrokingColor(MACRO_COLORtoDOUBLE(CouleurTranche,
                                                                     t)); // Couleur ligne format RGB avec 0 à 255 = 0.0 à 1.0
                        painter.SetColor(1.0, 1.0, 1.0);                  // Fond du tampon
                        painter.Rectangle(TamponEpaisseur / 2, TamponEpaisseur / 2, TamponLargeur - TamponEpaisseur, TamponHauteur - TamponEpaisseur);
                        painter.FillAndStroke();

                        /* -- Les lignes internes -- */
                        painter.DrawLine(0.0, TamponH1, TamponLargeur, TamponH1);
                        painter.DrawLine(0.0, TamponH2, TamponLargeur, TamponH2);
                        painter.DrawLine(TamponS1, TamponH1, TamponS1, TamponH2);
#ifndef EPR
                        painter.DrawLine(TamponS2, 0.0, TamponS2, TamponH1);
#endif

                        painter.SetFont(pFont); // Utilise pFont pour écrire...
                        painter.SetColor(MACRO_COLORtoDOUBLE(CouleurTranche,
                                                             t)); // Couleur texte format RGB avec 0 à 255 = 0.0 à 1.0
                        PoDoFo::PdfString utf8SiteDe(
                          reinterpret_cast<const PoDoFo::pdf_utf8*>(QString("Site de " + ui->Folioter_Txt_NomDuSite->text()).toStdString().c_str()));
                        PoDoFo::PdfString utf8Tranche(
                          reinterpret_cast<const PoDoFo::pdf_utf8*>(QString("Tr. " + QString::number(t)).toStdString().c_str()));
                        PoDoFo::PdfString utf8REE(reinterpret_cast<const PoDoFo::pdf_utf8*>(ui->Folioter_Txt_RefREE->text().toStdString().c_str()));
                        PoDoFo::PdfString utf8Indice(
                          reinterpret_cast<const PoDoFo::pdf_utf8*>(QString("Ind. " + ui->Folioter_Txt_IndiceREE->text()).toStdString().c_str()));
                        PoDoFo::PdfString utf8Folio(reinterpret_cast<const PoDoFo::pdf_utf8*>(
                          QString("Folio " +
                                  QString::number(ui->Folioter_Spin_PremierePage->value() + i -
                                                  (ui->Folioter_Radio_FolioPartiel->isChecked() ? ui->Folioter_Spin_Partiel_Debut->value() - 1 : 0)))
                            .toStdString()
                            .c_str()));
                        PoDoFo::PdfString utf8Cycle(
                          reinterpret_cast<const PoDoFo::pdf_utf8*>(QString("Cycle " + QString::number(t) + CodeTranche[t]).toStdString().c_str()));
                        painter.DrawTextAligned(TamponMargL,
                                                TamponMargH,
                                                TamponS2 - 2 * TamponMargL,
                                                utf8Folio,
                                                PoDoFo::EPdfAlignment::ePdfAlignment_Left); // OK
#ifndef EPR
                        painter.DrawTextAligned(TamponS2 + TamponMargL,
                                                TamponMargH,
                                                (TamponLargeur - TamponS2) - 2 * TamponMargL,
                                                utf8Cycle,
                                                PoDoFo::EPdfAlignment::ePdfAlignment_Left); // OK
#endif
                        painter.DrawTextAligned(TamponMargL,
                                                TamponH1 + TamponMargH,
                                                TamponS1 - 2 * TamponMargL,
                                                utf8REE,
                                                PoDoFo::EPdfAlignment::ePdfAlignment_Left); // OK
                        painter.DrawTextAligned(TamponS1 + TamponMargL,
                                                TamponH1 + TamponMargH,
                                                (TamponLargeur - TamponS1) - 2 * TamponMargL,
                                                utf8Indice,
                                                PoDoFo::EPdfAlignment::ePdfAlignment_Left); // OK
                        painter.DrawTextAligned(TamponMargL,
                                                TamponH2 + TamponMargH,
                                                TamponLargeur - 2 * TamponMargL,
                                                utf8SiteDe,
                                                PoDoFo::EPdfAlignment::ePdfAlignment_Left); // OK
                        painter.DrawTextAligned(TamponMargL,
                                                TamponH2 + TamponMargH,
                                                TamponLargeur - 2 * TamponMargL,
                                                utf8Tranche,
                                                PoDoFo::EPdfAlignment::ePdfAlignment_Right); // OK
                        painter.FinishPage();

                        /*
                         * Affinage du Rect
                         *   0,vecHeight
                         *   X**************X vecWidth,vecHeight
                         *   *              *
                         *   *              *
                         *   *              *
                         *   *              *
                         *   *              *
                         *   *              *
                         *   *              *
                         *   *              *
                         *   X**************X 0,vecwidth
                         *  0,0
                         */

                        if (ui->OPT_Radio_HG->isChecked()) { // Haut Gauche
                            rect.SetLeft(0.0 + (double)ui->OPT_Spin_Largeur->value());
                            rect.SetBottom(vecMediaBox[i].GetHeight() - rect.GetHeight() - (double)ui->OPT_Spin_Hauteur->value());
                            if (vecRotation[i] == 0) {
                                rect.SetLeft(0.0 + (double)ui->OPT_Spin_Largeur->value());
                                rect.SetBottom(vecMediaBox[i].GetHeight() - rect.GetHeight() - (double)ui->OPT_Spin_Hauteur->value());
                            } else if (vecRotation[i] == 90) {
                                rect.SetLeft(0.0 + (double)ui->OPT_Spin_Largeur->value());
                                rect.SetBottom((double)ui->OPT_Spin_Hauteur->value());
                            } else if (vecRotation[i] == 180) {
                                rect.SetLeft(vecMediaBox[i].GetWidth() - rect.GetWidth() - (double)ui->OPT_Spin_Largeur->value());
                                rect.SetBottom(0.0 + (double)ui->OPT_Spin_Hauteur->value());
                            } else if (vecRotation[i] == 270) {
                                rect.SetLeft(vecMediaBox[i].GetWidth() - rect.GetWidth() - (double)ui->OPT_Spin_Hauteur->value());
                                rect.SetBottom(vecMediaBox[i].GetHeight() - rect.GetHeight() - (double)ui->OPT_Spin_Largeur->value());
                            }
                        } else if (ui->OPT_Radio_HD->isChecked()) { // Haut Droite
                            if (vecRotation[i] == 0) {
                                rect.SetLeft(vecMediaBox[i].GetWidth() - rect.GetWidth() - (double)ui->OPT_Spin_Largeur->value());
                                rect.SetBottom(vecMediaBox[i].GetHeight() - rect.GetHeight() - (double)ui->OPT_Spin_Hauteur->value());
                            } else if (vecRotation[i] == 90) {
                                rect.SetLeft(0.0 + (double)ui->OPT_Spin_Hauteur->value());
                                rect.SetBottom(vecMediaBox[i].GetHeight() - rect.GetHeight() - (double)ui->OPT_Spin_Largeur->value());
                            } else if (vecRotation[i] == 180) {
                                rect.SetLeft(0.0 + (double)ui->OPT_Spin_Largeur->value());
                                rect.SetBottom(0.0 + (double)ui->OPT_Spin_Hauteur->value());
                            } else if (vecRotation[i] == 270) {
                                rect.SetLeft(vecMediaBox[i].GetWidth() - rect.GetWidth() - (double)ui->OPT_Spin_Hauteur->value());
                                rect.SetBottom(0.0 + (double)ui->OPT_Spin_Largeur->value());
                            }
                        } else if (ui->OPT_Radio_BG->isChecked()) { // Bas Gauche
                            if (vecRotation[i] == 0) {
                                rect.SetLeft(0.0 + (double)ui->OPT_Spin_Largeur->value());
                                rect.SetBottom(0.0 + (double)ui->OPT_Spin_Hauteur->value());
                            } else if (vecRotation[i] == 90) { // A Voir
                                rect.SetLeft(vecMediaBox[i].GetWidth() - rect.GetWidth() - (double)ui->OPT_Spin_Hauteur->value());
                                rect.SetBottom(0.0 + (double)ui->OPT_Spin_Largeur->value());
                            } else if (vecRotation[i] == 180) { // A Voir
                                rect.SetLeft(vecMediaBox[i].GetWidth() - rect.GetWidth() - (double)ui->OPT_Spin_Largeur->value());
                                rect.SetBottom(vecMediaBox[i].GetHeight() - rect.GetHeight() - (double)ui->OPT_Spin_Hauteur->value());
                            } else if (vecRotation[i] == 270) { // A voir
                                rect.SetLeft(0.0 + (double)ui->OPT_Spin_Hauteur->value());
                                rect.SetBottom(vecMediaBox[i].GetHeight() - rect.GetHeight() - (double)ui->OPT_Spin_Largeur->value());
                            }
                        } else if (ui->OPT_Radio_BD->isChecked()) { // Bas Droite
                            if (vecRotation[i] == 0) {
                                rect.SetLeft(vecMediaBox[i].GetWidth() - rect.GetWidth() - (double)ui->OPT_Spin_Largeur->value());
                                rect.SetBottom(0.0 + (double)ui->OPT_Spin_Hauteur->value());
                            } else if (vecRotation[i] == 90) {
                                rect.SetLeft(vecMediaBox[i].GetWidth() - rect.GetWidth() - (double)ui->OPT_Spin_Hauteur->value());
                                rect.SetBottom(vecMediaBox[i].GetHeight() - rect.GetHeight() - (double)ui->OPT_Spin_Largeur->value());
                            } else if (vecRotation[i] == 180) {
                                rect.SetLeft(0.0 + (double)ui->OPT_Spin_Largeur->value());
                                rect.SetBottom(vecMediaBox[i].GetHeight() - rect.GetHeight() - (double)ui->OPT_Spin_Hauteur->value());
                            } else if (vecRotation[i] == 270) {
                                rect.SetLeft(0.0 + (double)ui->OPT_Spin_Hauteur->value());
                                rect.SetBottom(0.0 + (double)ui->OPT_Spin_Largeur->value());
                            }
                        }
                        PoDoFo::PdfAnnotation* pAnnotation = pPage->CreateAnnotation(PoDoFo::EPdfAnnotation::ePdfAnnotation_Stamp, rect);
                        pAnnotation->SetFlags(PoDoFo::ePdfAnnotationFlags_Print);
                        pAnnotation->SetTitle(PoDoFo::PdfString(QString("Tampon_p" + QString::number(i + 1)).toStdWString().c_str()));
                        pAnnotation->SetAppearanceStream(&xObj);
                        painter.FinishPage();
                    }                                             // Fin des tampons
                }                                                 // Fin du for i = 0 to Nb Pages
                if (ui->Folioter_Radio_FolioPartiel->isChecked()) // Partiel -> Suppression des pages inutiles
                {
                    qint64 nbPageRemoveFromStart = mStarting;
                    qint64 nbPageRemoveFromEnd   = vecMediaBox.size() - mEnding;
                    for (qint64 iA = 0; iA < nbPageRemoveFromStart; iA++) // Suppression du début
                        document.GetPagesTree()->DeletePage(0);
                    for (qint64 iA = 0; iA < nbPageRemoveFromEnd; iA++) // Suppression de la fin
                        document.GetPagesTree()->DeletePage(document.GetPageCount() - 1);
                }
                if (ModeAjoutPDG) {
                    // Setup des textes renseignes et case cochés
                    {
                        { // ENUMERATION
                            for (int i = 1; i < ui->PDG_ListeWidget->count(); ++i) {
                                blocQuestion* mBlocquestion =
                                  qobject_cast<blocQuestion*>(ui->PDG_ListeWidget->itemWidget(ui->PDG_ListeWidget->item(i)));
                                auto reponse          = mBlocquestion->RetourneDonnee();
                                QString ReponseFormat = reponse->Reponse;
                                ReponseFormat         = QString(ReponseFormat).replace("{RetourLigne}", "\n");
                                if (reponse->TypeDeBloc == blocQuestion::TypeBloc::Bloc_Texte_Simple)
                                    mPDGHelper.ListeQuestion[reponse->IndexControle].DefautQuestion = ReponseFormat;
                                if (reponse->TypeDeBloc == blocQuestion::TypeBloc::Bloc_Texte_Multiligne)
                                    mPDGHelper.ListeQuestion[reponse->IndexControle].DefautQuestion = ReponseFormat;
                                if (reponse->TypeDeBloc == blocQuestion::TypeBloc::Bloc_Case_Coche)
                                    mPDGHelper.ListeQuestion[reponse->IndexControle].CheckboxValue = reponse->EtatCoche;
                            }
                        }
                        mPDGHelper.ArrayFromREEMAKER.ReferenceSite = ui->Folioter_Txt_NomDuSite->text();
                        mPDGHelper.ArrayFromREEMAKER.NumeroTranche = QString::number(t);
                        mPDGHelper.ArrayFromREEMAKER.ReferenceREE  = ui->Folioter_Txt_RefREE->text();
                        mPDGHelper.ArrayFromREEMAKER.IndiceREE     = ui->Folioter_Txt_IndiceREE->text();
                    }
                    { // Setup des couleurs
                        mPDGHelper.ArrayFromREEMAKER.REErouge       = CouleurTranche[t].red();
                        mPDGHelper.ArrayFromREEMAKER.REEvert        = CouleurTranche[t].green();
                        mPDGHelper.ArrayFromREEMAKER.REEbleu        = CouleurTranche[t].blue();
                        mPDGHelper.ArrayFromREEMAKER.REErougeAccent = CouleurAccentuation[t].red();
                        mPDGHelper.ArrayFromREEMAKER.REEvertAccent  = CouleurAccentuation[t].green();
                        mPDGHelper.ArrayFromREEMAKER.REEbleuAccent  = CouleurAccentuation[t].blue();
                    }
                    PoDoFo::PdfPage* pPage = document.InsertPage(PoDoFo::PdfRect(0.0, 0.0, 595.0, 842.0), 0);
                    PoDoFo::PdfPainter painter;
                    painter.SetPage(pPage);
                    mPDGHelper.DrawOnPage_v2(painter, document);
                    painter.FinishPage();
                    Consigne("Génération de la page de garde pour la tranche n°" + QString::number(t));
                }
                document.Write(PDFSortieTranche.toStdWString().c_str());
                if (ui->OPT_Check_OuvrirApres->isChecked())
                    QDesktopServices::openUrl(QUrl::fromLocalFile(PDFSortieTranche));
            } catch (const PoDoFo::PdfError& e) {
                Consigne("Erreur à la génération du document : " + QString(e.what()) + " tranche n°" + QString::number(t));
            } catch (...) {
                Consigne("Erreur inconnue à la génération du document tranche n°" + QString::number(t));
            }
            Consigne("Génération de la procédure folioté pour la tranche n°" + QString::number(t));
        } // Fin du for i = 0 to 10
    });
    thread->start();
    while (thread->isRunning()) {
        QThread::msleep(16);
        progressLock.lock();
        qint64 cpITRANCHE = iTranche;
        qint64 cpIPAGE    = iPage;
        progressLock.unlock();
        QString NewText = "Tranche " + QString::number(cpITRANCHE) + " : Création des tampons...";
        if (progress.labelText() != NewText) {
            progress.setLabelText("Tranche " + QString::number(cpITRANCHE) + " : Création des tampons...");
            QCoreApplication::processEvents();
        }
        if (progress.value() != cpIPAGE) {
            progress.setValue(cpIPAGE);
            QCoreApplication::processEvents();
        }
    }
    progress.close();
    if (ModeAjoutPDG) {
        ui->PDG_ListeWidget->clear();
        ui->PDG_Texte_PDGEnCours->setText("");
    }
    ModeAjoutPDG = false;
    Consigne("Fin de la génération des pages de gardes");
    MGBoxContinuer("Information", "<b>Fin de la génération des pages de gardes</b>", QMessageBox::Icon::Information);
}

/** on_Folioter_Btn_FoliotageAvecPDG_clicked:
  Mets en place les variables afin de réaliser un foliotage avec une page de
 garede

  @return Aucun
*/
void
MainWindow::on_Folioter_Btn_FoliotageAvecPDG_clicked()
{
    ModeAjoutPDG = true;
    ui->PDG_Btn_GenerePDF->setText("  Valider la page de garde et continuer");
    ui->tabWidget->setTabVisible(0, false);
    ui->tabWidget->setTabVisible(3, false);
    ui->tabWidget->setTabVisible(5, false);
    ui->tabWidget->setCurrentIndex(2);
    ui->PDG_ListeWidget->clear();
    {
        bool TrouvePDG = false;
        // Test si ce nom est dans combo integré
        for (int var = 0; var < ui->PDG_Combo_Integre->count(); ++var) {
            if (ui->PDG_Combo_Integre->itemText(var).toUpper() == ui->PDG_Texte_PDGEnCours->text().toUpper()) {
                ui->PDG_Combo_Integre->setCurrentIndex(var);
                ui->PDG_Texte_PDGEnCours->setText(ui->PDG_Combo_Integre->itemText(var).toUpper());
                ChargerPageDeGarde(ui->PDG_Combo_Integre->itemText(var), CheminPDGBase);
                TrouvePDG = true;
                break;
            }
        }
        // Test si ce nom est dans Combo utilisateur
        if (!TrouvePDG)
            for (int var = 0; var < ui->PDG_Combo_Utilisateur->count(); ++var) {
                if (ui->PDG_Combo_Utilisateur->itemText(var).toUpper() == ui->PDG_Texte_PDGEnCours->text().toUpper()) {
                    ui->PDG_Combo_Utilisateur->setCurrentIndex(var);
                    ui->PDG_Texte_PDGEnCours->setText(ui->PDG_Combo_Utilisateur->itemText(var).toUpper());
                    ChargerPageDeGarde(ui->PDG_Combo_Utilisateur->itemText(var), CheminPDGUtilisateur);
                    TrouvePDG = true;
                    break;
                }
            }
        // Aucun, on le reset
        if (!TrouvePDG)
            for (int var = 0; var < ui->PDG_Combo_Integre->count(); ++var) {
                if (ui->PDG_Combo_Integre->itemText(var).toLower() == "page de garde standard bpa") {
                    ui->PDG_Combo_Integre->setCurrentIndex(var);
                    ui->PDG_Texte_PDGEnCours->setText(ui->PDG_Combo_Integre->itemText(var).toUpper());
                    ChargerPageDeGarde(ui->PDG_Combo_Integre->itemText(var), CheminPDGBase);
                    break;
                }
            }
    }
    Consigne("L'utilisateur doit choisir la page de garde à utiliser");
}

/** on_PDG_Btn_GenerePDF_clicked:
  Génération manuelle de la page de garde uniquement

  @return Aucun
*/
void
MainWindow::on_PDG_Btn_GenerePDF_clicked()
{
    if (ui->PDG_ListeWidget->count() == 0) {
        MGBoxContinuer("REEMaker - Attention",
                       "<b>La liste est vide ou aucune page de garde n'a été chargée.<br>L'opération sera annulée.</b>",
                       QMessageBox::Icon::Information);
        return;
    }
    if (ui->PDG_ListeWidget->count() > 0) {
        int NombreNonRemplis       = 0;
        int IndexPremierNonRemplis = -1;
        for (int i = 1; i < ui->PDG_ListeWidget->count(); ++i) {
            blocQuestion* mBlocquestion = qobject_cast<blocQuestion*>(ui->PDG_ListeWidget->itemWidget(ui->PDG_ListeWidget->item(i)));
            auto reponse                = mBlocquestion->RetourneDonnee();
            if (reponse->EstObligatoire && (reponse->TypeDeBloc == blocQuestion::Bloc_Texte_Simple) && (reponse->Reponse == ""))
                NombreNonRemplis++;
            if (reponse->EstObligatoire && (reponse->TypeDeBloc == blocQuestion::Bloc_Texte_Multiligne) && (reponse->Reponse == ""))
                NombreNonRemplis++;
            if ((NombreNonRemplis == 1) && (IndexPremierNonRemplis == -1))
                IndexPremierNonRemplis = i;
        }
        if (NombreNonRemplis > 0) {
            MGBoxContinuer("REEMaker - Erreur",
                           "<b>Tout les champs obligatoires ne sont pas renseignés.<br>Nombre de champs à renseigner pour continuer : " +
                             QString::number(NombreNonRemplis) + "</b>",
                           QMessageBox::Icon::Information);
            ui->PDG_ListeWidget->setCurrentRow(IndexPremierNonRemplis); //+1 car header = 0
            return;
        }
    }
    if (ModeAjoutPDG) {
        ui->tabWidget->setTabVisible(0, true);
        ui->tabWidget->setTabVisible(3, true);
        ui->tabWidget->setTabVisible(5, true);
        ui->tabWidget->setCurrentIndex(0);
        ui->PDG_Btn_GenerePDF->setText("  Générer cette page de garde en PDF");
        QCoreApplication::processEvents();
        ui->Folioter_Btn_FoliotageSansPDG->setFocus();
        on_Folioter_Btn_FoliotageSansPDG_clicked();
        Consigne("La page de garde est paramétrée, début du foliotage");
        return;
    }

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Choix des tranches pour la génération");
    QCheckBox* mTrancheCB0      = new QCheckBox("Tranche  0");
    QCheckBox* mTrancheCB1      = new QCheckBox("Tranche  1");
    QCheckBox* mTrancheCB2      = new QCheckBox("Tranche  2");
    QCheckBox* mTrancheCB3      = new QCheckBox("Tranche  3");
    QCheckBox* mTrancheCB4      = new QCheckBox("Tranche  4");
    QCheckBox* mTrancheCB5      = new QCheckBox("Tranche  5");
    QCheckBox* mTrancheCB6      = new QCheckBox("Tranche  6");
    QCheckBox* mTrancheCB7      = new QCheckBox("Tranche  7");
    QCheckBox* mTrancheCB8      = new QCheckBox("Tranche  8");
    QCheckBox* mTrancheCB9      = new QCheckBox("Tranche  9");
    QLabel* Label0              = new QLabel("Séléctionner les tranches concernées et "
                                             "renseigner les informations suivantes :");
    QLabel* Label1              = new QLabel("Nom site : ");
    QLineEdit* TextBoxNomSite   = new QLineEdit("");
    QLabel* Label2              = new QLabel("Référence REE : ");
    QLineEdit* TextBoxReference = new QLineEdit("");
    QLabel* Label3              = new QLabel("Indice REE : ");
    QLineEdit* TextBoxIndice    = new QLineEdit("");

    msgBox.setCheckBox(mTrancheCB0);
    QGridLayout* Maingrid = qobject_cast<QGridLayout*>(msgBox.layout());

    int row = 0, column = 0, rowSpan = 1, columnSpan = 1;

    QGridLayout* grid = new QGridLayout;

    //    Redéplacer un widget existant
    //        QLayoutItem * const item = grid->itemAt(1);//Label
    //        if(dynamic_cast<QWidgetItem *>(item))
    //            grid->addWidget(item->widget(), 0, 0, 1, 8);

    Maingrid->addLayout(grid, 0, 0, 1, 2);
    grid->addWidget(Label0, row, column + 0, rowSpan, columnSpan + 9);
    grid->addWidget(mTrancheCB0, row + 1, column + 0, rowSpan, columnSpan);
    grid->addWidget(mTrancheCB1, row + 1, column + 1, rowSpan, columnSpan);
    grid->addWidget(mTrancheCB2, row + 1, column + 2, rowSpan, columnSpan);
    grid->addWidget(mTrancheCB3, row + 1, column + 3, rowSpan, columnSpan);
    grid->addWidget(mTrancheCB4, row + 1, column + 4, rowSpan, columnSpan);
    grid->addWidget(mTrancheCB5, row + 1, column + 5, rowSpan, columnSpan);
    grid->addWidget(mTrancheCB6, row + 1, column + 6, rowSpan, columnSpan);
    grid->addWidget(mTrancheCB7, row + 1, column + 7, rowSpan, columnSpan);
    grid->addWidget(mTrancheCB8, row + 1, column + 8, rowSpan, columnSpan);
    grid->addWidget(mTrancheCB9, row + 1, column + 9, rowSpan, columnSpan);
    QGridLayout* grid2 = new QGridLayout;
    Maingrid->addLayout(grid2, 1, 0);
    grid2->addWidget(Label1, row, column, rowSpan, columnSpan);
    grid2->addWidget(TextBoxNomSite, row, column + 1, rowSpan, columnSpan);
    grid2->addWidget(Label2, row, column + 2, rowSpan, columnSpan);
    grid2->addWidget(TextBoxReference, row, column + 3, rowSpan, columnSpan);
    grid2->addWidget(Label3, row, column + 4, rowSpan, columnSpan);
    grid2->addWidget(TextBoxIndice, row, column + 5, rowSpan, columnSpan);

    QAbstractButton* pOui = msgBox.addButton(" Générer ", QMessageBox::YesRole);
    QAbstractButton* pNon = msgBox.addButton(" Annuler ", QMessageBox::NoRole);

    TextBoxNomSite->setText((PDGManuelNomSite == "") ? ui->Folioter_Txt_NomDuSite->text() : PDGManuelNomSite);
    TextBoxNomSite->setClearButtonEnabled(true);
    TextBoxNomSite->setValidator(validUPPER);
    TextBoxReference->setText((PDGManuelRefREE == "") ? ui->Folioter_Txt_RefREE->text() : PDGManuelRefREE);
    TextBoxReference->setClearButtonEnabled(true);
    TextBoxReference->setValidator(validUPPER);
    TextBoxIndice->setText((PDGManuelIndice == "") ? ui->Folioter_Txt_IndiceREE->text() : PDGManuelIndice);
    TextBoxIndice->setClearButtonEnabled(true);
    TextBoxIndice->setValidator(validUPPER);
    QVector<qint16> Tranche;
    msgBox.exec();

    if (msgBox.clickedButton() == pOui) {
        if (mTrancheCB0->isChecked())
            Tranche.append(0);
        if (mTrancheCB1->isChecked())
            Tranche.append(1);
        if (mTrancheCB2->isChecked())
            Tranche.append(2);
        if (mTrancheCB3->isChecked())
            Tranche.append(3);
        if (mTrancheCB4->isChecked())
            Tranche.append(4);
        if (mTrancheCB5->isChecked())
            Tranche.append(5);
        if (mTrancheCB6->isChecked())
            Tranche.append(6);
        if (mTrancheCB7->isChecked())
            Tranche.append(7);
        if (mTrancheCB8->isChecked())
            Tranche.append(8);
        if (mTrancheCB9->isChecked())
            Tranche.append(9);
        if (Tranche.count() == 0) {
            MGBoxContinuer("REEMaker", "<b>Aucune tranche n'a été sélectionnée, abandon de l'opération</b>", QMessageBox::Icon::Warning);
            return;
        }

        QString NomSite = TextBoxNomSite->text();
        QString RefREE  = TextBoxReference->text();
        QString Indice  = TextBoxIndice->text();
        if (NomSite == "" || RefREE == "" || Indice == "") {
            MGBoxContinuer("REEMaker - Erreur",
                           "<b>Nom du site, Référence du REE et Indice requis pour continuer, abandon de l'opération</b>",
                           QMessageBox::Icon::Warning);
            return;
        }
        QString PDFASauver = QFileDialog::getSaveFileName(this, "Enregistrer la page de garde sous", "", "Fichier PDF (*.pdf)");
        if (PDFASauver == "")
            return;
        PDGManuelNomSite = NomSite;
        PDGManuelRefREE  = RefREE;
        PDGManuelIndice  = Indice;
        PDFASauver       = PDFASauver.chopped(4); // Retirer le .pdf de fin (4 caractère)
        foreach (auto valTranche, Tranche) {
            PoDoFo::PdfMemDocument document;
            // Setup des textes renseignes et case cochés
            QVector<QColor> CouleurTranche;
            CouleurTranche.append(ui->OPT_Btn_ColTranche0->getColor());
            CouleurTranche.append(ui->OPT_Btn_ColTranche1->getColor());
            CouleurTranche.append(ui->OPT_Btn_ColTranche2->getColor());
            CouleurTranche.append(ui->OPT_Btn_ColTranche3->getColor());
            CouleurTranche.append(ui->OPT_Btn_ColTranche4->getColor());
            CouleurTranche.append(ui->OPT_Btn_ColTranche5->getColor());
            CouleurTranche.append(ui->OPT_Btn_ColTranche6->getColor());
            CouleurTranche.append(ui->OPT_Btn_ColTranche7->getColor());
            CouleurTranche.append(ui->OPT_Btn_ColTranche8->getColor());
            CouleurTranche.append(ui->OPT_Btn_ColTranche9->getColor());
            QVector<QColor> CouleurAccentuation;
            CouleurAccentuation.append(ui->OPT_Btn_ColAccent0->getColor());
            CouleurAccentuation.append(ui->OPT_Btn_ColAccent1->getColor());
            CouleurAccentuation.append(ui->OPT_Btn_ColAccent2->getColor());
            CouleurAccentuation.append(ui->OPT_Btn_ColAccent3->getColor());
            CouleurAccentuation.append(ui->OPT_Btn_ColAccent4->getColor());
            CouleurAccentuation.append(ui->OPT_Btn_ColAccent5->getColor());
            CouleurAccentuation.append(ui->OPT_Btn_ColAccent6->getColor());
            CouleurAccentuation.append(ui->OPT_Btn_ColAccent7->getColor());
            CouleurAccentuation.append(ui->OPT_Btn_ColAccent8->getColor());
            CouleurAccentuation.append(ui->OPT_Btn_ColAccent9->getColor());
            for (int i = 1; i < ui->PDG_ListeWidget->count(); ++i) {
                blocQuestion* mBlocquestion = qobject_cast<blocQuestion*>(ui->PDG_ListeWidget->itemWidget(ui->PDG_ListeWidget->item(i)));
                auto reponse                = mBlocquestion->RetourneDonnee();
                QString ReponseFormat       = reponse->Reponse;
                ReponseFormat               = QString(ReponseFormat).replace("{RetourLigne}", "\n");
                if (reponse->TypeDeBloc == blocQuestion::TypeBloc::Bloc_Texte_Simple)
                    mPDGHelper.ListeQuestion[reponse->IndexControle].DefautQuestion = ReponseFormat;
                if (reponse->TypeDeBloc == blocQuestion::TypeBloc::Bloc_Texte_Multiligne)
                    mPDGHelper.ListeQuestion[reponse->IndexControle].DefautQuestion = ReponseFormat;
                if (reponse->TypeDeBloc == blocQuestion::TypeBloc::Bloc_Case_Coche)
                    mPDGHelper.ListeQuestion[reponse->IndexControle].CheckboxValue = reponse->EtatCoche;
            }
            mPDGHelper.ArrayFromREEMAKER.ReferenceSite  = NomSite;
            mPDGHelper.ArrayFromREEMAKER.NumeroTranche  = QString::number(valTranche);
            mPDGHelper.ArrayFromREEMAKER.ReferenceREE   = RefREE;
            mPDGHelper.ArrayFromREEMAKER.IndiceREE      = Indice;
            mPDGHelper.ArrayFromREEMAKER.REErouge       = CouleurTranche[valTranche].red();
            mPDGHelper.ArrayFromREEMAKER.REEvert        = CouleurTranche[valTranche].green();
            mPDGHelper.ArrayFromREEMAKER.REEbleu        = CouleurTranche[valTranche].blue();
            mPDGHelper.ArrayFromREEMAKER.REErougeAccent = CouleurAccentuation[valTranche].red();
            mPDGHelper.ArrayFromREEMAKER.REEvertAccent  = CouleurAccentuation[valTranche].green();
            mPDGHelper.ArrayFromREEMAKER.REEbleuAccent  = CouleurAccentuation[valTranche].blue();

            PoDoFo::PdfPage* pPage = document.InsertPage(PoDoFo::PdfRect(0.0, 0.0, 595.0, 842.0), 0);
            PoDoFo::PdfPainter painter;
            painter.SetPage(pPage);
            mPDGHelper.DrawOnPage_v2(painter, document);
            painter.FinishPage();
            document.Write(QString(PDFASauver + "_Tr" + QString::number(valTranche) + ".pdf").toStdWString().c_str());
            if (ui->OPT_Check_OuvrirApres->isChecked())
                QDesktopServices::openUrl(QUrl::fromLocalFile(QString(PDFASauver + "_Tr" + QString::number(valTranche) + ".pdf")));
            Consigne("Génération de la page de garde pour la tranche n°" + QString::number(valTranche));
        }
    }
    if (msgBox.clickedButton() == pNon)
        return;
}

/** GetVersion:
  Récupère sous forme de QString la version de l'executable

  @param QString fName
  @return QString
*/
QString
MainWindow::GetVersion(QString fName)
{
    QString Result = "";

    DWORD verHandle = 0;
    UINT size       = 0;
    LPBYTE lpBuffer = NULL;
    DWORD verSize   = GetFileVersionInfoSize(fName.toStdWString().c_str(), &verHandle);

    if (verSize != 0) {
        LPSTR verData = new char[verSize];

        if (GetFileVersionInfo(fName.toStdWString().c_str(), verHandle, verSize, verData))
            if (VerQueryValue(verData, QString("\\").toStdWString().c_str(), (VOID FAR * FAR*)&lpBuffer, &size))
                if (size) {
                    VS_FIXEDFILEINFO* verInfo = (VS_FIXEDFILEINFO*)lpBuffer;
                    if (verInfo->dwSignature == 0xfeef04bd)
                        Result = QString("%1.%2.%3.%4")
                                   .arg(QString::number((verInfo->dwFileVersionMS >> 16) & 0xff),
                                        QString::number((verInfo->dwFileVersionMS >> 0) & 0xff),
                                        QString::number((verInfo->dwFileVersionLS >> 16) & 0xff),
                                        QString::number((verInfo->dwFileVersionLS >> 0) & 0xff));
                }
        delete[] verData;
    }
    return Result;
}

/** ChargerPageDeGarde:
  Charge une page de garde pour réaliser le foliotage d'un REE

  @param QString Nom
  @param QString Chemin
  @return bool
*/
bool
MainWindow::ChargerPageDeGarde(QString Nom, QString Chemin)
{
    ui->PDG_ListeWidget->setSortingEnabled(false);
    ui->PDG_ListeWidget->clear();
    mPDGHelper.ClearList();
    mPDGHelper.SetBaseModelePath(Chemin);
    try {
        mPDGHelper.OpenAndParseConfig_v2(Nom + ".txt");
        {
            // HEADER DE LA LIST
            customHeader* mcustomHeader = new customHeader(this);
            QListWidgetItem* item;
            item = new QListWidgetItem(ui->PDG_ListeWidget);
            ui->PDG_ListeWidget->addItem(item);
            item->setSizeHint(mcustomHeader->minimumSizeHint());
            ui->PDG_ListeWidget->setItemWidget(item, mcustomHeader);
        }

        for (qsizetype lPDG = 0; lPDG < mPDGHelper.ListeQuestion.size(); lPDG++) {
            if (mPDGHelper.ListeQuestion[lPDG].EstCheckbox) {
                blocQuestion::ItemDefinition StructBloc = blocQuestion::ItemDefinition();
                StructBloc.TypeDeBloc                   = blocQuestion::TypeBloc::Bloc_Case_Coche;
                StructBloc.IndexControle                = lPDG;
                StructBloc.NomControle                  = QString("CheckBox " + QString::number(StructBloc.IndexControle));
                StructBloc.Question                     = mPDGHelper.ListeQuestion[lPDG].LaQuestion;
                StructBloc.Aide                         = mPDGHelper.ListeQuestion[lPDG].AideQuestion;
                StructBloc.EstObligatoire               = mPDGHelper.ListeQuestion[lPDG].Obligatoire;
                StructBloc.EtatCoche                    = mPDGHelper.ListeQuestion[lPDG].CheckboxValue;
                blocQuestion* mblocQuestion             = new blocQuestion(this, StructBloc, ThemeDark);
                QListWidgetItem* item;
                item = new QListWidgetItem(ui->PDG_ListeWidget);
                mblocQuestion->setWhatsThis(StructBloc.NomControle);
                ui->PDG_ListeWidget->addItem(item);
                item->setSizeHint(mblocQuestion->minimumSizeHint());
                ui->PDG_ListeWidget->setItemWidget(item, mblocQuestion);
            } else if (mPDGHelper.ListeQuestion[lPDG].EstLigneTexte) {
                blocQuestion::ItemDefinition StructBloc = blocQuestion::ItemDefinition();
                if (mPDGHelper.ListeQuestion[lPDG].EstMajuscule)
                    StructBloc.TexteMajuscule = true;
                if (mPDGHelper.ListeQuestion[lPDG].EstMinuscule)
                    StructBloc.TexteMinuscule = true;
                if (mPDGHelper.ListeQuestion[lPDG].EstChiffre)
                    StructBloc.TexteDecimal = true;

                StructBloc.TypeDeBloc       = blocQuestion::TypeBloc::Bloc_Texte_Simple;
                StructBloc.IndexControle    = lPDG;
                StructBloc.NomControle      = QString("Texte Ligne " + QString::number(StructBloc.IndexControle));
                StructBloc.Question         = mPDGHelper.ListeQuestion[lPDG].LaQuestion;
                StructBloc.Aide             = mPDGHelper.ListeQuestion[lPDG].AideQuestion;
                StructBloc.Reponse          = mPDGHelper.ListeQuestion[lPDG].DefautQuestion;
                StructBloc.Maximum          = mPDGHelper.ListeQuestion[lPDG].Maximum;
                StructBloc.EstObligatoire   = mPDGHelper.ListeQuestion[lPDG].Obligatoire;
                blocQuestion* mblocQuestion = new blocQuestion(this, StructBloc, ThemeDark);
                QListWidgetItem* item;
                item = new QListWidgetItem(ui->PDG_ListeWidget);
                mblocQuestion->setWhatsThis(StructBloc.NomControle);
                ui->PDG_ListeWidget->addItem(item);
                item->setSizeHint(mblocQuestion->minimumSizeHint());
                ui->PDG_ListeWidget->setItemWidget(item, mblocQuestion);
            } else if (mPDGHelper.ListeQuestion[lPDG].EstMultiLigneTexte) {
                blocQuestion::ItemDefinition StructBloc = blocQuestion::ItemDefinition();
                if (mPDGHelper.ListeQuestion[lPDG].EstMajuscule)
                    StructBloc.TexteMajuscule = true;
                if (mPDGHelper.ListeQuestion[lPDG].EstMinuscule)
                    StructBloc.TexteMinuscule = true;
                if (mPDGHelper.ListeQuestion[lPDG].EstChiffre)
                    StructBloc.TexteDecimal = true;

                StructBloc.TypeDeBloc       = blocQuestion::TypeBloc::Bloc_Texte_Multiligne;
                StructBloc.IndexControle    = lPDG;
                StructBloc.NomControle      = QString("Texte Multiligne " + QString::number(StructBloc.IndexControle));
                StructBloc.Question         = mPDGHelper.ListeQuestion[lPDG].LaQuestion;
                StructBloc.Aide             = mPDGHelper.ListeQuestion[lPDG].AideQuestion;
                StructBloc.Reponse          = mPDGHelper.ListeQuestion[lPDG].DefautQuestion;
                StructBloc.Maximum          = mPDGHelper.ListeQuestion[lPDG].Maximum;
                StructBloc.EstObligatoire   = mPDGHelper.ListeQuestion[lPDG].Obligatoire;
                blocQuestion* mblocQuestion = new blocQuestion(this, StructBloc, ThemeDark);
                QListWidgetItem* item;
                item = new QListWidgetItem(ui->PDG_ListeWidget);
                mblocQuestion->setWhatsThis(StructBloc.NomControle);
                ui->PDG_ListeWidget->addItem(item);
                item->setSizeHint(mblocQuestion->minimumSizeHint());
                ui->PDG_ListeWidget->setItemWidget(item, mblocQuestion);
            } else {
                Consigne("Erreur anormal...");
            }
        }
    } catch (...) {
        return false;
    }
    Consigne("Page de garde '" + Nom + "' chargée");
    return true;
}

/** on_PDG_Btn_SauvegardePDGutilisateur_clicked:
  Permet de sauvegarder sur disque dans le dossier utilisateur la page de
 garde actuelle

  @return bool
*/
void
MainWindow::on_PDG_Btn_SauvegardePDGutilisateur_clicked()
{
    if (ui->PDG_ListeWidget->count() == 0) {
        MGBoxContinuer("REEMaker - Attention",
                       "<b>La liste est vide ou aucune page de garde n'a été chargée.<br>L'opération est annulée.</b>",
                       QMessageBox::Icon::Warning);
        return;
    }
    bool ok;
    QString NomUtilisateur =
      QInputDialog::getText(this, "Enregistrement page de garde", "Saisir un nom pour la page de garde :", QLineEdit::Normal, "", &ok);
    if (ok && !NomUtilisateur.isEmpty()) {
        { // ENUMERATION
            for (int i = 1; i < ui->PDG_ListeWidget->count(); ++i) {
                blocQuestion* mBlocquestion = qobject_cast<blocQuestion*>(ui->PDG_ListeWidget->itemWidget(ui->PDG_ListeWidget->item(i)));
                auto reponse                = mBlocquestion->RetourneDonnee();
                QString ReponseFormat       = reponse->Reponse;
                ReponseFormat               = QString(ReponseFormat).replace("\n", "{RetourLigne}");

                if (reponse->TypeDeBloc == blocQuestion::TypeBloc::Bloc_Texte_Simple)
                    mPDGHelper.ListeQuestion[reponse->IndexControle].DefautQuestion = ReponseFormat;
                if (reponse->TypeDeBloc == blocQuestion::TypeBloc::Bloc_Texte_Multiligne)
                    mPDGHelper.ListeQuestion[reponse->IndexControle].DefautQuestion = ReponseFormat;
                if (reponse->TypeDeBloc == blocQuestion::TypeBloc::Bloc_Case_Coche)
                    mPDGHelper.ListeQuestion[reponse->IndexControle].CheckboxValue = reponse->EtatCoche;
            }
        }
        mPDGHelper.BurstVersDisque(CheminPDGUtilisateur + NomUtilisateur + ".txt");

        // On rafraichit la liste :
        {
            ui->PDG_Combo_Utilisateur->clear();
            qint64 DefautIndex = -1;
            QDir directory(QFileInfo(CheminPDGUtilisateur).filePath());
            QStringList lstPDGBase = directory.entryList(QStringList() << "*.txt", QDir::Files, QDir::Name | QDir::IgnoreCase);
            foreach (QString lPDG, lstPDGBase) {
                ui->PDG_Combo_Utilisateur->addItem(QFileInfo(CheminPDGUtilisateur + lPDG).baseName());
                if (lPDG.toLower() == NomUtilisateur.toLower() + ".txt") // C'est la nouvelle page de garde crée
                    DefautIndex = ui->PDG_Combo_Utilisateur->count() - 1;
            }
            if (DefautIndex != -1) {
                ui->PDG_Combo_Utilisateur->setCurrentIndex(DefautIndex);
                ui->PDG_Texte_PDGEnCours->setText(
                  QString("' %1 '   [Page de garde utilisateur]").arg(ui->PDG_Combo_Utilisateur->itemText(DefautIndex)));
                ChargerPageDeGarde(ui->PDG_Combo_Utilisateur->itemText(DefautIndex), CheminPDGUtilisateur);
            }
        }
        Consigne("Page de garde " + NomUtilisateur + " sauvegardée");
    }
}

/** on_PDG_Combo_Integre_activated:
  Charge la page de garde dans le dossier intégré

  @param int index
  @return Aucun
*/
void
MainWindow::on_PDG_Combo_Integre_activated(int index)
{
    ui->PDG_Texte_PDGEnCours->setText(ui->PDG_Combo_Integre->itemText(index).toUpper());
    ChargerPageDeGarde(ui->PDG_Combo_Integre->itemText(index), CheminPDGBase);
}

/** on_PDG_Combo_Utilisateur_activated:
  Charge la page de garde dans le dossier utilisateur

  @param int index
  @return Aucun
*/
void
MainWindow::on_PDG_Combo_Utilisateur_activated(int index)
{
    ui->PDG_Texte_PDGEnCours->setText(ui->PDG_Combo_Utilisateur->itemText(index).toUpper());
    ChargerPageDeGarde(ui->PDG_Combo_Utilisateur->itemText(index), CheminPDGUtilisateur);
}

/** on_EDIT_Bouton_OuvrirPDG_clicked:
  Ouvre une page de garde pour édition

  @return Aucun
*/
void
MainWindow::on_EDIT_Bouton_OuvrirPDG_clicked()
{
    PDGOuvertePourEdition = QFileDialog::getOpenFileName(
      this, "Ouvrir un fichier page de garde", CheminPDGBase, "Fichier TXT (*.txt)", nullptr, QFileDialog::HideNameFilterDetails);
    if (PDGOuvertePourEdition == "")
        return;
    ChargePDG(PDGOuvertePourEdition);
    ui->tabWidget->setTabText(4, QString("Edition de page de gardes - %1").arg(QFileInfo(PDGOuvertePourEdition).baseName()));
    Consigne("Page de garde " + PDGOuvertePourEdition + " ouverte pour édition");
}

/** on_EDIT_Bouton_NouvellePDG_clicked:
  Réinitialise le contenu de la liste de bloc de page de garde

  @return Aucun
*/
void
MainWindow::on_EDIT_Bouton_NouvellePDG_clicked()
{
    ui->tabWidget->setTabText(4, "Edition de page de gardes - Nouvelle page de garde");
    ui->EDIT_Liste->setUpdatesEnabled(false);
    ui->EDIT_Liste->clear();
    ui->EDIT_Liste->setUpdatesEnabled(true);
    PDGOuvertePourEdition = "";
    Consigne("Démarrage d'une nouvelle page de garde");
}

/** on_EDIT_Bouton_EnregistrePDG_clicked:
  Ecrase la page de garde ouverte avec les nouvelles informations

  @return Aucun
*/
void
MainWindow::on_EDIT_Bouton_EnregistrePDG_clicked()
{
    if (PDGOuvertePourEdition == "")
        on_EDIT_Bouton_EnregistrePDGSous_clicked();
    else {
        SauvePDG(PDGOuvertePourEdition);
        Consigne("Page de garde " + PDGOuvertePourEdition + " écrasée");
    }
}

/** on_EDIT_Bouton_EnregistrePDG_clicked:
  Enregistre la page de garde dans le chemin choisi

  @return Aucun
*/
void
MainWindow::on_EDIT_Bouton_EnregistrePDGSous_clicked()
{
    QString PDFASauver = QFileDialog::getSaveFileName(this, "Enregistrer la page de garde sous", PDGOuvertePourEdition, "Fichier TXT (*.txt)");
    if (PDFASauver == "")
        return;
    if (SauvePDG(PDFASauver)) {
        PDGOuvertePourEdition = PDFASauver;
        Consigne("Page de garde " + PDGOuvertePourEdition + " créée sur disque");
    }
}

/** RetourneIndexLibre:
  Retourne le prochain numéro d'index libre

  @return qint64
*/
qint64
MainWindow::RetourneIndexLibre()
{
    QVector<qint64> ListeIndex;
    if (ui->EDIT_Liste->count() > 0)
        for (int i = 0; i < ui->EDIT_Liste->count(); ++i) {
            auto itemWidget = qobject_cast<BlocEditeur*>(ui->EDIT_Liste->itemWidget(ui->EDIT_Liste->item(i)));
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

/** on_EDIT_Bouton_Ligne_clicked:
  Ajoute un controle DessineTexte

  @return Aucun
*/
void
MainWindow::on_EDIT_Bouton_Ligne_clicked()
{
    Bloc::ItemDefinition StructBloc = Bloc::ItemDefinition();
    StructBloc.TypeAction           = Bloc::TypeAction::DESSINELIGNE;
    StructBloc.IndexControle        = RetourneIndexLibre();
    StructBloc.NomControle          = QString("DESSINETEXTE " + QString::number(StructBloc.IndexControle));
    StructBloc.Couleur              = "#000000";
    BlocEditeur* mBlocEditeur       = new BlocEditeur(mPressePapier, this, StructBloc, ThemeDark);
    ui->EDIT_Liste->setUpdatesEnabled(false);
    QListWidgetItem* item;
    item = new QListWidgetItem(ui->EDIT_Liste);
    mBlocEditeur->setWhatsThis(StructBloc.NomControle);
    ui->EDIT_Liste->addItem(item);
    item->setSizeHint(mBlocEditeur->minimumSizeHint());
    ui->EDIT_Liste->setItemWidget(item, mBlocEditeur);
    ui->EDIT_Liste->setUpdatesEnabled(true);
    Consigne("Ajout du bloc " + StructBloc.NomControle);
}

/** on_EDIT_Bouton_RectVide_clicked:
  Ajoute un controle DESSINERECTANGLEVIDE

  @return Aucun
*/
void
MainWindow::on_EDIT_Bouton_RectVide_clicked()
{
    Bloc::ItemDefinition StructBloc = Bloc::ItemDefinition();
    StructBloc.TypeAction           = Bloc::TypeAction::DESSINERECTANGLEVIDE;
    StructBloc.IndexControle        = RetourneIndexLibre();
    StructBloc.NomControle          = QString("DESSINERECTANGLEVIDE " + QString::number(StructBloc.IndexControle));
    StructBloc.Couleur              = "#000000";
    BlocEditeur* mBlocEditeur       = new BlocEditeur(mPressePapier, this, StructBloc, ThemeDark);
    ui->EDIT_Liste->setUpdatesEnabled(false);
    QListWidgetItem* item;
    item = new QListWidgetItem(ui->EDIT_Liste);
    mBlocEditeur->setWhatsThis(StructBloc.NomControle);
    ui->EDIT_Liste->addItem(item);
    item->setSizeHint(mBlocEditeur->minimumSizeHint());
    ui->EDIT_Liste->setItemWidget(item, mBlocEditeur);
    ui->EDIT_Liste->setUpdatesEnabled(true);
    Consigne("Ajout du bloc " + StructBloc.NomControle);
}

/** on_EDIT_Bouton_RectGrille_clicked:
  Ajoute un controle DESSINERECTANGLEGRILLE

  @return Aucun
*/
void
MainWindow::on_EDIT_Bouton_RectGrille_clicked()
{
    Bloc::ItemDefinition StructBloc = Bloc::ItemDefinition();
    StructBloc.TypeAction           = Bloc::TypeAction::DESSINERECTANGLEGRILLE;
    StructBloc.IndexControle        = RetourneIndexLibre();
    StructBloc.NomControle          = QString("DESSINERECTANGLEGRILLE " + QString::number(StructBloc.IndexControle));
    StructBloc.Couleur              = "#000000";
    BlocEditeur* mBlocEditeur       = new BlocEditeur(mPressePapier, this, StructBloc, ThemeDark);
    ui->EDIT_Liste->setUpdatesEnabled(false);
    QListWidgetItem* item;
    item = new QListWidgetItem(ui->EDIT_Liste);
    mBlocEditeur->setWhatsThis(StructBloc.NomControle);
    ui->EDIT_Liste->addItem(item);
    item->setSizeHint(mBlocEditeur->minimumSizeHint());
    ui->EDIT_Liste->setItemWidget(item, mBlocEditeur);
    ui->EDIT_Liste->setUpdatesEnabled(true);
    Consigne("Ajout du bloc " + StructBloc.NomControle);
}

/** on_EDIT_Bouton_RectRemplis_clicked:
  Ajoute un controle DESSINERECTANGLEREMPLIS

  @return Aucun
*/
void
MainWindow::on_EDIT_Bouton_RectRemplis_clicked()
{
    Bloc::ItemDefinition StructBloc = Bloc::ItemDefinition();
    StructBloc.TypeAction           = Bloc::TypeAction::DESSINERECTANGLEREMPLIS;
    StructBloc.IndexControle        = RetourneIndexLibre();
    StructBloc.NomControle          = QString("DESSINERECTANGLEREMPLIS " + QString::number(StructBloc.IndexControle));
    StructBloc.Couleur              = "#000000";
    StructBloc.CouleurRemplissage   = "#000000";
    BlocEditeur* mBlocEditeur       = new BlocEditeur(mPressePapier, this, StructBloc, ThemeDark);
    ui->EDIT_Liste->setUpdatesEnabled(false);
    QListWidgetItem* item;
    item = new QListWidgetItem(ui->EDIT_Liste);
    mBlocEditeur->setWhatsThis(StructBloc.NomControle);
    ui->EDIT_Liste->addItem(item);
    item->setSizeHint(mBlocEditeur->minimumSizeHint());
    ui->EDIT_Liste->setItemWidget(item, mBlocEditeur);
    ui->EDIT_Liste->setUpdatesEnabled(true);
    Consigne("Ajout du bloc " + StructBloc.NomControle);
}

/** on_EDIT_Bouton_Texte_clicked:
  Ajoute un controle DESSINETEXTE

  @return Aucun
*/
void
MainWindow::on_EDIT_Bouton_Texte_clicked()
{
    Bloc::ItemDefinition StructBloc   = Bloc::ItemDefinition();
    StructBloc.TypeAction             = Bloc::TypeAction::DESSINETEXTE;
    StructBloc.IndexControle          = RetourneIndexLibre();
    StructBloc.NomControle            = QString("DESSINETEXTE " + QString::number(StructBloc.IndexControle));
    StructBloc.Couleur                = "#000000";
    StructBloc.Alignement_Horizontale = Bloc::AlignementHorizontale::Gauche;
    StructBloc.Alignement_Verticale   = Bloc::AlignementVerticale::Centre;
    BlocEditeur* mBlocEditeur         = new BlocEditeur(mPressePapier, this, StructBloc, ThemeDark);
    ui->EDIT_Liste->setUpdatesEnabled(false);
    QListWidgetItem* item;
    item = new QListWidgetItem(ui->EDIT_Liste);
    mBlocEditeur->setWhatsThis(StructBloc.NomControle);
    ui->EDIT_Liste->addItem(item);
    item->setSizeHint(mBlocEditeur->minimumSizeHint());
    ui->EDIT_Liste->setItemWidget(item, mBlocEditeur);
    ui->EDIT_Liste->setUpdatesEnabled(true);
    Consigne("Ajout du bloc " + StructBloc.NomControle);
}

/** on_EDIT_Bouton_TexteMulti_clicked:
  Ajoute un controle DESSINETEXTEMULTILIGNE

  @return Aucun
*/
void
MainWindow::on_EDIT_Bouton_TexteMulti_clicked()
{
    Bloc::ItemDefinition StructBloc   = Bloc::ItemDefinition();
    StructBloc.TypeAction             = Bloc::TypeAction::DESSINETEXTEMULTILIGNE;
    StructBloc.IndexControle          = RetourneIndexLibre();
    StructBloc.NomControle            = QString("DESSINETEXTEMULTILIGNE " + QString::number(StructBloc.IndexControle));
    StructBloc.Couleur                = "#000000";
    StructBloc.Alignement_Horizontale = Bloc::AlignementHorizontale::Gauche;
    StructBloc.Alignement_Verticale   = Bloc::AlignementVerticale::Centre;
    BlocEditeur* mBlocEditeur         = new BlocEditeur(mPressePapier, this, StructBloc, ThemeDark);
    ui->EDIT_Liste->setUpdatesEnabled(false);
    QListWidgetItem* item;
    item = new QListWidgetItem(ui->EDIT_Liste);
    mBlocEditeur->setWhatsThis(StructBloc.NomControle);
    ui->EDIT_Liste->addItem(item);
    item->setSizeHint(mBlocEditeur->minimumSizeHint());
    ui->EDIT_Liste->setItemWidget(item, mBlocEditeur);
    ui->EDIT_Liste->setUpdatesEnabled(true);
    Consigne("Ajout du bloc " + StructBloc.NomControle);
}

/** on_EDIT_Bouton_TexteQuestion_clicked:
  Ajoute un controle DESSINETEXTEQUESTION

  @return Aucun
*/
void
MainWindow::on_EDIT_Bouton_TexteQuestion_clicked()
{
    Bloc::ItemDefinition StructBloc   = Bloc::ItemDefinition();
    StructBloc.TypeAction             = Bloc::TypeAction::DESSINETEXTEQUESTION;
    StructBloc.IndexControle          = RetourneIndexLibre();
    StructBloc.NomControle            = QString("DESSINETEXTEQUESTION " + QString::number(StructBloc.IndexControle));
    StructBloc.Couleur                = "#000000";
    StructBloc.Alignement_Horizontale = Bloc::AlignementHorizontale::Gauche;
    StructBloc.Alignement_Verticale   = Bloc::AlignementVerticale::Centre;
    BlocEditeur* mBlocEditeur         = new BlocEditeur(mPressePapier, this, StructBloc, ThemeDark);
    ui->EDIT_Liste->setUpdatesEnabled(false);
    QListWidgetItem* item;
    item = new QListWidgetItem(ui->EDIT_Liste);
    mBlocEditeur->setWhatsThis(StructBloc.NomControle);
    ui->EDIT_Liste->addItem(item);
    item->setSizeHint(mBlocEditeur->minimumSizeHint());
    ui->EDIT_Liste->setItemWidget(item, mBlocEditeur);
    ui->EDIT_Liste->setUpdatesEnabled(true);
    Consigne("Ajout du bloc " + StructBloc.NomControle);
}

/** on_EDIT_Bouton_TexteMultiQuestion_clicked:
  Ajoute un controle DESSINETEXTEMULTILIGNEQUESTION

  @return Aucun
*/
void
MainWindow::on_EDIT_Bouton_TexteMultiQuestion_clicked()
{
    Bloc::ItemDefinition StructBloc   = Bloc::ItemDefinition();
    StructBloc.TypeAction             = Bloc::TypeAction::DESSINETEXTEMULTILIGNEQUESTION;
    StructBloc.IndexControle          = RetourneIndexLibre();
    StructBloc.NomControle            = QString("DESSINETEXTEMULTILIGNEQUESTION " + QString::number(StructBloc.IndexControle));
    StructBloc.Couleur                = "#000000";
    StructBloc.Alignement_Horizontale = Bloc::AlignementHorizontale::Gauche;
    StructBloc.Alignement_Verticale   = Bloc::AlignementVerticale::Centre;
    BlocEditeur* mBlocEditeur         = new BlocEditeur(mPressePapier, this, StructBloc, ThemeDark);
    ui->EDIT_Liste->setUpdatesEnabled(false);
    QListWidgetItem* item;
    item = new QListWidgetItem(ui->EDIT_Liste);
    mBlocEditeur->setWhatsThis(StructBloc.NomControle);
    ui->EDIT_Liste->addItem(item);
    item->setSizeHint(mBlocEditeur->minimumSizeHint());
    ui->EDIT_Liste->setItemWidget(item, mBlocEditeur);
    ui->EDIT_Liste->setUpdatesEnabled(true);
    Consigne("Ajout du bloc " + StructBloc.NomControle);
}

/** on_EDIT_Bouton_Checkbox_clicked:
  Ajoute un controle DESSINECHECKBOX

  @return Aucun
*/
void
MainWindow::on_EDIT_Bouton_Checkbox_clicked()
{
    Bloc::ItemDefinition StructBloc = Bloc::ItemDefinition();
    StructBloc.TypeAction           = Bloc::TypeAction::DESSINECHECKBOX;
    StructBloc.IndexControle        = RetourneIndexLibre();
    StructBloc.NomControle          = QString("DESSINECHECKBOX " + QString::number(StructBloc.IndexControle));
    StructBloc.Couleur              = "#000000";
    BlocEditeur* mBlocEditeur       = new BlocEditeur(mPressePapier, this, StructBloc, ThemeDark);
    ui->EDIT_Liste->setUpdatesEnabled(false);
    QListWidgetItem* item;
    item = new QListWidgetItem(ui->EDIT_Liste);
    mBlocEditeur->setWhatsThis(StructBloc.NomControle);
    ui->EDIT_Liste->addItem(item);
    item->setSizeHint(mBlocEditeur->minimumSizeHint());
    ui->EDIT_Liste->setItemWidget(item, mBlocEditeur);
    ui->EDIT_Liste->setUpdatesEnabled(true);
    Consigne("Ajout du bloc " + StructBloc.NomControle);
}

/** on_EDIT_Bouton_CheckboxQeustion_clicked:
  Ajoute un controle DESSINECHECKBOXQUESTION

  @return Aucun
*/
void
MainWindow::on_EDIT_Bouton_CheckboxQeustion_clicked()
{
    Bloc::ItemDefinition StructBloc = Bloc::ItemDefinition();
    StructBloc.TypeAction           = Bloc::TypeAction::DESSINECHECKBOXQUESTION;
    StructBloc.IndexControle        = RetourneIndexLibre();
    StructBloc.NomControle          = QString("DESSINECHECKBOXQUESTION " + QString::number(StructBloc.IndexControle));
    StructBloc.Couleur              = "#000000";
    BlocEditeur* mBlocEditeur       = new BlocEditeur(mPressePapier, this, StructBloc, ThemeDark);
    ui->EDIT_Liste->setUpdatesEnabled(false);
    QListWidgetItem* item;
    item = new QListWidgetItem(ui->EDIT_Liste);
    mBlocEditeur->setWhatsThis(StructBloc.NomControle);
    ui->EDIT_Liste->addItem(item);
    item->setSizeHint(mBlocEditeur->minimumSizeHint());
    ui->EDIT_Liste->setItemWidget(item, mBlocEditeur);
    ui->EDIT_Liste->setUpdatesEnabled(true);
    Consigne("Ajout du bloc " + StructBloc.NomControle);
}

/** on_EDIT_Bouton_MultiCheckboxQuestion_clicked:
  Ajoute un controle DESSINEMULTICHECKBOXQUESTION

  @return Aucun
*/
void
MainWindow::on_EDIT_Bouton_MultiCheckboxQuestion_clicked()
{
    Bloc::ItemDefinition StructBloc = Bloc::ItemDefinition();
    StructBloc.TypeAction           = Bloc::TypeAction::DESSINEMULTICHECKBOXQUESTION;
    StructBloc.IndexControle        = RetourneIndexLibre();
    StructBloc.NomControle          = QString("DESSINEMULTICHECKBOXQUESTION " + QString::number(StructBloc.IndexControle));
    StructBloc.Couleur              = "#000000";
    BlocEditeur* mBlocEditeur       = new BlocEditeur(mPressePapier, this, StructBloc, ThemeDark);
    ui->EDIT_Liste->setUpdatesEnabled(false);
    QListWidgetItem* item;
    item = new QListWidgetItem(ui->EDIT_Liste);
    mBlocEditeur->setWhatsThis(StructBloc.NomControle);
    ui->EDIT_Liste->addItem(item);
    item->setSizeHint(mBlocEditeur->minimumSizeHint());
    ui->EDIT_Liste->setItemWidget(item, mBlocEditeur);
    ui->EDIT_Liste->setUpdatesEnabled(true);
    Consigne("Ajout du bloc " + StructBloc.NomControle);
}

/** on_EDIT_Bouton_InsereImage_clicked:
  Ajoute un controle INSEREIMAGE

  @return Aucun
*/
void
MainWindow::on_EDIT_Bouton_InsereImage_clicked()
{
    Bloc::ItemDefinition StructBloc = Bloc::ItemDefinition();
    StructBloc.TypeAction           = Bloc::TypeAction::INSEREIMAGE;
    StructBloc.IndexControle        = RetourneIndexLibre();
    StructBloc.NomControle          = QString("INSEREIMAGE " + QString::number(StructBloc.IndexControle));
    StructBloc.CheminImage          = "../Img_resource/";
    BlocEditeur* mBlocEditeur       = new BlocEditeur(mPressePapier, this, StructBloc, ThemeDark);
    ui->EDIT_Liste->setUpdatesEnabled(false);
    QListWidgetItem* item;
    item = new QListWidgetItem(ui->EDIT_Liste);
    mBlocEditeur->setWhatsThis(StructBloc.NomControle);
    ui->EDIT_Liste->addItem(item);
    item->setSizeHint(mBlocEditeur->minimumSizeHint());
    ui->EDIT_Liste->setItemWidget(item, mBlocEditeur);
    ui->EDIT_Liste->setUpdatesEnabled(true);
    Consigne("Ajout du bloc " + StructBloc.NomControle);
}

/** on_EDIT_Bouton_PageSuivante_clicked:
  Ajoute un controle PAGESUIVANTE

  @return Aucun
*/
void
MainWindow::on_EDIT_Bouton_PageSuivante_clicked()
{
    Bloc::ItemDefinition StructBloc = Bloc::ItemDefinition();
    StructBloc.TypeAction           = Bloc::TypeAction::PAGESUIVANTE;
    StructBloc.IndexControle        = RetourneIndexLibre();
    StructBloc.NomControle          = QString("PAGESUIVANTE " + QString::number(StructBloc.IndexControle));
    BlocEditeur* mBlocEditeur       = new BlocEditeur(mPressePapier, this, StructBloc, ThemeDark);
    ui->EDIT_Liste->setUpdatesEnabled(false);
    QListWidgetItem* item;
    item = new QListWidgetItem(ui->EDIT_Liste);
    mBlocEditeur->setWhatsThis(StructBloc.NomControle);
    ui->EDIT_Liste->addItem(item);
    item->setSizeHint(mBlocEditeur->minimumSizeHint());
    ui->EDIT_Liste->setItemWidget(item, mBlocEditeur);
    ui->EDIT_Liste->setUpdatesEnabled(true);
    Consigne("Ajout du bloc " + StructBloc.NomControle);
}

/** on_EDIT_Bouton_Commentaire_clicked:
  Ajoute un controle COMMENTAIRE

  @return Aucun
*/
void
MainWindow::on_EDIT_Bouton_Commentaire_clicked()
{
    Bloc::ItemDefinition StructBloc = Bloc::ItemDefinition();
    StructBloc.TypeAction           = Bloc::TypeAction::COMMENTAIRE;
    StructBloc.IndexControle        = RetourneIndexLibre();
    StructBloc.NomControle          = QString("COMMENTAIRE " + QString::number(StructBloc.IndexControle));
    BlocEditeur* mBlocEditeur       = new BlocEditeur(mPressePapier, this, StructBloc, ThemeDark);
    ui->EDIT_Liste->setUpdatesEnabled(false);
    QListWidgetItem* item;
    item = new QListWidgetItem(ui->EDIT_Liste);
    mBlocEditeur->setWhatsThis(StructBloc.NomControle);
    ui->EDIT_Liste->addItem(item);
    item->setSizeHint(mBlocEditeur->minimumSizeHint());
    ui->EDIT_Liste->setItemWidget(item, mBlocEditeur);
    ui->EDIT_Liste->setUpdatesEnabled(true);
    Consigne("Ajout du bloc " + StructBloc.NomControle);
}

/** on_Edit_Bouton_OutilCouleur_clicked:
  Fait apparaitre un picked de couleur

  @return Aucun
*/
void
MainWindow::on_Edit_Bouton_OutilCouleur_clicked()
{
    MemColor = QColorDialog::getColor(MemColor, this);
}

/** GetNumArg:
  Récupère le nombre d'argument

  @param QVector<QString>& lVecList
  @param int indexDepart
  @return Aucun
*/
int
MainWindow::GetNumArg(QVector<QString>& lVecList, int indexDepart)
{
    int nbArg = 0;
    for (qsizetype i = indexDepart; i < lVecList.size(); i++) {
        if (lVecList[i].mid(0, 2) == "--")
            nbArg++;
        else
            break;
    }

    return nbArg;
}

/** GetNumCOMM:
  Récupère le nombre de commentaire

  @param QVector<QString>& lVecList
  @param int indexDepart
  @return Aucun
*/
int
MainWindow::GetNumCOMM(QVector<QString>& lVecList, int indexDepart)
{
    int nbArg = 0;
    for (qsizetype i = indexDepart; i < lVecList.size(); i++) {
        if (lVecList[i].mid(0, 2) == "::")
            nbArg++;
        else
            break;
    }
    return nbArg;
}

/** RetourneCleStr:
  Retourne la valeur STRING de la clé, si non trouvé, retourne
 "<!--CleNonTrouve-->"

  @param QVector<QString>& lVecKey
  @param QString Cle
  @param QString Defaut
  @return QString
*/
QString
MainWindow::RetourneCleStr(QVector<QString>& lVecKey, QString Cle, QString Defaut)
{
    for (qsizetype i = 0; i < lVecKey.size(); i++) {
        if (lVecKey[i].length() < Cle.length() + 2 /*--*/)
            continue;
        if (lVecKey[i].mid(2, Cle.length()) == Cle) {
            QString mVal = lVecKey[i].mid(2 + 1 /*=*/ + Cle.length());
            if (mVal.mid(0, 1) == "\"")
                mVal = mVal.mid(1);
            if (mVal.mid(mVal.length() - 1, 1) == "\"")
                mVal = mVal.mid(0, mVal.length() - 1);
            mVal = mVal.replace("{RetourLigne}", "\n");
            return mVal;
            break;
        }
    }
    return Defaut;
}

/** RetourneCleDouble:
  Retourne la valeur DOUBLE de la clé, si non trouvé, retourne
 (double)INT16_MAX

  @param QVector<QString>& lVecKey
  @param QString Cle
  @param QString Defaut
  @return double
*/
double
MainWindow::RetourneCleDouble(QVector<QString>& lVecKey, QString Cle, double Defaut)
{
    for (qsizetype i = 0; i < lVecKey.size(); i++) {
        if (lVecKey[i].length() < Cle.length() + 2 /*--*/)
            continue;
        if (lVecKey[i].mid(2, Cle.length()) == Cle) {
            return lVecKey[i].mid(2 + 1 /*=*/ + Cle.length()).toDouble();
            break;
        }
    }
    return Defaut;
}

/** RetourneCleInt:
  Retourne la valeur int de la clé, si non trouvé, retourne INT16_MAX

  @param QVector<QString>& lVecKey
  @param QString Cle
  @param QString Defaut
  @return int
*/
int
MainWindow::RetourneCleInt(QVector<QString>& lVecKey, QString Cle, int Defaut)
{
    for (qsizetype i = 0; i < lVecKey.size(); i++) {
        if (lVecKey[i].length() < Cle.length() + 2 /*--*/)
            continue;
        if (lVecKey[i].mid(2, Cle.length()) == Cle) {
            return lVecKey[i].mid(2 + 1 /*=*/ + Cle.length()).toInt();
            break;
        }
    }
    return Defaut;
}

/** RetourneCleBool:
  Retourne la valeur Bool de la clé, si non trouvé, retourne false

  @param QVector<QString>& lVecKey
  @param QString Cle
  @param QString Defaut
  @return Bool
*/
bool
MainWindow::RetourneCleBool(QVector<QString>& lVecKey, QString Cle, bool Defaut)
{
    for (qsizetype i = 0; i < lVecKey.size(); i++) {
        if (lVecKey[i].length() < Cle.length() + 2 /*--*/)
            continue;
        if (lVecKey[i].mid(2, Cle.length()) == Cle) {
            return true;
            break;
        }
    }
    return Defaut;
}

/** ChargePDG:
  Charge le contenu d'une page de garde en mémoire

  @param QString CheminPDG
  @return bool
*/
bool
MainWindow::ChargePDG(QString CheminPDG)
{
    QFile FichierConfig(CheminPDG);
    if (!FichierConfig.open(QIODevice::ReadOnly))
        return false; // Erreur ouverture
    QVector<QString> vecFichierPDG;
    vecFichierPDG.clear();
    QTextStream in(&FichierConfig);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.length() > 2) {
            if (line.mid(0, 2) == "ï»") // C'est le BOM UTF8
                line = line.mid(3);     // Suppresiion BOM
            if (line.length() < 2)
                continue;               // Trop petit maintenant, n passe à l'item suivant
            if (line.mid(0, 2) == "ï»") // On ignore le BOM du fichier UTF8
                continue;
            line = line.trimmed();
            vecFichierPDG.push_back(line);
        }
    }
    FichierConfig.close();

    QVector<Bloc::ItemDefinition> ListeAction;
    ListeAction.clear();

    for (qsizetype compteLigne = 0; compteLigne < vecFichierPDG.size(); compteLigne++) {
        Bloc::ItemDefinition mActBtn;
        QString tLine = vecFichierPDG[compteLigne];
        if (tLine == "DESSINELIGNE") {
            mActBtn.TypeAction     = Bloc::TypeAction::DESSINELIGNE;
            int nbARG              = GetNumArg(vecFichierPDG, compteLigne + 1);
            QVector<QString> cpVec = vecFichierPDG.mid(compteLigne + 1, nbARG);
            mActBtn.DebutX         = RetourneCleDouble(cpVec, "debutx", 0.0);
            mActBtn.DebutY         = RetourneCleDouble(cpVec, "debuty", 0.0);
            mActBtn.FinX           = RetourneCleDouble(cpVec, "finx", 0.0);
            mActBtn.FinY           = RetourneCleDouble(cpVec, "finy", 0.0);
            mActBtn.Epaisseur      = RetourneCleDouble(cpVec, "epaisseur", 1.0);
            mActBtn.Couleur        = QString("#%1%2%3").arg(QString("%1").arg(RetourneCleInt(cpVec, "rouge", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "vert", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "bleu", 0), 2, 16, QLatin1Char('0')));
            if (RetourneCleInt(cpVec, "rouge", 0) == -1)
                mActBtn.Couleur = "{ACC 1}";
            if (RetourneCleInt(cpVec, "rouge", 0) == -2)
                mActBtn.Couleur = "{ACC 2}";
            ListeAction.append(mActBtn);
            compteLigne += nbARG;
            continue;
        } else if (tLine == "DESSINERECTANGLEVIDE") {
            mActBtn.TypeAction     = Bloc::TypeAction::DESSINERECTANGLEVIDE;
            int nbARG              = GetNumArg(vecFichierPDG, compteLigne + 1);
            QVector<QString> cpVec = vecFichierPDG.mid(compteLigne + 1, nbARG);
            //            cpVec.insert(cpVec.begin(), vecFichierPDG.begin() +
            //            compteLigne + 1, vecFichierPDG.begin() + compteLigne + 1 +
            //            nbARG);
            mActBtn.DebutX    = RetourneCleDouble(cpVec, "debutx", 0.0);
            mActBtn.DebutY    = RetourneCleDouble(cpVec, "debuty", 0.0);
            mActBtn.Largeur   = RetourneCleDouble(cpVec, "largeur", 1.0);
            mActBtn.Hauteur   = RetourneCleDouble(cpVec, "hauteur", 1.0);
            mActBtn.Epaisseur = RetourneCleDouble(cpVec, "epaisseur", 1.0);
            mActBtn.Couleur   = QString("#%1%2%3").arg(QString("%1").arg(RetourneCleInt(cpVec, "rouge", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "vert", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "bleu", 0), 2, 16, QLatin1Char('0')));
            if (RetourneCleInt(cpVec, "rouge", 0) == -1)
                mActBtn.Couleur = "{ACC 1}";
            if (RetourneCleInt(cpVec, "rouge", 0) == -2)
                mActBtn.Couleur = "{ACC 2}";
            ListeAction.append(mActBtn);
            compteLigne += nbARG;
            continue;
        } else if (tLine == "DESSINERECTANGLEGRILLE") {
            mActBtn.TypeAction     = Bloc::TypeAction::DESSINERECTANGLEGRILLE;
            int nbARG              = GetNumArg(vecFichierPDG, compteLigne + 1);
            QVector<QString> cpVec = vecFichierPDG.mid(compteLigne + 1, nbARG);
            mActBtn.DebutX         = RetourneCleDouble(cpVec, "debutx", 0.0);
            mActBtn.DebutY         = RetourneCleDouble(cpVec, "debuty", 0.0);
            mActBtn.Largeur        = RetourneCleDouble(cpVec, "largeur", 1.0);
            mActBtn.Hauteur        = RetourneCleDouble(cpVec, "hauteur", 1.0);
            mActBtn.Epaisseur      = RetourneCleDouble(cpVec, "epaisseur", 1.0);
            mActBtn.Couleur        = QString("#%1%2%3").arg(QString("%1").arg(RetourneCleInt(cpVec, "rouge", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "vert", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "bleu", 0), 2, 16, QLatin1Char('0')));
            if (RetourneCleInt(cpVec, "rouge", 0) == -1)
                mActBtn.Couleur = "{ACC 1}";
            if (RetourneCleInt(cpVec, "rouge", 0) == -2)
                mActBtn.Couleur = "{ACC 2}";
            mActBtn.NombreColonne = RetourneCleInt(cpVec, "nombrecolonne", 1);
            mActBtn.NombreLigne   = RetourneCleInt(cpVec, "nombreligne", 1);
            ListeAction.append(mActBtn);
            compteLigne += nbARG;
            continue;
        } else if (tLine == "DESSINERECTANGLEREMPLIS") {
            mActBtn.TypeAction     = Bloc::TypeAction::DESSINERECTANGLEREMPLIS;
            int nbARG              = GetNumArg(vecFichierPDG, compteLigne + 1);
            QVector<QString> cpVec = vecFichierPDG.mid(compteLigne + 1, nbARG);
            mActBtn.DebutX         = RetourneCleDouble(cpVec, "debutx", 0.0);
            mActBtn.DebutY         = RetourneCleDouble(cpVec, "debuty", 0.0);
            mActBtn.Largeur        = RetourneCleDouble(cpVec, "largeur", 1.0);
            mActBtn.Hauteur        = RetourneCleDouble(cpVec, "hauteur", 1.0);
            mActBtn.Epaisseur      = RetourneCleDouble(cpVec, "epaisseur", 1.0);
            mActBtn.Couleur        = QString("#%1%2%3").arg(QString("%1").arg(RetourneCleInt(cpVec, "rouge", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "vert", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "bleu", 0), 2, 16, QLatin1Char('0')));
            if (RetourneCleInt(cpVec, "rouge", 0) == -1)
                mActBtn.Couleur = "{ACC 1}";
            if (RetourneCleInt(cpVec, "rouge", 0) == -2)
                mActBtn.Couleur = "{ACC 2}";
            mActBtn.CouleurRemplissage = QString("#%1%2%3").arg(QString("%1").arg(RetourneCleInt(cpVec, "remplisrouge", 0), 2, 16, QLatin1Char('0')),
                                                                QString("%1").arg(RetourneCleInt(cpVec, "remplisvert", 0), 2, 16, QLatin1Char('0')),
                                                                QString("%1").arg(RetourneCleInt(cpVec, "remplisbleu", 0), 2, 16, QLatin1Char('0')));
            if (RetourneCleInt(cpVec, "remplisrouge", 0) == -1)
                mActBtn.CouleurRemplissage = "{ACC 1}";
            if (RetourneCleInt(cpVec, "remplisrouge", 0) == -2)
                mActBtn.CouleurRemplissage = "{ACC 2}";
            ListeAction.append(mActBtn);
            compteLigne += nbARG;
            continue;
        } else if (tLine == "DESSINETEXTE") {
            mActBtn.TypeAction             = Bloc::TypeAction::DESSINETEXTE;
            int nbARG                      = GetNumArg(vecFichierPDG, compteLigne + 1);
            QVector<QString> cpVec         = vecFichierPDG.mid(compteLigne + 1, nbARG);
            mActBtn.DebutX                 = RetourneCleDouble(cpVec, "debutx", 0.0);
            mActBtn.DebutY                 = RetourneCleDouble(cpVec, "debuty", 0.0);
            mActBtn.Largeur                = RetourneCleDouble(cpVec, "largeur", 1.0);
            mActBtn.Hauteur                = RetourneCleDouble(cpVec, "hauteur", 1.0);
            mActBtn.TaillePolice           = RetourneCleDouble(cpVec, "taillepolice", 10.0);
            mActBtn.Alignement_Horizontale = Bloc::AlignementHorizontale::Gauche;
            if (RetourneCleInt(cpVec, "alignlargeur", 0) == 0)
                mActBtn.Alignement_Horizontale = Bloc::AlignementHorizontale::Gauche;
            if (RetourneCleInt(cpVec, "alignlargeur", 0) == 1)
                mActBtn.Alignement_Horizontale = Bloc::AlignementHorizontale::Milieu;
            if (RetourneCleInt(cpVec, "alignlargeur", 0) == 2)
                mActBtn.Alignement_Horizontale = Bloc::AlignementHorizontale::Droite;
            mActBtn.Alignement_Verticale = Bloc::AlignementVerticale::Centre;
            if (RetourneCleInt(cpVec, "alignhauteur", 1) == 0)
                mActBtn.Alignement_Verticale = Bloc::AlignementVerticale::Haut;
            if (RetourneCleInt(cpVec, "alignhauteur", 1) == 1)
                mActBtn.Alignement_Verticale = Bloc::AlignementVerticale::Centre;
            if (RetourneCleInt(cpVec, "alignhauteur", 1) == 2)
                mActBtn.Alignement_Verticale = Bloc::AlignementVerticale::Bas;
            mActBtn.ValeurSplit = RetourneCleInt(cpVec, "split", 0);
            mActBtn.Couleur     = QString("#%1%2%3").arg(QString("%1").arg(RetourneCleInt(cpVec, "rouge", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "vert", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "bleu", 0), 2, 16, QLatin1Char('0')));
            if (RetourneCleInt(cpVec, "rouge", 0) == -1)
                mActBtn.Couleur = "{ACC 1}";
            if (RetourneCleInt(cpVec, "rouge", 0) == -2)
                mActBtn.Couleur = "{ACC 2}";

            mActBtn.Texte = RetourneCleStr(cpVec, "texte");

            mActBtn.Gras           = RetourneCleBool(cpVec, "gras");
            mActBtn.GrasEtItalique = RetourneCleBool(cpVec, "grasitalic");
            mActBtn.Italique       = RetourneCleBool(cpVec, "italic");
            mActBtn.Monospace      = RetourneCleBool(cpVec, "monospace");

            ListeAction.append(mActBtn);
            compteLigne += nbARG;
            continue;
        } else if (tLine == "DESSINETEXTEMULTILIGNE") {
            mActBtn.TypeAction             = Bloc::TypeAction::DESSINETEXTEMULTILIGNE;
            int nbARG                      = GetNumArg(vecFichierPDG, compteLigne + 1);
            QVector<QString> cpVec         = vecFichierPDG.mid(compteLigne + 1, nbARG);
            mActBtn.DebutX                 = RetourneCleDouble(cpVec, "debutx", 0.0);
            mActBtn.DebutY                 = RetourneCleDouble(cpVec, "debuty", 0.0);
            mActBtn.Largeur                = RetourneCleDouble(cpVec, "largeur", 1.0);
            mActBtn.Hauteur                = RetourneCleDouble(cpVec, "hauteur", 1.0);
            mActBtn.TaillePolice           = RetourneCleDouble(cpVec, "taillepolice", 10.0);
            mActBtn.Alignement_Horizontale = Bloc::AlignementHorizontale::Gauche;
            if (RetourneCleInt(cpVec, "alignlargeur", 0) == 0)
                mActBtn.Alignement_Horizontale = Bloc::AlignementHorizontale::Gauche;
            if (RetourneCleInt(cpVec, "alignlargeur", 0) == 1)
                mActBtn.Alignement_Horizontale = Bloc::AlignementHorizontale::Milieu;
            if (RetourneCleInt(cpVec, "alignlargeur", 0) == 2)
                mActBtn.Alignement_Horizontale = Bloc::AlignementHorizontale::Droite;
            mActBtn.Alignement_Verticale = Bloc::AlignementVerticale::Centre;
            if (RetourneCleInt(cpVec, "alignhauteur", 1) == 0)
                mActBtn.Alignement_Verticale = Bloc::AlignementVerticale::Haut;
            if (RetourneCleInt(cpVec, "alignhauteur", 1) == 1)
                mActBtn.Alignement_Verticale = Bloc::AlignementVerticale::Centre;
            if (RetourneCleInt(cpVec, "alignhauteur", 1) == 2)
                mActBtn.Alignement_Verticale = Bloc::AlignementVerticale::Bas;
            mActBtn.Couleur = QString("#%1%2%3").arg(QString("%1").arg(RetourneCleInt(cpVec, "rouge", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "vert", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "bleu", 0), 2, 16, QLatin1Char('0')));
            if (RetourneCleInt(cpVec, "rouge", 0) == -1)
                mActBtn.Couleur = "{ACC 1}";
            if (RetourneCleInt(cpVec, "rouge", 0) == -2)
                mActBtn.Couleur = "{ACC 2}";

            mActBtn.TexteMultiligne = RetourneCleStr(cpVec, "texte");

            mActBtn.Gras           = RetourneCleBool(cpVec, "gras");
            mActBtn.GrasEtItalique = RetourneCleBool(cpVec, "grasitalic");
            mActBtn.Italique       = RetourneCleBool(cpVec, "italic");
            mActBtn.Monospace      = RetourneCleBool(cpVec, "monospace");

            ListeAction.append(mActBtn);
            compteLigne += nbARG;
            continue;
        } else if (tLine == "DESSINETEXTEQUESTION") {
            mActBtn.TypeAction             = Bloc::TypeAction::DESSINETEXTEQUESTION;
            int nbARG                      = GetNumArg(vecFichierPDG, compteLigne + 1);
            QVector<QString> cpVec         = vecFichierPDG.mid(compteLigne + 1, nbARG);
            mActBtn.DebutX                 = RetourneCleDouble(cpVec, "debutx", 0.0);
            mActBtn.DebutY                 = RetourneCleDouble(cpVec, "debuty", 0.0);
            mActBtn.Largeur                = RetourneCleDouble(cpVec, "largeur", 1.0);
            mActBtn.Hauteur                = RetourneCleDouble(cpVec, "hauteur", 1.0);
            mActBtn.TaillePolice           = RetourneCleDouble(cpVec, "taillepolice", 10.0);
            mActBtn.Alignement_Horizontale = Bloc::AlignementHorizontale::Gauche;
            if (RetourneCleInt(cpVec, "alignlargeur", 0) == 0)
                mActBtn.Alignement_Horizontale = Bloc::AlignementHorizontale::Gauche;
            if (RetourneCleInt(cpVec, "alignlargeur", 0) == 1)
                mActBtn.Alignement_Horizontale = Bloc::AlignementHorizontale::Milieu;
            if (RetourneCleInt(cpVec, "alignlargeur", 0) == 2)
                mActBtn.Alignement_Horizontale = Bloc::AlignementHorizontale::Droite;
            mActBtn.Alignement_Verticale = Bloc::AlignementVerticale::Centre;
            if (RetourneCleInt(cpVec, "alignhauteur", 1) == 0)
                mActBtn.Alignement_Verticale = Bloc::AlignementVerticale::Haut;
            if (RetourneCleInt(cpVec, "alignhauteur", 1) == 1)
                mActBtn.Alignement_Verticale = Bloc::AlignementVerticale::Centre;
            if (RetourneCleInt(cpVec, "alignhauteur", 1) == 2)
                mActBtn.Alignement_Verticale = Bloc::AlignementVerticale::Bas;
            mActBtn.ValeurSplit      = RetourneCleInt(cpVec, "split", 0);
            mActBtn.LongueurMaximale = RetourneCleInt(cpVec, "max", 4000);
            mActBtn.Couleur          = QString("#%1%2%3").arg(QString("%1").arg(RetourneCleInt(cpVec, "rouge", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "vert", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "bleu", 0), 2, 16, QLatin1Char('0')));
            if (RetourneCleInt(cpVec, "rouge", 0) == -1)
                mActBtn.Couleur = "{ACC 1}";
            if (RetourneCleInt(cpVec, "rouge", 0) == -2)
                mActBtn.Couleur = "{ACC 2}";

            mActBtn.Question       = RetourneCleStr(cpVec, "question");
            mActBtn.QuestionAide   = RetourneCleStr(cpVec, "aidequestion");
            mActBtn.QuestionDefaut = RetourneCleStr(cpVec, "defautquestion");

            mActBtn.Gras           = RetourneCleBool(cpVec, "gras");
            mActBtn.GrasEtItalique = RetourneCleBool(cpVec, "grasitalic");
            mActBtn.Italique       = RetourneCleBool(cpVec, "italic");
            mActBtn.Monospace      = RetourneCleBool(cpVec, "monospace");
            mActBtn.Obligatoire    = RetourneCleBool(cpVec, "obligatoire");
            mActBtn.Chiffre        = RetourneCleBool(cpVec, "chiffre");
            mActBtn.Majuscule      = RetourneCleBool(cpVec, "majuscule");
            mActBtn.Minuscule      = RetourneCleBool(cpVec, "minuscule");

            ListeAction.append(mActBtn);
            compteLigne += nbARG;
            continue;
        } else if (tLine == "DESSINETEXTEMULTILIGNEQUESTION") {
            mActBtn.TypeAction             = Bloc::TypeAction::DESSINETEXTEMULTILIGNEQUESTION;
            int nbARG                      = GetNumArg(vecFichierPDG, compteLigne + 1);
            QVector<QString> cpVec         = vecFichierPDG.mid(compteLigne + 1, nbARG);
            mActBtn.DebutX                 = RetourneCleDouble(cpVec, "debutx", 0.0);
            mActBtn.DebutY                 = RetourneCleDouble(cpVec, "debuty", 0.0);
            mActBtn.Largeur                = RetourneCleDouble(cpVec, "largeur", 1.0);
            mActBtn.Hauteur                = RetourneCleDouble(cpVec, "hauteur", 1.0);
            mActBtn.TaillePolice           = RetourneCleDouble(cpVec, "taillepolice", 10.0);
            mActBtn.Alignement_Horizontale = Bloc::AlignementHorizontale::Gauche;
            if (RetourneCleInt(cpVec, "alignlargeur", 0) == 0)
                mActBtn.Alignement_Horizontale = Bloc::AlignementHorizontale::Gauche;
            if (RetourneCleInt(cpVec, "alignlargeur", 0) == 1)
                mActBtn.Alignement_Horizontale = Bloc::AlignementHorizontale::Milieu;
            if (RetourneCleInt(cpVec, "alignlargeur", 0) == 2)
                mActBtn.Alignement_Horizontale = Bloc::AlignementHorizontale::Droite;
            mActBtn.Alignement_Verticale = Bloc::AlignementVerticale::Centre;
            if (RetourneCleInt(cpVec, "alignhauteur", 1) == 0)
                mActBtn.Alignement_Verticale = Bloc::AlignementVerticale::Haut;
            if (RetourneCleInt(cpVec, "alignhauteur", 1) == 1)
                mActBtn.Alignement_Verticale = Bloc::AlignementVerticale::Centre;
            if (RetourneCleInt(cpVec, "alignhauteur", 1) == 2)
                mActBtn.Alignement_Verticale = Bloc::AlignementVerticale::Bas;
            mActBtn.LongueurMaximale = RetourneCleInt(cpVec, "max", 4000);
            mActBtn.Couleur          = QString("#%1%2%3").arg(QString("%1").arg(RetourneCleInt(cpVec, "rouge", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "vert", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "bleu", 0), 2, 16, QLatin1Char('0')));
            if (RetourneCleInt(cpVec, "rouge", 0) == -1)
                mActBtn.Couleur = "{ACC 1}";
            if (RetourneCleInt(cpVec, "rouge", 0) == -2)
                mActBtn.Couleur = "{ACC 2}";

            mActBtn.Question       = RetourneCleStr(cpVec, "question");
            mActBtn.QuestionAide   = RetourneCleStr(cpVec, "aidequestion");
            mActBtn.QuestionDefaut = RetourneCleStr(cpVec, "defautquestion");

            mActBtn.Gras           = RetourneCleBool(cpVec, "gras");
            mActBtn.GrasEtItalique = RetourneCleBool(cpVec, "grasitalic");
            mActBtn.Italique       = RetourneCleBool(cpVec, "italic");
            mActBtn.Monospace      = RetourneCleBool(cpVec, "monospace");
            mActBtn.Obligatoire    = RetourneCleBool(cpVec, "obligatoire");
            mActBtn.Majuscule      = RetourneCleBool(cpVec, "majuscule");
            mActBtn.Minuscule      = RetourneCleBool(cpVec, "minuscule");

            ListeAction.append(mActBtn);
            compteLigne += nbARG;
            continue;
        } else if (tLine == "INSEREIMAGE") {
            mActBtn.TypeAction     = Bloc::TypeAction::INSEREIMAGE;
            int nbARG              = GetNumArg(vecFichierPDG, compteLigne + 1);
            QVector<QString> cpVec = vecFichierPDG.mid(compteLigne + 1, nbARG);
            mActBtn.DebutX         = RetourneCleDouble(cpVec, "debutx", 0.0);
            mActBtn.DebutY         = RetourneCleDouble(cpVec, "debuty", 0.0);
            mActBtn.Largeur        = RetourneCleDouble(cpVec, "largeur", 1.0);
            mActBtn.Hauteur        = RetourneCleDouble(cpVec, "hauteur", 1.0);
            mActBtn.CheminImage    = RetourneCleStr(cpVec, "chemin");
            ListeAction.append(mActBtn);
            compteLigne += nbARG;
            continue;
        } else if (tLine == "DESSINECHECKBOX") {
            mActBtn.TypeAction     = Bloc::TypeAction::DESSINECHECKBOX;
            int nbARG              = GetNumArg(vecFichierPDG, compteLigne + 1);
            QVector<QString> cpVec = vecFichierPDG.mid(compteLigne + 1, nbARG);
            mActBtn.DebutX         = RetourneCleDouble(cpVec, "debutx", 0.0);
            mActBtn.DebutY         = RetourneCleDouble(cpVec, "debuty", 0.0);
            mActBtn.Largeur        = RetourneCleDouble(cpVec, "largeur", 1.0);
            mActBtn.Hauteur        = RetourneCleDouble(cpVec, "hauteur", 1.0);
            mActBtn.Epaisseur      = RetourneCleDouble(cpVec, "epaisseur", 1.0);
            mActBtn.Couleur        = QString("#%1%2%3").arg(QString("%1").arg(RetourneCleInt(cpVec, "rouge", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "vert", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "bleu", 0), 2, 16, QLatin1Char('0')));
            if (RetourneCleInt(cpVec, "rouge", 0) == -1)
                mActBtn.Couleur = "{ACC 1}";
            if (RetourneCleInt(cpVec, "rouge", 0) == -2)
                mActBtn.Couleur = "{ACC 2}";
            ListeAction.append(mActBtn);
            compteLigne += nbARG;
            continue;
        } else if (tLine == "DESSINECHECKBOXQUESTION") {
            mActBtn.TypeAction     = Bloc::TypeAction::DESSINECHECKBOXQUESTION;
            int nbARG              = GetNumArg(vecFichierPDG, compteLigne + 1);
            QVector<QString> cpVec = vecFichierPDG.mid(compteLigne + 1, nbARG);
            mActBtn.DebutX         = RetourneCleDouble(cpVec, "debutx", 0.0);
            mActBtn.DebutY         = RetourneCleDouble(cpVec, "debuty", 0.0);
            mActBtn.Largeur        = RetourneCleDouble(cpVec, "largeur", 1.0);
            mActBtn.Hauteur        = RetourneCleDouble(cpVec, "hauteur", 1.0);
            mActBtn.Epaisseur      = RetourneCleDouble(cpVec, "epaisseur", 1.0);
            mActBtn.Couleur        = QString("#%1%2%3").arg(QString("%1").arg(RetourneCleInt(cpVec, "rouge", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "vert", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "bleu", 0), 2, 16, QLatin1Char('0')));
            if (RetourneCleInt(cpVec, "rouge", 0) == -1)
                mActBtn.Couleur = "{ACC 1}";
            if (RetourneCleInt(cpVec, "rouge", 0) == -2)
                mActBtn.Couleur = "{ACC 2}";

            mActBtn.Question       = RetourneCleStr(cpVec, "question");
            mActBtn.QuestionAide   = RetourneCleStr(cpVec, "aidequestion");
            mActBtn.QuestionDefaut = RetourneCleStr(cpVec, "defautquestion");

            mActBtn.Obligatoire = RetourneCleBool(cpVec, "obligatoire");

            ListeAction.append(mActBtn);
            compteLigne += nbARG;
            continue;
        } else if (tLine == "DESSINEMULTICHECKBOXQUESTION") {
            mActBtn.TypeAction     = Bloc::TypeAction::DESSINEMULTICHECKBOXQUESTION;
            int nbARG              = GetNumArg(vecFichierPDG, compteLigne + 1);
            QVector<QString> cpVec = vecFichierPDG.mid(compteLigne + 1, nbARG);
            mActBtn.DebutX         = RetourneCleDouble(cpVec, "debutx1", 0.0);
            mActBtn.DebutY         = RetourneCleDouble(cpVec, "debuty1", 0.0);
            mActBtn.DebutX2        = RetourneCleDouble(cpVec, "debutx2", 0.0);
            mActBtn.DebutY2        = RetourneCleDouble(cpVec, "debuty2", 0.0);
            mActBtn.Largeur        = RetourneCleDouble(cpVec, "largeur", 1.0);
            mActBtn.Hauteur        = RetourneCleDouble(cpVec, "hauteur", 1.0);
            mActBtn.Epaisseur      = RetourneCleDouble(cpVec, "epaisseur", 1.0);
            mActBtn.Couleur        = QString("#%1%2%3").arg(QString("%1").arg(RetourneCleInt(cpVec, "rouge", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "vert", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "bleu", 0), 2, 16, QLatin1Char('0')));
            if (RetourneCleInt(cpVec, "rouge", 0) == -1)
                mActBtn.Couleur = "{ACC 1}";
            if (RetourneCleInt(cpVec, "rouge", 0) == -2)
                mActBtn.Couleur = "{ACC 2}";

            mActBtn.Question       = RetourneCleStr(cpVec, "question");
            mActBtn.QuestionAide   = RetourneCleStr(cpVec, "aidequestion");
            mActBtn.QuestionDefaut = RetourneCleStr(cpVec, "defautquestion");
            mActBtn.Obligatoire    = RetourneCleBool(cpVec, "obligatoire");

            ListeAction.append(mActBtn);
            compteLigne += nbARG;
            continue;
        } else if (tLine == "PAGESUIVANTE") {
            mActBtn.TypeAction = Bloc::TypeAction::PAGESUIVANTE;

            ListeAction.append(mActBtn);
            continue;
        } else {
            // Commentaire
            mActBtn.TypeAction     = Bloc::TypeAction::COMMENTAIRE;
            int nbARG              = GetNumCOMM(vecFichierPDG, compteLigne);
            QVector<QString> cpVec = vecFichierPDG.mid(compteLigne + 1, nbARG - 1);
            mActBtn.Commentaire    = "";
            for (qsizetype z = 0; z < cpVec.size(); z++) {
                mActBtn.Commentaire += cpVec[z].mid(2);
                if (z < cpVec.size() - 1)
                    mActBtn.Commentaire += "\n";
            }
            compteLigne += nbARG - 1;
            ListeAction.append(mActBtn);
            continue;
        }
    }
    ui->EDIT_Liste->clear();
    BlocEditeur BlocHelper;
    BlocHelper.PeuplerListe(ui->EDIT_Liste, ListeAction, 0);
    return true;
}

/** SauvePDG:
  Sauvegarde la page de garde actuelle sur disque

  @param QString FichierSortie
  @return bool
*/
bool
MainWindow::SauvePDG(QString FichierSortie)
{
    if (ui->EDIT_Liste->count() == 0) {
        return false;
    }
    QFile Fichier(FichierSortie);
    if (!Fichier.open(QIODevice::WriteOnly))
        return false;
    QTextStream f_out(&Fichier);

    for (int i = 0; i < ui->EDIT_Liste->count(); ++i) {
        BlocEditeur* mBlocEditeur = qobject_cast<BlocEditeur*>(ui->EDIT_Liste->itemWidget(ui->EDIT_Liste->item(i)));
        auto reponse              = mBlocEditeur->RetourneDonnee();
        switch (reponse.TypeAction) {
            case Bloc::TypeAction::DESSINELIGNE: {
                f_out << "DESSINELIGNE" << Qt::endl;
                f_out << "--debutx=" << reponse.DebutX << Qt::endl;
                f_out << "--debuty=" << reponse.DebutY << Qt::endl;
                f_out << "--finx=" << reponse.FinX << Qt::endl;
                f_out << "--finy=" << reponse.FinY << Qt::endl;
                if (reponse.Couleur == "{ACC 1}") {
                    f_out << "--rouge=" << -1 << Qt::endl;
                    f_out << "--vert=" << -1 << Qt::endl;
                    f_out << "--bleu=" << -1 << Qt::endl;
                } else if (reponse.Couleur == "{ACC 2}") {
                    f_out << "--rouge=" << -2 << Qt::endl;
                    f_out << "--vert=" << -2 << Qt::endl;
                    f_out << "--bleu=" << -2 << Qt::endl;
                } else {
                    bool bStatus = false;
                    f_out << "--rouge=" << reponse.Couleur.mid(1, 2).toUInt(&bStatus, 16) << Qt::endl;
                    f_out << "--vert=" << reponse.Couleur.mid(3, 2).toUInt(&bStatus, 16) << Qt::endl;
                    f_out << "--bleu=" << reponse.Couleur.mid(5, 2).toUInt(&bStatus, 16) << Qt::endl;
                }
                if (reponse.Epaisseur != -1000.0)
                    f_out << "--epaisseur=" << reponse.Epaisseur << Qt::endl;
            } break;
            case Bloc::TypeAction::DESSINERECTANGLEVIDE: {
                f_out << "DESSINERECTANGLEVIDE\n";
                f_out << "--debutx=" << reponse.DebutX << Qt::endl;
                f_out << "--debuty=" << reponse.DebutY << Qt::endl;
                f_out << "--largeur=" << reponse.Largeur << Qt::endl;
                f_out << "--hauteur=" << reponse.Hauteur << Qt::endl;
                if (reponse.Couleur == "{ACC 1}") {
                    f_out << "--rouge=" << -1 << Qt::endl;
                    f_out << "--vert=" << -1 << Qt::endl;
                    f_out << "--bleu=" << -1 << Qt::endl;
                } else if (reponse.Couleur == "{ACC 2}") {
                    f_out << "--rouge=" << -2 << Qt::endl;
                    f_out << "--vert=" << -2 << Qt::endl;
                    f_out << "--bleu=" << -2 << Qt::endl;
                } else {
                    bool bStatus = false;
                    f_out << "--rouge=" << reponse.Couleur.mid(1, 2).toUInt(&bStatus, 16) << Qt::endl;
                    f_out << "--vert=" << reponse.Couleur.mid(3, 2).toUInt(&bStatus, 16) << Qt::endl;
                    f_out << "--bleu=" << reponse.Couleur.mid(5, 2).toUInt(&bStatus, 16) << Qt::endl;
                }
                if (reponse.Epaisseur != -1000.0)
                    f_out << "--epaisseur=" << reponse.Epaisseur << Qt::endl;
            } break;
            case Bloc::TypeAction::DESSINERECTANGLEGRILLE: {
                f_out << "DESSINERECTANGLEGRILLE\n";
                f_out << "--debutx=" << reponse.DebutX << Qt::endl;
                f_out << "--debuty=" << reponse.DebutY << Qt::endl;
                f_out << "--largeur=" << reponse.Largeur << Qt::endl;
                f_out << "--hauteur=" << reponse.Hauteur << Qt::endl;
                f_out << "--nombrecolonne=" << reponse.NombreColonne << Qt::endl;
                f_out << "--nombreligne=" << reponse.NombreLigne << Qt::endl;
                if (reponse.Couleur == "{ACC 1}") {
                    f_out << "--rouge=" << -1 << Qt::endl;
                    f_out << "--vert=" << -1 << Qt::endl;
                    f_out << "--bleu=" << -1 << Qt::endl;
                } else if (reponse.Couleur == "{ACC 2}") {
                    f_out << "--rouge=" << -2 << Qt::endl;
                    f_out << "--vert=" << -2 << Qt::endl;
                    f_out << "--bleu=" << -2 << Qt::endl;
                } else {
                    bool bStatus = false;
                    f_out << "--rouge=" << reponse.Couleur.mid(1, 2).toUInt(&bStatus, 16) << Qt::endl;
                    f_out << "--vert=" << reponse.Couleur.mid(3, 2).toUInt(&bStatus, 16) << Qt::endl;
                    f_out << "--bleu=" << reponse.Couleur.mid(5, 2).toUInt(&bStatus, 16) << Qt::endl;
                }
                if (reponse.Epaisseur != -1000.0)
                    f_out << "--epaisseur=" << reponse.Epaisseur << Qt::endl;
            } break;
            case Bloc::TypeAction::DESSINERECTANGLEREMPLIS: {
                f_out << "DESSINERECTANGLEREMPLIS\n";
                f_out << "--debutx=" << reponse.DebutX << Qt::endl;
                f_out << "--debuty=" << reponse.DebutY << Qt::endl;
                f_out << "--largeur=" << reponse.Largeur << Qt::endl;
                f_out << "--hauteur=" << reponse.Hauteur << Qt::endl;
                if (reponse.Couleur == "{ACC 1}") {
                    f_out << "--rouge=" << -1 << Qt::endl;
                    f_out << "--vert=" << -1 << Qt::endl;
                    f_out << "--bleu=" << -1 << Qt::endl;
                } else if (reponse.Couleur == "{ACC 2}") {
                    f_out << "--rouge=" << -2 << Qt::endl;
                    f_out << "--vert=" << -2 << Qt::endl;
                    f_out << "--bleu=" << -2 << Qt::endl;
                } else {
                    bool bStatus = false;
                    f_out << "--rouge=" << reponse.Couleur.mid(1, 2).toUInt(&bStatus, 16) << Qt::endl;
                    f_out << "--vert=" << reponse.Couleur.mid(3, 2).toUInt(&bStatus, 16) << Qt::endl;
                    f_out << "--bleu=" << reponse.Couleur.mid(5, 2).toUInt(&bStatus, 16) << Qt::endl;
                }
                if (reponse.CouleurRemplissage == "{ACC 1}") {
                    f_out << "--remplisrouge=" << -1 << Qt::endl;
                    f_out << "--remplisvert=" << -1 << Qt::endl;
                    f_out << "--remplisbleu=" << -1 << Qt::endl;
                } else if (reponse.CouleurRemplissage == "{ACC 2}") {
                    f_out << "--remplisrouge=" << -2 << Qt::endl;
                    f_out << "--remplisvert=" << -2 << Qt::endl;
                    f_out << "--remplisbleu=" << -2 << Qt::endl;
                } else {
                    bool bStatus = false;
                    f_out << "--remplisrouge=" << reponse.CouleurRemplissage.mid(1, 2).toUInt(&bStatus, 16) << Qt::endl;
                    f_out << "--remplisvert=" << reponse.CouleurRemplissage.mid(3, 2).toUInt(&bStatus, 16) << Qt::endl;
                    f_out << "--remplisbleu=" << reponse.CouleurRemplissage.mid(5, 2).toUInt(&bStatus, 16) << Qt::endl;
                }
                if (reponse.Epaisseur != -1000.0)
                    f_out << "--epaisseur=" << reponse.Epaisseur << Qt::endl;
            } break;
            case Bloc::TypeAction::DESSINETEXTE: {
                f_out << "DESSINETEXTE\n";
                f_out << "--debutx=" << reponse.DebutX << Qt::endl;
                f_out << "--debuty=" << reponse.DebutY << Qt::endl;
                f_out << "--largeur=" << reponse.Largeur << Qt::endl;
                f_out << "--hauteur=" << reponse.Hauteur << Qt::endl;
                f_out << "--texte=\"" << reponse.Texte << "\"" << Qt::endl;
                if (reponse.Alignement_Horizontale == Bloc::AlignementHorizontale::Gauche)
                    f_out << "--alignlargeur=" << 0 << Qt::endl;
                if (reponse.Alignement_Horizontale == Bloc::AlignementHorizontale::Milieu)
                    f_out << "--alignlargeur=" << 1 << Qt::endl;
                if (reponse.Alignement_Horizontale == Bloc::AlignementHorizontale::Droite)
                    f_out << "--alignlargeur=" << 2 << Qt::endl;
                if (reponse.Alignement_Verticale == Bloc::AlignementVerticale::Haut)
                    f_out << "--alignlargeur=" << 0 << Qt::endl;
                if (reponse.Alignement_Verticale == Bloc::AlignementVerticale::Centre)
                    f_out << "--alignlargeur=" << 1 << Qt::endl;
                if (reponse.Alignement_Verticale == Bloc::AlignementVerticale::Bas)
                    f_out << "--alignlargeur=" << 2 << Qt::endl;
                if (reponse.Couleur == "{ACC 1}") {
                    f_out << "--rouge=" << -1 << Qt::endl;
                    f_out << "--vert=" << -1 << Qt::endl;
                    f_out << "--bleu=" << -1 << Qt::endl;
                } else if (reponse.Couleur == "{ACC 2}") {
                    f_out << "--rouge=" << -2 << Qt::endl;
                    f_out << "--vert=" << -2 << Qt::endl;
                    f_out << "--bleu=" << -2 << Qt::endl;
                } else {
                    bool bStatus = false;
                    f_out << "--rouge=" << reponse.Couleur.mid(1, 2).toUInt(&bStatus, 16) << Qt::endl;
                    f_out << "--vert=" << reponse.Couleur.mid(3, 2).toUInt(&bStatus, 16) << Qt::endl;
                    f_out << "--bleu=" << reponse.Couleur.mid(5, 2).toUInt(&bStatus, 16) << Qt::endl;
                }
                if (reponse.TaillePolice > 0)
                    f_out << "--taillepolice=" << reponse.TaillePolice << Qt::endl;
                if (reponse.ValeurSplit > 0 /*!= -1000.0*/)
                    f_out << "--split=" << reponse.ValeurSplit << Qt::endl;
                if (reponse.Gras)
                    f_out << "--gras" << Qt::endl;
                if (reponse.Italique)
                    f_out << "--italic" << Qt::endl;
                if (reponse.GrasEtItalique)
                    f_out << "--grasitalic" << Qt::endl;
                if (reponse.Monospace)
                    f_out << "--monospace" << Qt::endl;
            } break;
            case Bloc::TypeAction::DESSINETEXTEMULTILIGNE: {
                f_out << "DESSINETEXTEMULTILIGNE\n";
                f_out << "--debutx=" << reponse.DebutX << Qt::endl;
                f_out << "--debuty=" << reponse.DebutY << Qt::endl;
                f_out << "--largeur=" << reponse.Largeur << Qt::endl;
                f_out << "--hauteur=" << reponse.Hauteur << Qt::endl;
                QString TexteMulti = reponse.Texte;
                TexteMulti.replace("\r\n", "{RetourLigne}");
                TexteMulti.replace("\n", "{RetourLigne}");
                f_out << "--texte=\"" << TexteMulti << "\"" << Qt::endl;
                if (reponse.Alignement_Horizontale == Bloc::AlignementHorizontale::Gauche)
                    f_out << "--alignlargeur=" << 0 << Qt::endl;
                if (reponse.Alignement_Horizontale == Bloc::AlignementHorizontale::Milieu)
                    f_out << "--alignlargeur=" << 1 << Qt::endl;
                if (reponse.Alignement_Horizontale == Bloc::AlignementHorizontale::Droite)
                    f_out << "--alignlargeur=" << 2 << Qt::endl;
                if (reponse.Alignement_Verticale == Bloc::AlignementVerticale::Haut)
                    f_out << "--alignlargeur=" << 0 << Qt::endl;
                if (reponse.Alignement_Verticale == Bloc::AlignementVerticale::Centre)
                    f_out << "--alignlargeur=" << 1 << Qt::endl;
                if (reponse.Alignement_Verticale == Bloc::AlignementVerticale::Bas)
                    f_out << "--alignlargeur=" << 2 << Qt::endl;
                if (reponse.Couleur == "{ACC 1}") {
                    f_out << "--rouge=" << -1 << Qt::endl;
                    f_out << "--vert=" << -1 << Qt::endl;
                    f_out << "--bleu=" << -1 << Qt::endl;
                } else if (reponse.Couleur == "{ACC 2}") {
                    f_out << "--rouge=" << -2 << Qt::endl;
                    f_out << "--vert=" << -2 << Qt::endl;
                    f_out << "--bleu=" << -2 << Qt::endl;
                } else {
                    bool bStatus = false;
                    f_out << "--rouge=" << reponse.Couleur.mid(1, 2).toUInt(&bStatus, 16) << Qt::endl;
                    f_out << "--vert=" << reponse.Couleur.mid(3, 2).toUInt(&bStatus, 16) << Qt::endl;
                    f_out << "--bleu=" << reponse.Couleur.mid(5, 2).toUInt(&bStatus, 16) << Qt::endl;
                }
                if (reponse.TaillePolice > 0)
                    f_out << "--taillepolice=" << reponse.TaillePolice << Qt::endl;
                if (reponse.ValeurSplit > 0)
                    f_out << "--split=" << reponse.ValeurSplit << Qt::endl;
                if (reponse.Gras)
                    f_out << "--gras" << Qt::endl;
                if (reponse.Italique)
                    f_out << "--italic" << Qt::endl;
                if (reponse.GrasEtItalique)
                    f_out << "--grasitalic" << Qt::endl;
                if (reponse.Monospace)
                    f_out << "--monospace" << Qt::endl;
            } break;
            case Bloc::TypeAction::DESSINETEXTEQUESTION: {
                f_out << "DESSINETEXTEQUESTION\n";
                f_out << "--debutx=" << reponse.DebutX << Qt::endl;
                f_out << "--debuty=" << reponse.DebutY << Qt::endl;
                f_out << "--largeur=" << reponse.Largeur << Qt::endl;
                f_out << "--hauteur=" << reponse.Hauteur << Qt::endl;
                f_out << "--question=\"" << reponse.Question << "\"" << Qt::endl;
                f_out << "--aidequestion=\"" << reponse.QuestionAide << "\"" << Qt::endl;
                f_out << "--defautquestion=\"" << reponse.QuestionDefaut << "\"" << Qt::endl;
                if (reponse.Alignement_Horizontale == Bloc::AlignementHorizontale::Gauche)
                    f_out << "--alignlargeur=" << 0 << Qt::endl;
                if (reponse.Alignement_Horizontale == Bloc::AlignementHorizontale::Milieu)
                    f_out << "--alignlargeur=" << 1 << Qt::endl;
                if (reponse.Alignement_Horizontale == Bloc::AlignementHorizontale::Droite)
                    f_out << "--alignlargeur=" << 2 << Qt::endl;
                if (reponse.Alignement_Verticale == Bloc::AlignementVerticale::Haut)
                    f_out << "--alignlargeur=" << 0 << Qt::endl;
                if (reponse.Alignement_Verticale == Bloc::AlignementVerticale::Centre)
                    f_out << "--alignlargeur=" << 1 << Qt::endl;
                if (reponse.Alignement_Verticale == Bloc::AlignementVerticale::Bas)
                    f_out << "--alignlargeur=" << 2 << Qt::endl;
                if (reponse.Couleur == "{ACC 1}") {
                    f_out << "--rouge=" << -1 << Qt::endl;
                    f_out << "--vert=" << -1 << Qt::endl;
                    f_out << "--bleu=" << -1 << Qt::endl;
                } else if (reponse.Couleur == "{ACC 2}") {
                    f_out << "--rouge=" << -2 << Qt::endl;
                    f_out << "--vert=" << -2 << Qt::endl;
                    f_out << "--bleu=" << -2 << Qt::endl;
                } else {
                    bool bStatus = false;
                    f_out << "--rouge=" << reponse.Couleur.mid(1, 2).toUInt(&bStatus, 16) << Qt::endl;
                    f_out << "--vert=" << reponse.Couleur.mid(3, 2).toUInt(&bStatus, 16) << Qt::endl;
                    f_out << "--bleu=" << reponse.Couleur.mid(5, 2).toUInt(&bStatus, 16) << Qt::endl;
                }
                if (reponse.TaillePolice > 0)
                    f_out << "--taillepolice=" << reponse.TaillePolice << Qt::endl;
                if (reponse.Obligatoire)
                    f_out << "--obligatoire" << Qt::endl;
                if (reponse.Gras)
                    f_out << "--gras" << Qt::endl;
                if (reponse.Italique)
                    f_out << "--italic" << Qt::endl;
                if (reponse.GrasEtItalique)
                    f_out << "--grasitalic" << Qt::endl;
                if (reponse.Monospace)
                    f_out << "--monospace" << Qt::endl;
                if (reponse.Chiffre)
                    f_out << "--chiffre" << Qt::endl;
                if (reponse.Minuscule)
                    f_out << "--minuscule" << Qt::endl;
                if (reponse.Majuscule)
                    f_out << "--majuscule" << Qt::endl;
                if (reponse.LongueurMaximale != -1000.0)
                    f_out << "--max=" << reponse.LongueurMaximale << Qt::endl;
            } break;
            case Bloc::TypeAction::DESSINETEXTEMULTILIGNEQUESTION: {
                f_out << "DESSINETEXTEMULTILIGNEQUESTION\n";
                f_out << "--debutx=" << reponse.DebutX << Qt::endl;
                f_out << "--debuty=" << reponse.DebutY << Qt::endl;
                f_out << "--largeur=" << reponse.Largeur << Qt::endl;
                f_out << "--hauteur=" << reponse.Hauteur << Qt::endl;
                f_out << "--question=\"" << reponse.Question << "\"" << Qt::endl;
                f_out << "--aidequestion=\"" << reponse.QuestionAide << "\"" << Qt::endl;
                QString TexteMulti = reponse.QuestionDefautMultiligne;
                TexteMulti.replace("\r\n", "{RetourLigne}");
                TexteMulti.replace("\n", "{RetourLigne}");
                f_out << "--defautquestion=\"" << TexteMulti << "\"" << Qt::endl;

                if (reponse.Alignement_Horizontale == Bloc::AlignementHorizontale::Gauche)
                    f_out << "--alignlargeur=" << 0 << Qt::endl;
                if (reponse.Alignement_Horizontale == Bloc::AlignementHorizontale::Milieu)
                    f_out << "--alignlargeur=" << 1 << Qt::endl;
                if (reponse.Alignement_Horizontale == Bloc::AlignementHorizontale::Droite)
                    f_out << "--alignlargeur=" << 2 << Qt::endl;
                if (reponse.Alignement_Verticale == Bloc::AlignementVerticale::Haut)
                    f_out << "--alignlargeur=" << 0 << Qt::endl;
                if (reponse.Alignement_Verticale == Bloc::AlignementVerticale::Centre)
                    f_out << "--alignlargeur=" << 1 << Qt::endl;
                if (reponse.Alignement_Verticale == Bloc::AlignementVerticale::Bas)
                    f_out << "--alignlargeur=" << 2 << Qt::endl;
                if (reponse.Couleur == "{ACC 1}") {
                    f_out << "--rouge=" << -1 << Qt::endl;
                    f_out << "--vert=" << -1 << Qt::endl;
                    f_out << "--bleu=" << -1 << Qt::endl;
                } else if (reponse.Couleur == "{ACC 2}") {
                    f_out << "--rouge=" << -2 << Qt::endl;
                    f_out << "--vert=" << -2 << Qt::endl;
                    f_out << "--bleu=" << -2 << Qt::endl;
                } else {
                    bool bStatus = false;
                    f_out << "--rouge=" << reponse.Couleur.mid(1, 2).toUInt(&bStatus, 16) << Qt::endl;
                    f_out << "--vert=" << reponse.Couleur.mid(3, 2).toUInt(&bStatus, 16) << Qt::endl;
                    f_out << "--bleu=" << reponse.Couleur.mid(5, 2).toUInt(&bStatus, 16) << Qt::endl;
                }
                if (reponse.TaillePolice > 0)
                    f_out << "--taillepolice=" << reponse.TaillePolice << Qt::endl;
                if (reponse.Obligatoire)
                    f_out << "--obligatoire" << Qt::endl;
                if (reponse.Gras)
                    f_out << "--gras" << Qt::endl;
                if (reponse.Italique)
                    f_out << "--italic" << Qt::endl;
                if (reponse.GrasEtItalique)
                    f_out << "--grasitalic" << Qt::endl;
                if (reponse.Monospace)
                    f_out << "--monospace" << Qt::endl;
                if (reponse.Minuscule)
                    f_out << "--minuscule" << Qt::endl;
                if (reponse.Majuscule)
                    f_out << "--majuscule" << Qt::endl;
                if (reponse.LongueurMaximale > 0)
                    f_out << "--max=" << reponse.LongueurMaximale << Qt::endl;
            } break;
            case Bloc::TypeAction::INSEREIMAGE: {
                f_out << "INSEREIMAGE\n";
                f_out << "--debutx=" << reponse.DebutX << Qt::endl;
                f_out << "--debuty=" << reponse.DebutY << Qt::endl;
                f_out << "--largeur=" << reponse.Largeur << Qt::endl;
                f_out << "--hauteur=" << reponse.Hauteur << Qt::endl;
                f_out << "--chemin=\"" << reponse.CheminImage << "\"" << Qt::endl;
            } break;
            case Bloc::TypeAction::DESSINECHECKBOX: {
                f_out << "DESSINECHECKBOX\n";
                f_out << "--debutx=" << reponse.DebutX << Qt::endl;
                f_out << "--debuty=" << reponse.DebutY << Qt::endl;
                f_out << "--largeur=" << reponse.Largeur << Qt::endl;
                f_out << "--hauteur=" << reponse.Hauteur << Qt::endl;
                if (reponse.Couleur == "{ACC 1}") {
                    f_out << "--rouge=" << -1 << Qt::endl;
                    f_out << "--vert=" << -1 << Qt::endl;
                    f_out << "--bleu=" << -1 << Qt::endl;
                } else if (reponse.Couleur == "{ACC 2}") {
                    f_out << "--rouge=" << -2 << Qt::endl;
                    f_out << "--vert=" << -2 << Qt::endl;
                    f_out << "--bleu=" << -2 << Qt::endl;
                } else {
                    bool bStatus = false;
                    f_out << "--rouge=" << reponse.Couleur.mid(1, 2).toUInt(&bStatus, 16) << Qt::endl;
                    f_out << "--vert=" << reponse.Couleur.mid(3, 2).toUInt(&bStatus, 16) << Qt::endl;
                    f_out << "--bleu=" << reponse.Couleur.mid(5, 2).toUInt(&bStatus, 16) << Qt::endl;
                }
                if (reponse.Epaisseur > 0)
                    f_out << "--epaisseur=" << reponse.Epaisseur << Qt::endl;
            } break;
            case Bloc::TypeAction::DESSINECHECKBOXQUESTION: {
                f_out << "DESSINECHECKBOXQUESTION\n";
                f_out << "--debutx=" << reponse.DebutX << Qt::endl;
                f_out << "--debuty=" << reponse.DebutY << Qt::endl;
                f_out << "--largeur=" << reponse.Largeur << Qt::endl;
                f_out << "--hauteur=" << reponse.Hauteur << Qt::endl;
                if (reponse.Couleur == "{ACC 1}") {
                    f_out << "--rouge=" << -1 << Qt::endl;
                    f_out << "--vert=" << -1 << Qt::endl;
                    f_out << "--bleu=" << -1 << Qt::endl;
                } else if (reponse.Couleur == "{ACC 2}") {
                    f_out << "--rouge=" << -2 << Qt::endl;
                    f_out << "--vert=" << -2 << Qt::endl;
                    f_out << "--bleu=" << -2 << Qt::endl;
                } else {
                    bool bStatus = false;
                    f_out << "--rouge=" << reponse.Couleur.mid(1, 2).toUInt(&bStatus, 16) << Qt::endl;
                    f_out << "--vert=" << reponse.Couleur.mid(3, 2).toUInt(&bStatus, 16) << Qt::endl;
                    f_out << "--bleu=" << reponse.Couleur.mid(5, 2).toUInt(&bStatus, 16) << Qt::endl;
                }
                if (reponse.Epaisseur > 0)
                    f_out << "--epaisseur=" << reponse.Epaisseur << Qt::endl;
                f_out << "--question=\"" << reponse.Question << "\"" << Qt::endl;
                f_out << "--aidequestion=\"" << reponse.QuestionAide << "\"" << Qt::endl;
                f_out << "--defautquestion=\"" << reponse.QuestionDefaut << "\"" << Qt::endl;
                if (reponse.Obligatoire)
                    f_out << "--obligatoire" << Qt::endl;
            } break;
            case Bloc::TypeAction::DESSINEMULTICHECKBOXQUESTION: {
                f_out << "DESSINEMULTICHECKBOXQUESTION\n";
                f_out << "--debutx1=" << reponse.DebutX << Qt::endl;
                f_out << "--debuty1=" << reponse.DebutY << Qt::endl;
                f_out << "--debutx2=" << reponse.DebutX2 << Qt::endl;
                f_out << "--debuty2=" << reponse.DebutY2 << Qt::endl;
                f_out << "--largeur=" << reponse.Largeur << Qt::endl;
                f_out << "--hauteur=" << reponse.Hauteur << Qt::endl;
                if (reponse.Couleur == "{ACC 1}") {
                    f_out << "--rouge=" << -1 << Qt::endl;
                    f_out << "--vert=" << -1 << Qt::endl;
                    f_out << "--bleu=" << -1 << Qt::endl;
                } else if (reponse.Couleur == "{ACC 2}") {
                    f_out << "--rouge=" << -2 << Qt::endl;
                    f_out << "--vert=" << -2 << Qt::endl;
                    f_out << "--bleu=" << -2 << Qt::endl;
                } else {
                    bool bStatus = false;
                    f_out << "--rouge=" << reponse.Couleur.mid(1, 2).toUInt(&bStatus, 16) << Qt::endl;
                    f_out << "--vert=" << reponse.Couleur.mid(3, 2).toUInt(&bStatus, 16) << Qt::endl;
                    f_out << "--bleu=" << reponse.Couleur.mid(5, 2).toUInt(&bStatus, 16) << Qt::endl;
                }
                if (reponse.Epaisseur > 0)
                    f_out << "--epaisseur=" << reponse.Epaisseur << Qt::endl;
                f_out << "--question=\"" << reponse.Question << "\"" << Qt::endl;
                f_out << "--aidequestion=\"" << reponse.QuestionAide << "\"" << Qt::endl;
                f_out << "--defautquestion=\"" << reponse.QuestionDefaut << "\"" << Qt::endl;
                if (reponse.Obligatoire)
                    f_out << "--obligatoire" << Qt::endl;
            } break;
            case Bloc::TypeAction::PAGESUIVANTE: {
                f_out << "PAGESUIVANTE\n";
            } break;
            case Bloc::TypeAction::COMMENTAIRE: {
                QString TexteMulti = reponse.Commentaire;
                TexteMulti.replace("\n", "\n:: ");
                f_out << ":: " << TexteMulti << Qt::endl;
            } break;
        }
    }
    Fichier.close();
    return true;
}

/** on_APROPOS_Texte_anchorClicked:
  Ouvre une URL cliquée

  @param const QUrl& arg1
  @return Aucun
*/
void
MainWindow::on_APROPOS_Texte_anchorClicked(const QUrl& arg1)
{
    QDesktopServices::openUrl(arg1);
    Consigne("Ouverture du lien  " + arg1.toDisplayString(QUrl::None));
}

/** on_APROPOS_Texte_highlighted:
  Affiche dans la status bar l'URL

  @param const QUrl& arg1
  @return Aucun
*/
void
MainWindow::on_APROPOS_Texte_highlighted(const QUrl& arg1)
{
    ui->statusbar->showMessage(arg1.toDisplayString(QUrl::None), 2000);
}

/** RepareGhostScript:
  Recréer un fichier PDF à l'aide de Ghostscript

  @param QString FichierSource
  @param qint16 ValeurMax
  @param bool Downsample
  @return ValeurRetour
*/
MainWindow::ValeurRetour
MainWindow::RepareGhostScript(QString FichierSource, qint16 ValeurMax, bool Downsample)
{
    QString CheminBaseGhostScript = QCoreApplication::applicationDirPath() + "/GhostScript/";
    if (!QFile::exists(CheminBaseGhostScript + "bin/gswin64c.exe"))
        return ValeurRetour::GhostScriptAbsent;
    if (FichierSource == "")
        return ValeurRetour::AucunFichierEntree;

    QString FichierBKUP = QFileInfo(FichierSource).path() + "/" + QFileInfo(FichierSource).baseName() + "_backup" +
                          QDateTime::currentDateTime().toString("_ddMMyy_hhmmss") + ".pdf";

    if (!QFile::rename(FichierSource, FichierBKUP))
        return ValeurRetour::ErreurDeplacement;

    QProgressDialog* progress;

    if (ValeurMax <= 0) {
        Consigne("Il y a 0 pages vu dans le document, abandon de la réparation");
        return ValeurRetour::ZeroPages;
    }
    progress = new QProgressDialog(this);
    if (ThemeDark)
        progress->setStyle(new DarkStyle);
    progress->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    progress->setWindowModality(Qt::WindowModal);
    progress->setMaximum(ValeurMax);
    progress->setValue(0);
    progress->setAutoClose(false);
    progress->setCancelButton(0);
    progress->setWindowIcon(this->windowIcon());
    progress->installEventFilter(keyPressEater);
    progress->setLabelText("Analyse GhostScript\nSauvegarde du fichier PDF");
    progress->show();
    {
        RECT FenetrePrincipale;
        GetWindowRect((HWND)this->winId(), &FenetrePrincipale);
        progress->setGeometry(FenetrePrincipale.left + (this->geometry().width() / 2) - (progress->width() / 2),
                              FenetrePrincipale.top + (this->geometry().height() / 2) - (progress->height() / 2),
                              progress->width(),
                              progress->height());
    }
    QCoreApplication::processEvents();
    auto procGhost = new QProcess();
    QString Windir = "";
    {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        Windir                  = env.value("windir");
    }
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("GS_LIB", QDir::toNativeSeparators(CheminBaseGhostScript) + "lib\\");
    env.insert("GS_DLL", QDir::toNativeSeparators(CheminBaseGhostScript) + "bin\\");
    env.insert("GS_BIN", QDir::toNativeSeparators(CheminBaseGhostScript) + "bin\\");
    env.insert("GS_RESOURCE", QDir::toNativeSeparators(CheminBaseGhostScript) + "resource\\init");
    env.insert("GS_FONTPATH", Windir + "\\fonts");
    procGhost->setProcessEnvironment(env);
    QString ConstructedArguments = QString("-dBATCH -dNOPAUSE -dSHORTERRORS -sDEVICE=pdfwrite %4 -I\"%1\" "
                                           "-sOutputFile=\"%2\" \"%3\"")
                                     .arg(CheminBaseGhostScript + "resource/init",
                                          FichierSource,
                                          FichierBKUP,
                                          Downsample ? "-dColorConversionStrategy=/LeaveColorUnchanged "
                                                       "-dDownsampleMonoImages=false "
                                                       "-dDownsampleGrayImages=false "
                                                       "-dDownsampleColorImages=false "
                                                       "-dAutoFilterColorImages=false "
                                                       "-dAutoFilterGrayImages=false "
                                                       "-dColorImageFilter=/FlateEncode "
                                                       "-dGrayImageFilter=/FlateEncode"
                                                     : "");
    procGhost->setNativeArguments(ConstructedArguments);
    procGhost->start(CheminBaseGhostScript + "bin/gswin64c.exe");
    if (ValeurMax != 0)
        progress->setLabelText("Analyse GhostScript\nOuverture du fichier PDF");
    qint16 PageEnCours = 0;
    while (procGhost->state() != QProcess::NotRunning) {
        auto Lecture = QString(procGhost->readAllStandardOutput()).split("\n", Qt::SkipEmptyParts);
        if (Lecture.count() > 0)
            if (Lecture.last().startsWith("Page ")) {
                QString str = Lecture.last().mid(5);
                bool ok;
                int PEnCours = str.toInt(&ok);
                if (ok)
                    PageEnCours = PEnCours;
            }
        if (ValeurMax != 0) {
            progress->setValue(PageEnCours);
            //            if (PageEnCours > 0)
            //                if (progress != nullptr)
            progress->setLabelText("Analyse GhostScript\nPage actuel : " + QString::number(PageEnCours) + "/" + QString::number(ValeurMax));
        }
        QThread::msleep(100);
        QCoreApplication::processEvents();
    }
    if (ValeurMax != 0)
        progress->close();
    if (procGhost->exitCode() < 0 || procGhost->exitStatus() == QProcess::CrashExit) {
        QFile::rename(FichierBKUP, FichierSource);
        return ValeurRetour::ErreurDeGhostScript;
    }
    return ValeurRetour::Succes;
}

/** PDFInfoNombrePage:
  Utilise PDFInfo pour récupérer le nombre de pages dans un fichier PDF en
 partant du principe qu'il est corrompu et que PoDoFo ne peut l'ouvrir

  @param QString FichierSource
  @return qint16
*/
qint16
MainWindow::PDFInfoNombrePage(QString FichierSource)
{
    QString CheminPDFINFO = QCoreApplication::applicationDirPath() + "/PdfToPPM/pdfinfo.exe";
    MiseEnPlaceListInfo();

    if (!QFile::exists(CheminPDFINFO)) {
        return -1;
    }
    if (FichierSource == "") {
        return -1;
    }

    QProcess* procPDFINFO = new QProcess();
    procPDFINFO->setArguments({ "-isodates", FichierSource });
    procPDFINFO->setProgram(CheminPDFINFO);

    procPDFINFO->start();

    while (procPDFINFO->state() != QProcess::NotRunning) {
        QThread::msleep(250);
        QCoreApplication::processEvents();
    }

    qint16 NombrePage = -1;
    auto Lecture      = QString(procPDFINFO->readAllStandardOutput()).split("\n", Qt::SkipEmptyParts);
    if (Lecture.count() > 0) {
        foreach (QString LigneConsole, Lecture) {
            if (LigneConsole.trimmed().startsWith("Pages:")) {
                bool ok;
                NombrePage = LigneConsole.trimmed().mid(6).trimmed().toInt(&ok, 10);
                if (!ok)
                    NombrePage = -1;
                break;
            }
        }
    } else {
        Lecture.prepend("Erreur à l'ouverture du fichier PDF");
        ui->FolioListeINFO->addItems(Lecture);
        return -1;
    }
    procPDFINFO->close();
    QStringList qslOriginal = { "Author:",    "Creator:",        "Producer:",  "CreationDate:", "ModDate:",    "Custom Metadata:", "Metadata Stream:",
                                "Tagged:",    "UserProperties:", "Suspects:",  "Form:",         "JavaScript:", "Pages:",           "Encrypted:",
                                "Page size:", "Page rot:",       "File size:", "Optimized:",    "PDF version:" };
    QStringList qslCorrige  = { "Auteur                : ", "Créateur              : ", "Producteur            : ", "Date création         : ",
                                "Date modification     : ", "Métadata personnalisé : ", "Flux de métadata      : ", "Taggé                 : ",
                                "Propriété utilisateur : ", "Tag suspect           : ", "Formulaire            : ", "JavaScript            : ",
                                "Pages                 : ", "Cryptée               : ", "Taille de la page     : ", "Rotation de la page   : ",
                                "Taille du fichier     : ", "PDF optimisé          : ", "Version PDF           : " };
    for (int var = 0; var < Lecture.count(); ++var) {
        for (int index = 0; index < qslCorrige.count(); ++index)
            if (Lecture[var].startsWith(qslOriginal[index]))
                Lecture[var] = qslCorrige[index] + Lecture[var].mid(qslOriginal[index].length()).trimmed();
        if (Lecture[var].endsWith("none"))
            Lecture[var] = Lecture[var].left(Lecture[var].length() - 4) + "aucun";
        if (Lecture[var].endsWith("no"))
            Lecture[var] = Lecture[var].left(Lecture[var].length() - 2) + "non";
        if (Lecture[var].endsWith("yes"))
            Lecture[var] = Lecture[var].left(Lecture[var].length() - 3) + "oui";
        if (Lecture[var].endsWith("bytes"))
            Lecture[var] = Lecture[var].left(Lecture[var].length() - 5) + "octets";
        if (Lecture[var].startsWith(qslCorrige[3]) || Lecture[var].startsWith(qslCorrige[4])) // Mise en forme des dates
        {
            if (Lecture[var].mid(34, 1) == "T")
                Lecture[var] = Lecture[var].replace(34, 1, ' ');
            if (Lecture[var].endsWith("+01"))
                Lecture[var] = Lecture[var].chopped(3);
        }
    }
    ui->FolioListeINFO->addItems(Lecture);
    return NombrePage;
}

/** CheckIntegrite:
  Contrôle l'intégrité des fichiers nécessaires au fonctionnement de REEMaker

  @return Aucun
*/
bool
MainWindow::CheckIntegrite()
{
    QStringList FichierAchercher = { "libasprintf-0.dll",
                                     "libatomic-1.dll",
                                     "libbrotlicommon.dll",
                                     "libbrotlidec.dll",
                                     "libbrotlienc.dll",
                                     "libbz2-1.dll",
                                     "libcharset-1.dll",
                                     "libcrypto-1_1-x64.dll",
                                     "libdeflate.dll",
                                     "libffi-8.dll",
                                     "libfontconfig-1.dll",
                                     "libfreetype-6.dll",
                                     "libgcc_s_seh-1.dll",
                                     "libgettextlib-0-21.dll",
                                     "libgettextpo-0.dll",
                                     "libgettextsrc-0-21.dll",
                                     "libgio-2.0-0.dll",
                                     "libglib-2.0-0.dll",
                                     "libgmodule-2.0-0.dll",
                                     "libgobject-2.0-0.dll",
                                     "libgomp-1.dll",
                                     "libgraphite2.dll",
                                     "libgthread-2.0-0.dll",
                                     "libharfbuzz-0.dll",
                                     "libharfbuzz-gobject-0.dll",
                                     "libharfbuzz-icu-0.dll",
                                     "libharfbuzz-subset-0.dll",
                                     "libiconv-2.dll",
                                     "libidn-12.dll",
                                     "libintl-8.dll",
                                     "libjbig-0.dll",
                                     "libjpeg-8.dll",
                                     "liblerc.dll",
                                     "liblzma-5.dll",
                                     "libp11-kit-0.dll",
                                     "libpcre2-16-0.dll",
                                     "libpcre2-32-0.dll",
                                     "libpcre2-8-0.dll",
                                     "libpcre2-posix-3.dll",
                                     "libpng16-16.dll",
                                     "libpodofo.dll",
                                     "libquadmath-0.dll",
                                     "libssl-1_1-x64.dll",
                                     "libssp-0.dll",
                                     "libstdc++-6.dll",
                                     "libtasn1-6.dll",
                                     "libtiff-6.dll",
                                     "libtiffxx-6.dll",
                                     "libunistring-2.dll",
                                     "libwebp-7.dll",
                                     "libwebpdecoder-3.dll",
                                     "libwebpdemux-2.dll",
                                     "libwebpmux-3.dll",
                                     "libwinpthread-1.dll",
                                     "libzstd.dll",
                                     "lua54.dll",
                                     "qt6core.dll",
                                     "qt6gui.dll",
                                     "qt6widgets.dll",
                                     "Qt6Network.dll",
                                     "reemaker.exe",
                                     "7za.exe",
                                     "REEMakerAide.pdf",
                                     "zlib1.dll",
                                     "ghostscript/bin/gsdll64.dll",
                                     "ghostscript/bin/gsdll64.lib",
                                     "ghostscript/bin/gswin64.exe",
                                     "ghostscript/bin/gswin64c.exe",
                                     "ghostscript/iccprofiles/a98.icc",
                                     "ghostscript/iccprofiles/default_cmyk.icc",
                                     "ghostscript/iccprofiles/default_gray.icc",
                                     "ghostscript/iccprofiles/default_rgb.icc",
                                     "ghostscript/iccprofiles/esrgb.icc",
                                     "ghostscript/iccprofiles/gray_to_k.icc",
                                     "ghostscript/iccprofiles/lab.icc",
                                     "ghostscript/iccprofiles/ps_cmyk.icc",
                                     "ghostscript/iccprofiles/ps_gray.icc",
                                     "ghostscript/iccprofiles/ps_rgb.icc",
                                     "ghostscript/iccprofiles/rommrgb.icc",
                                     "ghostscript/iccprofiles/scrgb.icc",
                                     "ghostscript/iccprofiles/sgray.icc",
                                     "ghostscript/iccprofiles/srgb.icc",
                                     "ghostscript/lib/acctest.ps",
                                     "ghostscript/lib/afmdiff.awk",
                                     "ghostscript/lib/align.ps",
                                     "ghostscript/lib/bj8.rpd",
                                     "ghostscript/lib/bj8gc12f.upp",
                                     "ghostscript/lib/bj8hg12f.upp",
                                     "ghostscript/lib/bj8oh06n.upp",
                                     "ghostscript/lib/bj8pa06n.upp",
                                     "ghostscript/lib/bj8pp12f.upp",
                                     "ghostscript/lib/bj8ts06n.upp",
                                     "ghostscript/lib/bjc610a0.upp",
                                     "ghostscript/lib/bjc610a1.upp",
                                     "ghostscript/lib/bjc610a2.upp",
                                     "ghostscript/lib/bjc610a3.upp",
                                     "ghostscript/lib/bjc610a4.upp",
                                     "ghostscript/lib/bjc610a5.upp",
                                     "ghostscript/lib/bjc610a6.upp",
                                     "ghostscript/lib/bjc610a7.upp",
                                     "ghostscript/lib/bjc610a8.upp",
                                     "ghostscript/lib/bjc610b1.upp",
                                     "ghostscript/lib/bjc610b2.upp",
                                     "ghostscript/lib/bjc610b3.upp",
                                     "ghostscript/lib/bjc610b4.upp",
                                     "ghostscript/lib/bjc610b6.upp",
                                     "ghostscript/lib/bjc610b7.upp",
                                     "ghostscript/lib/bjc610b8.upp",
                                     "ghostscript/lib/caption.ps",
                                     "ghostscript/lib/cat.ps",
                                     "ghostscript/lib/cbjc600.ppd",
                                     "ghostscript/lib/cbjc800.ppd",
                                     "ghostscript/lib/cdj550.upp",
                                     "ghostscript/lib/cdj690.upp",
                                     "ghostscript/lib/cdj690ec.upp",
                                     "ghostscript/lib/cid2code.ps",
                                     "ghostscript/lib/dnj750c.upp",
                                     "ghostscript/lib/dnj750m.upp",
                                     "ghostscript/lib/docie.ps",
                                     "ghostscript/lib/dvipdf",
                                     "ghostscript/lib/eps2eps",
                                     "ghostscript/lib/eps2eps.bat",
                                     "ghostscript/lib/eps2eps.cmd",
                                     "ghostscript/lib/fapiconfig-fco",
                                     "ghostscript/lib/fcofontmap-pclps3",
                                     "ghostscript/lib/fcofontmap-ps3",
                                     "ghostscript/lib/font2pcl.ps",
                                     "ghostscript/lib/fontmap.atb",
                                     "ghostscript/lib/fontmap.atm",
                                     "ghostscript/lib/fontmap.os2",
                                     "ghostscript/lib/fontmap.osf",
                                     "ghostscript/lib/fontmap.sgi",
                                     "ghostscript/lib/fontmap.sol",
                                     "ghostscript/lib/fontmap.ult",
                                     "ghostscript/lib/fontmap.vms",
                                     "ghostscript/lib/ghostpdf.cat",
                                     "ghostscript/lib/ghostpdf.inf",
                                     "ghostscript/lib/ghostpdf.ppd",
                                     "ghostscript/lib/ghostpdf.readme",
                                     "ghostscript/lib/gsbj",
                                     "ghostscript/lib/gsbj.bat",
                                     "ghostscript/lib/gsdj",
                                     "ghostscript/lib/gsdj.bat",
                                     "ghostscript/lib/gsdj500",
                                     "ghostscript/lib/gsdj500.bat",
                                     "ghostscript/lib/gslj",
                                     "ghostscript/lib/gslj.bat",
                                     "ghostscript/lib/gslp",
                                     "ghostscript/lib/gslp.bat",
                                     "ghostscript/lib/gslp.ps",
                                     "ghostscript/lib/gsnd",
                                     "ghostscript/lib/gsnd.bat",
                                     "ghostscript/lib/gsndt.bat",
                                     "ghostscript/lib/gsnup.ps",
                                     "ghostscript/lib/gssetgs.bat",
                                     "ghostscript/lib/gssetgs32.bat",
                                     "ghostscript/lib/gssetgs64.bat",
                                     "ghostscript/lib/gst.bat",
                                     "ghostscript/lib/gstt.bat",
                                     "ghostscript/lib/gs_ce_e.ps",
                                     "ghostscript/lib/gs_css_e.ps",
                                     "ghostscript/lib/gs_il2_e.ps",
                                     "ghostscript/lib/gs_kanji.ps",
                                     "ghostscript/lib/gs_ksb_e.ps",
                                     "ghostscript/lib/gs_l.xbm",
                                     "ghostscript/lib/gs_l.xpm",
                                     "ghostscript/lib/gs_lgo_e.ps",
                                     "ghostscript/lib/gs_lgx_e.ps",
                                     "ghostscript/lib/gs_l_m.xbm",
                                     "ghostscript/lib/gs_m.xbm",
                                     "ghostscript/lib/gs_m.xpm",
                                     "ghostscript/lib/gs_m_m.xbm",
                                     "ghostscript/lib/gs_s.xbm",
                                     "ghostscript/lib/gs_s.xpm",
                                     "ghostscript/lib/gs_s_m.xbm",
                                     "ghostscript/lib/gs_t.xbm",
                                     "ghostscript/lib/gs_t.xpm",
                                     "ghostscript/lib/gs_t_m.xbm",
                                     "ghostscript/lib/gs_wl1_e.ps",
                                     "ghostscript/lib/gs_wl2_e.ps",
                                     "ghostscript/lib/gs_wl5_e.ps",
                                     "ghostscript/lib/ht_ccsto.ps",
                                     "ghostscript/lib/image-qa.ps",
                                     "ghostscript/lib/info-macos.plist",
                                     "ghostscript/lib/jispaper.ps",
                                     "ghostscript/lib/jobseparator.ps",
                                     "ghostscript/lib/landscap.ps",
                                     "ghostscript/lib/lines.ps",
                                     "ghostscript/lib/lp386.bat",
                                     "ghostscript/lib/lp386r2.bat",
                                     "ghostscript/lib/lpgs.bat",
                                     "ghostscript/lib/lpr2.bat",
                                     "ghostscript/lib/lprsetup.sh",
                                     "ghostscript/lib/mkcidfm.ps",
                                     "ghostscript/lib/necp2x.upp",
                                     "ghostscript/lib/necp2x6.upp",
                                     "ghostscript/lib/pdf2dsc",
                                     "ghostscript/lib/pdf2dsc.bat",
                                     "ghostscript/lib/pdf2dsc.ps",
                                     "ghostscript/lib/pdf2ps",
                                     "ghostscript/lib/pdf2ps.bat",
                                     "ghostscript/lib/pdf2ps.cmd",
                                     "ghostscript/lib/pdfa_def.ps",
                                     "ghostscript/lib/pdfx_def.ps",
                                     "ghostscript/lib/pdf_info.ps",
                                     "ghostscript/lib/pf2afm",
                                     "ghostscript/lib/pf2afm.bat",
                                     "ghostscript/lib/pf2afm.cmd",
                                     "ghostscript/lib/pf2afm.ps",
                                     "ghostscript/lib/pfbtopfa",
                                     "ghostscript/lib/pfbtopfa.bat",
                                     "ghostscript/lib/pfbtopfa.ps",
                                     "ghostscript/lib/ppath.ps",
                                     "ghostscript/lib/pphs",
                                     "ghostscript/lib/pphs.ps",
                                     "ghostscript/lib/prfont.ps",
                                     "ghostscript/lib/printafm",
                                     "ghostscript/lib/printafm.ps",
                                     "ghostscript/lib/ps2ai.ps",
                                     "ghostscript/lib/ps2ascii",
                                     "ghostscript/lib/ps2ascii.bat",
                                     "ghostscript/lib/ps2ascii.cmd",
                                     "ghostscript/lib/ps2epsi",
                                     "ghostscript/lib/ps2epsi.bat",
                                     "ghostscript/lib/ps2epsi.cmd",
                                     "ghostscript/lib/ps2epsi.ps",
                                     "ghostscript/lib/ps2pdf",
                                     "ghostscript/lib/ps2pdf.bat",
                                     "ghostscript/lib/ps2pdf.cmd",
                                     "ghostscript/lib/ps2pdf12",
                                     "ghostscript/lib/ps2pdf12.bat",
                                     "ghostscript/lib/ps2pdf12.cmd",
                                     "ghostscript/lib/ps2pdf13",
                                     "ghostscript/lib/ps2pdf13.bat",
                                     "ghostscript/lib/ps2pdf13.cmd",
                                     "ghostscript/lib/ps2pdf14",
                                     "ghostscript/lib/ps2pdf14.bat",
                                     "ghostscript/lib/ps2pdf14.cmd",
                                     "ghostscript/lib/ps2pdfwr",
                                     "ghostscript/lib/ps2pdfxx.bat",
                                     "ghostscript/lib/ps2ps",
                                     "ghostscript/lib/ps2ps.bat",
                                     "ghostscript/lib/ps2ps.cmd",
                                     "ghostscript/lib/ps2ps2",
                                     "ghostscript/lib/ps2ps2.bat",
                                     "ghostscript/lib/ps2ps2.cmd",
                                     "ghostscript/lib/ras1.upp",
                                     "ghostscript/lib/ras24.upp",
                                     "ghostscript/lib/ras3.upp",
                                     "ghostscript/lib/ras32.upp",
                                     "ghostscript/lib/ras4.upp",
                                     "ghostscript/lib/ras8m.upp",
                                     "ghostscript/lib/rinkj-2200-setup",
                                     "ghostscript/lib/rollconv.ps",
                                     "ghostscript/lib/st640ih.upp",
                                     "ghostscript/lib/st640ihg.upp",
                                     "ghostscript/lib/st640p.upp",
                                     "ghostscript/lib/st640pg.upp",
                                     "ghostscript/lib/st640pl.upp",
                                     "ghostscript/lib/st640plg.upp",
                                     "ghostscript/lib/stc.upp",
                                     "ghostscript/lib/stc1520h.upp",
                                     "ghostscript/lib/stc2.upp",
                                     "ghostscript/lib/stc200_h.upp",
                                     "ghostscript/lib/stc2s_h.upp",
                                     "ghostscript/lib/stc2_h.upp",
                                     "ghostscript/lib/stc300.upp",
                                     "ghostscript/lib/stc300bl.upp",
                                     "ghostscript/lib/stc300bm.upp",
                                     "ghostscript/lib/stc500p.upp",
                                     "ghostscript/lib/stc500ph.upp",
                                     "ghostscript/lib/stc600ih.upp",
                                     "ghostscript/lib/stc600p.upp",
                                     "ghostscript/lib/stc600pl.upp",
                                     "ghostscript/lib/stc640p.upp",
                                     "ghostscript/lib/stc800ih.upp",
                                     "ghostscript/lib/stc800p.upp",
                                     "ghostscript/lib/stc800pl.upp",
                                     "ghostscript/lib/stcany.upp",
                                     "ghostscript/lib/stcany_h.upp",
                                     "ghostscript/lib/stcinfo.ps",
                                     "ghostscript/lib/stcolor.ps",
                                     "ghostscript/lib/stc_h.upp",
                                     "ghostscript/lib/stc_l.upp",
                                     "ghostscript/lib/stocht.ps",
                                     "ghostscript/lib/traceimg.ps",
                                     "ghostscript/lib/traceop.ps",
                                     "ghostscript/lib/uninfo.ps",
                                     "ghostscript/lib/unix-lpr.sh",
                                     "ghostscript/lib/viewcmyk.ps",
                                     "ghostscript/lib/viewgif.ps",
                                     "ghostscript/lib/viewjpeg.ps",
                                     "ghostscript/lib/viewmiff.ps",
                                     "ghostscript/lib/viewpbm.ps",
                                     "ghostscript/lib/viewpcx.ps",
                                     "ghostscript/lib/viewps2a.ps",
                                     "ghostscript/lib/viewpwg.ps",
                                     "ghostscript/lib/viewraw.ps",
                                     "ghostscript/lib/viewrgb.ps",
                                     "ghostscript/lib/winmaps.ps",
                                     "ghostscript/lib/wmakebat.bat",
                                     "ghostscript/lib/zeroline.ps",
                                     "ghostscript/lib/zugferd.ps",
                                     "ghostscript/resource/cidfont/artifexbullet",
                                     "ghostscript/resource/cidfsubst/droidsansfallback.ttf",
                                     "ghostscript/resource/cmap/78-euc-h",
                                     "ghostscript/resource/cmap/78-euc-v",
                                     "ghostscript/resource/cmap/78-h",
                                     "ghostscript/resource/cmap/78-rksj-h",
                                     "ghostscript/resource/cmap/78-rksj-v",
                                     "ghostscript/resource/cmap/78-v",
                                     "ghostscript/resource/cmap/78ms-rksj-h",
                                     "ghostscript/resource/cmap/78ms-rksj-v",
                                     "ghostscript/resource/cmap/83pv-rksj-h",
                                     "ghostscript/resource/cmap/90ms-rksj-h",
                                     "ghostscript/resource/cmap/90ms-rksj-v",
                                     "ghostscript/resource/cmap/90msp-rksj-h",
                                     "ghostscript/resource/cmap/90msp-rksj-v",
                                     "ghostscript/resource/cmap/90pv-rksj-h",
                                     "ghostscript/resource/cmap/90pv-rksj-v",
                                     "ghostscript/resource/cmap/add-h",
                                     "ghostscript/resource/cmap/add-rksj-h",
                                     "ghostscript/resource/cmap/add-rksj-v",
                                     "ghostscript/resource/cmap/add-v",
                                     "ghostscript/resource/cmap/adobe-cns1-0",
                                     "ghostscript/resource/cmap/adobe-cns1-1",
                                     "ghostscript/resource/cmap/adobe-cns1-2",
                                     "ghostscript/resource/cmap/adobe-cns1-3",
                                     "ghostscript/resource/cmap/adobe-cns1-4",
                                     "ghostscript/resource/cmap/adobe-cns1-5",
                                     "ghostscript/resource/cmap/adobe-cns1-6",
                                     "ghostscript/resource/cmap/adobe-cns1-7",
                                     "ghostscript/resource/cmap/adobe-gb1-0",
                                     "ghostscript/resource/cmap/adobe-gb1-1",
                                     "ghostscript/resource/cmap/adobe-gb1-2",
                                     "ghostscript/resource/cmap/adobe-gb1-3",
                                     "ghostscript/resource/cmap/adobe-gb1-4",
                                     "ghostscript/resource/cmap/adobe-gb1-5",
                                     "ghostscript/resource/cmap/adobe-japan1-0",
                                     "ghostscript/resource/cmap/adobe-japan1-1",
                                     "ghostscript/resource/cmap/adobe-japan1-2",
                                     "ghostscript/resource/cmap/adobe-japan1-3",
                                     "ghostscript/resource/cmap/adobe-japan1-4",
                                     "ghostscript/resource/cmap/adobe-japan1-5",
                                     "ghostscript/resource/cmap/adobe-japan1-6",
                                     "ghostscript/resource/cmap/adobe-japan2-0",
                                     "ghostscript/resource/cmap/adobe-korea1-0",
                                     "ghostscript/resource/cmap/adobe-korea1-1",
                                     "ghostscript/resource/cmap/adobe-korea1-2",
                                     "ghostscript/resource/cmap/b5-h",
                                     "ghostscript/resource/cmap/b5-v",
                                     "ghostscript/resource/cmap/b5pc-h",
                                     "ghostscript/resource/cmap/b5pc-v",
                                     "ghostscript/resource/cmap/cns-euc-h",
                                     "ghostscript/resource/cmap/cns-euc-v",
                                     "ghostscript/resource/cmap/cns1-h",
                                     "ghostscript/resource/cmap/cns1-v",
                                     "ghostscript/resource/cmap/cns2-h",
                                     "ghostscript/resource/cmap/cns2-v",
                                     "ghostscript/resource/cmap/eten-b5-h",
                                     "ghostscript/resource/cmap/eten-b5-v",
                                     "ghostscript/resource/cmap/etenms-b5-h",
                                     "ghostscript/resource/cmap/etenms-b5-v",
                                     "ghostscript/resource/cmap/ethk-b5-h",
                                     "ghostscript/resource/cmap/ethk-b5-v",
                                     "ghostscript/resource/cmap/euc-h",
                                     "ghostscript/resource/cmap/euc-v",
                                     "ghostscript/resource/cmap/ext-h",
                                     "ghostscript/resource/cmap/ext-rksj-h",
                                     "ghostscript/resource/cmap/ext-rksj-v",
                                     "ghostscript/resource/cmap/ext-v",
                                     "ghostscript/resource/cmap/gb-euc-h",
                                     "ghostscript/resource/cmap/gb-euc-v",
                                     "ghostscript/resource/cmap/gb-h",
                                     "ghostscript/resource/cmap/gb-v",
                                     "ghostscript/resource/cmap/gbk-euc-h",
                                     "ghostscript/resource/cmap/gbk-euc-v",
                                     "ghostscript/resource/cmap/gbk2k-h",
                                     "ghostscript/resource/cmap/gbk2k-v",
                                     "ghostscript/resource/cmap/gbkp-euc-h",
                                     "ghostscript/resource/cmap/gbkp-euc-v",
                                     "ghostscript/resource/cmap/gbpc-euc-h",
                                     "ghostscript/resource/cmap/gbpc-euc-v",
                                     "ghostscript/resource/cmap/gbt-euc-h",
                                     "ghostscript/resource/cmap/gbt-euc-v",
                                     "ghostscript/resource/cmap/gbt-h",
                                     "ghostscript/resource/cmap/gbt-v",
                                     "ghostscript/resource/cmap/gbtpc-euc-h",
                                     "ghostscript/resource/cmap/gbtpc-euc-v",
                                     "ghostscript/resource/cmap/h",
                                     "ghostscript/resource/cmap/hankaku",
                                     "ghostscript/resource/cmap/hiragana",
                                     "ghostscript/resource/cmap/hkdla-b5-h",
                                     "ghostscript/resource/cmap/hkdla-b5-v",
                                     "ghostscript/resource/cmap/hkdlb-b5-h",
                                     "ghostscript/resource/cmap/hkdlb-b5-v",
                                     "ghostscript/resource/cmap/hkgccs-b5-h",
                                     "ghostscript/resource/cmap/hkgccs-b5-v",
                                     "ghostscript/resource/cmap/hkm314-b5-h",
                                     "ghostscript/resource/cmap/hkm314-b5-v",
                                     "ghostscript/resource/cmap/hkm471-b5-h",
                                     "ghostscript/resource/cmap/hkm471-b5-v",
                                     "ghostscript/resource/cmap/hkscs-b5-h",
                                     "ghostscript/resource/cmap/hkscs-b5-v",
                                     "ghostscript/resource/cmap/hojo-euc-h",
                                     "ghostscript/resource/cmap/hojo-euc-v",
                                     "ghostscript/resource/cmap/hojo-h",
                                     "ghostscript/resource/cmap/hojo-v",
                                     "ghostscript/resource/cmap/identity-h",
                                     "ghostscript/resource/cmap/identity-utf16-h",
                                     "ghostscript/resource/cmap/identity-v",
                                     "ghostscript/resource/cmap/katakana",
                                     "ghostscript/resource/cmap/ksc-euc-h",
                                     "ghostscript/resource/cmap/ksc-euc-v",
                                     "ghostscript/resource/cmap/ksc-h",
                                     "ghostscript/resource/cmap/ksc-johab-h",
                                     "ghostscript/resource/cmap/ksc-johab-v",
                                     "ghostscript/resource/cmap/ksc-v",
                                     "ghostscript/resource/cmap/kscms-uhc-h",
                                     "ghostscript/resource/cmap/kscms-uhc-hw-h",
                                     "ghostscript/resource/cmap/kscms-uhc-hw-v",
                                     "ghostscript/resource/cmap/kscms-uhc-v",
                                     "ghostscript/resource/cmap/kscpc-euc-h",
                                     "ghostscript/resource/cmap/kscpc-euc-v",
                                     "ghostscript/resource/cmap/nwp-h",
                                     "ghostscript/resource/cmap/nwp-v",
                                     "ghostscript/resource/cmap/rksj-h",
                                     "ghostscript/resource/cmap/rksj-v",
                                     "ghostscript/resource/cmap/roman",
                                     "ghostscript/resource/cmap/unicns-ucs2-h",
                                     "ghostscript/resource/cmap/unicns-ucs2-v",
                                     "ghostscript/resource/cmap/unicns-utf16-h",
                                     "ghostscript/resource/cmap/unicns-utf16-v",
                                     "ghostscript/resource/cmap/unicns-utf32-h",
                                     "ghostscript/resource/cmap/unicns-utf32-v",
                                     "ghostscript/resource/cmap/unicns-utf8-h",
                                     "ghostscript/resource/cmap/unicns-utf8-v",
                                     "ghostscript/resource/cmap/unigb-ucs2-h",
                                     "ghostscript/resource/cmap/unigb-ucs2-v",
                                     "ghostscript/resource/cmap/unigb-utf16-h",
                                     "ghostscript/resource/cmap/unigb-utf16-v",
                                     "ghostscript/resource/cmap/unigb-utf32-h",
                                     "ghostscript/resource/cmap/unigb-utf32-v",
                                     "ghostscript/resource/cmap/unigb-utf8-h",
                                     "ghostscript/resource/cmap/unigb-utf8-v",
                                     "ghostscript/resource/cmap/unihojo-ucs2-h",
                                     "ghostscript/resource/cmap/unihojo-ucs2-v",
                                     "ghostscript/resource/cmap/unihojo-utf16-h",
                                     "ghostscript/resource/cmap/unihojo-utf16-v",
                                     "ghostscript/resource/cmap/unihojo-utf32-h",
                                     "ghostscript/resource/cmap/unihojo-utf32-v",
                                     "ghostscript/resource/cmap/unihojo-utf8-h",
                                     "ghostscript/resource/cmap/unihojo-utf8-v",
                                     "ghostscript/resource/cmap/unijis-ucs2-h",
                                     "ghostscript/resource/cmap/unijis-ucs2-hw-h",
                                     "ghostscript/resource/cmap/unijis-ucs2-hw-v",
                                     "ghostscript/resource/cmap/unijis-ucs2-v",
                                     "ghostscript/resource/cmap/unijis-utf16-h",
                                     "ghostscript/resource/cmap/unijis-utf16-v",
                                     "ghostscript/resource/cmap/unijis-utf32-h",
                                     "ghostscript/resource/cmap/unijis-utf32-v",
                                     "ghostscript/resource/cmap/unijis-utf8-h",
                                     "ghostscript/resource/cmap/unijis-utf8-v",
                                     "ghostscript/resource/cmap/unijis2004-utf16-h",
                                     "ghostscript/resource/cmap/unijis2004-utf16-v",
                                     "ghostscript/resource/cmap/unijis2004-utf32-h",
                                     "ghostscript/resource/cmap/unijis2004-utf32-v",
                                     "ghostscript/resource/cmap/unijis2004-utf8-h",
                                     "ghostscript/resource/cmap/unijis2004-utf8-v",
                                     "ghostscript/resource/cmap/unijispro-ucs2-hw-v",
                                     "ghostscript/resource/cmap/unijispro-ucs2-v",
                                     "ghostscript/resource/cmap/unijispro-utf8-v",
                                     "ghostscript/resource/cmap/unijisx0213-utf32-h",
                                     "ghostscript/resource/cmap/unijisx0213-utf32-v",
                                     "ghostscript/resource/cmap/unijisx02132004-utf32-h",
                                     "ghostscript/resource/cmap/unijisx02132004-utf32-v",
                                     "ghostscript/resource/cmap/uniks-ucs2-h",
                                     "ghostscript/resource/cmap/uniks-ucs2-v",
                                     "ghostscript/resource/cmap/uniks-utf16-h",
                                     "ghostscript/resource/cmap/uniks-utf16-v",
                                     "ghostscript/resource/cmap/uniks-utf32-h",
                                     "ghostscript/resource/cmap/uniks-utf32-v",
                                     "ghostscript/resource/cmap/uniks-utf8-h",
                                     "ghostscript/resource/cmap/uniks-utf8-v",
                                     "ghostscript/resource/cmap/v",
                                     "ghostscript/resource/cmap/wp-symbol",
                                     "ghostscript/resource/colorspace/defaultcmyk",
                                     "ghostscript/resource/colorspace/defaultgray",
                                     "ghostscript/resource/colorspace/defaultrgb",
                                     "ghostscript/resource/colorspace/sgray",
                                     "ghostscript/resource/colorspace/srgb",
                                     "ghostscript/resource/colorspace/trivialcmyk",
                                     "ghostscript/resource/decoding/fco_dingbats",
                                     "ghostscript/resource/decoding/fco_symbol",
                                     "ghostscript/resource/decoding/fco_unicode",
                                     "ghostscript/resource/decoding/fco_wingdings",
                                     "ghostscript/resource/decoding/latin1",
                                     "ghostscript/resource/decoding/standardencoding",
                                     "ghostscript/resource/decoding/unicode",
                                     "ghostscript/resource/encoding/ceencoding",
                                     "ghostscript/resource/encoding/expertencoding",
                                     "ghostscript/resource/encoding/expertsubsetencoding",
                                     "ghostscript/resource/encoding/notdefencoding",
                                     "ghostscript/resource/encoding/wingdings",
                                     "ghostscript/resource/font/c059-bdita",
                                     "ghostscript/resource/font/c059-bold",
                                     "ghostscript/resource/font/c059-italic",
                                     "ghostscript/resource/font/c059-roman",
                                     "ghostscript/resource/font/d050000l",
                                     "ghostscript/resource/font/nimbusmonops-bold",
                                     "ghostscript/resource/font/nimbusmonops-bolditalic",
                                     "ghostscript/resource/font/nimbusmonops-italic",
                                     "ghostscript/resource/font/nimbusmonops-regular",
                                     "ghostscript/resource/font/nimbusroman-bold",
                                     "ghostscript/resource/font/nimbusroman-bolditalic",
                                     "ghostscript/resource/font/nimbusroman-italic",
                                     "ghostscript/resource/font/nimbusroman-regular",
                                     "ghostscript/resource/font/nimbussans-bold",
                                     "ghostscript/resource/font/nimbussans-bolditalic",
                                     "ghostscript/resource/font/nimbussans-italic",
                                     "ghostscript/resource/font/nimbussans-regular",
                                     "ghostscript/resource/font/nimbussansnarrow-bold",
                                     "ghostscript/resource/font/nimbussansnarrow-boldoblique",
                                     "ghostscript/resource/font/nimbussansnarrow-oblique",
                                     "ghostscript/resource/font/nimbussansnarrow-regular",
                                     "ghostscript/resource/font/p052-bold",
                                     "ghostscript/resource/font/p052-bolditalic",
                                     "ghostscript/resource/font/p052-italic",
                                     "ghostscript/resource/font/p052-roman",
                                     "ghostscript/resource/font/standardsymbolsps",
                                     "ghostscript/resource/font/urwbookman-demi",
                                     "ghostscript/resource/font/urwbookman-demiitalic",
                                     "ghostscript/resource/font/urwbookman-light",
                                     "ghostscript/resource/font/urwbookman-lightitalic",
                                     "ghostscript/resource/font/urwgothic-book",
                                     "ghostscript/resource/font/urwgothic-bookoblique",
                                     "ghostscript/resource/font/urwgothic-demi",
                                     "ghostscript/resource/font/urwgothic-demioblique",
                                     "ghostscript/resource/font/z003-mediumitalic",
                                     "ghostscript/resource/idiomset/ppi_cutils",
                                     "ghostscript/resource/idiomset/pscript5idiom",
                                     "ghostscript/resource/init/cidfmap",
                                     "ghostscript/resource/init/fapicidfmap",
                                     "ghostscript/resource/init/fapiconfig",
                                     "ghostscript/resource/init/fapifontmap",
                                     "ghostscript/resource/init/fcofontmap-pclps2",
                                     "ghostscript/resource/init/fontmap",
                                     "ghostscript/resource/init/fontmap.gs",
                                     "ghostscript/resource/init/gs_agl.ps",
                                     "ghostscript/resource/init/gs_btokn.ps",
                                     "ghostscript/resource/init/gs_cet.ps",
                                     "ghostscript/resource/init/gs_cff.ps",
                                     "ghostscript/resource/init/gs_cidcm.ps",
                                     "ghostscript/resource/init/gs_ciddc.ps",
                                     "ghostscript/resource/init/gs_cidfm.ps",
                                     "ghostscript/resource/init/gs_cidfn.ps",
                                     "ghostscript/resource/init/gs_cidtt.ps",
                                     "ghostscript/resource/init/gs_cmap.ps",
                                     "ghostscript/resource/init/gs_cspace.ps",
                                     "ghostscript/resource/init/gs_dbt_e.ps",
                                     "ghostscript/resource/init/gs_diskn.ps",
                                     "ghostscript/resource/init/gs_dps1.ps",
                                     "ghostscript/resource/init/gs_dps2.ps",
                                     "ghostscript/resource/init/gs_dscp.ps",
                                     "ghostscript/resource/init/gs_epsf.ps",
                                     "ghostscript/resource/init/gs_fapi.ps",
                                     "ghostscript/resource/init/gs_fntem.ps",
                                     "ghostscript/resource/init/gs_fonts.ps",
                                     "ghostscript/resource/init/gs_frsd.ps",
                                     "ghostscript/resource/init/gs_icc.ps",
                                     "ghostscript/resource/init/gs_il1_e.ps",
                                     "ghostscript/resource/init/gs_img.ps",
                                     "ghostscript/resource/init/gs_init.ps",
                                     "ghostscript/resource/init/gs_lev2.ps",
                                     "ghostscript/resource/init/gs_ll3.ps",
                                     "ghostscript/resource/init/gs_mex_e.ps",
                                     "ghostscript/resource/init/gs_mgl_e.ps",
                                     "ghostscript/resource/init/gs_mro_e.ps",
                                     "ghostscript/resource/init/gs_pdfwr.ps",
                                     "ghostscript/resource/init/gs_pdf_e.ps",
                                     "ghostscript/resource/init/gs_res.ps",
                                     "ghostscript/resource/init/gs_resmp.ps",
                                     "ghostscript/resource/init/gs_setpd.ps",
                                     "ghostscript/resource/init/gs_statd.ps",
                                     "ghostscript/resource/init/gs_std_e.ps",
                                     "ghostscript/resource/init/gs_sym_e.ps",
                                     "ghostscript/resource/init/gs_trap.ps",
                                     "ghostscript/resource/init/gs_ttf.ps",
                                     "ghostscript/resource/init/gs_typ32.ps",
                                     "ghostscript/resource/init/gs_typ42.ps",
                                     "ghostscript/resource/init/gs_type1.ps",
                                     "ghostscript/resource/init/gs_wan_e.ps",
                                     "ghostscript/resource/init/pdf_base.ps",
                                     "ghostscript/resource/init/pdf_draw.ps",
                                     "ghostscript/resource/init/pdf_font.ps",
                                     "ghostscript/resource/init/pdf_main.ps",
                                     "ghostscript/resource/init/pdf_ops.ps",
                                     "ghostscript/resource/init/pdf_rbld.ps",
                                     "ghostscript/resource/init/pdf_sec.ps",
                                     "ghostscript/resource/init/xlatmap",
                                     "ghostscript/resource/substcid/cns1-wmode",
                                     "ghostscript/resource/substcid/gb1-wmode",
                                     "ghostscript/resource/substcid/japan1-wmode",
                                     "ghostscript/resource/substcid/korea1-wmode",
                                     "pdftoppm/cairo.dll",
                                     "pdftoppm/charset.dll",
                                     "pdftoppm/deflate.dll",
                                     "pdftoppm/fontconfig-1.dll",
                                     "pdftoppm/freetype.dll",
                                     "pdftoppm/iconv.dll",
                                     "pdftoppm/lcms2.dll",
                                     "pdftoppm/lerc.dll",
                                     "pdftoppm/libcrypto-3-x64.dll",
                                     "pdftoppm/libcurl.dll",
                                     "pdftoppm/libexpat.dll",
                                     "pdftoppm/liblzma.dll",
                                     "pdftoppm/libpng16.dll",
                                     "pdftoppm/libssh2.dll",
                                     "pdftoppm/libtiff.dll",
                                     "pdftoppm/libzstd.dll",
                                     "pdftoppm/openjp2.dll",
                                     "pdftoppm/pdfinfo.exe",
                                     "pdftoppm/pdftoppm.exe",
                                     "pdftoppm/poppler-cpp.dll",
                                     "pdftoppm/poppler-glib.dll",
                                     "pdftoppm/poppler.dll",
                                     "pdftoppm/tiff.dll",
                                     "pdftoppm/zlib.dll",
                                     "pdftoppm/zstd.dll",
                                     "platforms/qminimal.dll",
                                     "platforms/qoffscreen.dll",
                                     "platforms/qwindows.dll",
                                     "plugins/tls/qcertonlybackend.dll",
                                     "plugins/tls/qopensslbackend.dll",
                                     "plugins/tls/qschannelbackend.dll",
                                     "police/roboto-bold.ttf",
                                     "police/roboto-bolditalic.ttf",
                                     "police/roboto-italic.ttf",
                                     "police/roboto-regular.ttf",
                                     "police/robotomono-bold.ttf",
                                     "police/robotomono-bolditalic.ttf",
                                     "police/robotomono-italic.ttf",
                                     "police/robotomono-regular.ttf" };
    QStringList Resultat;
    foreach (QString Fichier, FichierAchercher) {
        if (!QFileInfo::exists(QCoreApplication::applicationDirPath() + "/" + Fichier)) {
            Resultat.append(QString("Le fichier %1 est absent de l'installation").arg(Fichier));
            continue;
        }
        if (QFileInfo(QCoreApplication::applicationDirPath() + "/" + Fichier).size() == 0)
            Resultat.append(QString("Le fichier %1 est présent mais de taille nulle").arg(Fichier));
    }
    if (Resultat.count() > 0) {
        MGBoxContinuer("Un ou plusieurs fichiers sont manquants ou corrompus",
                       "<b>L'application est susceptible de dysfonctionner.</b>",
                       QMessageBox::Icon::Warning,
                       "<i>Il est recommandé de réinstaller l'application</i>",
                       Resultat.join("\n"));
        Consigne("Erreur lors du contrôle d'intégrité, un ou plusieurs fichiers "
                 "sont manquants ou corrompus");
        Consigne("Voici les fichiers concernés :");
        foreach (auto Fichier, Resultat)
            Consigne(" " + Fichier);
        Consigne("Voici les fichiers concernés :");
    } else
        Consigne("Les fichiers présents ne sont pas corrompus, REEMaker peut "
                 "fonctionner correctement.");
    return true;
}

/** AfficheEditeurPDG:
  Action pour afficher / masquer l'éditeur de page de garde

  @return Aucun
*/
void
MainWindow::AfficheEditeurPDG()
{
    if (ui->tabWidget->currentIndex() == 1)
        return;                 // On est en plein processus de PDG
    if (savState.ModeEditeur) { // On est en mode éditeur
        ui->tabWidget->setTabVisible(0, true);
        ui->tabWidget->setTabVisible(2, true);
        ui->tabWidget->setTabVisible(3, true);
        ui->tabWidget->setTabVisible(4, false);
        ui->tabWidget->setCurrentIndex(savState.OngletActif);
        this->setMinimumWidth(940);
        AfficheEditAction->setText("Basculer vers l'&Editeur de page de garde");
        savState.ModeEditeur = false;
    } else { // On est en mode normal
        savState.OngletActif = ui->tabWidget->currentIndex();
        ui->tabWidget->setTabVisible(0, false);
        ui->tabWidget->setTabVisible(2, false);
        ui->tabWidget->setTabVisible(3, false);
        ui->tabWidget->setTabVisible(4, true);
        ui->tabWidget->setCurrentIndex(4);
        ui->tabWidget->setTabText(4, "Edition de page de gardes - Nouvelle page de garde");
        AfficheEditAction->setText("Sortir de l'&Editeur de page de garde");
        savState.Geometrie = this->geometry();
        this->setMinimumWidth(1280);
        if (savState.Geometrie.width() < 1280)
            this->resize(1280, this->size().height());

        savState.ModeEditeur = true;
    }
}

/** AfficheAide:
  Action pour afficher l'aide de REEMaker

  @return Aucun
*/
void
MainWindow::AfficheAide()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + "/REEMakerAide.pdf"));
}

/** BasculerClairSombre:
  Permet la bascule entre le mode clair et sombre, propose un redémarrage de
 l'application

  @return Aucun
*/
void
MainWindow::BasculerClairSombre()
{
    QMessageBox mBox;
    mBox.setWindowTitle(QString("Modification du thème de l'application "));
    mBox.setWindowIcon(this->windowIcon());
    mBox.setText(QString("Sélectionner le thème à appliquer (Thème actuel : %1) :").arg(ThemeDark ? "Sombre" : "Clair"));
    QAbstractButton* pButtonClair =
      mBox.addButton(QString(" Appliquer le mode clair%1").arg(ThemeDark ? "\n (Redémarrage de REEMaker obligatoire)" : ""), QMessageBox::YesRole);
    QAbstractButton* pButtonSombre =
      mBox.addButton(QString(" Appliquer le mode sombre%1").arg(!ThemeDark ? "\n (Redémarrage de REEMaker obligatoire)" : ""), QMessageBox::NoRole);
    QAbstractButton* pButtonAnnuler = mBox.addButton(" Ne rien faire", QMessageBox::RejectRole);
    pButtonClair->setIcon(QIcon(":/ClairSombre"));
    pButtonSombre->setIcon(QIcon(":/ClairSombre"));
    pButtonAnnuler->setIcon(QIcon(":/RienFaire"));
    pButtonClair->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pButtonSombre->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pButtonAnnuler->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    //    if (ThemeDark)
    //        pButtonSombre->setMinimumHeight(pButtonClair->height());
    //    else
    //        pButtonClair->setMinimumHeight(pButtonSombre->height());
    // pButtonAnnuler->setMinimumHeight(pButtonSombre->height());
    {
        RECT FenetrePrincipale;
        GetWindowRect((HWND)this->winId(), &FenetrePrincipale);
        mBox.setGeometry(FenetrePrincipale.left + (this->geometry().width() / 2) - (550 / 2),
                         FenetrePrincipale.top + (this->geometry().height() / 2) - (250 / 2),
                         550,
                         250);
    }
    mBox.exec();

    if (mBox.clickedButton() == pButtonAnnuler) {
        return;
    }
    bool Redemarre = false;
    if (mBox.clickedButton() == pButtonClair) {
        if (ThemeDark) {
            ThemeDark = false;
            Redemarre = true;
        }
    }
    if (mBox.clickedButton() == pButtonSombre) {
        if (ThemeDark == false) {
            ThemeDark = true;
            qApp->quit();
            Redemarre = true;
        }
    }
    if (Redemarre) {
        qApp->quit();
        QProcess myProcess;
        myProcess.setProgram(qApp->arguments()[0]);
        myProcess.setArguments(qApp->arguments().mid(1));
        myProcess.QProcess::startDetached(qApp->arguments()[0], qApp->arguments().mid(1));
    }
}

/** on_PDG_Btn_SupprimePDGUtilisateur_clicked:
  Suppression de la page de garde sélectionnée

  @return Aucun
*/
void
MainWindow::on_PDG_Btn_SupprimePDGUtilisateur_clicked()
{
    if (ui->PDG_Combo_Utilisateur->count() == 0)
        return;
    auto Retour = MGBoxContinuerAnnuler("Suppression",
                                        "<b>Vous allez supprimer la page de garde...</b>",
                                        QMessageBox::Icon::Question,
                                        QString("<i>Page de garde concernée : '%1'</i>").arg(ui->PDG_Combo_Utilisateur->currentText()));
    if (Retour == ReponseMGBOX::repond_Annuler) {
        return;
    }
    if (Retour == ReponseMGBOX::repond_Continuer) {
        QFile::remove(CheminPDGUtilisateur + ui->PDG_Combo_Utilisateur->currentText() + ".txt");
        Consigne("Suppression de la page de garde utilisateur '" + ui->PDG_Combo_Utilisateur->currentText() + "'");
        ui->PDG_Combo_Utilisateur->removeItem(ui->PDG_Combo_Utilisateur->currentIndex());
        ui->PDG_ListeWidget->clear();
    }
}

/** LectureINI:
  Lecture des paramètres utilisateurs

  @return Aucun
*/
void
MainWindow::LectureINI()
{
    QSettings settings(QCoreApplication::applicationDirPath() + "/REEMAKER.ini", QSettings::IniFormat);
    ThemeDark = settings.value("REGLAGES/ThemeSombre", "false").toBool();
    ui->Folioter_Txt_NomDuSite->setText(settings.value("REGLAGES/TAMPON_NomDuSite", "").toString());
    ui->Folioter_Txt_RefREE->setText(settings.value("REGLAGES/TAMPON_ReferenceREE", "").toString());
    ui->Folioter_Txt_IndiceREE->setText(settings.value("REGLAGES/TAMPON_Indice", "").toString());
    ui->Folioter_Spin_PremierePage->setValue(settings.value("REGLAGES/Folio_premier_numero", "3").toInt());
    ui->Folioter_Texte_Tranche0->setText(settings.value("REGLAGES/CycleTranche0", "").toString());
    ui->Folioter_Texte_Tranche1->setText(settings.value("REGLAGES/CycleTranche1", "").toString());
    ui->Folioter_Texte_Tranche2->setText(settings.value("REGLAGES/CycleTranche2", "").toString());
    ui->Folioter_Texte_Tranche3->setText(settings.value("REGLAGES/CycleTranche3", "").toString());
    ui->Folioter_Texte_Tranche4->setText(settings.value("REGLAGES/CycleTranche4", "").toString());
    ui->Folioter_Texte_Tranche5->setText(settings.value("REGLAGES/CycleTranche5", "").toString());
    ui->Folioter_Texte_Tranche6->setText(settings.value("REGLAGES/CycleTranche6", "").toString());
    ui->Folioter_Texte_Tranche7->setText(settings.value("REGLAGES/CycleTranche7", "").toString());
    ui->Folioter_Texte_Tranche8->setText(settings.value("REGLAGES/CycleTranche8", "").toString());
    ui->Folioter_Texte_Tranche9->setText(settings.value("REGLAGES/CycleTranche9", "").toString());
    ui->PDG_Texte_PDGEnCours->setText(settings.value("REGLAGES/PDGutilise", "page de garde standard BPA").toString().toUpper());
    ui->OPT_BText_CoulTranche_0->setText(settings.value("REGLAGES/CouleurTampon0", "#0000FF").toString());
    ui->OPT_BText_CoulTranche_1->setText(settings.value("REGLAGES/CouleurTampon1", "#0000FF").toString());
    ui->OPT_BText_CoulTranche_2->setText(settings.value("REGLAGES/CouleurTampon2", "#0000FF").toString());
    ui->OPT_BText_CoulTranche_3->setText(settings.value("REGLAGES/CouleurTampon3", "#0000FF").toString());
    ui->OPT_BText_CoulTranche_4->setText(settings.value("REGLAGES/CouleurTampon4", "#0000FF").toString());
    ui->OPT_BText_CoulTranche_5->setText(settings.value("REGLAGES/CouleurTampon5", "#0000FF").toString());
    ui->OPT_BText_CoulTranche_6->setText(settings.value("REGLAGES/CouleurTampon6", "#0000FF").toString());
    ui->OPT_BText_CoulTranche_7->setText(settings.value("REGLAGES/CouleurTampon7", "#0000FF").toString());
    ui->OPT_BText_CoulTranche_8->setText(settings.value("REGLAGES/CouleurTampon8", "#0000FF").toString());
    ui->OPT_BText_CoulTranche_9->setText(settings.value("REGLAGES/CouleurTampon9", "#0000FF").toString());
    COLORfromTEXT(ui->OPT_BText_CoulTranche_0, ui->OPT_Btn_ColTranche0);
    COLORfromTEXT(ui->OPT_BText_CoulTranche_1, ui->OPT_Btn_ColTranche1);
    COLORfromTEXT(ui->OPT_BText_CoulTranche_2, ui->OPT_Btn_ColTranche2);
    COLORfromTEXT(ui->OPT_BText_CoulTranche_3, ui->OPT_Btn_ColTranche3);
    COLORfromTEXT(ui->OPT_BText_CoulTranche_4, ui->OPT_Btn_ColTranche4);
    COLORfromTEXT(ui->OPT_BText_CoulTranche_5, ui->OPT_Btn_ColTranche5);
    COLORfromTEXT(ui->OPT_BText_CoulTranche_6, ui->OPT_Btn_ColTranche6);
    COLORfromTEXT(ui->OPT_BText_CoulTranche_7, ui->OPT_Btn_ColTranche7);
    COLORfromTEXT(ui->OPT_BText_CoulTranche_8, ui->OPT_Btn_ColTranche8);
    COLORfromTEXT(ui->OPT_BText_CoulTranche_9, ui->OPT_Btn_ColTranche9);
    ui->OPT_BText_CoulAccent_0->setText(settings.value("REGLAGES/CouleurAccent0", "#E8E8E8").toString());
    ui->OPT_BText_CoulAccent_1->setText(settings.value("REGLAGES/CouleurAccent1", "#E8E8E8").toString());
    ui->OPT_BText_CoulAccent_2->setText(settings.value("REGLAGES/CouleurAccent2", "#E8E8E8").toString());
    ui->OPT_BText_CoulAccent_3->setText(settings.value("REGLAGES/CouleurAccent3", "#E8E8E8").toString());
    ui->OPT_BText_CoulAccent_4->setText(settings.value("REGLAGES/CouleurAccent4", "#E8E8E8").toString());
    ui->OPT_BText_CoulAccent_5->setText(settings.value("REGLAGES/CouleurAccent5", "#E8E8E8").toString());
    ui->OPT_BText_CoulAccent_6->setText(settings.value("REGLAGES/CouleurAccent6", "#E8E8E8").toString());
    ui->OPT_BText_CoulAccent_7->setText(settings.value("REGLAGES/CouleurAccent7", "#E8E8E8").toString());
    ui->OPT_BText_CoulAccent_8->setText(settings.value("REGLAGES/CouleurAccent8", "#E8E8E8").toString());
    ui->OPT_BText_CoulAccent_9->setText(settings.value("REGLAGES/CouleurAccent9", "#E8E8E8").toString());
    COLORfromTEXT(ui->OPT_BText_CoulAccent_0, ui->OPT_Btn_ColAccent0);
    COLORfromTEXT(ui->OPT_BText_CoulAccent_1, ui->OPT_Btn_ColAccent1);
    COLORfromTEXT(ui->OPT_BText_CoulAccent_2, ui->OPT_Btn_ColAccent2);
    COLORfromTEXT(ui->OPT_BText_CoulAccent_3, ui->OPT_Btn_ColAccent3);
    COLORfromTEXT(ui->OPT_BText_CoulAccent_4, ui->OPT_Btn_ColAccent4);
    COLORfromTEXT(ui->OPT_BText_CoulAccent_5, ui->OPT_Btn_ColAccent5);
    COLORfromTEXT(ui->OPT_BText_CoulAccent_6, ui->OPT_Btn_ColAccent6);
    COLORfromTEXT(ui->OPT_BText_CoulAccent_7, ui->OPT_Btn_ColAccent7);
    COLORfromTEXT(ui->OPT_BText_CoulAccent_8, ui->OPT_Btn_ColAccent8);
    COLORfromTEXT(ui->OPT_BText_CoulAccent_9, ui->OPT_Btn_ColAccent9);
    qint16 Emplacement = settings.value("REGLAGES/EmplacementTampon", "0").toInt();
    if (Emplacement == 0)
        ui->OPT_Radio_HG->setChecked(true);
    if (Emplacement == 1)
        ui->OPT_Radio_HD->setChecked(true);
    if (Emplacement == 2)
        ui->OPT_Radio_BG->setChecked(true);
    if (Emplacement == 3)
        ui->OPT_Radio_BD->setChecked(true);
    ui->OPT_Check_OuvrirApres->setChecked(settings.value("REGLAGES/OuvrirApres", true).toBool());
    ui->OPT_Check_GhostScript->setChecked(settings.value("REGLAGES/Reparer", true).toBool());
    ui->OPT_Check_MAJ->setChecked(settings.value("REGLAGES/MAJAUTO", false).toBool());
    ui->OPT_Spin_Largeur->setValue(settings.value("REGLAGES/MargeLateral", "20").toInt());
    ui->OPT_Spin_Hauteur->setValue(settings.value("REGLAGES/MargeVertical", "20").toInt());
    ui->OPT_Spin_ResolutionDPI->setValue(settings.value("REGLAGES/ResolutionDPI", "40").toInt());
    ui->OPT_Text_FolioAnnule->setText(settings.value("REGLAGES/FolioAnnule", "Folio annulé").toString());
    PDGManuelNomSite = settings.value("REGLAGES/MANUEL_NomDuSite", "").toString();
    PDGManuelRefREE  = settings.value("REGLAGES/MANUEL_ReferenceREE", "").toString();
    PDGManuelIndice  = settings.value("REGLAGES/MANUEL_Indice", "").toString();
    Consigne("Fichier de paramètre pris en compte");
}

/** MiseEnPlaceChemin:
  Mise en place des dossiers des applications, temporaires et de la police
 temporaire

  @return Aucun
*/
void
MainWindow::MiseEnPlaceChemin()
{
    CheminPoppler        = QCoreApplication::applicationDirPath() + "/PdfToPPM/pdftoppm.exe";
    CheminGhostScript    = QCoreApplication::applicationDirPath() + "/GhostScript/BatchGhostScript.bat";
    CheminPDGUtilisateur = QCoreApplication::applicationDirPath() + "/PDG_utilisateur/";
    {
        QDir directory(QFileInfo(CheminPDGUtilisateur).filePath());
        QStringList lstPDGBase = directory.entryList(QStringList() << "*.txt", QDir::Files, QDir::Name | QDir::IgnoreCase);
        foreach (QString lPDG, lstPDGBase) {
            ui->PDG_Combo_Utilisateur->addItem(QFileInfo(CheminPDGUtilisateur + lPDG).baseName());
        }
    }
    CheminPDGBase = QCoreApplication::applicationDirPath() + "/PDG_modele/";
    {
        QDir directory(QFileInfo(CheminPDGBase).filePath());
        QStringList lstPDGBase = directory.entryList(QStringList() << "*.txt", QDir::Files, QDir::Name | QDir::IgnoreCase);
        int DefautIndex        = -1;
        foreach (QString lPDG, lstPDGBase) {
            ui->PDG_Combo_Integre->addItem(QFileInfo(CheminPDGBase + lPDG).baseName());
            if (lPDG.toLower() == "page de garde standard bpa.txt") // On sait que cette PageDeGarde
                // est la : page de garde standard
                // BPA.txt
                DefautIndex = ui->PDG_Combo_Integre->count() - 1;
        }
        if (DefautIndex != -1) {
            ui->PDG_Combo_Integre->setCurrentIndex(DefautIndex);
        }
    }
    {
        bool TrouvePDG = false;
        /* -- Test si ce nom est dans combo integré -- */
        for (int var = 0; var < ui->PDG_Combo_Integre->count(); ++var) {
            if (ui->PDG_Combo_Integre->itemText(var).toUpper() == ui->PDG_Texte_PDGEnCours->text().toUpper()) {
                ui->PDG_Combo_Integre->setCurrentIndex(var);
                ui->PDG_Texte_PDGEnCours->setText(ui->PDG_Combo_Integre->itemText(var).toUpper());
                ChargerPageDeGarde(ui->PDG_Combo_Integre->itemText(var), CheminPDGBase);
                TrouvePDG = true;
                break;
            }
        }
        /* -- Test si ce nom est dans Combo utilisateur -- */
        if (!TrouvePDG)
            for (int var = 0; var < ui->PDG_Combo_Utilisateur->count(); ++var) {
                if (ui->PDG_Combo_Utilisateur->itemText(var).toUpper() == ui->PDG_Texte_PDGEnCours->text().toUpper()) {
                    ui->PDG_Combo_Utilisateur->setCurrentIndex(var);
                    ui->PDG_Texte_PDGEnCours->setText(ui->PDG_Combo_Utilisateur->itemText(var).toUpper());
                    ChargerPageDeGarde(ui->PDG_Combo_Utilisateur->itemText(var), CheminPDGUtilisateur);
                    TrouvePDG = true;
                    break;
                }
            }
        /* -- Aucun, on le reset -- */
        if (!TrouvePDG)
            for (int var = 0; var < ui->PDG_Combo_Integre->count(); ++var) {
                if (ui->PDG_Combo_Integre->itemText(var).toLower() == "page de garde standard bpa") {
                    ui->PDG_Combo_Integre->setCurrentIndex(var);
                    ui->PDG_Texte_PDGEnCours->setText(ui->PDG_Combo_Integre->itemText(var).toUpper());
                    ChargerPageDeGarde(ui->PDG_Combo_Integre->itemText(var), CheminPDGBase);
                    break;
                }
            }
    }
    QString RANDOM = QUuid::createUuid().toString();
    RANDOM.remove(QRegularExpression("{|}|-")); // Seulement les caractères en hexa
    CheminTemp = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/REEMAKER_" + RANDOM;
    QDir().mkpath(CheminTemp);
    TempTamponRoboto = CheminTemp + "/Roboto-Regular.ttf";
    QFile::copy(QCoreApplication::applicationDirPath() + "/Police/Roboto-Regular.ttf", TempTamponRoboto);
    /* -- Lecture de la fonte en mémoire en QByteArray -- */
    QFile file(QCoreApplication::applicationDirPath() + "/Police/robotomono-Regular.ttf");
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray blobFont = file.readAll();
        QFontDatabase::addApplicationFontFromData(blobFont);
        file.close();
    }
}

/** SauveINI:
  Sauvegarde des paramètres utilisateurs

  @return Aucun
*/
void
MainWindow::SauveINI()
{
    QSettings settings(QCoreApplication::applicationDirPath() + "/REEMAKER.ini",
                       QSettings::IniFormat); // Création du fichier en précisant que l'on
    // travaille avec un fichier de format INI.
    settings.beginGroup("REGLAGES");

    settings.setValue("TAMPON_NomDuSite", ui->Folioter_Txt_NomDuSite->text());
    settings.setValue("TAMPON_ReferenceREE", ui->Folioter_Txt_RefREE->text());
    settings.setValue("TAMPON_Indice", ui->Folioter_Txt_IndiceREE->text());
    settings.setValue("Folio_premier_numero", ui->Folioter_Spin_PremierePage->value());

    settings.setValue("CycleTranche0", ui->Folioter_Texte_Tranche0->text());
    settings.setValue("CycleTranche1", ui->Folioter_Texte_Tranche1->text());
    settings.setValue("CycleTranche2", ui->Folioter_Texte_Tranche2->text());
    settings.setValue("CycleTranche3", ui->Folioter_Texte_Tranche3->text());
    settings.setValue("CycleTranche4", ui->Folioter_Texte_Tranche4->text());
    settings.setValue("CycleTranche5", ui->Folioter_Texte_Tranche5->text());
    settings.setValue("CycleTranche6", ui->Folioter_Texte_Tranche6->text());
    settings.setValue("CycleTranche7", ui->Folioter_Texte_Tranche7->text());
    settings.setValue("CycleTranche8", ui->Folioter_Texte_Tranche8->text());
    settings.setValue("CycleTranche9", ui->Folioter_Texte_Tranche9->text());

    settings.setValue("PDGutilise", ui->PDG_Texte_PDGEnCours->text());

    settings.setValue("CouleurTampon0", ui->OPT_BText_CoulTranche_0->text());
    settings.setValue("CouleurTampon1", ui->OPT_BText_CoulTranche_1->text());
    settings.setValue("CouleurTampon2", ui->OPT_BText_CoulTranche_2->text());
    settings.setValue("CouleurTampon3", ui->OPT_BText_CoulTranche_3->text());
    settings.setValue("CouleurTampon4", ui->OPT_BText_CoulTranche_4->text());
    settings.setValue("CouleurTampon5", ui->OPT_BText_CoulTranche_5->text());
    settings.setValue("CouleurTampon6", ui->OPT_BText_CoulTranche_6->text());
    settings.setValue("CouleurTampon7", ui->OPT_BText_CoulTranche_7->text());
    settings.setValue("CouleurTampon8", ui->OPT_BText_CoulTranche_8->text());
    settings.setValue("CouleurTampon9", ui->OPT_BText_CoulTranche_9->text());

    settings.setValue("CouleurAccent0", ui->OPT_BText_CoulAccent_0->text());
    settings.setValue("CouleurAccent1", ui->OPT_BText_CoulAccent_1->text());
    settings.setValue("CouleurAccent2", ui->OPT_BText_CoulAccent_2->text());
    settings.setValue("CouleurAccent3", ui->OPT_BText_CoulAccent_3->text());
    settings.setValue("CouleurAccent4", ui->OPT_BText_CoulAccent_4->text());
    settings.setValue("CouleurAccent5", ui->OPT_BText_CoulAccent_5->text());
    settings.setValue("CouleurAccent6", ui->OPT_BText_CoulAccent_6->text());
    settings.setValue("CouleurAccent7", ui->OPT_BText_CoulAccent_7->text());
    settings.setValue("CouleurAccent8", ui->OPT_BText_CoulAccent_8->text());
    settings.setValue("CouleurAccent9", ui->OPT_BText_CoulAccent_9->text());

    if (ui->OPT_Radio_HG->isChecked())
        settings.setValue("EmplacementTampon", 0);
    if (ui->OPT_Radio_HD->isChecked())
        settings.setValue("EmplacementTampon", 1);
    if (ui->OPT_Radio_BG->isChecked())
        settings.setValue("EmplacementTampon", 2);
    if (ui->OPT_Radio_BD->isChecked())
        settings.setValue("EmplacementTampon", 3);

    settings.setValue("OuvrirApres", ui->OPT_Check_OuvrirApres->isChecked());
    settings.setValue("Reparer", ui->OPT_Check_GhostScript->isChecked());
    settings.setValue("MAJAUTO", ui->OPT_Check_MAJ->isChecked());
    settings.setValue("MargeLateral", ui->OPT_Spin_Largeur->value());
    settings.setValue("MargeVertical", ui->OPT_Spin_Hauteur->value());
    settings.setValue("ResolutionDPI", ui->OPT_Spin_ResolutionDPI->value());
    settings.setValue("FolioAnnule", ui->OPT_Text_FolioAnnule->text());
    settings.setValue("ThemeSombre", ThemeDark);
    settings.setValue("REGLAGES/MANUEL_NomDuSite", PDGManuelNomSite);
    settings.setValue("REGLAGES/MANUEL_ReferenceREE", PDGManuelRefREE);
    settings.setValue("REGLAGES/MANUEL_Indice", PDGManuelIndice);
    settings.endGroup();
    Consigne("Fichier de paramètre enregistré");
}

/** MiseEnPlaceValidator:
  Mise en place des validators pour écrire en majuscule

  @return Aucun
*/
void
MainWindow::MiseEnPlaceValidator()
{
    validUPPER = new MyValidatorUPPER(this);
    ui->Folioter_Txt_NomDuSite->setValidator(validUPPER);
    ui->Folioter_Txt_RefREE->setValidator(validUPPER);
    ui->Folioter_Txt_IndiceREE->setValidator(validUPPER);
    ui->Folioter_Texte_Tranche0->setValidator(validUPPER);
    ui->Folioter_Texte_Tranche1->setValidator(validUPPER);
    ui->Folioter_Texte_Tranche2->setValidator(validUPPER);
    ui->Folioter_Texte_Tranche3->setValidator(validUPPER);
    ui->Folioter_Texte_Tranche4->setValidator(validUPPER);
    ui->Folioter_Texte_Tranche5->setValidator(validUPPER);
    ui->Folioter_Texte_Tranche6->setValidator(validUPPER);
    ui->Folioter_Texte_Tranche7->setValidator(validUPPER);
    ui->Folioter_Texte_Tranche8->setValidator(validUPPER);
    ui->Folioter_Texte_Tranche9->setValidator(validUPPER);
}

/** MiseEnPlaceInterface:
  Mise en place de l'interface

  @return Aucun
*/
void
MainWindow::MiseEnPlaceInterface()
{
    mVersion = GetVersion(qApp->arguments()[0]);
    this->setWindowTitle(QString("REEMaker Version %1").arg(mVersion));
    ui->APROPOS_Texte->document()->setHtml(ui->APROPOS_Texte->document()->toHtml().replace("%VERSION%", mVersion));
    QTextCursor textCursor = ui->APROPOS_Texte->textCursor();
    textCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor, 1);
    ui->APROPOS_Texte->setTextCursor(textCursor); // The line to add

    ClairSombreAction = ui->menubar->addAction("Basculer entre le &mode clair et sombre");
    connect(ClairSombreAction, SIGNAL(triggered()), this, SLOT(BasculerClairSombre()));
    ui->menubar->addSeparator();
    AfficheEditAction = ui->menubar->addAction("Basculer vers l'&Editeur de page de garde");
    connect(AfficheEditAction, SIGNAL(triggered()), this, SLOT(AfficheEditeurPDG()));
    ui->menubar->addSeparator();
    helpAction = ui->menubar->addAction("&Afficher l'aide");
    connect(helpAction, SIGNAL(triggered()), this, SLOT(AfficheAide()));

    ui->Folioter_Group_RefTampon->setVisible(false);
    ui->Folioter_Group_Etendue->setVisible(false);
    ui->Folioter_Group_Annuler->setVisible(false);
    ui->Folioter_Group_ChoixTranche->setVisible(false);
    ui->Folioter_Btn_EtapeSuivante1->setEnabled(false);
    ui->Folioter_Btn_EtapeSuivante2->setEnabled(false);
    ui->tabWidget->setTabVisible(1, false);
    ui->Annulation_Liste->setColumnCount(2);
    ui->Annulation_Liste->setRowCount(0);
    ui->Annulation_Liste->setColumnWidth(0, 20);
    ui->Annulation_Liste->setColumnWidth(1, 160);
    ui->EDIT_Liste->setSortingEnabled(false);
    ui->tabWidget->setCurrentIndex(0);
    ui->tabWidget->setTabVisible(4, false);
    if (ThemeDark) {
        ui->APROPOS_Texte->setStyleSheet("QTextEdit, QListView {"
                                         "    background-color: rgb(235, 235, 235);"
                                         "    color: rgb(0, 0, 0);"
                                         "    a {color: white; };"
                                         "}");
        qDebug() << "StyleSheet DONE !";
    }
}

/** MiseEnPlaceListInfo:
  Mise en place de la police pour le ListInfo PDF et la première ligne en gras

  @return Aucun
*/
void
MainWindow::MiseEnPlaceListInfo()
{
    fontList = ui->FolioListeINFO->font();
    if (fontList.family() != "Roboto Mono")
        fontList.setPointSize(fontList.pointSize() -
                              /*1*/ 0 /*1 fait trop petit*/);
    fontList.setFamily("Roboto Mono");
    // Ajout de la police de LOG
    ui->listLOG->setFont(fontList);
    // Ajout de la première ligne de Info PDF
    ui->FolioListeINFO->setFont(fontList);
    ui->FolioListeINFO->clear();
    QLabel* mLabel = new QLabel(this);
    mLabel->setText("Information sur le fichier PDF");
    QFont font = fontList;
    font.setBold(true);
    font.setUnderline(true);
    mLabel->setFont(font);
    ui->FolioListeINFO->setUpdatesEnabled(false);
    QListWidgetItem* item;
    item = new QListWidgetItem(ui->FolioListeINFO);
    ui->FolioListeINFO->addItem(item);
    item->setSizeHint(mLabel->minimumSizeHint());
    ui->FolioListeINFO->setItemWidget(item, mLabel);
    ui->FolioListeINFO->setUpdatesEnabled(true);
}

/** on_FolioListeINFO_customContextMenuRequested:
  Apparition du menu contextuel pour copier en mémoire des infos de la liste
 d'information PDF

  @return Aucun
*/
void
MainWindow::on_FolioListeINFO_customContextMenuRequested(const QPoint& pos)
{
    Q_UNUSED(pos);
    QMenu* menu = new QMenu(this);

    menu->addAction(QString("Information PDF"));
    menu->addSeparator();
    menu->addAction(QIcon(":/Copier"), QString(" Copier cette ligne dans le presse-papier"), this, [&](bool) {
        QApplication::clipboard()->setText(ui->FolioListeINFO->currentItem()->text());
    });
    menu->actions()[0]->setEnabled(false);
    QCursor mCursor;
    POINT Souris;
    GetCursorPos(&Souris);
    mCursor.setPos(Souris.x, Souris.y);
    menu->popup(mCursor.pos() /*this->mapTo(this, QCursor::pos())*/);
    RECT FenetreActuel;
    GetWindowRect((HWND)menu->winId(), &FenetreActuel);
    MoveWindow((HWND)menu->winId(), Souris.x, Souris.y, (FenetreActuel.right - FenetreActuel.left), (FenetreActuel.bottom - FenetreActuel.top), TRUE);
}

/** resizeEvent:
  Override de la fonction resize de MainWindow

  @param QResizeEvent* event
  @return Aucun
*/
void
MainWindow::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    if (ui->tabWidget->currentIndex() == 1)
        if (!QPXpreview.isNull())
            ui->Annulation_LabelPrevisualisation->setPixmap(QPXpreview.scaled(ui->Annulation_LabelPrevisualisation->width() - 10,
                                                                              ui->Annulation_LabelPrevisualisation->height() - 10,
                                                                              Qt::KeepAspectRatio,
                                                                              Qt::SmoothTransformation));
}

/** DerniereVersion:
  Retourne la dernière version en ligne sur Github via les champs TAGS

  @return Aucun
*/
QString
MainWindow::DerniereVersion()
{
    bool timeout;
    QTimer* timer = new QTimer();
    connect(timer, &QTimer::timeout, this, [&]() { timeout = true; });
    QUrl url("https://api.github.com/repos/alwendya/REEMakerQT/tags");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkAccessManager nam;
    QNetworkReply* reply = nam.get(request);

    timeout = false;
    timer->start(5000);

    while (!timeout) {
        qApp->processEvents();
        if (reply->isFinished())
            break;
    }

    if (reply->isFinished()) {
        QByteArray response_data = reply->readAll();
        QJsonDocument json       = QJsonDocument::fromJson(response_data);
        return json[0]["name"].toString();
    } else {
        return QString("Timeout");
    }
}

/** DemarreLeTelechargement:
  Lance le téléchargement d'une URL
  @param QUrl updateUrl
  @return Aucun
*/
void
MainWindow::DemarreLeTelechargement(QUrl updateUrl)
{
    HttpDownload* myDownloader = new HttpDownload(nullptr);
    auto Initialisation        = myDownloader->DemarreTelechargement(
      updateUrl, QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/update.REEMaker.7z", "Mise à jour de REEMaker");
    if (Initialisation != HttpDownload::StatutDemarrage::TelechargementDemarre) {
        if (Initialisation == HttpDownload::StatutDemarrage::ErreurCreationFichierDestination)
            Consigne("Erreur au démarrage du téléchargement : Impossible d'écrire "
                     "sur disque le fichier téléchargé");
        if (Initialisation == HttpDownload::StatutDemarrage::AbandonFichierDestinationExistant)
            Consigne("Erreur au démarrage du téléchargement : Le fichier destination "
                     "existe déjà, l'utilisateur à choisi de ne pas l'écraser");
        if (Initialisation == HttpDownload::StatutDemarrage::PasDeDestination)
            Consigne("Erreur au démarrage du téléchargement : Chemin de destination "
                     "non renseigné");
        if (Initialisation == HttpDownload::StatutDemarrage::PasDeSource)
            Consigne("Erreur au démarrage du téléchargement : URL source non renseigné");
    } else {
        while (true) {
            QThread::msleep(250);
            QCoreApplication::processEvents();
            auto Statut = myDownloader->RetourneStatut();

            if (Statut == HttpDownload::EtatFinTelechargement::EnCours)
                qDebug().noquote().nospace() << "C'est normal, sa tourne...";
            if (Statut == HttpDownload::EtatFinTelechargement::AbandonnerParUtilisateur) {
                Consigne("Opération abandonner par l'utilisateur");
                break;
            }
            if (Statut == HttpDownload::EtatFinTelechargement::ErreurDeTelechargement) {
                Consigne("Erreur inconnue au téléchargement...");
                break;
            }
            if (Statut == HttpDownload::EtatFinTelechargement::TermineSansErreur) {
                Consigne("Le téléchargement de la mise à jour à réussi, décompression "
                         "et msie en place...");
                QStringList Operations;
                Operations << QString(" & \"%1\" e -y \"%2\" -o\"%3\"")
                                .arg(QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + "/7za.exe"),
                                     QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/update.REEMaker.7z"),
                                     QDir::toNativeSeparators(QCoreApplication::applicationDirPath()));
                Operations << QString(" & \"%1\"").arg(QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + "/REEMaker.exe"));
                OperationApresQuitter(Operations); // Lance l'update après 5 secondes
                QApplication::quit();
                break;
            }
        }
    }
    delete myDownloader;
}

/** Consigne:
  Consigne les informations à différentes destinations

  @param Message : Le texte à consigner
  @param EcritSurDisque: Ecrit dans le fichier log sur disque
  @param AfficheDansLog : Affiche dans le ListView LOG
  @param AfficheDansDebug : Affiche dans la fenetre de Debug
  @return Aucun
*/
void
MainWindow::Consigne(QString Message, bool EcritSurDisque, bool AfficheDansLog, bool AfficheDansDebug)
{
    /* -- Ici on écrit le Message dans qDebug() -- */
    if (AfficheDansDebug)
        qDebug().nospace().noquote() << QDateTime::currentDateTime().toString("hh:mm:ss : ") << Message;
    /* -- Ici on écrit le Message dans ListLog -- */
    if (AfficheDansLog)
        ui->listLOG->insertItem(0, QDateTime::currentDateTime().toString("hh:mm:ss : ") + Message);
    if (EcritSurDisque) {
        /* -- Ici on va ecrire sur disque -- */
        QString CheminLOG = QCoreApplication::applicationDirPath() + "/Journal de bord.txt";
        QStringList ContenuJDB;
        if (QFile::exists(CheminLOG)) // On charge l'ancien si existant
        {
            QFile FichierJDB(CheminLOG);
            if (FichierJDB.open(QIODevice::ReadOnly)) {
                QTextStream in(&FichierJDB);
                while (!in.atEnd())
                    ContenuJDB.append(in.readLine());
                FichierJDB.close();
            }
        }
        if (ContenuJDB.count() > 1000) // On supprime les trops anciens (Max 1000 entrées)
            ContenuJDB.resize(1000);
        ContenuJDB.prepend(QDateTime::currentDateTime().toString("hh:mm:ss : ") + Message);
        { // On sauvegarde le tout
            QFile FichierJDB(CheminLOG);
            if (FichierJDB.open(QIODevice::WriteOnly)) {
                QTextStream f_out(&FichierJDB);
                for (qsizetype nLigne = 0; nLigne < ContenuJDB.size(); nLigne++)
                    f_out << ContenuJDB[nLigne] << Qt::endl;
                FichierJDB.close();
            }
        }
    }
}

void
MainWindow::RecoisLogMessage(const QString& arg)
{
    Consigne(arg);
}

bool
MainWindow::CentreHWND(HWND Fenetre)
{
    MonitorRects monitors;
    int NumEcran = 0;
    RECT FenetreAvecSouris;
    SetRect(&FenetreAvecSouris, 0, 0, 0, 0);
    foreach (auto var, monitors.rcMonitors) {
        POINT Souris;
        GetCursorPos(&Souris);
        RECT InterRect;
        RECT rSouris;
        SetRect(&rSouris, Souris.x, Souris.y, Souris.x + 10, Souris.y + 10);
        if (IntersectRect(&InterRect, &var, &rSouris) == TRUE) {
            qDebug() << "Souris dans l'écran n°" << NumEcran << "[" << monitors.qvName[NumEcran] << "]";
            SetRect(&FenetreAvecSouris, var.left, var.top, var.right, var.bottom);
        }
        NumEcran++;
    }
    if ((FenetreAvecSouris.right - FenetreAvecSouris.left) > 0) { // On peut centrer
        RECT FenetreActuel;
        GetWindowRect(Fenetre, &FenetreActuel);
        MoveWindow(Fenetre,
                   /*Left*/ FenetreAvecSouris.left + ((FenetreAvecSouris.right - FenetreAvecSouris.left) /*WidthEcran par2*/ / 2) -
                     ((FenetreActuel.right - FenetreActuel.left) /*WidthFenetre*/ / 2),
                   /*top*/ FenetreAvecSouris.top + ((FenetreAvecSouris.bottom - FenetreAvecSouris.top) /*HeightEcran par2*/ / 2) -
                     ((FenetreActuel.bottom - FenetreActuel.top) /*HeightFenetre*/ / 2),
                   /*width*/ FenetreActuel.right - FenetreActuel.left,
                   /*height*/ FenetreActuel.bottom - FenetreActuel.top,
                   TRUE);
    }
    return true;
}

MainWindow::ReponseMGBOX
MainWindow::MGBoxOuiNon(QString Titre, QString Message, QMessageBox::Icon icone, QString InformativeMessage, QString DetailledMessage)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(Titre);
    msgBox.setText(Message);
    msgBox.setInformativeText(InformativeMessage);
    msgBox.setDetailedText(DetailledMessage);
    msgBox.setIcon(icone);
    msgBox.setTextFormat(Qt::RichText);
    QAbstractButton* BoutonOui = msgBox.addButton("Oui", QMessageBox::YesRole);
    QAbstractButton* BoutonNon = msgBox.addButton("Non", QMessageBox::NoRole);
    {
        RECT FenetrePrincipale;
        GetWindowRect((HWND)this->winId(), &FenetrePrincipale);
        QSpacerItem* horizontalSpacer = new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding); // La fenetre fera 500px au minimum
        QGridLayout* layout           = (QGridLayout*)msgBox.layout();
        layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
        msgBox.setGeometry(FenetrePrincipale.left + (this->geometry().width() / 2) - (500 / 2),
                           FenetrePrincipale.top + (this->geometry().height() / 2) - (250 / 2),
                           500,
                           250);
    }
    msgBox.exec();
    if (msgBox.clickedButton() == BoutonOui) {
        return MainWindow::ReponseMGBOX::repond_Oui;
    } else if (msgBox.clickedButton() == BoutonNon) {
        return MainWindow::ReponseMGBOX::repond_Non;
    }
    return MainWindow::ReponseMGBOX::repond_Vide;
}

MainWindow::ReponseMGBOX
MainWindow::MGBoxContinuerAnnuler(QString Titre, QString Message, QMessageBox::Icon icone, QString InformativeMessage, QString DetailledMessage)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(Titre);
    msgBox.setText(Message);
    msgBox.setInformativeText(InformativeMessage);
    msgBox.setDetailedText(DetailledMessage);
    msgBox.setIcon(icone);
    msgBox.setTextFormat(Qt::RichText);
    QAbstractButton* BoutonContinuer = msgBox.addButton("Continuer", QMessageBox::AcceptRole);
    QAbstractButton* BoutonAnnuler   = msgBox.addButton("Annuler", QMessageBox::RejectRole);
    {
        RECT FenetrePrincipale;
        GetWindowRect((HWND)this->winId(), &FenetrePrincipale);
        QSpacerItem* horizontalSpacer = new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding); // La fenetre fera 500px au minimum
        QGridLayout* layout           = (QGridLayout*)msgBox.layout();
        layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
        msgBox.setGeometry(FenetrePrincipale.left + (this->geometry().width() / 2) - (500 / 2),
                           FenetrePrincipale.top + (this->geometry().height() / 2) - (250 / 2),
                           500,
                           250);
    }
    msgBox.exec();
    if (msgBox.clickedButton() == BoutonContinuer) {
        return MainWindow::ReponseMGBOX::repond_Continuer;
    } else if (msgBox.clickedButton() == BoutonAnnuler) {
        return MainWindow::ReponseMGBOX::repond_Annuler;
    }
    return MainWindow::ReponseMGBOX::repond_Vide;
}

void
MainWindow::MGBoxContinuer(QString Titre, QString Message, QMessageBox::Icon icone, QString InformativeMessage, QString DetailledMessage)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(Titre);
    msgBox.setText(Message);
    msgBox.setInformativeText(InformativeMessage);
    msgBox.setDetailedText(DetailledMessage);
    msgBox.setIcon(icone);
    msgBox.setTextFormat(Qt::RichText);
    msgBox.addButton("Continuer", QMessageBox::AcceptRole);
    /* -- Recherche du bouton 'Show details...' -- */
    if (DetailledMessage != "") {
        foreach (QAbstractButton* button, msgBox.buttons())
            if (msgBox.buttonRole(button) == QMessageBox::ActionRole) { // Le seul ActionRole est le 'Show details...'
                button->click();
                break;
            }
    }
    {
        RECT FenetrePrincipale;
        GetWindowRect((HWND)this->winId(), &FenetrePrincipale);
        QSpacerItem* horizontalSpacer = new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding); // La fenetre fera 500px au minimum
        QGridLayout* layout           = (QGridLayout*)msgBox.layout();
        layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
        msgBox.setGeometry(FenetrePrincipale.left + (this->geometry().width() / 2) - (500 / 2),
                           FenetrePrincipale.top + (this->geometry().height() / 2) - (250 / 2),
                           500,
                           250);
    }
    msgBox.exec();
    return;
}
