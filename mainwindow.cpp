/* -------------------------------------------------------------------------- */
/*      REEMaker 5 -- Grégory WENTZEL (c) 2023   Code sous licence GPL3       */
/* -------------------------------------------------------------------------- */

#include "mainwindow.h"
#include "ui_mainwindow.h"

// clazy:excludeall=qstring-ref

#define POSE_OVERLAY_BLUR                                                                                                                            \
    auto overlay = new OverlayBlurWidget(this, /*radius*/ 4.0, /*downscale*/ 0.5);                                                                   \
    overlay->raise();                                                                                                                                \
    overlay->refreshSnapshot();

#define DEPOSE_OVERLAY_BLUR overlay->deleteLater();

enum class CanWriteFile
{
    Yes_I_Can,
    Erreur_DossierLectureSeul,
    Erreur_FichierLectureSeul,
    Indetermine
};

struct VersionLogiciel
{
    int Majeur = 0;
    int Mineur = 0;
    int Patch = 0;
    int Build = 0;
    QString Version;
};

VersionLogiciel
getNumericVersion(const QString& versionString)
{
    QString numericOnly;
    for (const QChar& character : versionString) {
        if (character.isDigit() || character == '.') {
            numericOnly += character;
        }
    }
    if (numericOnly.isEmpty())
        return VersionLogiciel();
    auto splitty = numericOnly.split('.', Qt::SkipEmptyParts);
    if (splitty.count() != 4)
        return VersionLogiciel();
    VersionLogiciel Reponse;
    Reponse.Majeur = splitty[0].toInt();
    Reponse.Mineur = splitty[1].toInt();
    Reponse.Patch = splitty[2].toInt();
    Reponse.Build = splitty[3].toInt();
    Reponse.Version = numericOnly;
    return Reponse;
}

bool
compareVersions(const VersionLogiciel& a, const VersionLogiciel& b)
{
    if (a.Majeur > b.Majeur) {
        return true;
    }
    if (a.Majeur < b.Majeur) {
        return false;
    }

    if (a.Mineur > b.Mineur) {
        return true;
    }
    if (a.Mineur < b.Mineur) {
        return false;
    }

    if (a.Patch > b.Patch) {
        return true;
    }
    if (a.Patch < b.Patch) {
        return false;
    }

    if (a.Build > b.Build) {
        return true;
    }
    if (a.Build < b.Build) {
        return false;
    }

    // Les versions sont identiques, donc a n'est pas > b
    return false;
}

bool
isDirectoryWritable(const QString& dirPath)
{
    QDir dir(dirPath);
    if (!dir.exists()) {
        return false; // Le répertoire n'existe pas, donc pas d'écriture
                      // possible
    }
    QString tempFileName = dirPath + "/temp_write_test_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    QFile tempFile(tempFileName);
    if (tempFile.open(QIODevice::WriteOnly)) {
        tempFile.close();
        tempFile.remove();
        return true; // L'écriture est possible
    } else {
        return false; // Impossible d'écrire dans le répertoire
    }
}

CanWriteFile
canWriteToFile(const QString& filePath)
{
    // Crée un objet QFile pour le fichier spécifié.
    QFile file(filePath);

    // Teste si le fichier existe déjà.
    if (file.exists()) {
        // Si le fichier existe, vérifie si on a les droits d'écriture pour le
        // remplacer. Note : La méthode `open()` avec `QIODevice::WriteOnly`
        // tentera de supprimer le fichier s'il existe. Si cela échoue, c'est
        // qu'on n'a pas les droits.
        if (file.open(QIODevice::WriteOnly)) {
            file.close();
            return CanWriteFile::Yes_I_Can;
        }
        return CanWriteFile::Erreur_FichierLectureSeul;
    } else {
        // Si le fichier n'existe pas, vérifie si le répertoire parent est
        // accessible en écriture.
        if (isDirectoryWritable(QFileInfo(filePath).absoluteDir().path()))
            return CanWriteFile::Yes_I_Can;
        else
            return CanWriteFile::Erreur_DossierLectureSeul;
    }
    return CanWriteFile::Indetermine;
}

QString
formatFileSize(qint64 sizeInBytes)
{
    // Si la taille est négative, on retourne 0
    if (sizeInBytes < 0) {
        return "0 octets";
    }

    const double KB = 1024.0;
    const double MB = KB * 1024.0;
    const double GB = MB * 1024.0;
    const double TB = GB * 1024.0;
    const double PB = TB * 1024.0;

    if (sizeInBytes < KB) {
        return QString("%1 octets").arg(sizeInBytes);
    } else if (sizeInBytes < MB) {
        return QString("%1 Ko").arg(sizeInBytes / KB, 0, 'f', 2);
    } else if (sizeInBytes < GB) {
        return QString("%1 Mo").arg(sizeInBytes / MB, 0, 'f', 2);
    } else if (sizeInBytes < TB) {
        return QString("%1 Go").arg(sizeInBytes / GB, 0, 'f', 2);
    } else if (sizeInBytes < PB) {
        return QString("%1 To").arg(sizeInBytes / TB, 0, 'f', 2);
    } else {
        return QString("%1 Po").arg(sizeInBytes / PB, 0, 'f', 2);
    }
}

PoDoFo::PdfString
QStringToPdfString(const QString& qstr)
{
    std::string utf8Text = qstr.toUtf8().toStdString();
    return PoDoFo::PdfString(utf8Text);
}

void
centerProgressDialogOnCurrentScreen(QProgressDialog* dialog)
{
    if (!dialog) {
        return;
    }

    // 1. Trouver l'écran où se trouve le curseur
    QPoint cursorPosition = QCursor::pos();
    QScreen* currentScreen = QApplication::screenAt(cursorPosition);

    if (!currentScreen) {
        // Si l'écran n'est pas trouvé (cas rare), on utilise l'écran principal
        currentScreen = QApplication::primaryScreen();
    }

    // 2. Obtenir la géométrie de l'écran (y compris la barre des tâches)
    QRect screenGeometry = currentScreen->geometry();

    // 3. Calculer la position pour centrer le dialogue
    int dialogX = screenGeometry.x() + (screenGeometry.width() - dialog->width()) / 2;
    int dialogY = screenGeometry.y() + (screenGeometry.height() - dialog->height()) / 2;

    // 4. Déplacer le dialogue à la position calculée
    dialog->move(dialogX, dialogY);
}

void
centerProgressDialogOnParent(QProgressDialog* dialog)
{
    if (!dialog || !dialog->parentWidget()) {
        // Si le dialogue ou son parent n'existent pas, on ne fait rien
        return;
    }

    QWidget* parentWidget = dialog->parentWidget();

    // 1. Obtenir la position et la géométrie du widget parent
    QRect parentGeometry = parentWidget->geometry();

    // 2. Calculer la position pour centrer le dialogue par rapport au parent
    int dialogX = parentGeometry.x() + (parentGeometry.width() - dialog->width()) / 2;
    int dialogY = parentGeometry.y() + (parentGeometry.height() - dialog->height()) / 2;

    // 3. Déplacer le dialogue à la position calculée
    dialog->move(dialogX, dialogY);
}

void
centerMessageBoxOnParent(QMessageBox* dialog)
{
    if (!dialog || !dialog->parentWidget()) {
        // Si le dialogue ou son parent n'existent pas, on ne fait rien
        return;
    }

    QWidget* parentWidget = dialog->parentWidget();

    // 1. Obtenir la position et la géométrie du widget parent
    QRect parentGeometry = parentWidget->geometry();

    // 2. Calculer la position pour centrer le dialogue par rapport au parent
    int dialogX = parentGeometry.x() + (parentGeometry.width() - dialog->width()) / 2;
    int dialogY = parentGeometry.y() + (parentGeometry.height() - dialog->height()) / 2;

    // 3. Déplacer le dialogue à la position calculée
    dialog->move(dialogX, dialogY);
}

struct DPIResult
{
    UINT DpiX = 0;
    UINT DpiY = 0;
    double ScaleFactor = 0.0;
};
DPIResult
getDisplayScaleFactor(HWND hwnd)
{
    if (!hwnd) {
        return DPIResult();
    }
    // Obtient le handle du moniteur sur lequel se trouve la fenêtre
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    UINT dpiX = 96;
    UINT dpiY = 96;
    // Récupère le DPI du moniteur
    if (S_OK == GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY)) {
        double scaleFactor = static_cast<double>(dpiX) / 96.0;
        DPIResult _Result;
        _Result.DpiX = dpiX;
        _Result.DpiY = dpiY;
        _Result.ScaleFactor = scaleFactor;
        return _Result;
    } else
        qDebug() << "Erreur : GetDpiForMonitor";
    return DPIResult();
}

QString
getExtendedPath(const QString& path)
{
    if (path.isEmpty()) {
        return path;
    }

    // Le préfixe pour les chemins locaux est "\\?\"
    QString prefix = "\\\\?\\";

    // Pour les chemins UNC standards (\\server\share), le préfixe est
    // "\\?\UNC\"
    if (path.startsWith("\\\\")) {
        // Supprime le double backslash initial et ajoute le préfixe UNC étendu
        return "\\\\?\\UNC\\" + path.mid(2);
    } else {
        // Ajoute le préfixe pour les chemins de disque (C:\...)
        return prefix + path;
    }
}

void
MainWindow::copyFileWithProgress(const QString& sourcePath, const QString& destinationPath, bool AfficheDialogue)
{
    QFile sourceFile(sourcePath);
    if (!sourceFile.exists()) {
        qDebug() << "Erreur : Le fichier source n'existe pas.";
        return;
    }

    QFile destinationFile(destinationPath);
    if (!sourceFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Erreur : Impossible d'ouvrir le fichier source en lecture.";
        return;
    }

    if (!destinationFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qDebug() << "Erreur : Impossible d'ouvrir le fichier de destination en "
                    "écriture.";
        sourceFile.close();
        return;
    }

    qint64 fileSize = sourceFile.size();
    ProgressOverlay* po = nullptr;

    if (AfficheDialogue) {
        po = ProgressOverlay::showDeterminate(this, "Copie en cours...", fileSize, true, true, 140);
        po->enableBackdropBlur(true, 4.0, 0.5);
        po->setText("Copie en cours (" + QString::number(sourceFile.size() / 1024 / 1024) + " Mo)");
        po->setValue(0);
    }

    const qint64 bufferSize = 4096;
    QByteArray buffer;
    qint64 bytesCopied = 0;

    while (!sourceFile.atEnd()) {
        buffer = sourceFile.read(bufferSize);
        destinationFile.write(buffer);
        bytesCopied += buffer.size();
        if (AfficheDialogue)
            po->setValue(bytesCopied);
        QCoreApplication::processEvents();
    }
    sourceFile.close();
    destinationFile.close();
    qDebug() << "Copie terminée avec succès.";
    if (AfficheDialogue) {
        po->setValue(100);
        po->close();
    }
    po = nullptr;
}

#include <iomanip>
#include <random>
#include <sstream>
// Génère une chaîne hexadécimale de 64 bits aléatoire
std::string
generate_random_64bit_hex()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> distrib;

    uint64_t random_value = distrib(gen);

    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << random_value;
    return ss.str();
}

std::string
RandomPrefix(QString base)
{
    std::string prefix = base.toStdString();

    // Concaténer quatre valeurs de 64 bits pour obtenir 256 bits
    std::string random_string = generate_random_64bit_hex();

    return prefix + "_" + random_string;
}

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

    /// Régénaration des fichiers de l'archives
    // auto _FA = FastArchiver();
    // _FA.compressFolderBuffered("C:/GhostScript_10.05.1", "C:/Temp/GhostScript_base.zstd");
    // _FA.compressFolderBuffered("C:/Poppler_25.07.0", "C:/Temp/PdfToPPM_base.zstd");
    // _FA.decompressArchiveBuffered("C:/Temp/GhostScript_base.zstd", "C:/Temp/GhostScript_OUT");
    // _FA.decompressArchiveBuffered("C:/Temp/PdfToPPM_base.zstd", "C:/Temp/Poppler_OUT");
    connect(this, &MainWindow::EmetNotification, this, &MainWindow::RecoisNotification);

    ModeAjoutPDG = false;
    /* -- Lecture donnée utilisateur -- */
    LectureINI();
    /* -- Mise en place des dossiers des programmes et des pages de gardes -- */
    MiseEnPlaceChemin();
    /* -- Mise en place des validators MAJUSCULE -- */
    MiseEnPlaceValidator();
    /* -- Mise en place de l'interface (A besoin de la lecture INI avant!!!) --
     */
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
              auto retour =
                showDialogCustom("REEMaker - Version " + Version + " disponible !",
                                 "<b>Une mise à jour est disponible.<br>Voulez-vous la télécharger et l'installer ?</b><br><i>L'application "
                                 "sera redémarrée pour procéder à la mise à jour</i>",
                                 QMessageBox::Icon::Question,
                                 { "Continuer", "Annuler" });
              if (retour == 0 /*Continuer*/)
                  DemarreLeTelechargement(Chemin);
          },
          Qt::QueuedConnection);
        QThread* thread = QThread::create([&] {
            QThread::msleep(10000);
            auto Reponse = DerniereVersion();
            if (Reponse == "Timeout") {
                emit EmetNotification("REEMaker - Information<\nImpossible d'atteindre le serveur de mise à jour.");
                this->setWindowTitle(this->windowTitle() + " [Erreur à la récupération de la "
                                                           "dernière version disponible]");
                Consigne("Impossible d'atteindre le serveur de mise à jour "
                         "(https://github.com/alwendya/REEMakerQT).");
            } else {
                try {
                    QString cReponse = Reponse;
                    qDebug() << "Version Github :" << Reponse;
                    auto VersionServeurINT = getNumericVersion(Reponse);
                    auto VersionLocal = getNumericVersion(mVersion);
                    bool PlusRecent = compareVersions(VersionServeurINT, VersionLocal);
                    Consigne(QString("Version interne : %1, Version disponible : %2, la "
                                     "mise à jour %3 nécessaire")
                               .arg(VersionLocal.Version, VersionServeurINT.Version, QString(PlusRecent ? "est" : "n'est pas")),
                             false,
                             false,
                             true);
                    if (PlusRecent) {
                        emit EmetNotification("REEMaker - Information\nUne mise à jour est disponible.");
                        Consigne("Une mise à jour [" + Reponse + "] a été trouvée.");
                        emit LanceUneMAJ(Reponse,
                                         QUrl("https://github.com/alwendya/"
                                              "REEMakerQT/releases/download/" +
                                              Reponse + "/REEMaker.Update.7z"));
                    } else {
                        emit EmetNotification("REEMaker - Information\nVous êtes à jour.");
                        // this->setWindowTitle(this->windowTitle() + " [Vous êtes à jour]");
                    }
                } catch (...) {
                    emit EmetNotification("REEMaker - Information\nErreur inconnue à la récupération des données de mises à jour.");
                    Consigne(
                      "Téléchargement de la mise à jour, erreur du traitement de la réponse serveur, retenter la mise à jour ultérieurement...",
                      true,
                      false,
                      true);
                }
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
                    Commentaire = "\n\n" + QString(blob);
                    file.close();
                }
                QFile::remove(QCoreApplication::applicationDirPath() + "/Commentaire de mise à jour.txt");
            }
            showDialogCustom(
              "REEMaker - Information", "<b>Mise à jour effectué avec succès !</b>\n" + Commentaire, QMessageBox::Icon::Information, { "Continuer" });
        }
        thread->start();
    }
    /* -- On fait le lien entre PDGHelper et MainWindow -- */
    QObject::connect(&mPDGHelper, &PDGHelper::EnvoiLogMessage, this, &MainWindow::RecoisLogMessage);

    ui->tvx_img_liste->Accept_Image(true);
    ui->tvx_img_liste->Accept_PDF(false);
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

    POSE_OVERLAY_BLUR;
    PDFOuvert = QFileDialog::getOpenFileName(this, "Ouvrir un fichier PDF", LireDansIni("ChercheProcedure", "").toString(), "Fichier PDF (*.pdf)");
    DEPOSE_OVERLAY_BLUR;
    if (PDFOuvert == "") {
        return;
    }
    EcrireDansIni("ChercheProcedure", QFileInfo(PDFOuvert).dir().path());
    { /// Creation fichier temporaire local pour bypasser le path > 260 caractères
        auto ExtendedSource = getExtendedPath(QDir::toNativeSeparators(PDFOuvert));
        PDFOuvertLOCAL = CheminTemp + "/temp_" + QString::fromStdString(generate_random_64bit_hex()) + ".pdf";
        copyFileWithProgress(ExtendedSource, PDFOuvertLOCAL);
    }
    ui->Folioter_Txt_RechercheProcedure->setText(PDFOuvert);
    ui->Folioter_Label_NombreFolioAnnuler->setText("Aucun folio annulé");
    qint16 NombrePage = PDFInfoNombrePage(PDFOuvertLOCAL);
    Consigne("PDFInfo : Donné récupéré avec " + QString::number(NombrePage) + " pages");
    QCoreApplication::processEvents();
    if (ui->OPT_Check_GhostScript->isChecked()) {
        auto CodeErreur = RepareGhostScript(PDFOuvertLOCAL, NombrePage, false);
        switch (CodeErreur) {
            case ValeurRetour::AucunFichierEntree: {
                POSE_OVERLAY_BLUR;
                ModalConfig cfg;
                cfg.title = "REEMaker - Erreur";
                cfg.message = "<b>Fichier d'entrée introuvable</b>";
                cfg.icon = OverlayBlurWidget::iconFromStandard(QStyle::SP_MessageBoxCritical, this);
                cfg.buttons = { OverlayBlurWidget::makeButton(0, "Abandon", QDialogButtonBox::AcceptRole, true, false) };
                cfg.clickOutsideToClose = false;
                overlay->execModal(cfg);
                DEPOSE_OVERLAY_BLUR;
                Consigne("Erreur : Fichier d'entrée introuvable");
                ui->Folioter_Txt_RechercheProcedure->setText("");
                return;
            }
            case ValeurRetour::ErreurDeplacement: {
                POSE_OVERLAY_BLUR;
                ModalConfig cfg;
                cfg.title = "REEMaker - Erreur";
                cfg.message = "<b>Erreur dans la sauvegarde du document PDF</b>";
                cfg.icon = OverlayBlurWidget::iconFromStandard(QStyle::SP_MessageBoxCritical, this);
                cfg.buttons = { OverlayBlurWidget::makeButton(0, "Abandon", QDialogButtonBox::AcceptRole, true, false) };
                cfg.clickOutsideToClose = false;
                overlay->execModal(cfg);
                Consigne("Erreur dans la sauvegarde du document PDF");
                ui->Folioter_Txt_RechercheProcedure->setText("");
                DEPOSE_OVERLAY_BLUR;
                return;
            }
            case ValeurRetour::GhostScriptAbsent: {
                POSE_OVERLAY_BLUR;
                ModalConfig cfg;
                cfg.title = "REEMaker - Erreur";
                cfg.message = "<b>Erreur dans les fichiers de Ghostscript, réinstaller REEMaker</b>";
                cfg.icon = OverlayBlurWidget::iconFromStandard(QStyle::SP_MessageBoxCritical, this);
                cfg.buttons = { OverlayBlurWidget::makeButton(0, "Abandon", QDialogButtonBox::AcceptRole, true, false) };
                cfg.clickOutsideToClose = false;
                overlay->execModal(cfg);
                Consigne("Erreur dans les fichiers de Ghostscript, réinstaller REEMaker");
                ui->Folioter_Txt_RechercheProcedure->setText("");
                DEPOSE_OVERLAY_BLUR;
                return;
            }
            case ValeurRetour::ErreurDeGhostScript: {
                POSE_OVERLAY_BLUR;
                ModalConfig cfg;
                cfg.title = "REEMaker - Attention";
                cfg.message = "<b>Ghostscript n'a pu réparer le document.\nREEMaker va continuer avec le document PDF original.</b>";
                cfg.icon = OverlayBlurWidget::iconFromStandard(QStyle::SP_MessageBoxWarning, this);
                cfg.buttons = { OverlayBlurWidget::makeButton(0, "Abandon", QDialogButtonBox::AcceptRole, true, false) };
                cfg.clickOutsideToClose = false;
                overlay->execModal(cfg);
                DEPOSE_OVERLAY_BLUR;
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
        Consigne("Ouverture de '" + QFileInfo(PDFOuvertLOCAL).fileName() + "' réussi.");
    }
    try {

        vecMediaBox.clear();
        vecRotation.clear();
        vecFolioAnnuler.clear();
        PoDoFo::PdfMemDocument documentSource;
        documentSource.Load(QStringToPdfString(PDFOuvertLOCAL));
        NombrePages = documentSource.GetPages().GetCount();
        auto* po = ProgressOverlay::showDeterminate(this, tr("Préparation..."), NombrePages, true, true, 140);
        po->enableBackdropBlur(true, 4.0, 0.5);
        po->setValue(0);
        po->setText(QObject::tr("Analyse page %1 / %2...").arg(1).arg(NombrePages));
        for (int i = 0; i < NombrePages; i++) {
            po->setValue(i);
            po->setText(QObject::tr("Analyse page %1 / %2...").arg(i + 1).arg(NombrePages));
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            PoDoFo::PdfPage& pPage = documentSource.GetPages().GetPageAt(i);
            auto pMediaBox = pPage.GetMediaBox();
            auto pRotation = pPage.GetRotation();
            vecMediaBox.push_back(pMediaBox);
            vecRotation.push_back(pRotation);
            vecFolioAnnuler.push_back(false);
        }
        ui->Folioter_Group_RefTampon->setVisible(false);
        ui->Folioter_Group_Etendue->setVisible(false);
        ui->Folioter_Group_Annuler->setVisible(false);
        ui->Folioter_Group_ChoixTranche->setVisible(false);
        ui->Folioter_Btn_EtapeSuivante1->setEnabled(true);
        ui->Folioter_Btn_EtapeSuivante2->setEnabled(false);
        po->close();
    } catch (const PoDoFo::PdfError& e) {
        MiseEnPlaceListInfo();
        vecMediaBox.clear();
        vecRotation.clear();
        vecFolioAnnuler.clear();
        PDFOuvert = "";
        ui->Folioter_Txt_RechercheProcedure->setText("");
        ui->Folioter_Label_NombreFolioAnnuler->setText("Aucun folio annulé");

        if (e.GetCode() == PoDoFo::PdfErrorCode::FileNotFound) {
            ui->FolioListeINFO->addItem("Information sur le fichier PDF :");
            ui->FolioListeINFO->addItem("Erreur : Le fichier est introuvable");
            Consigne("Erreur : Le fichier est introuvable");
        } else if (e.GetCode() == PoDoFo::PdfErrorCode::BrokenFile) {
            ui->FolioListeINFO->addItem("Information sur le fichier PDF :");
            ui->FolioListeINFO->addItem("Erreur : Le fichier est endommagé, "
                                        "essayez d'activer le contrôle "
                                        "par GhostScript dans les options");
            Consigne("Erreur : Le fichier est endommagé, essayez d'activer le "
                     "contrôle par GhostScript dans les options");
        } else if (e.GetCode() == PoDoFo::PdfErrorCode::InvalidPDF) {
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
    qint64 PageFin = NombrePages;
    if (ui->Folioter_Radio_FolioPartiel->isChecked()) {
        PageDebut = ui->Folioter_Spin_Partiel_Debut->value();
        PageFin = ui->Folioter_Spin_Partiel_Fin->value();
    }
    auto procPPM = new QProcess();
    QString PathOutputImage = QFileInfo(CheminTemp).filePath() + "/_64_";
    procPPM->setWorkingDirectory(QFileInfo(CheminPoppler).path());
    procPPM->setProgram(CheminPoppler);
    procPPM->setArguments({ "-f",
                            QString::number(PageDebut),
                            "-l",
                            QString::number(PageFin),
                            "-q",
                            "-r",
                            "10",
                            "-scale-to",
                            "64",
                            "-png",
                            PDFOuvertLOCAL,
                            PathOutputImage });

    auto* po = ProgressOverlay::showDeterminate(this,
                                                "Génération des miniatures...",
                                                PageFin - PageDebut + 1,
                                                /*center*/ true,
                                                /*blockInput*/ true,
                                                /*fadeMs*/ 140);
    po->enableBackdropBlur(true, 4.0, 0.5);
    po->setValue(0);
    po->setText(QString("Miniatures %1 / %2...").arg(0).arg(PageFin - PageDebut + 1));
    QCoreApplication::processEvents();

    procPPM->start();
    while (procPPM->state() != QProcess::NotRunning) {
        QThread::msleep(15);
        QDir directory(QFileInfo(CheminTemp).filePath());
        QStringList images = directory.entryList(QStringList() << "_64_*.png", QDir::Files, QDir::Name | QDir::IgnoreCase);
        po->setValue(images.count());
        po->setText(QString("Miniatures %1 / %2...").arg(images.count()).arg(PageFin - PageDebut + 1));
        QCoreApplication::processEvents();
        // if (progress.wasCanceled()) {
        //     procPPM->kill();
        //     procPPM->waitForFinished(-1);
        //     images = directory.entryList(QStringList() << "_64_*.png", QDir::Files, QDir::Name | QDir::IgnoreCase);
        //     foreach (QString filename, images) {
        //         directory.remove(filename);
        //     }
        //     ui->tabWidget->setTabVisible(0, true);
        //     ui->tabWidget->setTabVisible(1, false);
        //     ui->tabWidget->setTabVisible(2, true);
        //     ui->tabWidget->setTabVisible(3, true);
        //     ui->tabWidget->setTabVisible(5, true);
        //     ui->tabWidget->setCurrentIndex(0);
        //     progress.close();
        //     DEPOSE_OVERLAY_BLUR;
        //     return;
        // }
    }
    procPPM->close();
    po->close();
    Consigne("Fin de génération des miniatures par Poppler");
    QDir directory(QFileInfo(CheminTemp).filePath());
    QStringList images = directory.entryList(QStringList() << "_64_*.png", QDir::Files, QDir::Name | QDir::IgnoreCase);
    vecFolioAnnuler.clear();
    vecFolioAnnuler.resize(vecMediaBox.count()); // On remet tout à false
    ui->Annulation_Liste->setRowCount(images.count());
    for (int var = 0; var < images.count(); ++var) {
        images[var] = QFileInfo(CheminTemp).filePath() + "/" + images[var];
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
    int NumeroPage = ui->Annulation_Liste->item(row, /*column*/ 1)->text().mid(5).toInt();
    auto procPPM = new QProcess();
    QString PathOutputImage = QFileInfo(CheminTemp).filePath() + "/_HR_";
    procPPM->setWorkingDirectory(QFileInfo(CheminPoppler).path());
    procPPM->setProgram(CheminPoppler); // QDir::toNativeSeparators
    procPPM->setArguments(
      { "-f", QString::number(NumeroPage), "-l", QString::number(NumeroPage), "-q", "-r", "100", "-png", PDFOuvertLOCAL, PathOutputImage });
    QProgressDialog progress("Préparation de l'image...", "Annuler", 0, 0, this);
    progress.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMaximum(0);
    progress.setAutoClose(false);
    progress.setCancelButton(0);
    progress.show();
    progress.setWindowIcon(this->windowIcon());
    progress.installEventFilter(keyPressEater);
    QCoreApplication::processEvents();
    centerProgressDialogOnParent(&progress);
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
    {
        POSE_OVERLAY_BLUR;
        while (1) {
            PDFASauver = QFileDialog::getSaveFileName(this, "Enregistrer le fichier PDF", PropositionNomFichier, "Fichier PDF (*.pdf)");
            if (PDFASauver == "") {
                DEPOSE_OVERLAY_BLUR;
                return;
            }
            if (isDirectoryWritable(QFileInfo(PDFASauver).path()))
                break;
            else {
                // Message erreur
                ModalConfig cfg;
                cfg.title = "REEMaker - Erreur";
                cfg.message = "Il est <b>impossible d'écrire</b> dans le dossier que vous avez "
                              "sélectionné.<br>Merci de sélectionner un autre dossier ou "
                              "abandonner l'opération.";
                cfg.icon = OverlayBlurWidget::iconFromStandard(QStyle::SP_MessageBoxCritical, this);
                cfg.buttons = { OverlayBlurWidget::makeButton(1, "Abandonner", QDialogButtonBox::RejectRole, false, false),
                                OverlayBlurWidget::makeButton(0, "Sélectionner un autre emplacement", QDialogButtonBox::AcceptRole, true, true) };
                cfg.clickOutsideToClose = false;
                cfg.escapeButtonId = -1;
                int ret = overlay->execModal(cfg);
                if (ret == 1 /*Abandonner*/) {
                    DEPOSE_OVERLAY_BLUR;
                    return;
                }
                continue;
            }
        }
        DEPOSE_OVERLAY_BLUR;
    }
    qint64 mStarting = 0;
    qint64 mEnding = vecMediaBox.size();
    if (ui->Folioter_Radio_FolioPartiel->isChecked()) // Partiel
    {
        mStarting = ui->Folioter_Spin_Partiel_Debut->value() - 1;
        mEnding = ui->Folioter_Spin_Partiel_Fin->value();
    }
    auto* po = ProgressOverlay::showDeterminate(this, "Création des tampons ...", mEnding - mStarting, true, true, 140);
    po->enableBackdropBlur(true, 4.0, 0.5);
    qint64 iTranche = 0;
    qint64 iPage = 0;
    QMutex progressLock;
    QThread* thread = QThread::create([&] {
        bool Tampon_classique = ui->OPT_Tampon_classique->isChecked();
        double TamponLargeur = Tampon_classique ? 150.0 : 358.0;
        double TamponHauteur = Tampon_classique ? 60.0 : 16.0;
        constexpr double TamponMargH = 7.0;
        constexpr double TamponMargL = 4.0;
        constexpr double TamponH1 = 20.0;
        constexpr double TamponH2 = 40.0;
        constexpr double TamponS1 = 100.0;
        constexpr double TamponS2 = 60.0;
        constexpr double TamponEpaisseur = 1.0;
        constexpr double TamponPolice = 8.0;
        constexpr double TamponCompactBarre1 = 110.0;
        constexpr double TamponCompactBarre2 = 134.0;
        constexpr double TamponCompactBarre3 = 214.0;
        constexpr double TamponCompactBarre4 = 253.0;
        constexpr double TamponCompactBarre5 = 305.0;
        // PdfError exPDFError;

        Consigne("Démarrage de la génération des tampons...");
        for (int t = 0; t < 10; ++t) {
            progressLock.lock();
            iTranche = t;
            progressLock.unlock();
            // On test si la tranche est cochée, sinon on continue à la
            // prochaine
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
                PoDoFo::PdfMemDocument document;
                document.Load(PDFOuvertLOCAL.toUtf8().constData());

                PoDoFo::PdfFontCreateParams paramsNoSubset;
                paramsNoSubset.Flags = PdfFontCreateFlags::None;

                PoDoFo::PdfFont& pFont =
                  document.GetFonts().GetOrCreateFont(QDir::toNativeSeparators(TempTamponRoboto).toStdString().c_str(), paramsNoSubset);

                PoDoFo::PdfColor couleur = PoDoFo::PdfColor(CouleurTranche[t].redF(), CouleurTranche[t].greenF(), CouleurTranche[t].blueF());
                PoDoFo::PdfColor couleurBlanc = PoDoFo::PdfColor(1.0, 1.0, 1.0);
                PoDoFo::PdfColor couleurAcc =
                  PoDoFo::PdfColor(CouleurAccentuation[t].redF(), CouleurAccentuation[t].greenF(), CouleurAccentuation[t].blueF());
                qDebug() << "couleur de base:" << couleur.GetRed() << couleur.GetGreen() << couleur.GetBlue();
                qDebug() << "couleur d'accentuation :" << couleurAcc.GetRed() << couleurAcc.GetGreen() << couleurAcc.GetBlue();
                /* -- Début de la génération pour chaque pages -- */
                for (qint64 i = mStarting; i < mEnding; i++) {
                    progressLock.lock();
                    iPage = i;
                    progressLock.unlock();
                    PoDoFo::PdfPage& pPage = document.GetPages().GetPageAt(i);
                    /*
                     * Bien respecté l'ordre des déclaration afin de ne pas
                     * avoir à utiliser les matrices de rotations
                     */

                    /*
                     * -- Ajout mention folio annulé avec texte défini par
                     * utilisateur --
                     */
                    {
                        if (vecFolioAnnuler[i]) {
                            const double DualSpace = /*12*/ 60.0;
                            auto PageWidth = vecMediaBox[i].Width - 2 * DualSpace;
                            auto PageHeight = vecMediaBox[i].Height - 2 * DualSpace;
                            PoDoFo::PdfPainter painter;
                            PoDoFo::Rect rect(0, 0, 0, 0);
                            // Quick ajuste du sens largeur / hauteur
                            if (vecRotation[i] == 90 || vecRotation[i] == 270) {
                                rect.Width = PageHeight;
                                rect.Height = PageWidth;
                            } else {
                                rect.Width = PageWidth;
                                rect.Height = PageHeight;
                            }
                            auto xObj = document.CreateXObjectForm(rect);
                            painter.SetCanvas(*xObj);
                            painter.Save();
                            /// DESSIN sans rotation
                            {
                                painter.SetStrokeStyle(PdfStrokeStyle::Solid, false, 2.0);

                                painter.TextState.SetFont(pFont, 70.0);
                                PoDoFo::PdfString TexteFolioAnnulee(QStringToPdfString(ui->OPT_Text_FolioAnnule->text()));
                                painter.GraphicsState.SetStrokingColor(couleur);
                                painter.GraphicsState.SetNonStrokingColor(couleur);
                                painter.DrawLine(2.0, 2.0, 2.0, rect.Height);
                                painter.DrawLine(DualSpace / 2.0, rect.Height, rect.Width, 0.0);

                                painter.DrawTextMultiLine(
                                  TexteFolioAnnulee,
                                  0.0,
                                  0.0,
                                  rect.Width,
                                  rect.Height,
                                  { PdfDrawTextStyle::Regular, PdfHorizontalAlignment::Center, PdfVerticalAlignment::Center });
                            }
                            painter.Restore();
                            painter.FinishDrawing();
                            /// Positionnement
                            {
                                rect.X = DualSpace;
                                rect.Y = DualSpace;
                                rect.Width = PageWidth;
                                rect.Height = PageHeight;
                            }
                            PdfAnnotation& annotation = pPage.GetAnnotations().CreateAnnot(PdfAnnotationType::Stamp, rect);
                            PdfDictionary apDict;
                            apDict.AddKey(PdfName("N"), xObj->GetObject().GetIndirectReference());
                            annotation.GetDictionary().AddKey(PdfName("AP"),
                                                              apDict); // /AP/N = apparence normal
                            annotation.SetFlags(PdfAnnotationFlags::Print);
                            annotation.SetTitle(QStringToPdfString("Cancel_p" + QString::number(i + 1)));
                        }
                    }
                    /*
                     * -- Fin ajout mention folio annulé avec texte défini par
                     * utilisateur
                     */

                    /*
                     * -- Début de la génération des tampons --
                     */
                    {
                        PoDoFo::PdfPainter painter;
                        PoDoFo::Rect rect(0, 0, TamponLargeur, TamponHauteur);
                        auto xObj = document.CreateXObjectForm(rect);
                        painter.SetCanvas(*xObj);
                        painter.Save();
                        { /// Dessine normalement
                            painter.SetStrokeStyle(PdfStrokeStyle::Solid, false, TamponEpaisseur);
                            // Ligne et texte
                            painter.GraphicsState.SetStrokingColor(couleur);
                            // Fond du tampon
                            painter.GraphicsState.SetNonStrokingColor(couleurBlanc);

                            painter.DrawRectangle(TamponEpaisseur / 2,
                                                  TamponEpaisseur / 2,
                                                  TamponLargeur - TamponEpaisseur,
                                                  TamponHauteur - TamponEpaisseur,
                                                  PdfPathDrawMode::StrokeFill);
                            PoDoFo::PdfString utf8SiteDe(QStringToPdfString("Site de " + ui->Folioter_Txt_NomDuSite->text()));
                            PoDoFo::PdfString utf8Tranche(QStringToPdfString("Tr. " + QString::number(t)));
                            PoDoFo::PdfString utf8REE(QStringToPdfString(ui->Folioter_Txt_RefREE->text()));
                            PoDoFo::PdfString utf8Indice(QStringToPdfString("Ind. " + ui->Folioter_Txt_IndiceREE->text()));
                            PoDoFo::PdfString utf8Folio(QStringToPdfString(
                              "Folio " +
                              QString::number(ui->Folioter_Spin_PremierePage->value() + i -
                                              (ui->Folioter_Radio_FolioPartiel->isChecked() ? ui->Folioter_Spin_Partiel_Debut->value() - 1 : 0))));
                            PoDoFo::PdfString utf8Cycle(QStringToPdfString("Cycle " + CodeTranche[t]));
                            painter.TextState.SetFont(pFont, TamponPolice);
                            if (Tampon_classique) {
                                /* -- Tampon classique -- */
                                /* -- Les lignes internes -- */
                                painter.DrawLine(0.0, TamponH1, TamponLargeur, TamponH1);
                                painter.DrawLine(0.0, TamponH2, TamponLargeur, TamponH2);
                                painter.DrawLine(TamponS1, TamponH1, TamponS1, TamponH2);
                                painter.DrawLine(TamponS2, 0.0, TamponS2, TamponH1);

                                painter.GraphicsState.SetStrokingColor(couleur);
                                // Nonstroking = couleur du texte
                                painter.GraphicsState.SetNonStrokingColor(couleur);

                                painter.DrawTextAligned(
                                  utf8Folio, TamponMargL, TamponMargH, TamponS2 - 2 * TamponMargL, PdfHorizontalAlignment::Left);

                                painter.DrawTextAligned(utf8Cycle,
                                                        TamponS2 + TamponMargL,
                                                        TamponMargH,
                                                        (TamponLargeur - TamponS2) - 2 * TamponMargL,
                                                        PdfHorizontalAlignment::Left);
                                painter.DrawTextAligned(
                                  utf8REE, TamponMargL, TamponH1 + TamponMargH, TamponS1 - 2 * TamponMargL, PdfHorizontalAlignment::Left);
                                painter.DrawTextAligned(utf8Indice,
                                                        TamponS1 + TamponMargL,
                                                        TamponH1 + TamponMargH,
                                                        (TamponLargeur - TamponS1) - 2 * TamponMargL,
                                                        PdfHorizontalAlignment::Left);
                                painter.DrawTextAligned(
                                  utf8SiteDe, TamponMargL, TamponH2 + TamponMargH, TamponLargeur - 2 * TamponMargL, PdfHorizontalAlignment::Left);
                                painter.DrawTextAligned(
                                  utf8Tranche, TamponMargL, TamponH2 + TamponMargH, TamponLargeur - 2 * TamponMargL, PdfHorizontalAlignment::Right);
                            } else {
                                /* -- Tampon compact -- */
                                /* -- Dessin des lignes -- */
                                painter.DrawLine(TamponCompactBarre1, 0.0, TamponCompactBarre1, TamponHauteur);
                                painter.DrawLine(TamponCompactBarre2, 0.0, TamponCompactBarre2, TamponHauteur);
                                painter.DrawLine(TamponCompactBarre3, 0.0, TamponCompactBarre3, TamponHauteur);
                                painter.DrawLine(TamponCompactBarre4, 0.0, TamponCompactBarre4, TamponHauteur);
                                painter.DrawLine(TamponCompactBarre5, 0.0, TamponCompactBarre5, TamponHauteur);

                                painter.GraphicsState.SetStrokingColor(couleur);
                                // Nonstroking = couleur du texte
                                painter.GraphicsState.SetNonStrokingColor(couleur);

                                painter.DrawTextAligned(utf8SiteDe, 2.0, 5.0, TamponCompactBarre1 - 4.0, PdfHorizontalAlignment::Left);
                                painter.DrawTextAligned(utf8Tranche,
                                                        TamponCompactBarre1 + 2.0,
                                                        5.0,
                                                        (TamponCompactBarre2 - TamponCompactBarre1) - 4.0,
                                                        PdfHorizontalAlignment::Left);
                                painter.DrawTextAligned(utf8REE,
                                                        TamponCompactBarre2 + 2.0,
                                                        5.0,
                                                        (TamponCompactBarre3 - TamponCompactBarre2) - 4.0,
                                                        PdfHorizontalAlignment::Left);
                                painter.DrawTextAligned(utf8Indice,
                                                        TamponCompactBarre3 + 2.0,
                                                        5.0,
                                                        (TamponCompactBarre4 - TamponCompactBarre3) - 4.0,
                                                        PdfHorizontalAlignment::Left);
                                painter.DrawTextAligned(utf8Cycle,
                                                        TamponCompactBarre4 + 2.0,
                                                        5.0,
                                                        (TamponCompactBarre5 - TamponCompactBarre4) - 4.0,
                                                        PdfHorizontalAlignment::Left);
                                painter.DrawTextAligned(utf8Folio,
                                                        TamponCompactBarre5 + 2.0,
                                                        5.0,
                                                        (TamponLargeur - TamponCompactBarre5) - 4.0,
                                                        PdfHorizontalAlignment::Left);
                            }
                        }
                        painter.Restore();
                        painter.FinishDrawing();

                        { /// Positionnement
                            // Quick ajuste du sens largeur / hauteur
                            if (vecRotation[i] == 90 || vecRotation[i] == 270) {
                                rect.Width = TamponHauteur;
                                rect.Height = TamponLargeur;
                            } else {
                                rect.Width = TamponLargeur;
                                rect.Height = TamponHauteur;
                            }

                            if (ui->OPT_Radio_HG->isChecked()) { // Haut Gauche
                                if (vecRotation[i] == 0) {
                                    rect.X = 0.0 + (double)ui->OPT_Spin_Largeur->value();
                                    rect.Y = vecMediaBox[i].Height - rect.Height - (double)ui->OPT_Spin_Hauteur->value();
                                } else if (vecRotation[i] == 90) {
                                    rect.X = vecMediaBox[i].Width - rect.Width - (double)ui->OPT_Spin_Hauteur->value();
                                    rect.Y = vecMediaBox[i].Height - rect.Height - (double)ui->OPT_Spin_Largeur->value();
                                } else if (vecRotation[i] == 180) {
                                    rect.X = vecMediaBox[i].Width - rect.Width - (double)ui->OPT_Spin_Largeur->value();
                                    rect.Y = 0.0 + (double)ui->OPT_Spin_Hauteur->value();
                                } else if (vecRotation[i] == 270) {
                                    rect.X = 0.0 + (double)ui->OPT_Spin_Largeur->value();
                                    rect.Y = (double)ui->OPT_Spin_Hauteur->value();
                                }
                            } else if (ui->OPT_Radio_HD->isChecked()) { // Haut Droite
                                if (vecRotation[i] == 0) {
                                    rect.X = (vecMediaBox[i].Width - rect.Width - (double)ui->OPT_Spin_Largeur->value());
                                    rect.Y = (vecMediaBox[i].Height - rect.Height - (double)ui->OPT_Spin_Hauteur->value());
                                } else if (vecRotation[i] == 90) {
                                    rect.X = (vecMediaBox[i].Width - rect.Width - (double)ui->OPT_Spin_Hauteur->value());
                                    rect.Y = (0.0 + (double)ui->OPT_Spin_Largeur->value());
                                } else if (vecRotation[i] == 180) {
                                    rect.X = (0.0 + (double)ui->OPT_Spin_Largeur->value());
                                    rect.Y = (0.0 + (double)ui->OPT_Spin_Hauteur->value());
                                } else if (vecRotation[i] == 270) {
                                    rect.X = (0.0 + (double)ui->OPT_Spin_Hauteur->value());
                                    rect.Y = (vecMediaBox[i].Height - rect.Height - (double)ui->OPT_Spin_Largeur->value());
                                }
                            } else if (ui->OPT_Radio_BG->isChecked()) { // Bas Gauche
                                if (vecRotation[i] == 0) {
                                    rect.X = (0.0 + (double)ui->OPT_Spin_Largeur->value());
                                    rect.Y = (0.0 + (double)ui->OPT_Spin_Hauteur->value());
                                } else if (vecRotation[i] == 90) { // A Voir
                                    rect.X = (0.0 + (double)ui->OPT_Spin_Hauteur->value());
                                    rect.Y = (vecMediaBox[i].Height - rect.Height - (double)ui->OPT_Spin_Largeur->value());
                                } else if (vecRotation[i] == 180) { // A Voir
                                    rect.X = (vecMediaBox[i].Width - rect.Width - (double)ui->OPT_Spin_Largeur->value());
                                    rect.Y = (vecMediaBox[i].Height - rect.Height - (double)ui->OPT_Spin_Hauteur->value());
                                } else if (vecRotation[i] == 270) { // A voir
                                    rect.X = (vecMediaBox[i].Width - rect.Width - (double)ui->OPT_Spin_Hauteur->value());
                                    rect.Y = (0.0 + (double)ui->OPT_Spin_Largeur->value());
                                }
                            } else if (ui->OPT_Radio_BD->isChecked()) { // Bas Droite
                                if (vecRotation[i] == 0) {
                                    rect.X = (vecMediaBox[i].Width - rect.Width - (double)ui->OPT_Spin_Largeur->value());
                                    rect.Y = (0.0 + (double)ui->OPT_Spin_Hauteur->value());
                                } else if (vecRotation[i] == 90) {
                                    rect.X = (0.0 + (double)ui->OPT_Spin_Hauteur->value());
                                    rect.Y = (0.0 + (double)ui->OPT_Spin_Largeur->value());
                                } else if (vecRotation[i] == 180) {
                                    rect.X = (0.0 + (double)ui->OPT_Spin_Largeur->value());
                                    rect.Y = (vecMediaBox[i].Height - rect.Height - (double)ui->OPT_Spin_Hauteur->value());
                                } else if (vecRotation[i] == 270) {
                                    rect.X = (vecMediaBox[i].Width - rect.Width - (double)ui->OPT_Spin_Hauteur->value());
                                    rect.Y = (vecMediaBox[i].Height - rect.Height - (double)ui->OPT_Spin_Largeur->value());
                                }
                            }
                        }

                        PdfAnnotation& annotation = pPage.GetAnnotations().CreateAnnot(PdfAnnotationType::Stamp, rect);
                        PdfDictionary apDict;
                        apDict.AddKey(PdfName("N"), xObj->GetObject().GetIndirectReference());
                        annotation.GetDictionary().AddKey(PdfName("AP"), apDict); // /AP/N = apparence normal
                        annotation.SetFlags(PdfAnnotationFlags::Print);
                        annotation.SetTitle(QStringToPdfString("Tampon_p" + QString::number(i + 1)));
                    } /* -- Fin de la génération du tampons -- */
                } /* -- Fin du traitement de la page i -- */
                /* -- Fin du for i = 0 to Nb Pages -- */

                /* -- Foliotage partiel -> Suppression des pages inutiles -- */
                if (ui->Folioter_Radio_FolioPartiel->isChecked()) {
                    qint64 nbPageRemoveFromStart = mStarting;
                    qint64 nbPageRemoveFromEnd = vecMediaBox.size() - mEnding;
                    for (qint64 iA = 0; iA < nbPageRemoveFromStart; iA++) // Suppression du début
                        document.GetPages().RemovePageAt(0);
                    for (qint64 iA = 0; iA < nbPageRemoveFromEnd; iA++) // Suppression de la fin
                        document.GetPages().RemovePageAt(document.GetPages().GetCount() - 1);
                }
                if (ModeAjoutPDG) {
                    uint64_t PosInsertion = 0;
                    // Setup des textes renseignes et case cochés
                    {
                        { // ENUMERATION
                            mPDGHelper.vecVARIABLE.clear();
                            for (int i = 1; i < ui->PDG_ListeWidget->count(); ++i) {
                                blocQuestion* mBlocquestion =
                                  qobject_cast<blocQuestion*>(ui->PDG_ListeWidget->itemWidget(ui->PDG_ListeWidget->item(i)));
                                auto reponse = mBlocquestion->RetourneDonnee();
                                QString ReponseFormat = reponse->Reponse;
                                ReponseFormat = QString(ReponseFormat).replace("{RetourLigne}", "\n");
                                if (reponse->TypeDeBloc == blocQuestion::TypeBloc::Bloc_Texte_Simple)
                                    mPDGHelper.ListeQuestion[reponse->IndexControle].DefautQuestion = ReponseFormat;
                                if (reponse->TypeDeBloc == blocQuestion::TypeBloc::Bloc_Texte_Multiligne)
                                    mPDGHelper.ListeQuestion[reponse->IndexControle].DefautQuestion = ReponseFormat;
                                if (reponse->TypeDeBloc == blocQuestion::TypeBloc::Bloc_Case_Coche)
                                    mPDGHelper.ListeQuestion[reponse->IndexControle].CheckboxValue = reponse->EtatCoche;
                                if (reponse->NomVariable != "") {
                                    PDGHelper::stockVariable lVAR;
                                    lVAR.Variable = reponse->NomVariable;
                                    lVAR.Valeur = ReponseFormat;
                                    mPDGHelper.vecVARIABLE.append(lVAR);
                                }
                            }
                        }
                        mPDGHelper.ArrayFromREEMAKER.ReferenceSite = ui->Folioter_Txt_NomDuSite->text();
                        mPDGHelper.ArrayFromREEMAKER.NumeroTranche = QString::number(t);
                        mPDGHelper.ArrayFromREEMAKER.ReferenceREE = ui->Folioter_Txt_RefREE->text();
                        mPDGHelper.ArrayFromREEMAKER.IndiceREE = ui->Folioter_Txt_IndiceREE->text();
                    }
                    { // Setup des couleurs
                        mPDGHelper.ArrayFromREEMAKER.REErouge = CouleurTranche[t].red();
                        mPDGHelper.ArrayFromREEMAKER.REEvert = CouleurTranche[t].green();
                        mPDGHelper.ArrayFromREEMAKER.REEbleu = CouleurTranche[t].blue();
                        mPDGHelper.ArrayFromREEMAKER.REErougeAccent = CouleurAccentuation[t].red();
                        mPDGHelper.ArrayFromREEMAKER.REEvertAccent = CouleurAccentuation[t].green();
                        mPDGHelper.ArrayFromREEMAKER.REEbleuAccent = CouleurAccentuation[t].blue();
                    }
                    PoDoFo::PdfPage& pPage = document.GetPages().CreatePageAt(PosInsertion, PoDoFo::Rect(0.0, 0.0, 595.0, 842.0));
                    PoDoFo::PdfPainter painter;
                    painter.SetCanvas(pPage);
                    mPDGHelper.DrawOnPage_v2(painter, document);
                    painter.FinishDrawing();
                    Consigne("Génération de la page de garde pour la tranche n°" + QString::number(t));
                }

                /// Creation d'un fichier temporaire en local pour bypasser le
                /// path > 260 caractères
                auto ExtendedDest = getExtendedPath(QDir::toNativeSeparators(PDFSortieTranche));
                QString tempOut = CheminTemp + "/temp_" + QString::fromStdString(generate_random_64bit_hex()) + ".pdf";
                document.Save(tempOut.toUtf8().constData(), PdfSaveOptions::None);
                copyFileWithProgress(tempOut, ExtendedDest, false);
                QFile::remove(tempOut);

                if (ui->OPT_Check_OuvrirApres->isChecked())
                    QDesktopServices::openUrl(QUrl::fromLocalFile(PDFSortieTranche));
            } catch (const PoDoFo::PdfError& e) {
                Consigne("Erreur à la génération du document : " + QString(e.what()) + " tranche n°" + QString::number(t) + "@" + QString(__FILE__) +
                         " line:" + QString::number(__LINE__));
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
        qint64 cpIPAGE = iPage;
        progressLock.unlock();
        QString NewText = "Tranche " + QString::number(cpITRANCHE) + " : Création des tampons...";
        po->setValue(cpIPAGE - mStarting);
        po->setText("Tranche " + QString::number(cpITRANCHE) + " : Création des tampons...");
        QCoreApplication::processEvents();
    }
    po->close();
    if (ModeAjoutPDG) {
        ui->PDG_ListeWidget->clear();
        ui->PDG_Texte_PDGEnCours->setText("");
    }
    ModeAjoutPDG = false;
    Consigne("Fin de la génération des pages de gardes");
    {
        POSE_OVERLAY_BLUR;
        ModalConfig cfg;
        cfg.title = "REEMaker - Information";
        cfg.message = "<b>Fin de la génération des pages de gardes</b>";
        cfg.icon = OverlayBlurWidget::iconFromStandard(QStyle::SP_MessageBoxInformation, this);
        cfg.buttons = { OverlayBlurWidget::makeButton(0, tr("Continuer"), QDialogButtonBox::AcceptRole, true, true) };
        cfg.clickOutsideToClose = false;
        cfg.escapeButtonId = -1;
        overlay->execModal(cfg);
        DEPOSE_OVERLAY_BLUR;
    }
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
    {
        POSE_OVERLAY_BLUR;
        if (ui->PDG_ListeWidget->count() == 0) {
            ModalConfig cfg;
            cfg.title = "REEMaker - Attention";
            cfg.message = "La liste est vide ou aucune page de garde n'a été "
                          "chargée.<br>L'opération sera annulée.";
            cfg.icon = OverlayBlurWidget::iconFromStandard(QStyle::SP_MessageBoxWarning, this);
            cfg.buttons = { OverlayBlurWidget::makeButton(0, tr("Annuler"), QDialogButtonBox::AcceptRole, true, true) };
            cfg.clickOutsideToClose = false;
            cfg.escapeButtonId = -1; // ou ne pas le renseigner du tout

            overlay->execModal(cfg);
            DEPOSE_OVERLAY_BLUR;
            return;
        }
        if (ui->PDG_ListeWidget->count() > 0) {
            int NombreNonRemplis = 0;
            int IndexPremierNonRemplis = -1;
            for (int i = 1; i < ui->PDG_ListeWidget->count(); ++i) {
                blocQuestion* mBlocquestion = qobject_cast<blocQuestion*>(ui->PDG_ListeWidget->itemWidget(ui->PDG_ListeWidget->item(i)));
                auto reponse = mBlocquestion->RetourneDonnee();
                if (reponse->EstObligatoire && (reponse->TypeDeBloc == blocQuestion::Bloc_Texte_Simple) && (reponse->Reponse == ""))
                    NombreNonRemplis++;
                if (reponse->EstObligatoire && (reponse->TypeDeBloc == blocQuestion::Bloc_Texte_Multiligne) && (reponse->Reponse == ""))
                    NombreNonRemplis++;
                if ((NombreNonRemplis == 1) && (IndexPremierNonRemplis == -1))
                    IndexPremierNonRemplis = i;
            }
            if (NombreNonRemplis > 0) {
                ModalConfig cfg;
                cfg.title = "REEMaker - Attention";
                cfg.message = "Tout les champs obligatoires ne sont pas renseignés.<br>Nombre de champs à renseigner pour continuer : " +
                              QString::number(NombreNonRemplis);
                cfg.icon = OverlayBlurWidget::iconFromStandard(QStyle::SP_MessageBoxWarning, this);
                cfg.buttons = { OverlayBlurWidget::makeButton(0, tr("Annuler"), QDialogButtonBox::AcceptRole, true, true) };
                cfg.clickOutsideToClose = false;
                cfg.escapeButtonId = -1; // ou ne pas le renseigner du tout

                overlay->execModal(cfg);
                ui->PDG_ListeWidget->setCurrentRow(IndexPremierNonRemplis); //+1 car header = 0
                DEPOSE_OVERLAY_BLUR;
                return;
            }
        }
        DEPOSE_OVERLAY_BLUR;
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

    SiteInfoData out, defaults;
    defaults.nomSite = (PDGManuelNomSite == "") ? ui->Folioter_Txt_NomDuSite->text() : PDGManuelNomSite;
    defaults.nomREE = (PDGManuelRefREE == "") ? ui->Folioter_Txt_RefREE->text() : PDGManuelRefREE;
    defaults.indiceREE = (PDGManuelIndice == "") ? ui->Folioter_Txt_IndiceREE->text() : PDGManuelIndice;

    QString TranchePrecedente = LireDansIni("TranchePrecedentePicker", "0000000000").toString();
    for (int i = 0; i < 10; ++i) {
        defaults.tranches[i] = (TranchePrecedente.at(i) == '1') ? 1 : 0;
    }
    bool UneTrancheCoche = false;
    int ok = execSiteInfoModal(this, out, &defaults);
    if (ok == 1) {
        TranchePrecedente = "";
        for (int var = 0; var < out.tranches.size(); ++var) {
            TranchePrecedente += out.tranches.at(var) ? "1" : "0";
            if (out.tranches.at(var)) {
                UneTrancheCoche = true;
            }
        }
    } else {
        return;
    }
    EcrireDansIni("TranchePrecedentePicker", TranchePrecedente);

    if (UneTrancheCoche) {
        QString PDFASauverPDG = "";
        POSE_OVERLAY_BLUR;
        QApplication::processEvents();
        while (1) {
            PDFASauverPDG = QFileDialog::getSaveFileName(
              this, "Enregistrer la page de garde sous", LireDansIni("ExportPageDeGardePDF", "").toString(), "Fichier PDF (*.pdf)");
            if (PDFASauverPDG == "") {
                DEPOSE_OVERLAY_BLUR;
                return;
            }
            if (isDirectoryWritable(QFileInfo(PDFASauverPDG).path()))
                break;
            else {
                // Message erreur
                ModalConfig cfg;
                cfg.title = "REEMaker - Erreur";
                cfg.message = "Il est <b>impossible d'écrire</b> dans le dossier que vous avez sélectionné.<br>Merci de sélectionner un autre "
                              "dossier ou abandonner l'opération.";
                cfg.icon = OverlayBlurWidget::iconFromStandard(QStyle::SP_MessageBoxCritical, this);
                cfg.buttons = { OverlayBlurWidget::makeButton(1, tr("Abandonner"), QDialogButtonBox::RejectRole, false, true),
                                OverlayBlurWidget::makeButton(0, tr("Sélectionner un autre emplacement"), QDialogButtonBox::AcceptRole, true, true) };
                cfg.clickOutsideToClose = false;
                cfg.escapeButtonId = -1; // ou ne pas le renseigner du tout

                int retour = overlay->execModal(cfg);

                if (retour == 1 /*Abandonner*/) {
                    return;
                }
                continue;
            }
        }
        DEPOSE_OVERLAY_BLUR;

        EcrireDansIni("ExportPageDeGardePDF", QFileInfo(PDFASauverPDG).dir().path());
        PDFASauverPDG = PDFASauverPDG.chopped(4); // Retirer le .pdf de fin (4 caractère)

        const int maxSteps = 10;
        auto* po = ProgressOverlay::showDeterminate(this,
                                                    "Préparation...",
                                                    maxSteps,
                                                    /*center*/ true,
                                                    /*blockInput*/ true,
                                                    /*fadeMs*/ 140);

        po->enableBackdropBlur(true, 4.0, 0.5);

        for (int valTranche = 0; valTranche < out.tranches.size(); ++valTranche) {
            po->setValue(valTranche);
            po->setText(QObject::tr("Tranche %1 / %2...").arg(valTranche).arg(maxSteps));
            QThread::msleep(25);
            QApplication::processEvents();

            if (out.tranches.at(valTranche) == false)
                continue;
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
            mPDGHelper.vecVARIABLE.clear();
            for (int i = 1; i < ui->PDG_ListeWidget->count(); ++i) {
                blocQuestion* mBlocquestion = qobject_cast<blocQuestion*>(ui->PDG_ListeWidget->itemWidget(ui->PDG_ListeWidget->item(i)));
                auto reponse = mBlocquestion->RetourneDonnee();
                QString ReponseFormat = reponse->Reponse;
                ReponseFormat = QString(ReponseFormat).replace("{RetourLigne}", "\n");
                if (reponse->TypeDeBloc == blocQuestion::TypeBloc::Bloc_Texte_Simple)
                    mPDGHelper.ListeQuestion[reponse->IndexControle].DefautQuestion = ReponseFormat;
                if (reponse->TypeDeBloc == blocQuestion::TypeBloc::Bloc_Texte_Multiligne)
                    mPDGHelper.ListeQuestion[reponse->IndexControle].DefautQuestion = ReponseFormat;
                if (reponse->TypeDeBloc == blocQuestion::TypeBloc::Bloc_Case_Coche)
                    mPDGHelper.ListeQuestion[reponse->IndexControle].CheckboxValue = reponse->EtatCoche;
                if (reponse->NomVariable != "") {
                    PDGHelper::stockVariable lVAR;
                    lVAR.Variable = reponse->NomVariable;
                    lVAR.Valeur = ReponseFormat;
                    mPDGHelper.vecVARIABLE.append(lVAR);
                }
            }
            mPDGHelper.ArrayFromREEMAKER.ReferenceSite = out.nomSite;
            mPDGHelper.ArrayFromREEMAKER.NumeroTranche = QString::number(valTranche);
            mPDGHelper.ArrayFromREEMAKER.ReferenceREE = out.nomREE;
            mPDGHelper.ArrayFromREEMAKER.IndiceREE = out.indiceREE;
            mPDGHelper.ArrayFromREEMAKER.REErouge = CouleurTranche[valTranche].red();
            mPDGHelper.ArrayFromREEMAKER.REEvert = CouleurTranche[valTranche].green();
            mPDGHelper.ArrayFromREEMAKER.REEbleu = CouleurTranche[valTranche].blue();
            mPDGHelper.ArrayFromREEMAKER.REErougeAccent = CouleurAccentuation[valTranche].red();
            mPDGHelper.ArrayFromREEMAKER.REEvertAccent = CouleurAccentuation[valTranche].green();
            mPDGHelper.ArrayFromREEMAKER.REEbleuAccent = CouleurAccentuation[valTranche].blue();

            PoDoFo::PdfPage& pPage = document.GetPages().CreatePageAt(0, PoDoFo::Rect(0.0, 0.0, 595.0, 842.0));
            PoDoFo::PdfPainter painter;
            painter.SetCanvas(pPage);
            mPDGHelper.DrawOnPage_v2(painter, document);
            painter.FinishDrawing();
            document.Save(QStringToPdfString(PDFASauverPDG + "_Tr" + QString::number(valTranche) + ".pdf"));
            Consigne("Génération de la page de garde pour la tranche n°" + QString::number(valTranche));
        }
        po->close();
        {
            POSE_OVERLAY_BLUR;
            ModalConfig cfg;
            cfg.title = "REEMaker - Fin de l'opération";
            cfg.message = "Les pages de gardes ont été générés\nSouhaitez vous afficher le dossier les contenants ?";
            cfg.icon = OverlayBlurWidget::iconFromStandard(QStyle::SP_MessageBoxQuestion, this);
            cfg.buttons = { OverlayBlurWidget::makeButton(1, tr("Oui"), QDialogButtonBox::RejectRole, false, true),
                            OverlayBlurWidget::makeButton(0, tr("Non"), QDialogButtonBox::AcceptRole, true, true) };
            cfg.clickOutsideToClose = false;
            cfg.escapeButtonId = -1;

            int retour = overlay->execModal(cfg);
            DEPOSE_OVERLAY_BLUR;
            if (retour == 1 /*Afficher les PdG*/) {
                QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(PDFASauverPDG).path()));
            }
        }
    }
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
    UINT size = 0;
    LPBYTE lpBuffer = NULL;
    DWORD verSize = GetFileVersionInfoSize(fName.toStdWString().c_str(), &verHandle);

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
                StructBloc.TypeDeBloc = blocQuestion::TypeBloc::Bloc_Case_Coche;
                StructBloc.IndexControle = lPDG;
                StructBloc.NomControle = QString("CheckBox " + QString::number(StructBloc.IndexControle));
                StructBloc.Question = mPDGHelper.ListeQuestion[lPDG].LaQuestion;
                StructBloc.Aide = mPDGHelper.ListeQuestion[lPDG].AideQuestion;
                StructBloc.EstObligatoire = mPDGHelper.ListeQuestion[lPDG].Obligatoire;
                StructBloc.EtatCoche = mPDGHelper.ListeQuestion[lPDG].CheckboxValue;
                blocQuestion* mblocQuestion = new blocQuestion(this, StructBloc);
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

                StructBloc.TypeDeBloc = blocQuestion::TypeBloc::Bloc_Texte_Simple;
                StructBloc.IndexControle = lPDG;
                StructBloc.NomControle = QString("Texte Ligne " + QString::number(StructBloc.IndexControle));
                StructBloc.Question = mPDGHelper.ListeQuestion[lPDG].LaQuestion;
                StructBloc.Aide = mPDGHelper.ListeQuestion[lPDG].AideQuestion;
                StructBloc.Reponse = mPDGHelper.ListeQuestion[lPDG].DefautQuestion;
                StructBloc.Maximum = mPDGHelper.ListeQuestion[lPDG].Maximum;
                StructBloc.EstObligatoire = mPDGHelper.ListeQuestion[lPDG].Obligatoire;
                StructBloc.NomVariable = mPDGHelper.ListeQuestion[lPDG].NomVariable;
                blocQuestion* mblocQuestion = new blocQuestion(this, StructBloc);
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

                StructBloc.TypeDeBloc = blocQuestion::TypeBloc::Bloc_Texte_Multiligne;
                StructBloc.IndexControle = lPDG;
                StructBloc.NomControle = QString("Texte Multiligne " + QString::number(StructBloc.IndexControle));
                StructBloc.Question = mPDGHelper.ListeQuestion[lPDG].LaQuestion;
                StructBloc.Aide = mPDGHelper.ListeQuestion[lPDG].AideQuestion;
                StructBloc.Reponse = mPDGHelper.ListeQuestion[lPDG].DefautQuestion;
                StructBloc.Maximum = mPDGHelper.ListeQuestion[lPDG].Maximum;
                StructBloc.EstObligatoire = mPDGHelper.ListeQuestion[lPDG].Obligatoire;
                StructBloc.NomVariable = mPDGHelper.ListeQuestion[lPDG].NomVariable;
                blocQuestion* mblocQuestion = new blocQuestion(this, StructBloc);
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
        ui->PDG_Texte_PDGEnCours->setText("");
        return false;
    }
    ui->PDG_Texte_PDGEnCours->setText(
      QString("' %1 '   [Page de garde %2]").arg(Nom, (Chemin.toLower() == CheminPDGUtilisateur.toLower()) ? "utilisateur" : "intégrée"));
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

    POSE_OVERLAY_BLUR;

    if (ui->PDG_ListeWidget->count() == 0) {
        OverlayBlurWidget* overlay = new OverlayBlurWidget(this, 18.0, 0.5);
        ModalConfig cfg;
        cfg.title = "REEMaker - Attention";
        cfg.message = "<b>La liste est vide ou aucune page de garde n'a été "
                      "chargée.<br>L'opération est annulée.</b>";
        cfg.icon = OverlayBlurWidget::iconFromStandard(QStyle::SP_MessageBoxWarning, this);
        cfg.buttons = { OverlayBlurWidget::makeButton(0, "Annuler", QDialogButtonBox::DestructiveRole, true, false) };
        cfg.clickOutsideToClose = false;
        cfg.escapeButtonId = -1;
        overlay->execModal(cfg);
        DEPOSE_OVERLAY_BLUR;
        return;
    }
    bool ok;
    QString NomUtilisateur = QInputDialog::getText(this,
                                                   "Enregistrement page de garde",
                                                   "Saisir un nom pour la page de garde :",
                                                   QLineEdit::Normal,
                                                   PDGOuverteEstUtilisateur ? ui->PDG_Combo_Utilisateur->currentText() : "",
                                                   &ok);
    if (ok && !NomUtilisateur.isEmpty()) {
        { // ENUMERATION
            for (int i = 1; i < ui->PDG_ListeWidget->count(); ++i) {
                blocQuestion* mBlocquestion = qobject_cast<blocQuestion*>(ui->PDG_ListeWidget->itemWidget(ui->PDG_ListeWidget->item(i)));
                auto DonneeBloc = mBlocquestion->RetourneDonnee();
                QString ReponseFormat = DonneeBloc->Reponse;
                ReponseFormat = QString(ReponseFormat).replace("\n", "{RetourLigne}");
                if ((DonneeBloc->TypeDeBloc == blocQuestion::TypeBloc::Bloc_Texte_Simple) ||
                    (DonneeBloc->TypeDeBloc == blocQuestion::TypeBloc::Bloc_Texte_Multiligne))
                    mPDGHelper.ListeQuestion[DonneeBloc->IndexControle].DefautQuestion = ReponseFormat;
                if (DonneeBloc->TypeDeBloc == blocQuestion::TypeBloc::Bloc_Case_Coche)
                    mPDGHelper.ListeQuestion[DonneeBloc->IndexControle].CheckboxValue = DonneeBloc->EtatCoche;
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
                ChargerPageDeGarde(ui->PDG_Combo_Utilisateur->itemText(DefautIndex), CheminPDGUtilisateur);
            }
        }
        Consigne("Page de garde " + NomUtilisateur + " sauvegardée");
    }
    DEPOSE_OVERLAY_BLUR;
}

/** on_PDG_Combo_Integre_activated:
  Charge la page de garde dans le dossier intégré

  @param int index
  @return Aucun
*/
void
MainWindow::on_PDG_Combo_Integre_activated(int index)
{
    PDGOuverteEstUtilisateur = false;
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
    PDGOuverteEstUtilisateur = true;
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
    StructBloc.TypeAction = Bloc::TypeAction::DESSINELIGNE;
    StructBloc.IndexControle = RetourneIndexLibre();
    StructBloc.NomControle = QString("DESSINETEXTE " + QString::number(StructBloc.IndexControle));
    StructBloc.Couleur = "#000000";
    BlocEditeur* mBlocEditeur = new BlocEditeur(mPressePapier, this, StructBloc);
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
    StructBloc.TypeAction = Bloc::TypeAction::DESSINERECTANGLEVIDE;
    StructBloc.IndexControle = RetourneIndexLibre();
    StructBloc.NomControle = QString("DESSINERECTANGLEVIDE " + QString::number(StructBloc.IndexControle));
    StructBloc.Couleur = "#000000";
    BlocEditeur* mBlocEditeur = new BlocEditeur(mPressePapier, this, StructBloc);
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
    StructBloc.TypeAction = Bloc::TypeAction::DESSINERECTANGLEGRILLE;
    StructBloc.IndexControle = RetourneIndexLibre();
    StructBloc.NomControle = QString("DESSINERECTANGLEGRILLE " + QString::number(StructBloc.IndexControle));
    StructBloc.Couleur = "#000000";
    BlocEditeur* mBlocEditeur = new BlocEditeur(mPressePapier, this, StructBloc);
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
    StructBloc.TypeAction = Bloc::TypeAction::DESSINERECTANGLEREMPLIS;
    StructBloc.IndexControle = RetourneIndexLibre();
    StructBloc.NomControle = QString("DESSINERECTANGLEREMPLIS " + QString::number(StructBloc.IndexControle));
    StructBloc.Couleur = "#000000";
    StructBloc.CouleurRemplissage = "#000000";
    BlocEditeur* mBlocEditeur = new BlocEditeur(mPressePapier, this, StructBloc);
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
    Bloc::ItemDefinition StructBloc = Bloc::ItemDefinition();
    StructBloc.TypeAction = Bloc::TypeAction::DESSINETEXTE;
    StructBloc.IndexControle = RetourneIndexLibre();
    StructBloc.NomControle = QString("DESSINETEXTE " + QString::number(StructBloc.IndexControle));
    StructBloc.Couleur = "#000000";
    StructBloc.Alignement_Horizontale = Bloc::AlignementHorizontale::Gauche;
    StructBloc.Alignement_Verticale = Bloc::AlignementVerticale::Centre;
    BlocEditeur* mBlocEditeur = new BlocEditeur(mPressePapier, this, StructBloc);
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
    Bloc::ItemDefinition StructBloc = Bloc::ItemDefinition();
    StructBloc.TypeAction = Bloc::TypeAction::DESSINETEXTEMULTILIGNE;
    StructBloc.IndexControle = RetourneIndexLibre();
    StructBloc.NomControle = QString("DESSINETEXTEMULTILIGNE " + QString::number(StructBloc.IndexControle));
    StructBloc.Couleur = "#000000";
    StructBloc.Alignement_Horizontale = Bloc::AlignementHorizontale::Gauche;
    StructBloc.Alignement_Verticale = Bloc::AlignementVerticale::Centre;
    BlocEditeur* mBlocEditeur = new BlocEditeur(mPressePapier, this, StructBloc);
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
    Bloc::ItemDefinition StructBloc = Bloc::ItemDefinition();
    StructBloc.TypeAction = Bloc::TypeAction::DESSINETEXTEQUESTION;
    StructBloc.IndexControle = RetourneIndexLibre();
    StructBloc.NomControle = QString("DESSINETEXTEQUESTION " + QString::number(StructBloc.IndexControle));
    StructBloc.Couleur = "#000000";
    StructBloc.Alignement_Horizontale = Bloc::AlignementHorizontale::Gauche;
    StructBloc.Alignement_Verticale = Bloc::AlignementVerticale::Centre;
    BlocEditeur* mBlocEditeur = new BlocEditeur(mPressePapier, this, StructBloc);
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
    Bloc::ItemDefinition StructBloc = Bloc::ItemDefinition();
    StructBloc.TypeAction = Bloc::TypeAction::DESSINETEXTEMULTILIGNEQUESTION;
    StructBloc.IndexControle = RetourneIndexLibre();
    StructBloc.NomControle = QString("DESSINETEXTEMULTILIGNEQUESTION " + QString::number(StructBloc.IndexControle));
    StructBloc.Couleur = "#000000";
    StructBloc.Alignement_Horizontale = Bloc::AlignementHorizontale::Gauche;
    StructBloc.Alignement_Verticale = Bloc::AlignementVerticale::Centre;
    BlocEditeur* mBlocEditeur = new BlocEditeur(mPressePapier, this, StructBloc);
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
    StructBloc.TypeAction = Bloc::TypeAction::DESSINECHECKBOX;
    StructBloc.IndexControle = RetourneIndexLibre();
    StructBloc.NomControle = QString("DESSINECHECKBOX " + QString::number(StructBloc.IndexControle));
    StructBloc.Couleur = "#000000";
    BlocEditeur* mBlocEditeur = new BlocEditeur(mPressePapier, this, StructBloc);
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
    StructBloc.TypeAction = Bloc::TypeAction::DESSINECHECKBOXQUESTION;
    StructBloc.IndexControle = RetourneIndexLibre();
    StructBloc.NomControle = QString("DESSINECHECKBOXQUESTION " + QString::number(StructBloc.IndexControle));
    StructBloc.Couleur = "#000000";
    BlocEditeur* mBlocEditeur = new BlocEditeur(mPressePapier, this, StructBloc);
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
    StructBloc.TypeAction = Bloc::TypeAction::DESSINEMULTICHECKBOXQUESTION;
    StructBloc.IndexControle = RetourneIndexLibre();
    StructBloc.NomControle = QString("DESSINEMULTICHECKBOXQUESTION " + QString::number(StructBloc.IndexControle));
    StructBloc.Couleur = "#000000";
    BlocEditeur* mBlocEditeur = new BlocEditeur(mPressePapier, this, StructBloc);
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
    StructBloc.TypeAction = Bloc::TypeAction::INSEREIMAGE;
    StructBloc.IndexControle = RetourneIndexLibre();
    StructBloc.NomControle = QString("INSEREIMAGE " + QString::number(StructBloc.IndexControle));
    StructBloc.CheminImage = "../Img_resource/";
    BlocEditeur* mBlocEditeur = new BlocEditeur(mPressePapier, this, StructBloc);
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
    StructBloc.TypeAction = Bloc::TypeAction::PAGESUIVANTE;
    StructBloc.IndexControle = RetourneIndexLibre();
    StructBloc.NomControle = QString("PAGESUIVANTE " + QString::number(StructBloc.IndexControle));
    BlocEditeur* mBlocEditeur = new BlocEditeur(mPressePapier, this, StructBloc);
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
    StructBloc.TypeAction = Bloc::TypeAction::COMMENTAIRE;
    StructBloc.IndexControle = RetourneIndexLibre();
    StructBloc.NomControle = QString("COMMENTAIRE " + QString::number(StructBloc.IndexControle));
    BlocEditeur* mBlocEditeur = new BlocEditeur(mPressePapier, this, StructBloc);
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
            mActBtn.TypeAction = Bloc::TypeAction::DESSINELIGNE;
            int nbARG = GetNumArg(vecFichierPDG, compteLigne + 1);
            QVector<QString> cpVec = vecFichierPDG.mid(compteLigne + 1, nbARG);
            mActBtn.DebutX = RetourneCleDouble(cpVec, "debutx", 0.0);
            mActBtn.DebutY = RetourneCleDouble(cpVec, "debuty", 0.0);
            mActBtn.FinX = RetourneCleDouble(cpVec, "finx", 0.0);
            mActBtn.FinY = RetourneCleDouble(cpVec, "finy", 0.0);
            mActBtn.Epaisseur = RetourneCleDouble(cpVec, "epaisseur", 1.0);
            mActBtn.Couleur = QString("#%1%2%3").arg(QString("%1").arg(RetourneCleInt(cpVec, "rouge", 0), 2, 16, QLatin1Char('0')),
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
            mActBtn.TypeAction = Bloc::TypeAction::DESSINERECTANGLEVIDE;
            int nbARG = GetNumArg(vecFichierPDG, compteLigne + 1);
            QVector<QString> cpVec = vecFichierPDG.mid(compteLigne + 1, nbARG);
            //            cpVec.insert(cpVec.begin(), vecFichierPDG.begin() +
            //            compteLigne + 1, vecFichierPDG.begin() + compteLigne +
            //            1 + nbARG);
            mActBtn.DebutX = RetourneCleDouble(cpVec, "debutx", 0.0);
            mActBtn.DebutY = RetourneCleDouble(cpVec, "debuty", 0.0);
            mActBtn.Largeur = RetourneCleDouble(cpVec, "largeur", 1.0);
            mActBtn.Hauteur = RetourneCleDouble(cpVec, "hauteur", 1.0);
            mActBtn.Epaisseur = RetourneCleDouble(cpVec, "epaisseur", 1.0);
            mActBtn.Couleur = QString("#%1%2%3").arg(QString("%1").arg(RetourneCleInt(cpVec, "rouge", 0), 2, 16, QLatin1Char('0')),
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
            mActBtn.TypeAction = Bloc::TypeAction::DESSINERECTANGLEGRILLE;
            int nbARG = GetNumArg(vecFichierPDG, compteLigne + 1);
            QVector<QString> cpVec = vecFichierPDG.mid(compteLigne + 1, nbARG);
            mActBtn.DebutX = RetourneCleDouble(cpVec, "debutx", 0.0);
            mActBtn.DebutY = RetourneCleDouble(cpVec, "debuty", 0.0);
            mActBtn.Largeur = RetourneCleDouble(cpVec, "largeur", 1.0);
            mActBtn.Hauteur = RetourneCleDouble(cpVec, "hauteur", 1.0);
            mActBtn.Epaisseur = RetourneCleDouble(cpVec, "epaisseur", 1.0);
            mActBtn.Couleur = QString("#%1%2%3").arg(QString("%1").arg(RetourneCleInt(cpVec, "rouge", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "vert", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "bleu", 0), 2, 16, QLatin1Char('0')));
            if (RetourneCleInt(cpVec, "rouge", 0) == -1)
                mActBtn.Couleur = "{ACC 1}";
            if (RetourneCleInt(cpVec, "rouge", 0) == -2)
                mActBtn.Couleur = "{ACC 2}";
            mActBtn.NombreColonne = RetourneCleInt(cpVec, "nombrecolonne", 1);
            mActBtn.NombreLigne = RetourneCleInt(cpVec, "nombreligne", 1);
            ListeAction.append(mActBtn);
            compteLigne += nbARG;
            continue;
        } else if (tLine == "DESSINERECTANGLEREMPLIS") {
            mActBtn.TypeAction = Bloc::TypeAction::DESSINERECTANGLEREMPLIS;
            int nbARG = GetNumArg(vecFichierPDG, compteLigne + 1);
            QVector<QString> cpVec = vecFichierPDG.mid(compteLigne + 1, nbARG);
            mActBtn.DebutX = RetourneCleDouble(cpVec, "debutx", 0.0);
            mActBtn.DebutY = RetourneCleDouble(cpVec, "debuty", 0.0);
            mActBtn.Largeur = RetourneCleDouble(cpVec, "largeur", 1.0);
            mActBtn.Hauteur = RetourneCleDouble(cpVec, "hauteur", 1.0);
            mActBtn.Epaisseur = RetourneCleDouble(cpVec, "epaisseur", 1.0);
            mActBtn.Couleur = QString("#%1%2%3").arg(QString("%1").arg(RetourneCleInt(cpVec, "rouge", 0), 2, 16, QLatin1Char('0')),
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
            mActBtn.TypeAction = Bloc::TypeAction::DESSINETEXTE;
            int nbARG = GetNumArg(vecFichierPDG, compteLigne + 1);
            QVector<QString> cpVec = vecFichierPDG.mid(compteLigne + 1, nbARG);
            mActBtn.DebutX = RetourneCleDouble(cpVec, "debutx", 0.0);
            mActBtn.DebutY = RetourneCleDouble(cpVec, "debuty", 0.0);
            mActBtn.Largeur = RetourneCleDouble(cpVec, "largeur", 1.0);
            mActBtn.Hauteur = RetourneCleDouble(cpVec, "hauteur", 1.0);
            mActBtn.TaillePolice = RetourneCleDouble(cpVec, "taillepolice", 10.0);
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
            mActBtn.Couleur = QString("#%1%2%3").arg(QString("%1").arg(RetourneCleInt(cpVec, "rouge", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "vert", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "bleu", 0), 2, 16, QLatin1Char('0')));
            if (RetourneCleInt(cpVec, "rouge", 0) == -1)
                mActBtn.Couleur = "{ACC 1}";
            if (RetourneCleInt(cpVec, "rouge", 0) == -2)
                mActBtn.Couleur = "{ACC 2}";

            mActBtn.Texte = RetourneCleStr(cpVec, "texte");

            mActBtn.Gras = RetourneCleBool(cpVec, "gras");
            mActBtn.GrasEtItalique = RetourneCleBool(cpVec, "grasitalic");
            mActBtn.Italique = RetourneCleBool(cpVec, "italic");
            mActBtn.Monospace = RetourneCleBool(cpVec, "monospace");

            ListeAction.append(mActBtn);
            compteLigne += nbARG;
            continue;
        } else if (tLine == "DESSINETEXTEMULTILIGNE") {
            mActBtn.TypeAction = Bloc::TypeAction::DESSINETEXTEMULTILIGNE;
            int nbARG = GetNumArg(vecFichierPDG, compteLigne + 1);
            QVector<QString> cpVec = vecFichierPDG.mid(compteLigne + 1, nbARG);
            mActBtn.DebutX = RetourneCleDouble(cpVec, "debutx", 0.0);
            mActBtn.DebutY = RetourneCleDouble(cpVec, "debuty", 0.0);
            mActBtn.Largeur = RetourneCleDouble(cpVec, "largeur", 1.0);
            mActBtn.Hauteur = RetourneCleDouble(cpVec, "hauteur", 1.0);
            mActBtn.TaillePolice = RetourneCleDouble(cpVec, "taillepolice", 10.0);
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

            mActBtn.Gras = RetourneCleBool(cpVec, "gras");
            mActBtn.GrasEtItalique = RetourneCleBool(cpVec, "grasitalic");
            mActBtn.Italique = RetourneCleBool(cpVec, "italic");
            mActBtn.Monospace = RetourneCleBool(cpVec, "monospace");

            ListeAction.append(mActBtn);
            compteLigne += nbARG;
            continue;
        } else if (tLine == "DESSINETEXTEQUESTION") {
            mActBtn.TypeAction = Bloc::TypeAction::DESSINETEXTEQUESTION;
            int nbARG = GetNumArg(vecFichierPDG, compteLigne + 1);
            QVector<QString> cpVec = vecFichierPDG.mid(compteLigne + 1, nbARG);
            mActBtn.DebutX = RetourneCleDouble(cpVec, "debutx", 0.0);
            mActBtn.DebutY = RetourneCleDouble(cpVec, "debuty", 0.0);
            mActBtn.Largeur = RetourneCleDouble(cpVec, "largeur", 1.0);
            mActBtn.Hauteur = RetourneCleDouble(cpVec, "hauteur", 1.0);
            mActBtn.TaillePolice = RetourneCleDouble(cpVec, "taillepolice", 10.0);
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
            mActBtn.LongueurMaximale = RetourneCleInt(cpVec, "max", 4000);
            mActBtn.Couleur = QString("#%1%2%3").arg(QString("%1").arg(RetourneCleInt(cpVec, "rouge", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "vert", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "bleu", 0), 2, 16, QLatin1Char('0')));
            if (RetourneCleInt(cpVec, "rouge", 0) == -1)
                mActBtn.Couleur = "{ACC 1}";
            if (RetourneCleInt(cpVec, "rouge", 0) == -2)
                mActBtn.Couleur = "{ACC 2}";

            mActBtn.Question = RetourneCleStr(cpVec, "question");
            mActBtn.QuestionAide = RetourneCleStr(cpVec, "aidequestion");
            mActBtn.QuestionDefaut = RetourneCleStr(cpVec, "defautquestion");

            mActBtn.Gras = RetourneCleBool(cpVec, "gras");
            mActBtn.GrasEtItalique = RetourneCleBool(cpVec, "grasitalic");
            mActBtn.Italique = RetourneCleBool(cpVec, "italic");
            mActBtn.Monospace = RetourneCleBool(cpVec, "monospace");
            mActBtn.Obligatoire = RetourneCleBool(cpVec, "obligatoire");
            mActBtn.Chiffre = RetourneCleBool(cpVec, "chiffre");
            mActBtn.Majuscule = RetourneCleBool(cpVec, "majuscule");
            mActBtn.Minuscule = RetourneCleBool(cpVec, "minuscule");
            mActBtn.NomVariable = RetourneCleStr(cpVec, "nomvariable", "");
            ListeAction.append(mActBtn);
            compteLigne += nbARG;
            continue;
        } else if (tLine == "DESSINETEXTEMULTILIGNEQUESTION") {
            mActBtn.TypeAction = Bloc::TypeAction::DESSINETEXTEMULTILIGNEQUESTION;
            int nbARG = GetNumArg(vecFichierPDG, compteLigne + 1);
            QVector<QString> cpVec = vecFichierPDG.mid(compteLigne + 1, nbARG);
            mActBtn.DebutX = RetourneCleDouble(cpVec, "debutx", 0.0);
            mActBtn.DebutY = RetourneCleDouble(cpVec, "debuty", 0.0);
            mActBtn.Largeur = RetourneCleDouble(cpVec, "largeur", 1.0);
            mActBtn.Hauteur = RetourneCleDouble(cpVec, "hauteur", 1.0);
            mActBtn.TaillePolice = RetourneCleDouble(cpVec, "taillepolice", 10.0);
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
            mActBtn.Couleur = QString("#%1%2%3").arg(QString("%1").arg(RetourneCleInt(cpVec, "rouge", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "vert", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "bleu", 0), 2, 16, QLatin1Char('0')));
            if (RetourneCleInt(cpVec, "rouge", 0) == -1)
                mActBtn.Couleur = "{ACC 1}";
            if (RetourneCleInt(cpVec, "rouge", 0) == -2)
                mActBtn.Couleur = "{ACC 2}";

            mActBtn.Question = RetourneCleStr(cpVec, "question");
            mActBtn.QuestionAide = RetourneCleStr(cpVec, "aidequestion");
            mActBtn.QuestionDefaut = RetourneCleStr(cpVec, "defautquestion");

            mActBtn.Gras = RetourneCleBool(cpVec, "gras");
            mActBtn.GrasEtItalique = RetourneCleBool(cpVec, "grasitalic");
            mActBtn.Italique = RetourneCleBool(cpVec, "italic");
            mActBtn.Monospace = RetourneCleBool(cpVec, "monospace");
            mActBtn.Obligatoire = RetourneCleBool(cpVec, "obligatoire");
            mActBtn.Majuscule = RetourneCleBool(cpVec, "majuscule");
            mActBtn.Minuscule = RetourneCleBool(cpVec, "minuscule");
            mActBtn.NomVariable = RetourneCleStr(cpVec, "nomvariable", "");

            ListeAction.append(mActBtn);
            compteLigne += nbARG;
            continue;
        } else if (tLine == "INSEREIMAGE") {
            mActBtn.TypeAction = Bloc::TypeAction::INSEREIMAGE;
            int nbARG = GetNumArg(vecFichierPDG, compteLigne + 1);
            QVector<QString> cpVec = vecFichierPDG.mid(compteLigne + 1, nbARG);
            mActBtn.DebutX = RetourneCleDouble(cpVec, "debutx", 0.0);
            mActBtn.DebutY = RetourneCleDouble(cpVec, "debuty", 0.0);
            mActBtn.Largeur = RetourneCleDouble(cpVec, "largeur", 1.0);
            mActBtn.Hauteur = RetourneCleDouble(cpVec, "hauteur", 1.0);
            mActBtn.CheminImage = RetourneCleStr(cpVec, "chemin");
            ListeAction.append(mActBtn);
            compteLigne += nbARG;
            continue;
        } else if (tLine == "DESSINECHECKBOX") {
            mActBtn.TypeAction = Bloc::TypeAction::DESSINECHECKBOX;
            int nbARG = GetNumArg(vecFichierPDG, compteLigne + 1);
            QVector<QString> cpVec = vecFichierPDG.mid(compteLigne + 1, nbARG);
            mActBtn.DebutX = RetourneCleDouble(cpVec, "debutx", 0.0);
            mActBtn.DebutY = RetourneCleDouble(cpVec, "debuty", 0.0);
            mActBtn.Largeur = RetourneCleDouble(cpVec, "largeur", 1.0);
            mActBtn.Hauteur = RetourneCleDouble(cpVec, "hauteur", 1.0);
            mActBtn.Epaisseur = RetourneCleDouble(cpVec, "epaisseur", 1.0);
            mActBtn.Couleur = QString("#%1%2%3").arg(QString("%1").arg(RetourneCleInt(cpVec, "rouge", 0), 2, 16, QLatin1Char('0')),
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
            mActBtn.TypeAction = Bloc::TypeAction::DESSINECHECKBOXQUESTION;
            int nbARG = GetNumArg(vecFichierPDG, compteLigne + 1);
            QVector<QString> cpVec = vecFichierPDG.mid(compteLigne + 1, nbARG);
            mActBtn.DebutX = RetourneCleDouble(cpVec, "debutx", 0.0);
            mActBtn.DebutY = RetourneCleDouble(cpVec, "debuty", 0.0);
            mActBtn.Largeur = RetourneCleDouble(cpVec, "largeur", 1.0);
            mActBtn.Hauteur = RetourneCleDouble(cpVec, "hauteur", 1.0);
            mActBtn.Epaisseur = RetourneCleDouble(cpVec, "epaisseur", 1.0);
            mActBtn.Couleur = QString("#%1%2%3").arg(QString("%1").arg(RetourneCleInt(cpVec, "rouge", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "vert", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "bleu", 0), 2, 16, QLatin1Char('0')));
            if (RetourneCleInt(cpVec, "rouge", 0) == -1)
                mActBtn.Couleur = "{ACC 1}";
            if (RetourneCleInt(cpVec, "rouge", 0) == -2)
                mActBtn.Couleur = "{ACC 2}";

            mActBtn.Question = RetourneCleStr(cpVec, "question");
            mActBtn.QuestionAide = RetourneCleStr(cpVec, "aidequestion");
            mActBtn.QuestionDefaut = RetourneCleStr(cpVec, "defautquestion");

            mActBtn.Obligatoire = RetourneCleBool(cpVec, "obligatoire");

            ListeAction.append(mActBtn);
            compteLigne += nbARG;
            continue;
        } else if (tLine == "DESSINEMULTICHECKBOXQUESTION") {
            mActBtn.TypeAction = Bloc::TypeAction::DESSINEMULTICHECKBOXQUESTION;
            int nbARG = GetNumArg(vecFichierPDG, compteLigne + 1);
            QVector<QString> cpVec = vecFichierPDG.mid(compteLigne + 1, nbARG);
            mActBtn.DebutX = RetourneCleDouble(cpVec, "debutx1", 0.0);
            mActBtn.DebutY = RetourneCleDouble(cpVec, "debuty1", 0.0);
            mActBtn.DebutX2 = RetourneCleDouble(cpVec, "debutx2", 0.0);
            mActBtn.DebutY2 = RetourneCleDouble(cpVec, "debuty2", 0.0);
            mActBtn.Largeur = RetourneCleDouble(cpVec, "largeur", 1.0);
            mActBtn.Hauteur = RetourneCleDouble(cpVec, "hauteur", 1.0);
            mActBtn.Epaisseur = RetourneCleDouble(cpVec, "epaisseur", 1.0);
            mActBtn.Couleur = QString("#%1%2%3").arg(QString("%1").arg(RetourneCleInt(cpVec, "rouge", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "vert", 0), 2, 16, QLatin1Char('0')),
                                                     QString("%1").arg(RetourneCleInt(cpVec, "bleu", 0), 2, 16, QLatin1Char('0')));
            if (RetourneCleInt(cpVec, "rouge", 0) == -1)
                mActBtn.Couleur = "{ACC 1}";
            if (RetourneCleInt(cpVec, "rouge", 0) == -2)
                mActBtn.Couleur = "{ACC 2}";

            mActBtn.Question = RetourneCleStr(cpVec, "question");
            mActBtn.QuestionAide = RetourneCleStr(cpVec, "aidequestion");
            mActBtn.QuestionDefaut = RetourneCleStr(cpVec, "defautquestion");
            mActBtn.Obligatoire = RetourneCleBool(cpVec, "obligatoire");

            ListeAction.append(mActBtn);
            compteLigne += nbARG;
            continue;
        } else if (tLine == "PAGESUIVANTE") {
            mActBtn.TypeAction = Bloc::TypeAction::PAGESUIVANTE;

            ListeAction.append(mActBtn);
            continue;
        } else {
            // Commentaire
            mActBtn.TypeAction = Bloc::TypeAction::COMMENTAIRE;
            int nbARG = GetNumCOMM(vecFichierPDG, compteLigne);
            QVector<QString> cpVec = vecFichierPDG.mid(compteLigne + 1, nbARG - 1);
            mActBtn.Commentaire = "";
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
        auto reponse = mBlocEditeur->RetourneDonnee();
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
                if (reponse.NomVariable != "")
                    f_out << "--nomvariable=\"" << reponse.NomVariable.toLower() << "\"" << Qt::endl;
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
                if (reponse.NomVariable != "")
                    f_out << "--nomvariable=\"" << reponse.NomVariable.toLower() << "\"" << Qt::endl;
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

    if (ValeurMax <= 0) {
        Consigne("Il y a 0 pages vu dans le document, abandon de la réparation");
        return ValeurRetour::ZeroPages;
    }
    auto* po = ProgressOverlay::showDeterminate(this, "Analyse GhostScript > Sauvegarde du fichier PDF...", ValeurMax, true, true, 140);
    po->enableBackdropBlur(true, 4.0, 0.5);
    QCoreApplication::processEvents();

    auto procGhost = new QProcess();
    QString Windir = "";
    {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        Windir = env.value("windir");
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
                                                       "-dGrayImageFilter=/FlateEncode "
                                                       "-dAutoRotatePages=/None"
                                                     : "-dAutoRotatePages=/None");
    procGhost->setNativeArguments(ConstructedArguments);
    procGhost->start(CheminBaseGhostScript + "bin/gswin64c.exe");
    if (ValeurMax != 0)
        po->setText("Analyse GhostScript > Ouverture du fichier PDF");
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
            po->setValue(PageEnCours);
            po->setText("Analyse GhostScript> Page " + QString::number(PageEnCours) + "/" + QString::number(ValeurMax));
        }
        QThread::msleep(100);
        QCoreApplication::processEvents();
    }
    if (ValeurMax != 0)
        po->close();
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
    auto Lecture = QString(procPDFINFO->readAllStandardOutput()).split("\n", Qt::SkipEmptyParts);
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
    QStringList qslCorrige = { "Auteur                : ", "Créateur              : ", "Producteur            : ", "Date création         : ",
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
    Consigne("Démarrage du contrôle de l'intégrité des plugins et polices");
    QProgressDialog progress("Contrôle de l'intégrité des plugins...", "", 0, 0, this);
    progress.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMaximum(1);
    progress.setAutoClose(false);
    progress.setCancelButton(0);
    progress.setWindowIcon(this->windowIcon());
    progress.installEventFilter(keyPressEater);
    progress.show();
    QCoreApplication::processEvents();
    centerProgressDialogOnCurrentScreen(&progress);

    bool needRepairGScript = false;
    bool needRepairPoppler = false;
    bool needRepairPolice = false;
    uint64_t nombreFichier = 0;
    auto _FA = FastArchiver();
    /// Nombre fichier GhostScript
    auto _flistGScript = _FA.retrieveContentInformation(QCoreApplication::applicationDirPath() + "/GhostScript_base.zstd",
                                                        QCoreApplication::applicationDirPath() + "/GhostScript");
    qDebug() << "GHOSTSCRIPT : " << _flistGScript.size() << " entrées à tester";
    nombreFichier += _flistGScript.size();
    auto _flistPoppler = _FA.retrieveContentInformation(QCoreApplication::applicationDirPath() + "/PdfToPPM_base.zstd",
                                                        QCoreApplication::applicationDirPath() + "/PdfToPPM");
    qDebug() << "POPPLER : " << _flistPoppler.size() << " entrées à tester";
    nombreFichier += _flistPoppler.size();
    auto _flistPolice = _FA.retrieveContentInformation(QCoreApplication::applicationDirPath() + "/Police_base.zstd",
                                                       QCoreApplication::applicationDirPath() + "/Police");
    qDebug() << "POLICES : " << _flistPolice.size() << " entrées à tester";
    nombreFichier += _flistPolice.size();
    progress.setMaximum(nombreFichier);
    uint64_t ControledFile = 0;

    for (auto& var : _flistGScript) { /// GHOSTSCRIPT
        ControledFile++;
        progress.setLabelText(QString("Contrôle de l'intégrité du fichier "
                                      "%1/%2 du plugin Ghostscript")
                                .arg(ControledFile)
                                .arg(nombreFichier));
        progress.setValue(ControledFile);
        QCoreApplication::processEvents();
        if (var.isDir)
            continue;
        auto newHash = _FA.computeXXH64_FileHash(var.fullPath);
        if (newHash == "" /* Existe pas */ || newHash != var.xxhashFromArchive /*Corrompu*/) {
            needRepairGScript = true;
        }
    }
    for (auto& var : _flistPoppler) { /// POPPLER
        ControledFile++;
        progress.setLabelText(QString("Contrôle de l'intégrité du fichier %1/%2 du plugin Poppler").arg(ControledFile).arg(nombreFichier));
        progress.setValue(ControledFile);
        QCoreApplication::processEvents();
        if (var.isDir)
            continue;
        auto newHash = _FA.computeXXH64_FileHash(var.fullPath);
        if (newHash == "" /* Existe pas */ || newHash != var.xxhashFromArchive /*Corrompu*/) {
            needRepairPoppler = true;
        }
    }

    for (auto& var : _flistPolice) { /// POLICES
        ControledFile++;
        progress.setLabelText(QString("Contrôle de l'intégrité du fichier %1/%2 du pack de polices").arg(ControledFile).arg(nombreFichier));
        progress.setValue(ControledFile);
        QCoreApplication::processEvents();
        if (var.isDir)
            continue;
        auto newHash = _FA.computeXXH64_FileHash(var.fullPath);
        if (newHash == "" /* Existe pas */ || newHash != var.xxhashFromArchive /*Corrompu*/) {
            needRepairPolice = true;
        }
    }
    if (needRepairGScript) {
        Consigne("Le plugin Ghoscript n'est pas intègre, restauration du plugin");
        progress.setLabelText("Le plugin Ghoscript n'est pas intègre, restauration du plugin");
        QCoreApplication::processEvents();
        progress.setMaximum(1);
        progress.setValue(0);
        QThread::msleep(500);
        _FA.decompressArchiveBuffered(QCoreApplication::applicationDirPath() + "/GhostScript_base.zstd",
                                      QCoreApplication::applicationDirPath() + "/GhostScript");
        progress.setLabelText("Plugin Ghoscript restauré");
        progress.setValue(1);
        QCoreApplication::processEvents();
    } else
        qDebug() << "GhostScript est intègre !";
    if (needRepairPoppler) {
        Consigne("Le plugin Poppler n'est pas intègre, restauration du plugin");
        progress.setLabelText("Le plugin Poppler n'est pas intègre, restauration du plugin");
        progress.setMaximum(1);
        progress.setValue(0);
        QCoreApplication::processEvents();
        QThread::msleep(500);
        _FA.decompressArchiveBuffered(QCoreApplication::applicationDirPath() + "/PdfToPPM_base.zstd",
                                      QCoreApplication::applicationDirPath() + "/PdfToPPM");
        progress.setLabelText("Plugin Poppler restauré");
        progress.setValue(1);
        QCoreApplication::processEvents();
    } else
        qDebug() << "Poppler est intègre !";
    if (needRepairPolice) {
        Consigne("Le pack de police n'est pas intègre, restauration des polices");
        progress.setLabelText("Le pack de police n'est pas intègre, restauration des polices");
        progress.setMaximum(1);
        progress.setValue(0);
        QCoreApplication::processEvents();
        QThread::msleep(500);
        _FA.decompressArchiveBuffered(QCoreApplication::applicationDirPath() + "/Police_base.zstd",
                                      QCoreApplication::applicationDirPath() + "/Police");
        progress.setLabelText("Polices restaurés");
        progress.setValue(1);
        QCoreApplication::processEvents();
    } else
        qDebug() << "Les polices sont intègres !";
    progress.setLabelText("Fin de l'analyse d'intégrité");
    Consigne("Fin de l'analyse d'intégrité");
    QCoreApplication::processEvents();
    QThread::msleep(2000);
    progress.close();
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

/** on_PDG_Btn_SupprimePDGUtilisateur_clicked:
  Suppression de la page de garde sélectionnée

  @return Aucun
*/
void
MainWindow::on_PDG_Btn_SupprimePDGUtilisateur_clicked()
{
    if (ui->PDG_Combo_Utilisateur->count() == 0)
        return;
    POSE_OVERLAY_BLUR;
    ModalConfig cfg;
    cfg.title = "REEMaker - Suppression";
    cfg.message = "<b>Vous allez supprimer la page de garde " + QString("<i>'%1'</i>").arg(ui->PDG_Combo_Utilisateur->currentText()) + "</b>";
    cfg.icon = OverlayBlurWidget::iconFromStandard(QStyle::SP_MessageBoxQuestion, this);
    cfg.buttons = { OverlayBlurWidget::makeButton(0, "Confirmer", QDialogButtonBox::AcceptRole, true, false),
                    OverlayBlurWidget::makeButton(1, "Annuler", QDialogButtonBox::RejectRole, false, false) };
    cfg.clickOutsideToClose = false;
    cfg.escapeButtonId = -1;
    int retour = overlay->execModal(cfg);
    DEPOSE_OVERLAY_BLUR;
    if (retour == 1 /*Annuler*/) {
        return;
    }
    if (retour == 0 /*Confirmer*/) {
        QFile::remove(CheminPDGUtilisateur + ui->PDG_Combo_Utilisateur->currentText() + ".txt");
        Consigne("Suppression de la page de garde utilisateur '" + ui->PDG_Combo_Utilisateur->currentText() + "'");
        ui->PDG_ListeWidget->clear();
        ChargerPageDeGarde("page de garde standard BPA", CheminPDGBase);
    }
}

///
/// \brief Cherche une clé au format QVariant dans le groupe CLE_MANUEL
/// \param Cle
/// \param defaut
/// \return
///
QVariant
MainWindow::LireDansIni(QString Cle, QVariant defaut)
{
    QSettings settings(QCoreApplication::applicationDirPath() + "/REEMAKER.ini", QSettings::IniFormat);
    return settings.value("CLE_MANUEL/" + Cle, defaut);
}

///
/// \brief Ecrit une valeur dans la clé dans le groupe CLE_MANUEL
/// \param Cle
/// \param Valeur
/// \return
///
bool
MainWindow::EcrireDansIni(QString Cle, QVariant Valeur)
{
    QSettings settings(QCoreApplication::applicationDirPath() + "/REEMAKER.ini",
                       QSettings::IniFormat); // Création du fichier en précisant que l'on
    // travaille avec un fichier de format INI.
    settings.beginGroup("CLE_MANUEL");
    settings.setValue(Cle, Valeur);
    return true;
}

/** LectureINI:
  Lecture des paramètres utilisateurs

  @return Aucun
*/
void
MainWindow::LectureINI()
{
    QSettings settings(QCoreApplication::applicationDirPath() + "/REEMAKER.ini", QSettings::IniFormat);
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
    ui->OPT_Spin_Largeur->setValue(settings.value("REGLAGES/MargeLateral", "15").toInt());
    ui->OPT_Spin_Hauteur->setValue(settings.value("REGLAGES/MargeVertical", "15").toInt());
    ui->OPT_Spin_ResolutionDPI->setValue(settings.value("REGLAGES/ResolutionDPI", "40").toInt());
    ui->OPT_Text_FolioAnnule->setText(settings.value("REGLAGES/FolioAnnule", "Folio annulé").toString());
    PDGManuelNomSite = settings.value("REGLAGES/MANUEL_NomDuSite", "").toString();
    PDGManuelRefREE = settings.value("REGLAGES/MANUEL_ReferenceREE", "").toString();
    PDGManuelIndice = settings.value("REGLAGES/MANUEL_Indice", "").toString();
    bool TypeTampon = settings.value("REGLAGES/TAMPONCLASSIQUE", false).toBool();
    qDebug() << "TypeTampon:" << TypeTampon;
    ui->OPT_Tampon_classique->setChecked(TypeTampon);
    ui->OPT_Tampon_compact->setChecked(!TypeTampon);
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
    CheminPoppler = QCoreApplication::applicationDirPath() + "/PdfToPPM/pdftoppm.exe";
    CheminGhostScript = QCoreApplication::applicationDirPath() + "/GhostScript/BatchGhostScript.bat";
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
        int DefautIndex = -1;
        foreach (QString lPDG, lstPDGBase) {
            ui->PDG_Combo_Integre->addItem(QFileInfo(CheminPDGBase + lPDG).baseName());
            if (lPDG.toLower() == "page de garde standard bpa.txt") // On sait que cette
                                                                    // PageDeGarde
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
                ChargerPageDeGarde(ui->PDG_Combo_Integre->itemText(var), CheminPDGBase);
                TrouvePDG = true;
                PDGOuverteEstUtilisateur = false;
                break;
            }
        }
        /* -- Test si ce nom est dans Combo utilisateur -- */
        if (!TrouvePDG)
            for (int var = 0; var < ui->PDG_Combo_Utilisateur->count(); ++var) {
                if (ui->PDG_Combo_Utilisateur->itemText(var).toUpper() == ui->PDG_Texte_PDGEnCours->text().toUpper()) {
                    ui->PDG_Combo_Utilisateur->setCurrentIndex(var);
                    ChargerPageDeGarde(ui->PDG_Combo_Utilisateur->itemText(var), CheminPDGUtilisateur);
                    TrouvePDG = true;
                    PDGOuverteEstUtilisateur = true;
                    break;
                }
            }
        /* -- Aucun, on le reset -- */
        if (!TrouvePDG)
            for (int var = 0; var < ui->PDG_Combo_Integre->count(); ++var) {
                if (ui->PDG_Combo_Integre->itemText(var).toLower() == "page de garde standard bpa") {
                    ui->PDG_Combo_Integre->setCurrentIndex(var);
                    ChargerPageDeGarde(ui->PDG_Combo_Integre->itemText(var), CheminPDGBase);
                    PDGOuverteEstUtilisateur = false;
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
    settings.setValue("TAMPONCLASSIQUE", ui->OPT_Tampon_classique->isChecked());
    settings.setValue("MANUEL_NomDuSite", PDGManuelNomSite);
    settings.setValue("MANUEL_ReferenceREE", PDGManuelRefREE);
    settings.setValue("MANUEL_Indice", PDGManuelIndice);

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
        QJsonDocument json = QJsonDocument::fromJson(response_data);
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
    auto Initialisation = myDownloader->DemarreTelechargement(
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
            Consigne("Erreur au démarrage du téléchargement : URL source non "
                     "renseigné");
    } else {
        while (true) {
            QThread::msleep(250);
            QCoreApplication::processEvents();
            auto Statut = myDownloader->RetourneStatut();

            //            if (Statut ==
            //            HttpDownload::EtatFinTelechargement::EnCours)
            //                qDebug().noquote().nospace() << "C'est normal, sa
            //                tourne...";
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
                         "et mise en place...");
                QStringList Operations;
                Operations << QString(" & \"%1\" x -y \"%2\" -o\"%3\"")
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
        qDebug().nospace().noquote() << QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss : ") << Message;
    /* -- Ici on écrit le Message dans ListLog -- */
    if (AfficheDansLog)
        ui->listLOG->insertItem(0, QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss : ") + Message);
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
        ContenuJDB.prepend(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss : ") + Message);
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
            Consigne(
              QString("Centrage : Souris dans l'écran n°%1 [%2]").arg(QString::number(NumEcran), monitors.qvName[NumEcran]), false, false, true);
            SetRect(&FenetreAvecSouris, var.left, var.top, var.right, var.bottom);
        }
        NumEcran++;
    }
    if ((FenetreAvecSouris.right - FenetreAvecSouris.left) > 0) { // On peut centrer
        RECT FenetreActuel;
        GetWindowRect(Fenetre, &FenetreActuel);
        MoveWindow(Fenetre,
                   /*Left*/ FenetreAvecSouris.left +
                     ((FenetreAvecSouris.right - FenetreAvecSouris.left) /*WidthEcran par2*/
                      / 2) -
                     ((FenetreActuel.right - FenetreActuel.left) /*WidthFenetre*/ / 2),
                   /*top*/ FenetreAvecSouris.top +
                     ((FenetreAvecSouris.bottom - FenetreAvecSouris.top) /*HeightEcran par2*/
                      / 2) -
                     ((FenetreActuel.bottom - FenetreActuel.top) /*HeightFenetre*/ / 2),
                   /*width*/ FenetreActuel.right - FenetreActuel.left,
                   /*height*/ FenetreActuel.bottom - FenetreActuel.top,
                   TRUE);
    }
    return true;
}

void
MainWindow::on_tool_repair_run_button_clicked()
{
    POSE_OVERLAY_BLUR;

    QString PDF2Repair = QFileDialog::getOpenFileName(
      this, "Réparer un fichier PDF - Sélectionner le fichier PDF", LireDansIni("ProcedureRepare", "").toString(), "Fichier PDF (*.pdf)");
    if (PDF2Repair == "") {
        DEPOSE_OVERLAY_BLUR;
        return;
    }
    QString NewPathToSave = "";

    while (1) {
        NewPathToSave = QFileDialog::getSaveFileName(
          this, "Réparer un fichier PDF - Enregistrer le fichier PDF réparé", "Réparé - " + QFileInfo(PDF2Repair).fileName(), "Fichier PDF (*.pdf)");
        if (NewPathToSave == "") {
            DEPOSE_OVERLAY_BLUR;
            return;
        }
        auto retour = canWriteToFile(NewPathToSave);
        if (retour == CanWriteFile::Yes_I_Can)
            break; // Ok, on continue
        else {
            ModalConfig cfg;
            cfg.title = "REEMaker - Erreur";
            cfg.message = "Il est <b>impossible d'écrire</b> dans le fichier que vous avez "
                          "sélectionné.<br>Merci de sélectionner un autre emplacement ou "
                          "abandonner l'opération.";
            cfg.icon = OverlayBlurWidget::iconFromStandard(QStyle::SP_MessageBoxCritical, this);
            cfg.buttons = { OverlayBlurWidget::makeButton(1, tr("Abandonner"), QDialogButtonBox::RejectRole, false, false),
                            OverlayBlurWidget::makeButton(0, tr("Sélectionner un autre emplacement"), QDialogButtonBox::AcceptRole, true, true) };
            cfg.clickOutsideToClose = false;
            cfg.escapeButtonId = -1; // ou ne pas le renseigner du tout

            int ret = overlay->execModal(cfg);
            if (ret == 1 /*Abandonner*/) {
                DEPOSE_OVERLAY_BLUR;
                return;
            } else
                continue;
        }
    }
    EcrireDansIni("ProcedureRepare", QFileInfo(PDF2Repair).dir().path());

    /// Creation d'un fichier temporaire en local pour bypasser le path > 260
    /// caractères
    auto ExtendedSource = getExtendedPath(QDir::toNativeSeparators(PDF2Repair));
    QString tempPDF = CheminTemp + "/temp_" + QString::fromStdString(generate_random_64bit_hex()) + ".pdf";
    copyFileWithProgress(ExtendedSource, tempPDF);

    qint16 NombrePage = PDFInfoNombrePage(tempPDF);
    auto CodeErreur = RepareGhostScript(tempPDF, NombrePage, false);

    if (CodeErreur == ValeurRetour::Succes)
        copyFileWithProgress(tempPDF, NewPathToSave);
    else {
        ModalConfig cfg;
        cfg.title = "Réparer un fichier PDF - Erreur à la réparation";
        cfg.message = "Le document PDF n'a pu être réparé, abandon...";
        cfg.icon = OverlayBlurWidget::iconFromStandard(QStyle::SP_MessageBoxCritical, this);
        cfg.buttons = { OverlayBlurWidget::makeButton(0, "Abandon", QDialogButtonBox::AcceptRole, true, true) };
        cfg.clickOutsideToClose = false;
        cfg.escapeButtonId = -1;
        overlay->execModal(cfg);
    }

    try {
        QFile::remove(tempPDF);
    } catch (...) {
    }
    {
        ModalConfig cfg;
        cfg.title = "Réparer un fichier PDF - Opération terminée";
        cfg.message = "La réparation est terminée avec succès...";
        cfg.icon = OverlayBlurWidget::iconFromStandard(QStyle::SP_MessageBoxInformation, this);
        cfg.buttons = { OverlayBlurWidget::makeButton(0, "Continuer", QDialogButtonBox::AcceptRole, true, true) };
        cfg.clickOutsideToClose = false;
        cfg.escapeButtonId = -1;
        overlay->execModal(cfg);
    }
    DEPOSE_OVERLAY_BLUR;
}

void
MainWindow::on_tool_extract_images_search_button_clicked()
{
    POSE_OVERLAY_BLUR;

    QString PDF2Extract = QFileDialog::getOpenFileName(this,
                                                       "Extraire du contenu d'un fichier PDF en image - Sélectionner le fichier PDF",
                                                       LireDansIni("ProcedureExtraireIMAGE", "").toString(),
                                                       "Fichier PDF (*.pdf)");
    if (PDF2Extract == "") {
        DEPOSE_OVERLAY_BLUR;
        return;
    }
    EcrireDansIni("ProcedureExtraireIMAGE", QFileInfo(PDF2Extract).dir().path());
    ui->tool_extract_images_pdffile->setText(PDF2Extract);
    ui->tool_frame_extract_image->setEnabled(true);

    qint16 NBPage = PDFInfoNombrePage(PDF2Extract);

    ui->tool_extract_images_from->setMaximum(NBPage);
    ui->tool_extract_images_to->setMaximum(NBPage);
    ui->tool_extract_images_from->setValue(1);
    ui->tool_extract_images_to->setValue(NBPage);
    DEPOSE_OVERLAY_BLUR;
}

void
MainWindow::on_tool_extract_images_run_button_clicked()
{

    QString Dossier = LireDansIni("DossierExtractionImage", "").toString();

    QString PathOutputImage = "";
    {
        POSE_OVERLAY_BLUR;
        while (1) {
            PathOutputImage = QFileDialog::getExistingDirectory(this,
                                                                "Extraire du contenu d'un fichier PDF en Image - Dossier ou seront "
                                                                "extrait les images",
                                                                Dossier);
            if (PathOutputImage.isEmpty()) {
                DEPOSE_OVERLAY_BLUR;
                return;
            }

            auto retour = isDirectoryWritable(PathOutputImage);
            if (retour)
                break; // Ok, on continue
            else {
                ModalConfig cfg;
                cfg.title = "REEMaker - Erreur";
                cfg.message = "Il est <b>impossible d'écrire</b> dans le dossier que vous avez sélectionné.<br>Merci de sélectionner un autre "
                              "emplacement ou abandonner l'opération.";
                cfg.icon = OverlayBlurWidget::iconFromStandard(QStyle::SP_MessageBoxCritical, this);
                cfg.buttons = { OverlayBlurWidget::makeButton(0, "Sélectionner un autre emplacement", QDialogButtonBox::AcceptRole, true, true),
                                OverlayBlurWidget::makeButton(1, "Abandonner", QDialogButtonBox::RejectRole, false, true) };
                cfg.clickOutsideToClose = false;
                cfg.escapeButtonId = -1;
                int ret = overlay->execModal(cfg);
                if (ret == 1 /*Abandonner*/) {
                    DEPOSE_OVERLAY_BLUR;
                    return;
                } else
                    continue;
            }
        }
        DEPOSE_OVERLAY_BLUR;
    }

    EcrireDansIni("DossierExtractionImage", PathOutputImage);
    auto sav_path = PathOutputImage;

    PathOutputImage += "/extract_" + QFileInfo(ui->tool_extract_images_pdffile->text()).completeBaseName() + "_";

    // On travail avec un fichier temp court
    auto ExtendedSource = getExtendedPath(QDir::toNativeSeparators(ui->tool_extract_images_pdffile->text()));
    QString tempPDF = CheminTemp + "/temp_" + QString::fromStdString(generate_random_64bit_hex()) + ".pdf";
    copyFileWithProgress(ExtendedSource, tempPDF);

    qint64 PageDebut = ui->tool_extract_images_from->value();
    qint64 PageFin = ui->tool_extract_images_to->value();

    auto procPPM = new QProcess();
    procPPM->setWorkingDirectory(QFileInfo(CheminPoppler).path());
    procPPM->setProgram(CheminPoppler);
    QString format_fichier = "-png";
    if (ui->tool_extract_images_format->currentText().toUpper() == "JPEG")
        format_fichier = "-jpeg";
    if (ui->tool_extract_images_format->currentText().toUpper() == "PNG")
        format_fichier = "-png";
    if (ui->tool_extract_images_format->currentText().toUpper() == "TIFF")
        format_fichier = "-tiff";
    procPPM->setArguments(
      { "-f", QString::number(PageDebut), "-l", QString::number(PageFin), "-q", "-r", "300", format_fichier, tempPDF, PathOutputImage });
    auto* po = ProgressOverlay::showDeterminate(this, "Génération des images...", PageFin - PageDebut + 1, true, true, 140);
    po->enableBackdropBlur(true, 4.0, 0.5);
    QCoreApplication::processEvents();

    procPPM->start();
    while (procPPM->state() != QProcess::NotRunning) {
        QThread::msleep(250);
        QCoreApplication::processEvents();
        QString lPath = QFileInfo(PathOutputImage + ".").path();
        QDir directory(lPath);
        QString BaseSearch = QFileInfo(PathOutputImage).baseName();
        QStringList images = directory.entryList(QStringList() << "*.*", QDir::Files, QDir::Name | QDir::IgnoreCase);
        uint64_t img_found = 0;
        for (auto& var : images) {
            if (var.contains(BaseSearch))
                img_found++;
        }
        po->setText(QString("Génération image %1/%2").arg(img_found).arg(PageFin - PageDebut + 1));
        po->setValue(img_found);
    }
    {
        POSE_OVERLAY_BLUR;
        ModalConfig cfg;
        cfg.title = "Extraire du contenu d'un fichier PDF en image - Fin de l'opération";
        cfg.message = "L'extraction des images est terminée.\nOuverture du dossier de sortie";
        cfg.icon = OverlayBlurWidget::iconFromStandard(QStyle::SP_MessageBoxInformation, this);
        cfg.buttons = { OverlayBlurWidget::makeButton(0, "Continuer", QDialogButtonBox::AcceptRole, true, true) };
        cfg.clickOutsideToClose = false;
        cfg.escapeButtonId = -1;
        overlay->execModal(cfg);
        DEPOSE_OVERLAY_BLUR;
    }
    QFile::remove(tempPDF);
    ui->tool_extract_images_pdffile->setText("");
    ui->tool_frame_extract_image->setEnabled(false);
    po->close();
    QUrl folderUrl = QUrl::fromLocalFile(sav_path);
    // Ouvre le dossier dans l'explorateur
    QDesktopServices::openUrl(folderUrl);
}

void
MainWindow::on_tool_extract_pdf_search_clicked()
{
    POSE_OVERLAY_BLUR;
    QString PDF2Extract = QFileDialog::getOpenFileName(this,
                                                       "Extraire du contenu d'un fichier PDF en PDF - Sélectionner le fichier PDF",
                                                       LireDansIni("ProcedureExtrairePDF", "").toString(),
                                                       "Fichier PDF (*.pdf)");
    if (PDF2Extract == "") {
        DEPOSE_OVERLAY_BLUR;
        return;
    }
    EcrireDansIni("ProcedureExtrairePDF", QFileInfo(PDF2Extract).dir().path());
    ui->tool_extract_pdf_pdffile->setText(PDF2Extract);
    ui->tool_frame_extract_pdf->setEnabled(true);

    qint16 NBPage = PDFInfoNombrePage(PDF2Extract);

    ui->tool_extract_pdf_from->setMaximum(NBPage);
    ui->tool_extract_pdf_to->setMaximum(NBPage);
    ui->tool_extract_pdf_from->setValue(1);
    ui->tool_extract_pdf_to->setValue(NBPage);
    DEPOSE_OVERLAY_BLUR;
}

void
MainWindow::on_tool_extract_pdf_run_button_clicked()
{
    QString FichierSource = ui->tool_extract_pdf_pdffile->text();

    QString PathOutputPDF = "";
    {
        POSE_OVERLAY_BLUR;
        while (1) {
            PathOutputPDF =
              QFileDialog::getExistingDirectory(this,
                                                "Dossier ou seront extrait les pages en PDF",
                                                LireDansIni("DossierExtractionPDF", QVariant(QFileInfo(FichierSource).path())).toString());
            if (PathOutputPDF.isEmpty()) {
                DEPOSE_OVERLAY_BLUR;
                return;
            }
            auto retour = isDirectoryWritable(PathOutputPDF);
            if (retour)
                break; // Ok, on continue
            else {
                ModalConfig cfg;
                cfg.title = "REEMaker - Erreur";
                cfg.message = "Il est <b>impossible d'écrire</b> dans le dossier que vous avez sélectionné.<br>Merci de sélectionner un autre "
                              "emplacement ou abandonner l'opération.";
                cfg.icon = OverlayBlurWidget::iconFromStandard(QStyle::SP_MessageBoxCritical, this);
                cfg.buttons = { OverlayBlurWidget::makeButton(0, "Sélectionner un autre emplacement", QDialogButtonBox::AcceptRole, true, true),
                                OverlayBlurWidget::makeButton(1, "Abandonner", QDialogButtonBox::RejectRole, false, true) };
                cfg.clickOutsideToClose = false;
                cfg.escapeButtonId = -1;
                int ret = overlay->execModal(cfg);
                if (ret == 1 /*Abandonner*/) {
                    DEPOSE_OVERLAY_BLUR;
                    return;
                } else
                    continue;
            }
        }
        DEPOSE_OVERLAY_BLUR;
    }

    EcrireDansIni("DossierExtractionPDF", PathOutputPDF);

    // On travail avec un fichier temp court
    auto ExtendedSource = getExtendedPath(QDir::toNativeSeparators(FichierSource));
    QString tempPDF = CheminTemp + "/temp_" + QString::fromStdString(generate_random_64bit_hex()) + ".pdf";
    copyFileWithProgress(ExtendedSource, tempPDF);

    if (ui->tool_extract_pdf_option_multipage->isChecked()) {
        ///
        /// Extraction en multi page
        ///
        QString CheminBaseGhostScript = QCoreApplication::applicationDirPath() + "/GhostScript/";
        auto* po = ProgressOverlay::showIndeterminate(this, "Extraction avec GhostScript...", true, true, 140);
        po->enableBackdropBlur(true, 4.0, 0.5);
        QCoreApplication::processEvents();
        auto procGhost = new QProcess();
        QString Windir = "";
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        Windir = env.value("windir");
        env.insert("GS_LIB", QDir::toNativeSeparators(CheminBaseGhostScript) + "lib\\");
        env.insert("GS_DLL", QDir::toNativeSeparators(CheminBaseGhostScript) + "bin\\");
        env.insert("GS_BIN", QDir::toNativeSeparators(CheminBaseGhostScript) + "bin\\");
        env.insert("GS_RESOURCE", QDir::toNativeSeparators(CheminBaseGhostScript) + "resource\\init");
        env.insert("GS_FONTPATH", Windir + "\\fonts");
        procGhost->setProcessEnvironment(env);
        QString BaseSource = CheminTemp + "/temp_" + QString::fromStdString(generate_random_64bit_hex()) + "_";
        QString tempPDFGS = BaseSource + "%05d.pdf";

        QString ConstructedArguments =
          QString("-dBATCH -dNOPAUSE -dSHORTERRORS -sDEVICE=pdfwrite "
                  "-dPDFSETTINGS=/prepress %4 -I\"%1\" "
                  "-sOutputFile=\"%2\" \"%3\"")
            .arg(CheminBaseGhostScript + "resource/init",
                 tempPDFGS,
                 tempPDF,
                 QString("-dFirstPage=%1 -dLastPage=%2")
                   .arg(QString::number(ui->tool_extract_pdf_from->value()), QString::number(ui->tool_extract_pdf_to->value())));
        procGhost->setNativeArguments(ConstructedArguments);
        procGhost->start(CheminBaseGhostScript + "bin/gswin64c.exe");
        qDebug() << "Multi page : On démarre de la page " << ui->tool_extract_pdf_from->value() << " à la page " << ui->tool_extract_pdf_to->value();
        while (procGhost->state() != QProcess::NotRunning) {
            auto Lecture = QString(procGhost->readAllStandardOutput()).split("\n", Qt::SkipEmptyParts);
            if (Lecture.count() > 0)
                if (Lecture.last().size() < 20)
                    po->setText("GhostScript > " + Lecture.last().trimmed());
            QThread::msleep(25);
            QCoreApplication::processEvents();
        }
        QFile::remove(tempPDFGS);
        /// Maintenant on recopie en définitifs
        int NombrePagesTotal = ui->tool_extract_pdf_to->value() - ui->tool_extract_pdf_from->value() + 1;
        po->close();
        auto* poD = ProgressOverlay::showDeterminate(
          this, "Transfert page " + QString::number(ui->tool_extract_pdf_from->value()), NombrePagesTotal, true, true, 140);
        poD->enableBackdropBlur(true, 4.0, 0.5);
        QCoreApplication::processEvents();

        for (int i = 0; i < NombrePagesTotal; ++i) {
            poD->setValue(i);
            poD->setText("Transfert page " + QString::number(ui->tool_extract_pdf_from->value() + i));
            QCoreApplication::processEvents();
            QString Source = BaseSource + QString::asprintf("%0*d", 5, i + 1) + ".pdf";
            QString Destination = PathOutputPDF + "/" +
                                  QString("[page %1] ").arg(QString::asprintf("%0*d", 5, ui->tool_extract_pdf_from->value() + i)) +
                                  QFileInfo(FichierSource).fileName();

            QFile::copy(Source, Destination);
            QFile::remove(Source);
        }
        poD->close();
    } else {
        /// Extraction en un bloc
        QString CheminBaseGhostScript = QCoreApplication::applicationDirPath() + "/GhostScript/";
        auto* po = ProgressOverlay::showIndeterminate(this, "Extraction avec GhostScript...", true, true, 140);
        po->enableBackdropBlur(true, 4.0, 0.5);
        QCoreApplication::processEvents();
        auto procGhost = new QProcess();
        QString Windir = "";
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        Windir = env.value("windir");
        env.insert("GS_LIB", QDir::toNativeSeparators(CheminBaseGhostScript) + "lib\\");
        env.insert("GS_DLL", QDir::toNativeSeparators(CheminBaseGhostScript) + "bin\\");
        env.insert("GS_BIN", QDir::toNativeSeparators(CheminBaseGhostScript) + "bin\\");
        env.insert("GS_RESOURCE", QDir::toNativeSeparators(CheminBaseGhostScript) + "resource\\init");
        env.insert("GS_FONTPATH", Windir + "\\fonts");
        procGhost->setProcessEnvironment(env);
        QString fileName =
          PathOutputPDF + "/" +
          QString("[%1-%2] ")
            .arg(QString::asprintf("%0*d", 4, ui->tool_extract_pdf_from->value()), QString::asprintf("%0*d", 4, ui->tool_extract_pdf_to->value())) +
          QFileInfo(FichierSource).fileName();

        QString ConstructedArguments =
          QString("-dBATCH -dNOPAUSE -dSHORTERRORS -sDEVICE=pdfwrite "
                  "-dPDFSETTINGS=/prepress %4 -I\"%1\" "
                  "-sOutputFile=\"%2\" \"%3\"")
            .arg(CheminBaseGhostScript + "resource/init",
                 fileName,
                 tempPDF,
                 QString("-dFirstPage=%1 -dLastPage=%2")
                   .arg(QString::number(ui->tool_extract_pdf_from->value()), QString::number(ui->tool_extract_pdf_to->value())));
        procGhost->setNativeArguments(ConstructedArguments);
        procGhost->start(CheminBaseGhostScript + "bin/gswin64c.exe");
        while (procGhost->state() != QProcess::NotRunning) {
            auto Lecture = QString(procGhost->readAllStandardOutput()).split("\n", Qt::SkipEmptyParts);
            if (Lecture.count() > 0)
                if (Lecture.last().size() < 20)
                    po->setText("GhostScript > " + Lecture.last().trimmed());
            QThread::msleep(25);
            QCoreApplication::processEvents();
        }
        po->close();
    }
    {
        POSE_OVERLAY_BLUR;
        ModalConfig cfg;
        cfg.title = "Extraire du contenu d'un fichier PDF en PDF - Opération terminée";
        cfg.message = "L'opération d'extraction est terminée";
        cfg.icon = OverlayBlurWidget::iconFromStandard(QStyle::SP_MessageBoxInformation, this);
        cfg.buttons = { OverlayBlurWidget::makeButton(0, "Continuer", QDialogButtonBox::AcceptRole, true, true) };
        cfg.clickOutsideToClose = false;
        cfg.escapeButtonId = -1;
        overlay->execModal(cfg);
        DEPOSE_OVERLAY_BLUR;
    }
    /// Fin
    ui->tool_extract_pdf_pdffile->setText("");
    ui->tool_frame_extract_pdf->setEnabled(false);
    QFile::remove(tempPDF);
}

void
MainWindow::on_tool_merge_add_clicked()
{
    QString filter = "Fusionner du contenu dans un fichier PDF - Fichiers compatibles (*.pdf *.png *.jpg *.jpeg *.tiff *.webp *.bmp)";
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Sélectionner des fichiers", LireDansIni("ChercheItemFusion", "").toString(), filter);

    if (fileNames.isEmpty())
        return;
    EcrireDansIni("ChercheItemFusion", QFileInfo(fileNames.first()).dir().path());

    auto* po = ProgressOverlay::showDeterminate(this, "Analyse des fichiers...", fileNames.count(), true, true, 140);
    po->enableBackdropBlur(true, 4.0, 0.5);
    qint16 loop = 0;

    foreach (auto var, fileNames) {
        loop++;
        po->setValue(loop);
        po->setText("Fichier >" + QFileInfo(var).fileName());
        QThread::msleep(10);
        QApplication::processEvents();
        // Crée le QListWidgetItem
        QListWidgetItem* item = new QListWidgetItem(ui->toolmerge_listv2);
        // Crée votre widget personnalisé
        CustomListItemWidget* itemWidget = new CustomListItemWidget(var);
        if (itemWidget->getExtension() == CustomListItemWidget::typeFichier::PDF) {
            // Copie local
            auto ExtendedSource = getExtendedPath(QDir::toNativeSeparators(var));
            QString tempPDF = CheminTemp + "/temp_" + QString::fromStdString(generate_random_64bit_hex()) + ".pdf";
            copyFileWithProgress(ExtendedSource, tempPDF, false);
            qint16 NombrePage = PDFInfoNombrePage(tempPDF);
            itemWidget->setStartPageMax(NombrePage);
            itemWidget->setEndPageMax(NombrePage);
            itemWidget->setEndPage(NombrePage);
            QFile::remove(tempPDF);
        }
        // Ajoute le widget à l'item
        ui->toolmerge_listv2->setItemWidget(item, itemWidget);
        // Ajuste la taille de l'item pour qu'elle corresponde au widget
        item->setSizeHint(itemWidget->sizeHint());
    }
    po->close();
    if (FirstMergeAdd) {
        POSE_OVERLAY_BLUR;
        FirstMergeAdd = false;
        ModalConfig cfg;
        cfg.title = "Réorganiser l'ordre des documents et les pages";
        cfg.message = "Vous pouvez réorganiser les documents en les déplacant par drag'n'drop dans la liste.\n"
                      "Pour les documents PDF, vous pouvez aussi définir l'étendue des pages à utiliser pour la fusion.\n"
                      "Important : Vous pouvez ajouter plusieurs fois le même document en prenant des sections spécifiques du documents.\n"
                      "De cette manière, on peut ajouter des documents entres d'autres documents ou aussi en dupliquant des pages.\n"
                      "\n\nCe message n'apparaitra qu'une seul fois";
        cfg.icon = OverlayBlurWidget::iconFromStandard(QStyle::SP_MessageBoxInformation, this);
        cfg.buttons = { OverlayBlurWidget::makeButton(0, "Continuer", QDialogButtonBox::AcceptRole, true, true) };
        cfg.clickOutsideToClose = false;
        cfg.escapeButtonId = -1;
        overlay->execModal(cfg);
        DEPOSE_OVERLAY_BLUR;
    }
}

void
MainWindow::on_tool_merge_remove_clicked()
{
    QListWidgetItem* currentItem = ui->toolmerge_listv2->currentItem();
    if (currentItem) {
        delete ui->toolmerge_listv2->takeItem(ui->toolmerge_listv2->currentRow());
    }
}

void
MainWindow::on_tool_merge_clear_clicked()
{
    ui->toolmerge_listv2->clear();
}

void
MainWindow::on_tool_merge_saveas_clicked()
{
    if (ui->toolmerge_listv2->count() == 0)
        return;

    QString PDFFusionner = "";
    {
        POSE_OVERLAY_BLUR;
        while (1) {
            PDFFusionner = QFileDialog::getSaveFileName(this,
                                                        "Fusionner du contenu dans un fichier PDF - Emplacement du "
                                                        "fichier PDF fusionné",
                                                        LireDansIni("FichierPdfFusionner", "").toString(),
                                                        "Fichier PDF (*.pdf)");
            if (PDFFusionner.isEmpty()) {
                DEPOSE_OVERLAY_BLUR;
                return;
            }
            qDebug() << "Test sélection :" << PDFFusionner;
            auto retour = canWriteToFile(PDFFusionner);
            if (retour == CanWriteFile::Yes_I_Can)
                break; // Ok, on continue
            else {
                ModalConfig cfg;
                cfg.title = "REEMaker - Erreur";
                cfg.message = "Il est <b>impossible d'écrire</b> dans le dossier que vous avez sélectionné.<br>Merci de sélectionner un autre "
                              "emplacement ou abandonner l'opération.";
                cfg.icon = OverlayBlurWidget::iconFromStandard(QStyle::SP_MessageBoxCritical, this);
                cfg.buttons = { OverlayBlurWidget::makeButton(0, "Sélectionner un autre emplacement", QDialogButtonBox::AcceptRole, true, true),
                                OverlayBlurWidget::makeButton(1, "Abandonner", QDialogButtonBox::RejectRole, false, true) };
                cfg.clickOutsideToClose = false;
                cfg.escapeButtonId = -1;
                int ret = overlay->execModal(cfg);
                if (ret == 1 /*Abandonner*/) {
                    DEPOSE_OVERLAY_BLUR;
                    return;
                } else
                    continue;
            }
        }
        EcrireDansIni("FichierPdfFusionner", QFileInfo(PDFFusionner).dir().path());
        DEPOSE_OVERLAY_BLUR;
    }

    ImageConverter MyConverter(this);

    auto* po = ProgressOverlay::showDeterminate(this, "Préparation des fichiers...", ui->toolmerge_listv2->count(), true, true, 140);

    po->enableBackdropBlur(true, 4.0, 0.5);
    QCoreApplication::processEvents();

    QStringList ListeFichiers;
    QString BasetempPDF = CheminTemp + "/merge_" + QString::fromStdString(generate_random_64bit_hex()) + "_";
    /// PSEUDO
    for (int i = 0; i < ui->toolmerge_listv2->count(); ++i) {
        po->setValue(i + 1);

        QListWidgetItem* currentItem = ui->toolmerge_listv2->item(i);
        CustomListItemWidget* itemWidget = qobject_cast<CustomListItemWidget*>(ui->toolmerge_listv2->itemWidget(currentItem));

        QString Destination = BasetempPDF + QString::asprintf("%0*d", 5, i) + ".pdf";

        QString _Extension = itemWidget->getExtensionText().toUpper().replace(".", "");
        QString _FilePath = itemWidget->getFullPath();
        qint16 _FStart = itemWidget->getStartPage();
        qint16 _FEnd = itemWidget->getEndPage();
        qint16 _FEndMax = itemWidget->getEndPageMax();
        QFileInfo fileInfo(_FilePath);
        QString fileNameShort = fileInfo.fileName();
        const int ReduceTo = 20;
        if (fileNameShort.length() > ReduceTo) {
            fileNameShort = "..." + fileNameShort.right(ReduceTo - 3);
        }
        po->setText("Préparation de " + fileNameShort);
        QThread::msleep(5);
        QApplication::processEvents();
        if (_Extension == "PDF") { // SI PDF
            auto ExtendedSource = getExtendedPath(QDir::toNativeSeparators(_FilePath));
            if (_FStart != 1 || _FEnd != _FEndMax) // Extraction partiel
            {
                QString PDFinwork = CheminTemp + "/merge_" + QString::fromStdString(generate_random_64bit_hex()) + ".pdf";
                copyFileWithProgress(ExtendedSource, PDFinwork, false);
                /// Extraction en un bloc partiel
                QString CheminBaseGhostScript = QCoreApplication::applicationDirPath() + "/GhostScript/";
                QCoreApplication::processEvents();
                auto procGhost = new QProcess();
                QString Windir = "";
                QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
                Windir = env.value("windir");
                env.insert("GS_LIB", QDir::toNativeSeparators(CheminBaseGhostScript) + "lib\\");
                env.insert("GS_DLL", QDir::toNativeSeparators(CheminBaseGhostScript) + "bin\\");
                env.insert("GS_BIN", QDir::toNativeSeparators(CheminBaseGhostScript) + "bin\\");
                env.insert("GS_RESOURCE", QDir::toNativeSeparators(CheminBaseGhostScript) + "resource\\init");
                env.insert("GS_FONTPATH", Windir + "\\fonts");
                procGhost->setProcessEnvironment(env);
                QString ConstructedArguments = QString("-dBATCH -dNOPAUSE -dSHORTERRORS -sDEVICE=pdfwrite "
                                                       "-dPDFSETTINGS=/prepress %4 -I\"%1\" "
                                                       "-sOutputFile=\"%2\" \"%3\"")
                                                 .arg(CheminBaseGhostScript + "resource/init",
                                                      Destination,
                                                      PDFinwork,
                                                      QString("-dFirstPage=%1 -dLastPage=%2").arg(QString::number(_FStart), QString::number(_FEnd)));
                procGhost->setNativeArguments(ConstructedArguments);
                procGhost->start(CheminBaseGhostScript + "bin/gswin64c.exe");
                while (procGhost->state() != QProcess::NotRunning) {
                    QCoreApplication::processEvents();
                    QThread::msleep(25);
                    // Lecture
                    auto Lecture = QString(procGhost->readAllStandardOutput()).split("\n", Qt::SkipEmptyParts);
                    if (Lecture.count() > 0)
                        if (Lecture.last().trimmed().size() < 15)
                            po->setText("Préparation de " + fileNameShort + " > " + Lecture.last().trimmed());
                }
                ListeFichiers.append(Destination);
                QFile::remove(PDFinwork);
            } else {
                copyFileWithProgress(ExtendedSource, Destination, false);
                ListeFichiers.append(Destination);
            }
        } else { // SI IMAGE
            QString imgTemp = "";
            if (_Extension.endsWith("WEBP") || _Extension.endsWith("BMP") || _Extension.endsWith("TIFF")) {
                imgTemp = BasetempPDF + QString::fromStdString(generate_random_64bit_hex()) + ".png";
                // Convertir
                QImage image;
                if (!image.load(_FilePath)) {
                    qDebug() << "Erreur : Impossible de charger le fichier.";
                    continue;
                }
                if (!image.save(imgTemp, "PNG")) {
                    qDebug() << "Erreur : Impossible de sauvegarder l'image au format PNG.";
                    continue;
                }
            } else if (_Extension.endsWith("JPG") || _Extension.endsWith("JPEG")) {
                imgTemp = BasetempPDF + QString::fromStdString(generate_random_64bit_hex()) + ".jpg";
                copyFileWithProgress(_FilePath, imgTemp, false);
            } else if (_Extension.endsWith("PNG")) {
                imgTemp = BasetempPDF + QString::fromStdString(generate_random_64bit_hex()) + ".png";
                copyFileWithProgress(_FilePath, imgTemp, false);
            }
            MyConverter.convertImageToPdf(imgTemp, Destination);
            QFile::remove(imgTemp);
            ListeFichiers.append(Destination);
        }
    }
    /// ECRIRE LISTEFichiers dans TEMP.ListeTEXT
    QFile file(BasetempPDF + ".txt");
    {
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            po->close();
            POSE_OVERLAY_BLUR;
            ModalConfig cfg;
            cfg.title = "Fusionner du contenu dans un fichier PDF - Erreur";
            cfg.message = "Impossible d'ouvrir le fichier temporaire de liste de fusion en écriture, abandon.";
            cfg.icon = OverlayBlurWidget::iconFromStandard(QStyle::SP_MessageBoxCritical, this);
            cfg.buttons = { OverlayBlurWidget::makeButton(0, "Abandonner", QDialogButtonBox::AcceptRole, true, true) };
            cfg.clickOutsideToClose = false;
            cfg.escapeButtonId = -1;
            overlay->execModal(cfg);
            DEPOSE_OVERLAY_BLUR;
            return;
        } else {
            QTextStream out(&file);
            for (const QString& item : ListeFichiers) {
                out << "\"" << item << "\"\n";
            }
            file.close();
        }
        po->close();
    }

    /// GHOSTSCRIPT
    {
        ProgressOverlay* poI = ProgressOverlay::showIndeterminate(this, "Ghostscript > Fusion des fichiers préparés...", true, true, 140);
        poI->enableBackdropBlur(true, 4.0, 0.5);
        QCoreApplication::processEvents();

        QString CheminBaseGhostScript = QCoreApplication::applicationDirPath() + "/GhostScript/";
        auto procGhost = new QProcess();
        QString Windir = "";
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        Windir = env.value("windir");
        env.insert("GS_LIB", QDir::toNativeSeparators(CheminBaseGhostScript) + "lib\\");
        env.insert("GS_DLL", QDir::toNativeSeparators(CheminBaseGhostScript) + "bin\\");
        env.insert("GS_BIN", QDir::toNativeSeparators(CheminBaseGhostScript) + "bin\\");
        env.insert("GS_RESOURCE", QDir::toNativeSeparators(CheminBaseGhostScript) + "resource\\init");
        env.insert("GS_FONTPATH", Windir + "\\fonts");
        procGhost->setProcessEnvironment(env);
        QString ConstructedArguments = QString("-dBATCH -dNOPAUSE -dSHORTERRORS -sDEVICE=pdfwrite "
                                               "-dPDFSETTINGS=/prepress -I\"%1\" "
                                               "-sOutputFile=\"%2\" @\"%3\"")
                                         .arg(CheminBaseGhostScript + "resource/init", PDFFusionner, BasetempPDF + ".txt");
        procGhost->setNativeArguments(ConstructedArguments);
        procGhost->start(CheminBaseGhostScript + "bin/gswin64c.exe");
        while (procGhost->state() != QProcess::NotRunning) {
            QThread::msleep(25);
            QCoreApplication::processEvents();
        }
        poI->close();
    }
    QFile::remove(BasetempPDF + ".txt");
    foreach (auto fichier, ListeFichiers) {
        QFile::remove(fichier);
    }
    {
        POSE_OVERLAY_BLUR;
        ModalConfig cfg;
        cfg.title = "Fusionner du contenu dans un fichier PDF - Opération terminée";
        cfg.message = "L'opération de fusion est terminée.<br>Ouverture du dossier contenant le PDF fusionnée.";
        cfg.icon = OverlayBlurWidget::iconFromStandard(QStyle::SP_MessageBoxInformation, this);
        cfg.buttons = { OverlayBlurWidget::makeButton(0, "Continuer", QDialogButtonBox::AcceptRole, true, true) };
        cfg.clickOutsideToClose = false;
        cfg.escapeButtonId = -1;
        overlay->execModal(cfg);
        DEPOSE_OVERLAY_BLUR;
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(PDFFusionner).path()));
    }
}

int
MainWindow::showDialogCustom(QString Titre, QString Message, QMessageBox::Icon Icone, QList<QString> ListeBouton, int idDefautButton)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(Titre);
    msgBox.setText(Message);
    msgBox.setIcon(Icone);
    msgBox.setTextFormat(Qt::TextFormat::RichText);
    QList<QPushButton*> LstPtrBouton;
    for (auto& btnText : ListeBouton) {
        LstPtrBouton.append(msgBox.addButton(btnText, QMessageBox::ActionRole));
    }
    msgBox.setDefaultButton(LstPtrBouton[idDefautButton]);
    msgBox.exec();
    int Retour = -1;
    for (int var = 0; var < LstPtrBouton.size(); ++var) {
        if (LstPtrBouton[var] == msgBox.clickedButton()) {
            Retour = var;
            break;
        }
    }
    return Retour;
}

void
MainWindow::on_BasicBouton_clicked()
{
    switch (ui->BasiqueListe->currentIndex()) {
        case 0: // Texte to Upper
            ui->txtEditSortieBasiques->setPlainText(TextUtils::toUpper(ui->txtEditSource->toPlainText()));
            break;
        case 1: // Texte to lower
            ui->txtEditSortieBasiques->setPlainText(TextUtils::toLower(ui->txtEditSource->toPlainText()));
            break;
        case 2: // Switch case
            ui->txtEditSortieBasiques->setPlainText(TextUtils::toggleCase(ui->txtEditSource->toPlainText()));
            break;
        case 3: // Capitalize Words
            ui->txtEditSortieBasiques->setPlainText(TextUtils::capitalizeWords(ui->txtEditSource->toPlainText()));
            break;
        case 4: // Capitalize Sentence
            ui->txtEditSortieBasiques->setPlainText(TextUtils::capitalizeSentences(ui->txtEditSource->toPlainText()));
            break;
        case 5: // Reverse Texte
            ui->txtEditSortieBasiques->setPlainText(TextUtils::reverseText(ui->txtEditSource->toPlainText()));
            break;
        case 6: // Remove Dyacritics
            ui->txtEditSortieBasiques->setPlainText(TextUtils::removeDiacritics(ui->txtEditSource->toPlainText()));
            break;
        default:
            break;
    }
}

void
MainWindow::on_btnMEFTrimTexte_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    QString Result = TextUtils::trim(Source);
    ui->txtEditMiseEnFormeSorties->setPlainText(Result);
}

void
MainWindow::on_btnMEFTrimLigne_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    QString Result = TextUtils::trimLines(Source);
    ui->txtEditMiseEnFormeSorties->setPlainText(Result);
}

void
MainWindow::on_btnMEFCompress_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    QString Result = TextUtils::squeezeSpaces(Source);
    ui->txtEditMiseEnFormeSorties->setPlainText(Result);
}

void
MainWindow::on_btnMEFNormalize_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    QString Result = TextUtils::normalizeSpaces(Source);
    ui->txtEditMiseEnFormeSorties->setPlainText(Result);
}

void
MainWindow::on_btnMEFTab2Space_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    QString Result = TextUtils::tabsToSpaces(Source, ui->MEFTab2Space_spin->value());
    ui->txtEditMiseEnFormeSorties->setPlainText(Result);
}

void
MainWindow::on_btnMEFSpace2Tab_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    QString Result = TextUtils::spacesToTabs(Source, ui->MEFSpace2Tabs_spin->value());
    ui->txtEditMiseEnFormeSorties->setPlainText(Result);
}

void
MainWindow::on_btnMEFWrapText_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    QString Result = TextUtils::wrapText(Source, ui->MEFWrapText_spin->value(), ui->MEFWrapText_Checkbox->isChecked());
    ui->txtEditMiseEnFormeSorties->setPlainText(Result);
}

void
MainWindow::on_btnMEFIndent_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    QString Result = TextUtils::indent(Source, ui->MEFIndent_spin->value());
    ui->txtEditMiseEnFormeSorties->setPlainText(Result);
}

void
MainWindow::on_btnMEFUnindent_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    QString Result = TextUtils::unindent(Source, ui->MEFUnindent_spin->value());
    ui->txtEditMiseEnFormeSorties->setPlainText(Result);
}

void
MainWindow::on_btnMEFAlign_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    TextUtils::Align _align = TextUtils::Align::Left; // 0
    if (ui->MEFAlign_combo->currentIndex() == 1)
        _align = TextUtils::Align::Right;
    if (ui->MEFAlign_combo->currentIndex() == 2)
        _align = TextUtils::Align::Center;
    if (ui->MEFAlign_combo->currentIndex() == 3)
        _align = TextUtils::Align::Justify;
    QString Result = TextUtils::alignText(Source, _align, ui->MEFAlign_spin->value());
    ui->txtEditMiseEnFormeSorties->setPlainText(Result);
}

void
MainWindow::on_btnMEFJustify_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    QString Result = TextUtils::justifyLine(Source, ui->MEFJustify_spin->value());
    ui->txtEditMiseEnFormeSorties->setPlainText(Result);
}

void
MainWindow::on_btnNETT_SupLigneVide_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    QString Result = TextUtils::removeEmptyLines(Source);
    ui->txtEditNettoyage->setPlainText(Result);
}

void
MainWindow::on_btnNETT_SupLigneDouble_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    QString Result = TextUtils::removeDuplicateLines(
      Source, ui->NETT_SupLigneDouble_keepfirst->isChecked(), ui->NETT_DoubleLigneCasse->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);
    ui->txtEditNettoyage->setPlainText(Result);
}

void
MainWindow::on_btnNETT_SupCaractNonImprimable_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    QString Result = TextUtils::removeNonPrintable(Source);
    ui->txtEditNettoyage->setPlainText(Result);
}

void
MainWindow::on_btnNETT_NormaliserEOL_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    TextUtils::LineEnding _le = TextUtils::LineEnding::LF; // 0
    if (ui->NETT_NormaliserEOL_combo->currentIndex() == 1)
        _le = TextUtils::LineEnding::CRLF;
    if (ui->NETT_NormaliserEOL_combo->currentIndex() == 2)
        _le = TextUtils::LineEnding::CR;
    QString Result = TextUtils::normalizeLineEndings(Source, _le);
    ui->txtEditNettoyage->setPlainText(Result);
}

void
MainWindow::on_btnNETT_NormaliserEOLWhole_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    TextUtils::LineEnding _le = TextUtils::LineEnding::LF; // 0
    if (ui->NETT_NormaliserEOLWhole_combo->currentIndex() == 1)
        _le = TextUtils::LineEnding::CRLF;
    if (ui->NETT_NormaliserEOLWhole_combo->currentIndex() == 2)
        _le = TextUtils::LineEnding::CR;
    QString Result = TextUtils::ensureTrailingNewline(Source, _le);
    ui->txtEditNettoyage->setPlainText(Result);
}

void
MainWindow::on_btnLIGNESsplit_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    auto Result = TextUtils::splitLines(Source,
                                        ui->btnLIGNESsplit_separateur->text(),
                                        ui->btnLIGNESsplit_keepempty->isChecked(),
                                        ui->btnLIGNESsplit_casse->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive,
                                        ui->btnLIGNESsplit_trim->isChecked(),
                                        ui->btnLIGNESsplit_keepseparator->isChecked());
    ui->txtEditLIGNES->setPlainText(TextUtils::joinLines(Result));
}

void
MainWindow::on_btnLIGNESjoin_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    QString Result = TextUtils::joinLines(Source);
    ui->txtEditLIGNES->setPlainText(Result);
}

void
MainWindow::on_btnLIGNESsort_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    QString Result = TextUtils::sortLines(Source,
                                          ui->btnLIGNESsort_ascending->isChecked(),
                                          ui->btnLIGNESsort_doublons->isChecked(),
                                          ui->btnLIGNESsort_natural->isChecked(),
                                          ui->btnLIGNESsort_casse->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);
    ui->txtEditLIGNES->setPlainText(Result);
}

void
MainWindow::on_btnHTML_clicked()
{
    ui->txtHTML->setPlainText(TextUtils::stripHtmlTags(ui->txtEditSource->toPlainText()));
}

void
MainWindow::on_btnENCODEversbase64_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    QString Result = TextUtils::toBase64(Source);
    ui->txtENCODE->setPlainText(Result);
}

void
MainWindow::on_btnENCODEdebase64_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    QString Result = TextUtils::fromBase64(Source);
    ui->txtENCODE->setPlainText(Result);
}

void
MainWindow::on_btnENCODEmd5_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    ui->txtENCODE->setPlainText(TextUtils::hashMd5Hex(Source));
}

void
MainWindow::on_btnENCODEsha1_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    ui->txtENCODE->setPlainText(TextUtils::hashSha1Hex(Source));
}

void
MainWindow::on_btnEXTRACT_mail_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    auto Result = TextUtils::extractEmails(Source);
    ui->txtEXTRACT->setPlainText(Result.join("\n"));
}

void
MainWindow::on_btnEXTRACT_url_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    auto Result = TextUtils::extractUrls(Source);
    ui->txtEXTRACT->setPlainText(Result.join("\n"));
}

void
MainWindow::on_btnEXTRACT_tel_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    auto Result = TextUtils::extractPhones(Source);
    ui->txtEXTRACT->setPlainText(Result.join("\n"));
}

void
MainWindow::on_btnCHERCHE_recherche_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    auto Result = TextUtils::replace(Source,
                                     ui->btnCHERCHE_recherche_quoi->text(),
                                     ui->btnCHERCHE_recherche_par->text(),
                                     false,
                                     ui->btnCHERCHE_recherche_casse->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive,
                                     ui->btnCHERCHE_recherche_motentier->isChecked(),
                                     ui->btnCHERCHE_recherche_multiligne->isChecked(),
                                     false);
    ui->txtEditCHERCHE->setPlainText(Result);
}

void
MainWindow::on_btnCHERCHE_regex_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    QString Erreur = "";
    auto Result = TextUtils::regexReplace(Source,
                                          ui->CHERCHE_regex_formule->text(),
                                          ui->CHERCHE_regex_argument->text(),
                                          ui->CHERCHE_regex_casse->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive,
                                          true,
                                          false,
                                          &Erreur);
    ui->txtEditCHERCHE->setPlainText(Erreur.isEmpty() ? Result : Erreur);
}

void
MainWindow::on_btnCHERCHE_ajouter_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    QString Sequence = ui->CHERCHE_ajouter_quoi->text();
    bool AuDebut = ui->CHERCHE_ajouter_ou->currentIndex() == 0 ? true : false;
    bool Lignesvides = ui->CHERCHE_lignes_vides->isChecked();
    auto Split = TextUtils::splitLines(Source, Lignesvides);
    for (auto& var : Split)
        var = AuDebut ? Sequence + var : var + Sequence;
    ui->txtEditCHERCHE->setPlainText(Split.join('\n'));
}

void
MainWindow::on_btnCHERCHE_supprimer_clicked()
{
    QString Source = ui->txtEditSource->toPlainText();
    bool AuDebut = ui->CHERCHE_supprimer_ou->currentIndex() == 0 ? true : false;
    bool Lignesvides = ui->CHERCHE_lignes_vides_2->isChecked();
    auto nbCar = ui->CHERCHE_supprimer_apartirde->value();
    auto Split = TextUtils::splitLines(Source, Lignesvides);
    for (auto& var : Split)
        var = AuDebut ? var.mid(nbCar) : var.left(var.length() - nbCar);
    ui->txtEditCHERCHE->setPlainText(Split.join('\n'));
}

void
MainWindow::on_tvx_img_isRename_prefix_textChanged(const QString& arg1)
{
    QString Extension = "";
    if (ui->tvx_img_isRename_formatSortie->currentIndex() == 0)
        Extension = ".jpg";
    if (ui->tvx_img_isRename_formatSortie->currentIndex() == 1)
        Extension = ".webp";
    if (ui->tvx_img_isRename_formatSortie->currentIndex() == 2)
        Extension = ".png";
    if (ui->tvx_img_isRename_formatSortie->currentIndex() == 3)
        Extension = ".bmp";
    QString NomFichier = arg1 + QString::asprintf("%0*d", ui->tvx_img_isRename_iterateNbDigits->value(), ui->tvx_img_isRename_iterateStart->value()) +
                         ui->tvx_img_isRename_suffix->text() + Extension;
    ui->tvx_img_isRename_preview->setText(NomFichier);
}

void
MainWindow::on_tvx_img_isRename_suffix_textChanged(const QString& arg1)
{
    QString Extension = "";
    if (ui->tvx_img_isRename_formatSortie->currentIndex() == 0)
        Extension = ".jpg";
    if (ui->tvx_img_isRename_formatSortie->currentIndex() == 1)
        Extension = ".webp";
    if (ui->tvx_img_isRename_formatSortie->currentIndex() == 2)
        Extension = ".png";
    if (ui->tvx_img_isRename_formatSortie->currentIndex() == 3)
        Extension = ".bmp";
    QString NomFichier = ui->tvx_img_isRename_prefix->text() +
                         QString::asprintf("%0*d", ui->tvx_img_isRename_iterateNbDigits->value(), ui->tvx_img_isRename_iterateStart->value()) + arg1 +
                         Extension;
    ui->tvx_img_isRename_preview->setText(NomFichier);
}

void
MainWindow::on_tvx_img_isRename_iterateStart_valueChanged(int arg1)
{
    QString Extension = "";
    if (ui->tvx_img_isRename_formatSortie->currentIndex() == 0)
        Extension = ".jpg";
    if (ui->tvx_img_isRename_formatSortie->currentIndex() == 1)
        Extension = ".webp";
    if (ui->tvx_img_isRename_formatSortie->currentIndex() == 2)
        Extension = ".png";
    if (ui->tvx_img_isRename_formatSortie->currentIndex() == 3)
        Extension = ".bmp";
    QString NomFichier = ui->tvx_img_isRename_prefix->text() + QString::asprintf("%0*d", ui->tvx_img_isRename_iterateNbDigits->value(), arg1) +
                         ui->tvx_img_isRename_suffix->text() + Extension;
    ui->tvx_img_isRename_preview->setText(NomFichier);
}

void
MainWindow::on_tvx_img_isRename_iterateNbDigits_valueChanged(int arg1)
{
    QString Extension = "";
    if (ui->tvx_img_isRename_formatSortie->currentIndex() == 0)
        Extension = ".jpg";
    if (ui->tvx_img_isRename_formatSortie->currentIndex() == 1)
        Extension = ".webp";
    if (ui->tvx_img_isRename_formatSortie->currentIndex() == 2)
        Extension = ".png";
    if (ui->tvx_img_isRename_formatSortie->currentIndex() == 3)
        Extension = ".bmp";

    QString NomFichier = ui->tvx_img_isRename_prefix->text() + QString::asprintf("%0*d", arg1, ui->tvx_img_isRename_iterateStart->value()) +
                         ui->tvx_img_isRename_suffix->text() + Extension;
    ui->tvx_img_isRename_preview->setText(NomFichier);
}

void
MainWindow::on_btnTVXImage_Demarrer_clicked()
{
    if (ui->tvx_img_liste->count() == 0)
        return;

    bool MethodeOverwrite = true;
    {
        POSE_OVERLAY_BLUR;
        ModalConfig cfg;
        cfg.title = "REEMaker - Convertir une série d'images";
        cfg.message = "Que souhaitez vous faire ?\n - Ecraser les images existantes\n     "
                      "ou\n - Renommer / itérer les images transformés";
        cfg.icon = OverlayBlurWidget::iconFromStandard(QStyle::SP_MessageBoxQuestion, this);
        cfg.buttons = { OverlayBlurWidget::makeButton(0, "Ecraser", QDialogButtonBox::AcceptRole, true, true),
                        OverlayBlurWidget::makeButton(1, "Renommer / itérer", QDialogButtonBox::RejectRole, false, true) };
        cfg.clickOutsideToClose = false;
        cfg.escapeButtonId = -1;
        int ret = overlay->execModal(cfg);
        if (ret == 0)
            MethodeOverwrite = true;
        else
            MethodeOverwrite = false;
        DEPOSE_OVERLAY_BLUR;
    }

    QString PathOutputImage = "";
    {
        POSE_OVERLAY_BLUR;
        while (1 && (MethodeOverwrite == false)) {
            PathOutputImage = QFileDialog::getExistingDirectory(this,
                                                                "Convertir une série d'images - Dossier ou "
                                                                "seront enregistrés les images",
                                                                LireDansIni("ConvertirImageFolder", "").toString());
            if (PathOutputImage.isEmpty()) {
                DEPOSE_OVERLAY_BLUR;
                return;
            }

            auto isWritable = isDirectoryWritable(PathOutputImage);
            if (isWritable) {
                EcrireDansIni("ConvertirImageFolder", PathOutputImage);
                break; // Ok, on continue
            } else {
                ModalConfig cfg;
                cfg.title = "REEMaker - Erreur";
                cfg.message = "Il est <b>impossible d'écrire</b> dans le dossier que vous "
                              "avez sélectionné.<br>Merci de sélectionner un autre "
                              "emplacement ou abandonner l'opération.";
                cfg.icon = OverlayBlurWidget::iconFromStandard(QStyle::SP_MessageBoxCritical, this);
                cfg.buttons = { OverlayBlurWidget::makeButton(0, "Sélectionner un autre emplacement", QDialogButtonBox::AcceptRole, true, true),
                                OverlayBlurWidget::makeButton(1, "Abandonner", QDialogButtonBox::RejectRole, false, true) };
                cfg.clickOutsideToClose = false;
                cfg.escapeButtonId = -1;
                int ret = overlay->execModal(cfg);
                if (ret == 1 /*Abandonner*/) {
                    DEPOSE_OVERLAY_BLUR;
                    return;
                } else
                    continue;
            }
        }
        DEPOSE_OVERLAY_BLUR;
    }

    auto po = ProgressOverlay::showDeterminate(this, "Traitement en cours...", ui->tvx_img_liste->count(), true, true, 140);
    po->enableBackdropBlur(true, 4.0, 0.5);

    for (int i = 0; i < ui->tvx_img_liste->count(); ++i) {
        QListWidgetItem* currentItem = ui->tvx_img_liste->item(i);
        CustomListItemWidget* itemWidget = qobject_cast<CustomListItemWidget*>(ui->tvx_img_liste->itemWidget(currentItem));
        QString Imagesource = itemWidget->getFullPath();
        QString Extension = itemWidget->getExtensionText().toLower();

        QFileInfo fileInfo(Imagesource);
        auto TailleSourcebytes = fileInfo.size();
        QString fileNameShort = fileInfo.fileName();
        const int ReduceTo = 20;
        if (fileNameShort.length() > ReduceTo)
            fileNameShort = "..." + fileNameShort.right(ReduceTo - 3);

        po->setText("Traitement de " + fileNameShort);
        po->setValue(i);
        QApplication::processEvents();

        QImage image(Imagesource);
        if (image.isNull())
            continue; // Erreur > Suivante

        // Effet Rotation
        if (ui->tvx_img_isRotate->isChecked()) {
            if (ui->tvx_img_isRotate_isStandard->isChecked()) {
                if (ui->tvx_img_isRotate_standardvalue->currentIndex() == 0)
                    image = ImageUtils::rotate90(image);
                if (ui->tvx_img_isRotate_standardvalue->currentIndex() == 1)
                    image = ImageUtils::rotate180(image);
                if (ui->tvx_img_isRotate_standardvalue->currentIndex() == 2)
                    image = ImageUtils::rotate270(image);
            }
            if (ui->tvx_img_isRotate_isFree->isChecked()) {
                image = ImageUtils::rotate(image, ui->tvx_img_isRotate_isFree_value->value());
            }
        }

        // Effet miroir
        if (ui->tvx_img_isMirror_isVertical->isChecked()) {
            image = image.flipped(Qt::Vertical);
        }
        if (ui->tvx_img_isMirror_isHorizontal->isChecked()) {
            image = image.flipped(Qt::Horizontal);
        }

        // Redimensionner
        if (ui->tvx_img_isResize->isChecked()) {
            bool KeepRatio = ui->tvx_img_isResize_ratio->currentIndex() == 0 ? true : false;
            auto targetWidth = ui->tvx_img_isResize_width->value();
            auto targetHeight = ui->tvx_img_isResize_height->value();
            bool UsePercent = ui->tvx_img_isResize_Percent->isChecked();
            auto PercentValue = (ui->tvx_img_isResize_PercentValue->value() / 100.0);

            if (UsePercent)
                image = ImageUtils::resizeFit(image,
                                              QSize((double)image.width() * PercentValue, (double)image.height() * PercentValue),
                                              KeepRatio ? Qt::AspectRatioMode::KeepAspectRatio : Qt::AspectRatioMode::IgnoreAspectRatio);
            else
                image = ImageUtils::resizeFit(
                  image, QSize(targetWidth, targetHeight), KeepRatio ? Qt::AspectRatioMode::KeepAspectRatio : Qt::AspectRatioMode::IgnoreAspectRatio);
        }

        // Filigrane
        if (ui->tvx_img_isWatermark->isChecked()) {
            // Récupère les DPI de l'image
            int dpiX = image.dotsPerMeterX() / 39.37;
            QFont font("Segoe UI", ui->tvx_img_isWatermark_fontsize->value(), QFont::Bold);
            double DPISizedFont = (double)ui->tvx_img_isWatermark_fontsize->value() / (dpiX / 72.0);
            font.setPointSize(DPISizedFont); // Ajuste la taille de la police par rapport à un standard de 72 DPI

            ImageUtils::PositionTexte Position = ImageUtils::PositionTexte::HautGauche;
            if (ui->tvx_img_isWatermark_position->currentIndex() == 1)
                Position = ImageUtils::PositionTexte::HautMilieu;
            if (ui->tvx_img_isWatermark_position->currentIndex() == 2)
                Position = ImageUtils::PositionTexte::HautDroite;
            if (ui->tvx_img_isWatermark_position->currentIndex() == 3)
                Position = ImageUtils::PositionTexte::Centre;
            if (ui->tvx_img_isWatermark_position->currentIndex() == 4)
                Position = ImageUtils::PositionTexte::BasGauche;
            if (ui->tvx_img_isWatermark_position->currentIndex() == 5)
                Position = ImageUtils::PositionTexte::BasMilieu;
            if (ui->tvx_img_isWatermark_position->currentIndex() == 6)
                Position = ImageUtils::PositionTexte::BasDroite;
            image = ImageUtils::overlayText(
              image, ui->tvx_img_isWatermark_text->text(), Position, font, ui->tvx_img_isWatermark_couleur->getColor(), 0.8, 4, QColor(0, 0, 0, 160));
        }

        // Préparation nom de fichier
        QString NomFichier = "";

        if (MethodeOverwrite) // On écrase
            NomFichier = Imagesource;
        else { // On renomme
            if (ui->tvx_img_isRename_formatSortie->currentIndex() == 0)
                Extension = "jpg";
            if (ui->tvx_img_isRename_formatSortie->currentIndex() == 1)
                Extension = "webp";
            if (ui->tvx_img_isRename_formatSortie->currentIndex() == 2)
                Extension = "png";
            if (ui->tvx_img_isRename_formatSortie->currentIndex() == 3)
                Extension = "bmp";
            NomFichier =
              QDir(PathOutputImage)
                .filePath(ui->tvx_img_isRename_prefix->text() +
                          QString::asprintf("%0*d", ui->tvx_img_isRename_iterateNbDigits->value(), ui->tvx_img_isRename_iterateStart->value() + i) +
                          ui->tvx_img_isRename_suffix->text() + "." + Extension);
        }

        /// SAUVEGARDE
        {
            // Ecraser
            if (Extension == "jpg" || Extension == "jpeg" || Extension == "webp") {
                if (ui->tvx_img_isRename_formatSortie_Compress->isChecked())
                    image.save(NomFichier, Extension.toUpper().toStdString().c_str(), ui->tvx_img_isRename_formatSortie_CompressValue->value());
                else {
                    // Taille max
                    QString err;
                    int BaseSize = ui->tvx_img_isRename_formatSortie_LimitsizeValue->value();
                    qDebug() << "Début fichier " << NomFichier << "(" << TailleSourcebytes << ")";
                    while (true) {
                        auto Reussi =
                          ImageUtils::compressToMaxBytes(image, NomFichier, BaseSize * 1024, Extension.toUpper().toStdString().c_str(), 10, 95, &err);
                        if (Reussi)
                            break;
                        else {
                            BaseSize += 50;
                            qDebug() << "  @ Erreur = " << err << "\n   On passe à " << BaseSize << "Ko";
                            continue;
                        }
                    }
                    qDebug() << " Fin fichier " << NomFichier << "(" << QFile(NomFichier).size() << ")";
                }
            } else if (Extension == "png") {
                image.save(NomFichier, Extension.toUpper().toStdString().c_str(), 50);
            } else
                image.save(NomFichier, Extension.toUpper().toStdString().c_str());
        }
    }
    po->close();
}

void
MainWindow::on_tvx_img_isWatermark_couleur_clicked()
{
    QColorDialog* colorDialog = new QColorDialog(this);
    QColorDialog DummyDialog;
    DummyDialog.show();
    DummyDialog.close();
    {
        RECT FenetrePrincipale;
        GetWindowRect((HWND)this->winId(), &FenetrePrincipale);
        colorDialog->setGeometry(FenetrePrincipale.left + (this->geometry().width() / 2) - (DummyDialog.width() / 2),
                                 FenetrePrincipale.top + (this->geometry().height() / 2) - (DummyDialog.height() / 2),
                                 DummyDialog.width(),
                                 DummyDialog.height());
    }
    colorDialog->setCurrentColor(ui->tvx_img_isWatermark_couleur->getColor());
    colorDialog->exec();
    QColor lCOLOR = colorDialog->selectedColor();
    if (lCOLOR.isValid()) {
        ui->tvx_img_isWatermark_couleur->setColor(lCOLOR);
    };
}

void
MainWindow::on_tvx_img_add_clicked()
{
    QString filter = "Convertir une série d'images - Fichiers compatibles (*.png *.jpg *.jpeg *.tiff *.webp *.bmp)";
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Sélectionner des fichiers", LireDansIni("ChercheItemImage", "").toString(), filter);

    if (fileNames.isEmpty())
        return;
    EcrireDansIni("ChercheItemImage", QFileInfo(fileNames.first()).dir().path());

    auto* po = ProgressOverlay::showDeterminate(this, "Analyse des fichiers...", fileNames.count(), true, true, 140);
    po->enableBackdropBlur(true, 4.0, 0.5);
    qint16 loop = 0;

    foreach (auto var, fileNames) {
        loop++;
        po->setValue(loop);
        po->setText("Fichier >" + QFileInfo(var).fileName());
        QThread::msleep(10);
        QApplication::processEvents();
        // Crée le QListWidgetItem
        QListWidgetItem* item = new QListWidgetItem(ui->tvx_img_liste);
        // Crée votre widget personnalisé
        CustomListItemWidget* itemWidget = new CustomListItemWidget(var);
        // Ajoute le widget à l'item
        ui->tvx_img_liste->setItemWidget(item, itemWidget);
        // Ajuste la taille de l'item pour qu'elle corresponde au widget
        item->setSizeHint(itemWidget->sizeHint());
    }
    po->close();
    if (FirstPictureAdd) {
        POSE_OVERLAY_BLUR;
        FirstPictureAdd = false;
        ModalConfig cfg;
        cfg.title = "Réorganiser l'ordre des images";
        cfg.message = "Vous pouvez réorganiser les images en les déplacant par drag'n'drop dans la liste.\n"
                      "\n\nCe message n'apparaitra qu'une seul fois";
        cfg.icon = OverlayBlurWidget::iconFromStandard(QStyle::SP_MessageBoxInformation, this);
        cfg.buttons = { OverlayBlurWidget::makeButton(0, "Continuer", QDialogButtonBox::AcceptRole, true, true) };
        cfg.clickOutsideToClose = false;
        cfg.escapeButtonId = -1;
        overlay->execModal(cfg);
        DEPOSE_OVERLAY_BLUR;
    }
}

void
MainWindow::on_tvx_img_remove_clicked()
{
    QListWidgetItem* currentItem = ui->tvx_img_liste->currentItem();
    if (currentItem) {
        delete ui->tvx_img_liste->takeItem(ui->tvx_img_liste->currentRow());
    }
}

void
MainWindow::on_tvx_img_clear_clicked()
{
    ui->tvx_img_liste->clear();
}
