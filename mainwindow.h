/* -------------------------------------------------------------------------- */
/*      REEMaker 5 -- Grégory WENTZEL (c) 2023   Code sous licence GPL3       */
/* -------------------------------------------------------------------------- */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "BlocEditeur.h"
#include "blocQuestion.h"
#include "customHeader.h"
#include "pressepapier.h"
#include <PdgHelper.h>
#include <QClipboard>
#include <QColorDialog>
#include <QComboBox_custom.h>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QInputDialog>
#include <QJsonDocument>
#include <QKeyEvent>
#include <QList>
#include <QListWidgetItem>
#include <QMainWindow>
#include <QMessageBox>
#include <QMutex>
#include <QProcess>
#include <QProgressDialog>
#include <QScreen>
#include <QSettings>
#include <QStandardPaths>
#include <QThread>
#include <QTimer>
#include <QUuid>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkreply.h>
#include <QtNetwork/qnetworkrequest.h>
#include <include/podofo/podofo.h>
#include <qdialogbuttonbox.h>
#include <tlhelp32.h>
using namespace PoDoFo;
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
} // namespace Ui
QT_END_NAMESPACE

#define MACRO_COLORtoDOUBLE(vecteur, tr) (double)vecteur[tr].redF(), (double)vecteur[tr].greenF(), (double)vecteur[tr].blueF()

/** MACRO_COLOR:
    Macro pour récupérer une couleur d'un QColorDialog dans un bouton
   CustomPushButton

    @param CustomPushButton a
    @param QLineEdit b
*/
#define MACRO_COLOR(a, b)                                                                                                                            \
    QColorDialog* colorDialog = new QColorDialog(this);                                                                                              \
    QColorDialog DummyDialog;                                                                                                                        \
    DummyDialog.show();                                                                                                                              \
    DummyDialog.close();                                                                                                                             \
    {                                                                                                                                                \
        RECT FenetrePrincipale;                                                                                                                      \
        GetWindowRect((HWND)this->winId(), &FenetrePrincipale);                                                                                      \
        colorDialog->setGeometry(FenetrePrincipale.left + (this->geometry().width() / 2) - (DummyDialog.width() / 2),                                \
                                 FenetrePrincipale.top + (this->geometry().height() / 2) - (DummyDialog.height() / 2),                               \
                                 DummyDialog.width(),                                                                                                \
                                 DummyDialog.height());                                                                                              \
    }                                                                                                                                                \
    colorDialog->setCurrentColor(a->getColor());                                                                                                     \
    colorDialog->exec();                                                                                                                             \
    QColor lCOLOR = colorDialog->selectedColor();                                                                                                    \
    if (lCOLOR.isValid()) {                                                                                                                          \
        a->setColor(lCOLOR);                                                                                                                         \
        b->setText(lCOLOR.name().toUpper());                                                                                                         \
    };

/** COLORfromTEXT:
    Macro pour coloriser un CustomPushButton avec la couleur QLineEdit au
   format HTML #RRGGBB

    @param QLineEdit a
    @param CustomPushButton b
*/
#define COLORfromTEXT(a, b)                                                                                                                          \
    if (a->text().length() == 7)                                                                                                                     \
        b->setColor(QColor(a->text().mid(1, 2).toInt(nullptr, 16), a->text().mid(3, 2).toInt(nullptr, 16), a->text().mid(5, 2).toInt(nullptr, 16)));

/** HttpDownload:
    Class de téléchargement via HTTP et SSL
*/
class HttpDownload : public QObject
{
    Q_OBJECT

  public:
    explicit HttpDownload(QWidget* parent = 0)
    {
        progressDialog = new QProgressDialog(parent);
        progressDialog->setAutoClose(false);
        mThis = parent;
        connect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelDownload()));
    }

    ~HttpDownload() { qDebug() << "Deletion de HttpDownload"; }
    enum class EtatFinTelechargement : qint16
    {
        EnCours,
        AbandonnerParUtilisateur,
        ErreurDeTelechargement,
        TermineSansErreur
    };
    enum class StatutDemarrage : qint16
    {
        PasDeSource,
        PasDeDestination,
        AbandonFichierDestinationExistant,
        ErreurCreationFichierDestination,
        TelechargementDemarre
    };

    EtatFinTelechargement RetourneStatut() { return Termine; }
    StatutDemarrage DemarreTelechargement(QUrl UrlATelecharge, QString DestinationDisque, QString TitreFenetre)
    {
        manager = new QNetworkAccessManager(this);
        // get url
        url                = UrlATelecharge;
        mDestinationDisque = DestinationDisque;
        mTitreFenetre      = TitreFenetre;
        QFileInfo fileInfo(url.path());
        if (url.toDisplayString(QUrl::None) == "") {
            progressDialog->close();
            return StatutDemarrage::PasDeSource;
        }

        if (DestinationDisque.isEmpty()) {
            progressDialog->close();
            return StatutDemarrage::PasDeDestination;
        }

        if (QFile::exists(DestinationDisque)) {
            QFile::remove(DestinationDisque); // On le supprime
        }

        file = new QFile(DestinationDisque);
        if (!file->open(QIODevice::WriteOnly)) {
            QMessageBox::information(
              mThis, mTitreFenetre, QString("Impossible de d'enregistrer le fichier %1: %2.").arg(DestinationDisque).arg(file->errorString()));
            delete file;
            file = 0;
            progressDialog->close();
            return StatutDemarrage::ErreurCreationFichierDestination;
        }

        /* -- Interfacage de HttpResquestAborted avec le ProgressDialog -- */
        httpRequestAborted = false;

        progressDialog->setWindowTitle(mTitreFenetre);
        progressDialog->setLabelText("Téléchargement de la mise à jour");
        progressDialog->show();
        startRequest(url);
        return StatutDemarrage::TelechargementDemarre;
    }
  private slots:

    void startRequest(QUrl url)
    {
        Termine = EtatFinTelechargement::EnCours;
        reply   = manager->get(QNetworkRequest(url));
        connect(reply, SIGNAL(readyRead()), this, SLOT(httpReadyRead()));

        connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(updateDownloadProgress(qint64, qint64)));

        connect(reply, SIGNAL(finished()), this, SLOT(httpDownloadFinished()));
    }

    /* -- slot for readyRead() signal -- */
    void httpReadyRead()
    {
        if (file)
            file->write(reply->readAll());
    }

    /* -- slot for finished() signal from reply -- */
    void httpDownloadFinished()
    {
        // when canceled
        if (httpRequestAborted) {
            if (file) {
                file->close();
                file->remove();
                delete file;
                file = 0;
            }
            reply->deleteLater();
            progressDialog->hide();
            Termine = EtatFinTelechargement::AbandonnerParUtilisateur;
            reply   = 0;
            manager = 0;
            return;
        }

        /* -- Le téléchargement c'est fini sans erreur -- */
        progressDialog->hide();
        file->flush();
        file->close();

        if (reply->error()) {
            file->remove();
            QMessageBox::information(mThis, mTitreFenetre, QString("Le téléchargement a échoué : %1.").arg(reply->errorString()));
            Termine = EtatFinTelechargement::ErreurDeTelechargement;
        } else {
            Termine = EtatFinTelechargement::TermineSansErreur;
        }

        reply->deleteLater();
        reply = 0;
        delete file;
        file    = 0;
        manager = 0;
        //        delete this;
    }

    /* -- slot for downloadProgress() -- */
    void updateDownloadProgress(qint64 bytesRead, qint64 totalBytes)
    {
        if (httpRequestAborted)
            return;
        progressDialog->setMaximum(totalBytes);
        progressDialog->setValue(bytesRead);
    }

    void cancelDownload()
    {
        httpRequestAborted = true;
        reply->abort();
    }

  private:
    QUrl url;
    EtatFinTelechargement Termine = EtatFinTelechargement::EnCours;
    QNetworkAccessManager* manager;
    QNetworkReply* reply;
    QProgressDialog* progressDialog;
    QString mDestinationDisque;
    QString mTitreFenetre;
    QFile* file;
    bool httpRequestAborted;
    qint64 fileSize;
    QWidget* mThis;
};

/** MainWindow:
    Class principal, affichage de la fenêtre de REEMaker
*/
class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    struct Fenetre
    {
        QString NOM;
        HWND Handle;
        QString QSHandle;
        QString NomExecutable;
    };

    enum class ReponseMGBOX : qint8
    {
        repond_Vide,
        repond_Oui,
        repond_Non,
        repond_Annuler,
        repond_Continuer
    };
    struct SavState
    {
        bool ModeEditeur = false;
        int OngletActif  = -1;
        QRect Geometrie;
    };
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
            miex.cbSize    = sizeof(MONITORINFOEXW); // set cbSize member
            miex.rcMonitor = { 0, 0, 0, 0 };
            miex.rcWork    = { 0, 0, 0, 0 };
            if (GetMonitorInfoW(hMon, &miex)) {
                pThis->qvName.push_back(QString::fromWCharArray(miex.szDevice));
            };
            return TRUE;
        }

        MonitorRects() { EnumDisplayMonitors(0, 0, MonitorEnum, (LPARAM)this); }
    };
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    static BOOL CALLBACK StaticEnumWindowsProc(HWND hwnd, LPARAM lParam)
    {
        MainWindow* pThis = reinterpret_cast<MainWindow*>(lParam);
        return pThis->EnumWindowsProc(hwnd);
    }

    BOOL EnumWindowsProc(HWND hwnd)
    {
        WCHAR title[255];
        if (GetWindowText(hwnd, title, 255)) {
            if (hwnd == (HWND)this->winId()) // On skippe si c'est nous même
                return TRUE;
            Fenetre fenetre;
            fenetre.NOM      = QString::fromWCharArray(title).toLower();
            fenetre.Handle   = hwnd;
            fenetre.QSHandle = "0x" + QString::number((unsigned __int64)hwnd, 16);
            ListeFenetre.append(fenetre);
        }
        return TRUE;
    }
    void GestionMultiInstance()
    {
        ListeFenetre.clear();
        EnumWindows(StaticEnumWindowsProc, reinterpret_cast<LPARAM>(this));
        foreach (auto fenetre, ListeFenetre) {
            if (fenetre.NOM.toLower().startsWith(this->windowTitle().toLower().first(17))) {
                auto Retour = MGBoxContinuerAnnuler("Attention",
                                                    "Une session de REEMaker est déjà en cours d'utilisation",
                                                    QMessageBox::Icon::Warning,
                                                    "Cliquer sur <b> Continuer </b> pour ouvrir une nouvelle session de REEMaker ou <b> Annuler </b> "
                                                    "pour basculer sur la session ouverte");
                if (Retour == ReponseMGBOX::repond_Continuer)
                    return;
                if (Retour == ReponseMGBOX::repond_Annuler) {
                    DWORD dwMyID  = ::GetCurrentThreadId();
                    DWORD dwCurID = ::GetWindowThreadProcessId((HWND)this->winId(), NULL);
                    ::AttachThreadInput(dwCurID, dwMyID, TRUE);
                    ::SetWindowPos(fenetre.Handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
                    ::SetWindowPos(fenetre.Handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
                    ::SetForegroundWindow(fenetre.Handle);
                    ::SetFocus(fenetre.Handle);
                    ::SetActiveWindow(fenetre.Handle);
                    ::AttachThreadInput(dwCurID, dwMyID, FALSE);
                    Consigne(
                      "Une session de REEMaker est déjà ouverte, annulation de la session en cours d'ouverture et bascule sur la session existante.");
                }
                QApplication::processEvents();
                exit(EXIT_SUCCESS);
                QApplication::processEvents();
            }
        }
    }

  signals:
    void LanceUneMAJ(QString Version, QUrl Chemin);

  public slots:
    void RecoisLogMessage(const QString& arg);

  private slots:
    void Consigne(QString Message, bool EcritSurDisque = true, bool AfficheDansLog = true, bool AfficheDansDebug = true);

    void on_OPT_Btn_ColTranche0_clicked();

    void on_OPT_Btn_ColTranche1_clicked();

    void on_OPT_Btn_ColTranche2_clicked();

    void on_OPT_Btn_ColTranche3_clicked();

    void on_OPT_Btn_ColTranche4_clicked();

    void on_OPT_Btn_ColTranche5_clicked();

    void on_OPT_Btn_ColTranche6_clicked();

    void on_OPT_Btn_ColTranche7_clicked();

    void on_OPT_Btn_ColTranche8_clicked();

    void on_OPT_Btn_ColTranche9_clicked();

    void on_OPT_Btn_ColAccent0_clicked();

    void on_OPT_Btn_ColAccent1_clicked();

    void on_OPT_Btn_ColAccent2_clicked();

    void on_OPT_Btn_ColAccent3_clicked();

    void on_OPT_Btn_ColAccent4_clicked();

    void on_OPT_Btn_ColAccent5_clicked();

    void on_OPT_Btn_ColAccent6_clicked();

    void on_OPT_Btn_ColAccent7_clicked();

    void on_OPT_Btn_ColAccent8_clicked();

    void on_OPT_Btn_ColAccent9_clicked();

    void on_Folioter_Btn_RechercherPDF_clicked();

    void on_Folioter_Btn_EtapeSuivante1_clicked();

    void on_Folioter_Txt_NomDuSite_textChanged(const QString& arg1);

    void on_Folioter_Txt_RefREE_textChanged(const QString& arg1);

    void on_Folioter_Txt_IndiceREE_textChanged(const QString& arg1);

    void on_Folioter_Btn_EtapeSuivante2_clicked();

    void on_Folioter_Btn_EtapeSuivante4_clicked();

    void on_Folioter_Btn_EtapeSuivante3_clicked();

    void on_Folioter_Check_Tranche1_toggled(bool checked);

    void on_Folioter_Check_Tranche3_toggled(bool checked);

    void on_Folioter_Check_Tranche5_toggled(bool checked);

    void on_Folioter_Check_Tranche2_toggled(bool checked);

    void on_Folioter_Check_Tranche4_toggled(bool checked);

    void on_Folioter_Check_Tranche6_toggled(bool checked);

    void on_Folioter_Check_Tranche0_toggled(bool checked);

    void on_Folioter_Check_Tranche9_toggled(bool checked);

    void on_Folioter_Check_Tranche8_toggled(bool checked);

    void on_Folioter_Check_Tranche7_toggled(bool checked);

    void on_Folioter_Btn_ChoixFolioAnnuler_clicked();

    void on_Annulation_Btn_Valider_clicked();

    void on_Annulation_Btn_NePasAnnuler_clicked();

    void on_Annulation_Liste_cellClicked(int row, int column);

    void on_Annulation_Bouton_Tout_clicked();

    void on_Annulation_Bouton_Rien_clicked();

    void on_MainWindow_destroyed();

    void on_Folioter_Radio_FolioTotal_toggled(bool checked);

    void on_Folioter_Radio_FolioPartiel_toggled(bool checked);

    void on_Folioter_Btn_FoliotageSansPDG_clicked();

    void on_Folioter_Btn_FoliotageAvecPDG_clicked();

    void on_PDG_Btn_GenerePDF_clicked();

    void on_PDG_Btn_SauvegardePDGutilisateur_clicked();

    void on_PDG_Combo_Integre_activated(int index);

    void on_PDG_Combo_Utilisateur_activated(int index);

    void on_EDIT_Bouton_Ligne_clicked();

    void on_EDIT_Bouton_OuvrirPDG_clicked();

    void on_EDIT_Bouton_NouvellePDG_clicked();

    void on_EDIT_Bouton_EnregistrePDG_clicked();

    void on_EDIT_Bouton_EnregistrePDGSous_clicked();

    void on_Edit_Bouton_OutilCouleur_clicked();

    void on_EDIT_Bouton_RectVide_clicked();

    void on_EDIT_Bouton_RectGrille_clicked();

    void on_EDIT_Bouton_RectRemplis_clicked();

    void on_EDIT_Bouton_Texte_clicked();

    void on_EDIT_Bouton_TexteMulti_clicked();

    void on_EDIT_Bouton_TexteQuestion_clicked();

    void on_EDIT_Bouton_TexteMultiQuestion_clicked();

    void on_EDIT_Bouton_Checkbox_clicked();

    void on_EDIT_Bouton_CheckboxQeustion_clicked();

    void on_EDIT_Bouton_MultiCheckboxQuestion_clicked();

    void on_EDIT_Bouton_InsereImage_clicked();

    void on_EDIT_Bouton_PageSuivante_clicked();

    void on_EDIT_Bouton_Commentaire_clicked();

    void on_APROPOS_Texte_anchorClicked(const QUrl& arg1);

    void on_APROPOS_Texte_highlighted(const QUrl& arg1);

    void BasculerClairSombre();

    void AfficheEditeurPDG();

    void AfficheAide();

    void on_PDG_Btn_SupprimePDGUtilisateur_clicked();

    void on_FolioListeINFO_customContextMenuRequested(const QPoint& pos);

    MainWindow::ReponseMGBOX MGBoxOuiNon(QString Titre,
                                         QString Message,
                                         QMessageBox::Icon          = QMessageBox::NoIcon,
                                         QString InformativeMessage = "",
                                         QString DetailledMessage   = "");

    MainWindow::ReponseMGBOX MGBoxContinuerAnnuler(QString Titre,
                                                   QString Message,
                                                   QMessageBox::Icon          = QMessageBox::NoIcon,
                                                   QString InformativeMessage = "",
                                                   QString DetailledMessage   = "");

    void MGBoxContinuer(QString Titre,
                        QString Message,
                        QMessageBox::Icon          = QMessageBox::NoIcon,
                        QString InformativeMessage = "",
                        QString DetailledMessage   = "");

  protected:
    void resizeEvent(QResizeEvent* event);

  private:
    Ui::MainWindow* ui;
    class KeyPressEater : public QObject
    {
      protected:
        bool eventFilter(QObject* obj, QEvent* event) override
        {
            if (event->type() == QEvent::KeyPress) {
                QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
                Q_UNUSED(keyEvent);
                //                qDebug("Ate key press %d", keyEvent->key());
                return true;
            } else {
                // standard event processing
                return QObject::eventFilter(obj, event);
            }
        }
    };
    KeyPressEater* keyPressEater = new KeyPressEater();
    void OperationApresQuitter(QStringList mListe)
    {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        QString BaseCommande = QString("cmd.exe /C ping 1.1.1.1 -n 1 -w 5000 > Nul");
        BaseCommande.append(mListe.join(""));
        qDebug().nospace().noquote() << "Ligne de commande développée :" << Qt::endl << BaseCommande;
        CreateProcess(NULL, (LPWSTR)BaseCommande.toStdWString().c_str(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }

    QString DerniereVersion();
    void DemarreLeTelechargement(QUrl);
    bool ChargePDG(QString);
    bool SauvePDG(QString);
    int GetNumArg(QVector<QString>& lVecList, int indexDepart);
    int GetNumCOMM(QVector<QString>& lVecList, int indexDepart);
    QString RetourneCleStr(QVector<QString>& lVecKey, QString Cle, QString = "<!--CleNonTrouve-->");
    double RetourneCleDouble(QVector<QString>& lVecKey, QString Cle, double = (double)INT16_MAX);
    int RetourneCleInt(QVector<QString>& lVecKey, QString Cle, int = INT16_MAX);
    bool RetourneCleBool(QVector<QString>& lVecKey, QString Cle, bool = false);
    bool CentreHWND(HWND Fenetre);
    enum ValeurRetour : qint16
    {
        AucunFichierEntree,
        ErreurDeplacement,
        GhostScriptAbsent,
        ErreurDeGhostScript,
        ZeroPages,
        Succes
    };
    ValeurRetour RepareGhostScript(QString, qint16 = 0, bool = false);
    qint16 PDFInfoNombrePage(QString);
    void LectureINI();
    void SauveINI();
    void MiseEnPlaceChemin();
    void MiseEnPlaceValidator();
    void MiseEnPlaceInterface();
    void MiseEnPlaceListInfo();
    QFont fontList;
    QString PDFOuvert;
    QString PDGOuverte;
    QString CheminPoppler;
    QString CheminTemp;
    QString CheminGhostScript;
    // Termine avec un Slash
    QString CheminPDGBase;
    // Termine avec un Slash
    QString CheminPDGUtilisateur;
    QVector<PoDoFo::PdfRect> vecMediaBox;
    QVector<int> vecRotation;
    qint64 NombrePages;
    QVector<bool> vecFolioAnnuler;
    QString PDFASauver;
    bool ThemeDark = true;
    QPoint dragPosition;
    QString TempTamponRoboto;
    QString TempTamponRobotoMono;
    bool ModeAjoutPDG;
    QString GetVersion(QString fName);
    QString mVersion;
    PDGHelper mPDGHelper;
    class MyValidatorUPPER : public QValidator
    {
      public:
        MyValidatorUPPER(QObject* parent = nullptr)
          : QValidator(parent)
        {
        }
        State validate(QString& input, int&) const override
        {
            input = input.toUpper();
            return QValidator::Acceptable;
        }
    };
    MyValidatorUPPER* validUPPER;
    bool ChargerPageDeGarde(QString Nom, QString Chemin);
    QString PDGOuvertePourEdition = "";
    QColor MemColor               = { 0, 0, 0 };
    PressePapier* mPressePapier   = new PressePapier();
    qint64 RetourneIndexLibre();
    QString PDGManuelNomSite = "";
    QString PDGManuelRefREE  = "";
    QString PDGManuelIndice  = "";
    bool CheckIntegrite();
    QAction* ClairSombreAction;
    QAction* AfficheEditAction;
    QAction* helpAction;
    SavState savState;
    QPixmap QPXpreview;
    QVector<Fenetre> ListeFenetre;
};
#endif // MAINWINDOW_H
