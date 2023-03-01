#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QFileDialog>
#include <QColorDialog>
#include <QProgressDialog>
#include <QDateTime>
#include <QStandardPaths>
#include <QUuid>
#include <QProcess>
#include <QThread>
#include <QElapsedTimer>
#include <QSettings>
#include <QScreen>
#include <QKeyEvent>
#include <QMessageBox>
#include <QList>
#include <QMutex>
#include <QInputDialog>
#include <PdgHelper.h>
#include <QListWidgetItem>
#include <QDesktopServices>
#include <qdialogbuttonbox.h>
#include <QFontDatabase>
#include <QClipboard>
#include <QtNetwork/qnetworkrequest.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkreply.h>
#include <QElapsedTimer>
#include <QTimer>
#include <QJsonDocument>
#include <include/podofo/podofo.h>
#include "blocQuestion.h"
#include "BlocEditeur.h"
//  https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle#readme
#include "framelesswindow.h"
#include "DarkStyle.h"
#include "customHeader.h"
#include "pressepapier.h"
using namespace PoDoFo;
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class HttpDownload : public QObject
{
    Q_OBJECT

public:
    explicit HttpDownload(QWidget *parent = 0)
    {
        progressDialog = new QProgressDialog(parent);
        progressDialog->setAutoClose(false);
        mThis = parent;
        connect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelDownload()));
    }

    ~HttpDownload()
    {
        qDebug() << "Deletion de HttpDownload";
    }
    enum class EtatFinTelechargement : qint16 {EnCours, AbandonnerParUtilisateur, ErreurDeTelechargement, TermineSansErreur};
    enum class StatutDemarrage : qint16 {PasDeSource, PasDeDestination, AbandonFichierDestinationExistant, ErreurCreationFichierDestination, TelechargementDemarre};
public:
    EtatFinTelechargement RetourneStatut()
    {
        return Termine;
    }
    StatutDemarrage DemarreTelechargement(QUrl UrlATelecharge, QString DestinationDisque, QString TitreFenetre)
    {
        manager = new QNetworkAccessManager(this);
        // get url
        url = UrlATelecharge;
        mDestinationDisque = DestinationDisque;
        mTitreFenetre = TitreFenetre;
        QFileInfo fileInfo(url.path());
        if (url.toDisplayString(QUrl::None) == "")
        {
            progressDialog->close();
            return StatutDemarrage::PasDeSource;
        }

        if (DestinationDisque.isEmpty())
        {
            progressDialog->close();
            return StatutDemarrage::PasDeDestination;
        }

        if (QFile::exists(DestinationDisque)) {//On écrase toujours le fichier existant de MAJ
//            if (QMessageBox::question(mThis, mTitreFenetre, QString("Il existe déjà un fichier %1. Ecrire par dessus?").arg(QFileInfo(DestinationDisque).fileName()),
//                                      QMessageBox::Yes|QMessageBox::No, QMessageBox::No)
//                    == QMessageBox::No)
//            {
//                progressDialog->close();
//                return StatutDemarrage::AbandonFichierDestinationExistant;
//            }
            QFile::remove(DestinationDisque);// On le supprime
        }

        file = new QFile(DestinationDisque);
        if (!file->open(QIODevice::WriteOnly)) {
            QMessageBox::information(mThis, mTitreFenetre,
                                     QString("Impossible de d'enregistrer le fichier %1: %2.")
                                     .arg(DestinationDisque).arg(file->errorString()));
            delete file;
            file = 0;
            progressDialog->close();
            return StatutDemarrage::ErreurCreationFichierDestination;
        }

        // used for progressDialog
        // This will be set true when canceled from progress dialog
        httpRequestAborted = false;

        progressDialog->setWindowTitle(mTitreFenetre);
        progressDialog->setLabelText("Téléchargement de la mise à jour");
        progressDialog->show();
        // download button disabled after requesting download
        startRequest(url);
        return StatutDemarrage::TelechargementDemarre;
    }
private slots:

    void startRequest(QUrl url)
    {
        Termine = EtatFinTelechargement::EnCours;
        reply = manager->get(QNetworkRequest(url));
        connect(reply, SIGNAL(readyRead()),
                this, SLOT(httpReadyRead()));

        connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
                this, SLOT(updateDownloadProgress(qint64,qint64)));

        connect(reply, SIGNAL(finished()),
                this, SLOT(httpDownloadFinished()));
    }

    // slot for readyRead() signal
    void httpReadyRead()
    {
        if (file)
            file->write(reply->readAll());
    }

    // slot for finished() signal from reply
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
            reply = 0;
            manager = 0;
            return;
        }

        // download finished normally
        progressDialog->hide();
        file->flush();
        file->close();

        if (reply->error())
        {
            file->remove();
            QMessageBox::information(mThis, mTitreFenetre,
                                     QString("Le téléchargement a échoué : %1.")
                                     .arg(reply->errorString()));
            Termine = EtatFinTelechargement::ErreurDeTelechargement;
        }
        else
        {
            Termine = EtatFinTelechargement::TermineSansErreur;
        }

        reply->deleteLater();
        reply = 0;
        delete file;
        file = 0;
        manager = 0;
        //        delete this;
    }

    // slot for downloadProgress()
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
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    QProgressDialog *progressDialog;
    QString mDestinationDisque;
    QString mTitreFenetre;
    QFile *file;
    bool httpRequestAborted;
    qint64 fileSize;
    QWidget* mThis;

};



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    struct SavState
    {
        bool ModeEditeur = false;
        int OngletActif = -1;
        QRect Geometrie;
    };
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void LanceUneMAJ(QString Version, QUrl Chemin);

private slots:
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

    void on_Folioter_Txt_NomDuSite_textChanged(const QString &arg1);

    void on_Folioter_Txt_RefREE_textChanged(const QString &arg1);

    void on_Folioter_Txt_IndiceREE_textChanged(const QString &arg1);

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

    void on_APROPOS_Texte_anchorClicked(const QUrl &arg1);

    void on_APROPOS_Texte_highlighted(const QUrl &arg1);

    void BasculerClairSombre();

    void AfficheEditeurPDG();

    void AfficheAide();

    void on_PDG_Btn_SupprimePDGUtilisateur_clicked();

    void on_FolioListeINFO_customContextMenuRequested(const QPoint &pos);

protected:
    void resizeEvent(QResizeEvent *event);

private:
    Ui::MainWindow *ui;
    class KeyPressEater : public QObject
    {
    protected:
        bool eventFilter(QObject *obj, QEvent *event) override
        {
            if (event->type() == QEvent::KeyPress) {
                QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
                Q_UNUSED(keyEvent);
//                qDebug("Ate key press %d", keyEvent->key());
                return true;
            } else {
                // standard event processing
                return QObject::eventFilter(obj, event);
            }
        }
    };
    KeyPressEater *keyPressEater = new KeyPressEater();
    void OperationApresQuitter(QStringList mListe)
    {
        STARTUPINFO si = {0};
        PROCESS_INFORMATION pi = {0};
        QFileInfo CheminExe(QCoreApplication::applicationFilePath());
        QString BaseCommande = QString("cmd.exe /C ping 1.1.1.1 -n 1 -w 5000 > Nul");
        BaseCommande.append(mListe.join(""));
        qDebug().nospace().noquote() << "Ligne de commande développée :" << Qt::endl << BaseCommande;
        CreateProcess(NULL,(LPWSTR)BaseCommande.toStdWString().c_str(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }

    QString DerniereVersion();
    void DemarreLeTelechargement(QUrl);
    bool ChargePDG(QString);
    bool SauvePDG(QString);
    int GetNumArg(QVector<QString> &lVecList, int indexDepart);
    int GetNumCOMM(QVector<QString> &lVecList, int indexDepart);
    QString RetourneCleStr(QVector<QString>& lVecKey, QString Cle, QString = "<!--CleNonTrouve-->");
    double RetourneCleDouble(QVector<QString>& lVecKey, QString Cle, double = (double)INT16_MAX);
    int RetourneCleInt(QVector<QString>& lVecKey, QString Cle, int = INT16_MAX);
    bool RetourneCleBool(QVector<QString>& lVecKey, QString Cle, bool = false);
    enum ValeurRetour : qint16 {AucunFichierEntree, ErreurDeplacement, GhostScriptAbsent, ErreurDeGhostScript, Succes};
    ValeurRetour RepareGhostScript(QString , qint16 = 0, bool  = false);
    qint16 PDFInfoNombrePage(QString );
    void LectureINI();
    void  SauveINI();
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
    class MyValidatorUPPER: public QValidator {
    public:
        MyValidatorUPPER(QObject* parent=nullptr): QValidator(parent) {}
        State validate(QString& input, int&) const override {
            input = input.toUpper();
            return QValidator::Acceptable;
        }
    };
    MyValidatorUPPER* validUPPER;
    bool ChargerPageDeGarde(QString Nom, QString Chemin);
    QString PDGOuvertePourEdition = "";
    //    qint64 IndexBlocEditeur = 0;
    QColor MemColor = {0,0,0};
    PressePapier* mPressePapier = new PressePapier();
    qint64 RetourneIndexLibre();
    QString PDGManuelNomSite = "";
    QString PDGManuelRefREE = "";
    QString PDGManuelIndice = "";
    bool CheckIntegrite();
    QAction *ClairSombreAction;
    QAction *AfficheEditAction;
    QAction *helpAction;
    SavState savState;
    QPixmap QPXpreview;
};
#endif // MAINWINDOW_H
