/* -------------------------------------------------------------------------- */
/*      REEMaker 5 -- Grégory WENTZEL (c) 2023   Code sous licence GPL3       */
/* -------------------------------------------------------------------------- */

#ifndef PDGHELPER_HPP
#define PDGHELPER_HPP

#include <include/podofo/podofo.h>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class PDGHelper;
}
QT_END_NAMESPACE

/** PDGHelper:
    Class d'aide pour les pages de gardes
*/
class PDGHelper : public QObject {
    Q_OBJECT
   public:
    enum class TypeCommande {
        DESSINELIGNE,
        DESSINERECTANGLEVIDE,
        DESSINERECTANGLEGRILLE,
        DESSINERECTANGLEREMPLIS,
        DESSINETEXTE,
        DESSINETEXTEMULTILIGNE,
        DESSINETEXTEQUESTION,
        DESSINETEXTEMULTILIGNEQUESTION,
        INSEREIMAGE,
        DESSINECHECKBOX,
        DESSINECHECKBOXQUESTION,
        PAGESUIVANTE,
        DESSINEMULTICHECKBOXQUESTION
    };

    struct Question {
        int IndexQuestion;
        QString LaQuestion;
        QString AideQuestion;
        QString DefautQuestion;
        bool Obligatoire        = false;
        bool EstCheckbox        = false;
        bool CheckboxValue      = false;
        bool EstLigneTexte      = false;
        bool EstMultiLigneTexte = false;
        bool EstMajuscule       = false;
        bool EstMinuscule       = false;
        bool EstChiffre         = false;
        qint64 Maximum;
    };
    struct structReplaceArray {
        QString ReferenceSite = "";
        QString NumeroTranche = "";
        QString ReferenceREE  = "";
        QString IndiceREE     = "";
        int REErouge          = 0;
        int REEvert           = 0;
        int REEbleu           = 0;
        int REErougeAccent    = 0;
        int REEvertAccent     = 0;
        int REEbleuAccent     = 0;
    };
    PDGHelper();
    bool OpenAndParseConfig_v2(QString CheminConfig);
    int DrawOnPage_v2(PoDoFo::PdfPainter&, PoDoFo::PdfDocument&);
    int ItemCount();
    int ItemQuestionCount();
    void SetBaseModelePath(QString);
    bool BurstVersDisque(QString);
    void ClearList();
    QVector<Question> ListeQuestion;
    structReplaceArray ArrayFromREEMAKER;
    void EnvoiLogVersMainWindows(QString);
    ~PDGHelper();

   signals:
    void EnvoiLogMessage(const QString& arg);

   private:
    struct CmdKeys {
        QString Keys;
        QString Valeur;
    };
    struct structCOMMANDE {
        TypeCommande mTypeCommande;
        QVector<CmdKeys> mVecCommande;
    };
    QVector<structCOMMANDE> vecCommandeList;
    QString RetourneCleStr(QVector<CmdKeys>& lVecKey, QString Cle);
    double RetourneCleDouble(QVector<CmdKeys>& lVecKey, QString Cle);
    int RetourneCleInt(QVector<CmdKeys>& lVecKey, QString Cle);
    bool RetourneCleBool(QVector<CmdKeys>& lVecKey, QString Cle);
    double GetMaxFontSize(PoDoFo::PdfFont*&, double, double, double, QString, double = 1.1);
    QString BaseModelePath;
    QString DocumentOuvert;
};
#endif
