/* -------------------------------------------------------------------------- */
/*      REEMaker 5 -- Grégory WENTZEL (c) 2023   Code sous licence GPL3       */
/* -------------------------------------------------------------------------- */

#include "PdgHelper.h"
#include "mainwindow.h"

/** PDGHelper:
    Initialisation de l'aide de page de garde

    @param aucun
    @return aucun
*/
PDGHelper::PDGHelper() {
    // connect(this, &PDGHelper::EnvoiConsigne, this, &MyObject::objectDestroyed);
}

/** ~PDGHelper:
    Déchargement de l'aide de page de garde

    @param aucun
    @return aucun
*/
PDGHelper::~PDGHelper() {}

/** SetBaseModelePath:
    Met en mémoire le dossier des pages de gardes modèle

    @param lBaseModelePath Chemin complet vers le dossier contenant la page de
   garde utilisé pour le foliotage
    @return aucun
*/
void PDGHelper::SetBaseModelePath(QString lBaseModelePath) {
    BaseModelePath = lBaseModelePath;
    if (!BaseModelePath.endsWith("/"))
        BaseModelePath.append("/");
}

/** OpenAndParseConfig_v2:
    Ouvre et parse le contenu d'un fichier Page de garde

    @param CheminConfig Nom seul de la page de garde utilisé pour le foliotage
    @return bool
*/
bool PDGHelper::OpenAndParseConfig_v2(QString CheminConfig) {
    DocumentOuvert = "";
    QFile FichierConfig(BaseModelePath + CheminConfig);
    if (!FichierConfig.open(QIODevice::ReadOnly))
        return false;  // Erreur ouverture
    DocumentOuvert = CheminConfig;
    QVector<QString> vecFichierPDG;

    QTextStream in(&FichierConfig);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.length() > 2) {
            if (line.mid(0, 2) == "ï»")  // C'est le BOM UTF8
                line = line.mid(3);      // Suppresiion BOM
            if (line.length() < 2)
                continue;                                          // Trop petit maintenant, n passe à l'item suivant
            if (line.mid(0, 2) == "::" || line.mid(0, 2) == "ï»")  // C'est un commentaire on ignore ou le BOM du fichier UTF8
                continue;
            line = line.trimmed();
            vecFichierPDG.push_back(line);
        }
    }
    FichierConfig.close();
    EnvoiLogVersMainWindows(QString("Lecture du fichier '%1' terminée ").arg(CheminConfig));
    vecCommandeList.clear();
    structCOMMANDE mStructCommand;
    for (qsizetype compteLigne = 0; compteLigne < vecFichierPDG.size(); compteLigne++) {
        QString trimmedLine = vecFichierPDG[compteLigne].trimmed();
        if (trimmedLine.mid(0, 2) != "--")  // C'est pas un argument, on push le vec precedent puis on re-on
                                            // init mVecCommande avec la fonction
        {
            if (mStructCommand.mVecCommande.size() > 0)     // Plus que 0, c'est que l'on a cree un vecteur argument
                vecCommandeList.push_back(mStructCommand);  // Donc on le sauve
            mStructCommand.mVecCommande.clear();            // On re-init
            if (trimmedLine == "DESSINELIGNE")
                mStructCommand.mTypeCommande = TypeCommande::DESSINELIGNE;
            if (trimmedLine == "DESSINERECTANGLEVIDE")
                mStructCommand.mTypeCommande = TypeCommande::DESSINERECTANGLEVIDE;
            if (trimmedLine == "DESSINERECTANGLEGRILLE")
                mStructCommand.mTypeCommande = TypeCommande::DESSINERECTANGLEGRILLE;
            if (trimmedLine == "DESSINERECTANGLEREMPLIS")
                mStructCommand.mTypeCommande = TypeCommande::DESSINERECTANGLEREMPLIS;
            if (trimmedLine == "DESSINETEXTE")
                mStructCommand.mTypeCommande = TypeCommande::DESSINETEXTE;
            if (trimmedLine == "DESSINETEXTEMULTILIGNE")
                mStructCommand.mTypeCommande = TypeCommande::DESSINETEXTEMULTILIGNE;
            if (trimmedLine == "DESSINETEXTEQUESTION")
                mStructCommand.mTypeCommande = TypeCommande::DESSINETEXTEQUESTION;
            if (trimmedLine == "DESSINETEXTEMULTILIGNEQUESTION")
                mStructCommand.mTypeCommande = TypeCommande::DESSINETEXTEMULTILIGNEQUESTION;
            if (trimmedLine == "INSEREIMAGE")
                mStructCommand.mTypeCommande = TypeCommande::INSEREIMAGE;
            if (trimmedLine == "DESSINECHECKBOX")
                mStructCommand.mTypeCommande = TypeCommande::DESSINECHECKBOX;
            if (trimmedLine == "DESSINECHECKBOXQUESTION")
                mStructCommand.mTypeCommande = TypeCommande::DESSINECHECKBOXQUESTION;
            if (trimmedLine == "DESSINEMULTICHECKBOXQUESTION")
                mStructCommand.mTypeCommande = TypeCommande::DESSINEMULTICHECKBOXQUESTION;
            if (trimmedLine == "PAGESUIVANTE") {
                mStructCommand.mTypeCommande = TypeCommande::PAGESUIVANTE;
                CmdKeys mCmdKeys;
                mStructCommand.mVecCommande.push_back(mCmdKeys);  // On crée le nouveau
            }
        } else if (trimmedLine.mid(0, 2) == "--") {
            CmdKeys mCmdKeys;
            if (trimmedLine.contains('=')) {  // Il y a un égale c'et une variable
                mCmdKeys.Keys   = trimmedLine.mid(2, trimmedLine.indexOf('=') - 2).toLower();
                mCmdKeys.Valeur = trimmedLine.mid(mCmdKeys.Keys.length() + 2 + 1);
            } else {  // Il n'y a pas d'égale c'est un flag
                mCmdKeys.Keys   = trimmedLine.mid(2).toLower();
                mCmdKeys.Valeur = "true";  // On le met à true, l'important c'est qu'il ait une valeur
            }
            mStructCommand.mVecCommande.push_back(mCmdKeys);  // On crée le nouveau
        }
    }
    if (mStructCommand.mVecCommande.size() > 0)     // On a fini et un arhhument est construit
        vecCommandeList.push_back(mStructCommand);  // On le sauve
    ListeQuestion.clear();
    for (qsizetype i = 0; i < vecCommandeList.size(); i++) {
        if (vecCommandeList[i].mTypeCommande == TypeCommande::DESSINECHECKBOXQUESTION ||
            vecCommandeList[i].mTypeCommande == TypeCommande::DESSINEMULTICHECKBOXQUESTION ||
            vecCommandeList[i].mTypeCommande == TypeCommande::DESSINETEXTEQUESTION ||
            vecCommandeList[i].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNEQUESTION) {
            Question mQuestion;
            mQuestion.EstMinuscule   = RetourneCleBool(vecCommandeList[i].mVecCommande, "minuscule");
            mQuestion.EstMajuscule   = RetourneCleBool(vecCommandeList[i].mVecCommande, "majuscule");
            mQuestion.EstChiffre     = RetourneCleBool(vecCommandeList[i].mVecCommande, "chiffre");
            mQuestion.Obligatoire    = RetourneCleBool(vecCommandeList[i].mVecCommande, "obligatoire");
            mQuestion.LaQuestion     = RetourneCleStr(vecCommandeList[i].mVecCommande, "question");
            mQuestion.AideQuestion   = RetourneCleStr(vecCommandeList[i].mVecCommande, "aidequestion");
            mQuestion.IndexQuestion  = i;
            mQuestion.DefautQuestion = RetourneCleStr(vecCommandeList[i].mVecCommande, "defautquestion");
            if (vecCommandeList[i].mTypeCommande == TypeCommande::DESSINECHECKBOXQUESTION ||
                vecCommandeList[i].mTypeCommande == TypeCommande::DESSINEMULTICHECKBOXQUESTION) {
                mQuestion.EstCheckbox   = true;
                mQuestion.CheckboxValue = (mQuestion.DefautQuestion == "oui");
                OutputDebugString(L"");
            }
            if (vecCommandeList[i].mTypeCommande == TypeCommande::DESSINETEXTEQUESTION ||
                vecCommandeList[i].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNEQUESTION) {
                int MaxLength = RetourneCleInt(vecCommandeList[i].mVecCommande, "max");
                if (MaxLength == INT16_MAX)
                    mQuestion.Maximum = -1;
                else
                    mQuestion.Maximum = MaxLength;
            }
            if (vecCommandeList[i].mTypeCommande == TypeCommande::DESSINETEXTEQUESTION)
                mQuestion.EstLigneTexte = true;
            if (vecCommandeList[i].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNEQUESTION)
                mQuestion.EstMultiLigneTexte = true;
            ListeQuestion.push_back(mQuestion);
        }
    }
    EnvoiLogVersMainWindows(QString("Variable de '%1' en mémoire").arg(CheminConfig));
    return true;
}

/** BurstVersDisque:
    Décharge dans un fichier texte le contenu de l'aide de page de garde

    @param FichierSortie    Chemin complet (Dossier + Fichier) de sortie
    @return bool
*/
bool PDGHelper::BurstVersDisque(QString FichierSortie) {
    if (vecCommandeList.size() == 0) {
        qDebug() << "INFO: Aucune commande à dessiner...";
        return false;
    }

    QFile Fichier(FichierSortie);
    if (!Fichier.open(QIODevice::WriteOnly)) {
        EnvoiLogVersMainWindows(QString("Impossibilité de créer le fichier '%1'").arg(FichierSortie));
        return false;
    }
    QTextStream f_out(&Fichier);
    for (qsizetype lArg = 0; lArg < vecCommandeList.size(); lArg++) {
        switch (vecCommandeList[lArg].mTypeCommande) {
            case TypeCommande::DESSINELIGNE:
                f_out << "DESSINELIGNE" << Qt::endl;
                break;
            case TypeCommande::DESSINERECTANGLEVIDE:
                f_out << "DESSINERECTANGLEVIDE" << Qt::endl;
                break;
            case TypeCommande::DESSINERECTANGLEGRILLE:
                f_out << "DESSINERECTANGLEGRILLE" << Qt::endl;
                break;
            case TypeCommande::DESSINERECTANGLEREMPLIS:
                f_out << "DESSINERECTANGLEREMPLIS" << Qt::endl;
                break;
            case TypeCommande::DESSINETEXTE:
                f_out << "DESSINETEXTE" << Qt::endl;
                break;
            case TypeCommande::DESSINETEXTEMULTILIGNE:
                f_out << "DESSINETEXTEMULTILIGNE" << Qt::endl;
                break;
            case TypeCommande::DESSINETEXTEQUESTION:
                f_out << "DESSINETEXTEQUESTION" << Qt::endl;
                break;
            case TypeCommande::DESSINETEXTEMULTILIGNEQUESTION:
                f_out << "DESSINETEXTEMULTILIGNEQUESTION" << Qt::endl;
                break;
            case TypeCommande::INSEREIMAGE:
                f_out << "INSEREIMAGE" << Qt::endl;
                break;
            case TypeCommande::DESSINECHECKBOX:
                f_out << "DESSINECHECKBOX" << Qt::endl;
                break;
            case TypeCommande::DESSINECHECKBOXQUESTION:
                f_out << "DESSINECHECKBOXQUESTION" << Qt::endl;
                break;
            case TypeCommande::DESSINEMULTICHECKBOXQUESTION:
                f_out << "DESSINEMULTICHECKBOXQUESTION" << Qt::endl;
                break;
            case TypeCommande::PAGESUIVANTE:
                f_out << "PAGESUIVANTE" << Qt::endl;
                break;
        }
        for (qsizetype i = 0; i < vecCommandeList[lArg].mVecCommande.size(); i++) {
            QString wKeys   = vecCommandeList[lArg].mVecCommande[i].Keys;
            QString wValeur = vecCommandeList[lArg].mVecCommande[i].Valeur;
            // Cles avec valeur (int ou double)
            if (wKeys == "debutx" || wKeys == "debuty" || wKeys == "finx" || wKeys == "finy" || wKeys == "rouge" || wKeys == "vert" ||
                wKeys == "bleu" || wKeys == "epaisseur" || wKeys == "largeur" || wKeys == "hauteur" || wKeys == "nombrecolonne" ||
                wKeys == "nombreligne" || wKeys == "remplisrouge" || wKeys == "remplisvert" || wKeys == "remplisbleu" || wKeys == "alignlargeur" ||
                wKeys == "alignhauteur" || wKeys == "taillepolice" || wKeys == "split" || wKeys == "max" || wKeys == "debutx1" ||
                wKeys == "debutx2" || wKeys == "debuty1" || wKeys == "debuty2")
                f_out << "--" << wKeys << "=" << wValeur << Qt::endl;
            // Cles sans valeur ==> FLAG
            else if (wKeys == "gras" || wKeys == "grasitalic" || wKeys == "italic" || wKeys == "monospace" || wKeys == "obligatoire" ||
                     wKeys == "majuscule" || wKeys == "minuscule" || wKeys == "chiffre")
                f_out << "--" << wKeys << Qt::endl;
            // Cles avec string
            else if (wKeys == "texte" || wKeys == "question" || wKeys == "aidequestion" || wKeys == "chemin")
                f_out << "--" << wKeys << "=" << wValeur << Qt::endl;
            else if (wKeys == "defautquestion") {
                QString valDefautquestion = "";
                for (qsizetype iReponse = 0; iReponse < ListeQuestion.size(); iReponse++)
                    if (ListeQuestion[iReponse].IndexQuestion == lArg) {
                        if (ListeQuestion[iReponse].EstCheckbox)
                            valDefautquestion = ListeQuestion[iReponse].CheckboxValue ? "oui" : "non";
                        else
                            valDefautquestion = ListeQuestion[iReponse].DefautQuestion.trimmed();
                        break;
                    }
                f_out << "--" << wKeys << "=\"" << valDefautquestion << "\"" << Qt::endl;
            }
        }
    }
    EnvoiLogVersMainWindows(QString("Fin de la création du fichier '%1'").arg(FichierSortie));
    return true;
}

/** DrawOnPage_v2:
    Permet de dessiner la page de garde sur un document PoDoFo existant

    @param Painter      Painter utilisé par PoDoFo
    @param Document     Pointeur au document PoDoFo
    @return int : Le nombre de page créée
*/
int PDGHelper::DrawOnPage_v2(PoDoFo::PdfPainter& Painter, PoDoFo::PdfDocument& Document) {
    int NombrePageCree = 1;
    if (vecCommandeList.size() == 0) {
        EnvoiLogVersMainWindows(QString("Pas de création de page de garde, nombre d'objet = %1").arg(QString::number(0)));
        return 0;
    }
    QString CheminBASE              = QCoreApplication::applicationDirPath() + "/";  // Chemin de base de l'executable, fini pas par un slash
    QString CheminRobotoMonoRegular = CheminBASE + "Police/RobotoMono-Regular.ttf";
    if (!QFile::exists(CheminRobotoMonoRegular)) {
        EnvoiLogVersMainWindows(QString("Impossible de charger la police '%1', fichier manquant").arg(CheminRobotoMonoRegular));
        return 0;
    }
    QString CheminRobotoMonoBold = CheminBASE + "Police/RobotoMono-Bold.ttf";
    if (!QFile::exists(CheminRobotoMonoBold)) {
        EnvoiLogVersMainWindows(QString("Impossible de charger la police '%1', fichier manquant").arg(CheminRobotoMonoBold));
        return 0;
    }
    QString CheminRobotoMonoItalic = CheminBASE + "Police/RobotoMono-Italic.ttf";
    if (!QFile::exists(CheminRobotoMonoItalic)) {
        EnvoiLogVersMainWindows(QString("Impossible de charger la police '%1', fichier manquant").arg(CheminRobotoMonoItalic));
        return 0;
    }
    QString CheminRobotoMonoBoldItalic = CheminBASE + "Police/RobotoMono-BoldItalic.ttf";
    if (!QFile::exists(CheminRobotoMonoBoldItalic)) {
        EnvoiLogVersMainWindows(QString("Impossible de charger la police '%1', fichier manquant").arg(CheminRobotoMonoBoldItalic));
        return 0;
    }

    QString CheminRobotoRegular = CheminBASE + "Police/Roboto-Regular.ttf";
    if (!QFile::exists(CheminRobotoRegular)) {
        EnvoiLogVersMainWindows(QString("Impossible de charger la police '%1', fichier manquant").arg(CheminRobotoRegular));
        return 0;
    }
    QString CheminRobotoBold = CheminBASE + "Police/Roboto-Bold.ttf";
    if (!QFile::exists(CheminRobotoBold)) {
        EnvoiLogVersMainWindows(QString("Impossible de charger la police '%1', fichier manquant").arg(CheminRobotoBold));
        return 0;
    }
    QString CheminRobotoItalic = CheminBASE + "Police/Roboto-Italic.ttf";
    if (!QFile::exists(CheminRobotoItalic)) {
        EnvoiLogVersMainWindows(QString("Impossible de charger la police '%1', fichier manquant").arg(CheminRobotoItalic));
        return 0;
    }
    QString CheminRobotoBoldItalic = CheminBASE + "Police/Roboto-BoldItalic.ttf";
    if (!QFile::exists(CheminRobotoBoldItalic)) {
        EnvoiLogVersMainWindows(QString("Impossible de charger la police '%1', fichier manquant").arg(CheminRobotoBoldItalic));
        return 0;
    }

    //  Origine police : Licence Apache
    //  https://github.com/google/fonts/tree/main/apache
    PoDoFo::PdfFont* pFontMono =
        Document.CreateFontSubset("Roboto Mono", false, false, false, PoDoFo::PdfEncodingFactory::GlobalWinAnsiEncodingInstance(),
                                  CheminRobotoMonoRegular.toStdString().c_str());
    PoDoFo::PdfFont* pFontMonoBOLD =
        Document.CreateFontSubset("Roboto Mono Bold", true, false, false, PoDoFo::PdfEncodingFactory::GlobalWinAnsiEncodingInstance(),
                                  CheminRobotoMonoBold.toStdString().c_str());
    PoDoFo::PdfFont* pFontMonoITALIC =
        Document.CreateFontSubset("Roboto Mono Italic", false, true, false, PoDoFo::PdfEncodingFactory::GlobalWinAnsiEncodingInstance(),
                                  CheminRobotoMonoItalic.toStdString().c_str());
    PoDoFo::PdfFont* pFontMonoBOLDITALIC =
        Document.CreateFontSubset("Roboto Mono Bold Italic", true, true, false, PoDoFo::PdfEncodingFactory::GlobalWinAnsiEncodingInstance(),
                                  CheminRobotoMonoBoldItalic.toStdString().c_str());
    PoDoFo::PdfFont* pFontReg = Document.CreateFontSubset("Roboto", false, false, false, PoDoFo::PdfEncodingFactory::GlobalWinAnsiEncodingInstance(),
                                                          CheminRobotoRegular.toStdString().c_str());
    PoDoFo::PdfFont* pFontRegBOLD = Document.CreateFontSubset(
        "Roboto Bold", true, false, false, PoDoFo::PdfEncodingFactory::GlobalWinAnsiEncodingInstance(), CheminRobotoBold.toStdString().c_str());
    PoDoFo::PdfFont* pFontRegITALIC = Document.CreateFontSubset(
        "Roboto Italic", false, true, false, PoDoFo::PdfEncodingFactory::GlobalWinAnsiEncodingInstance(), CheminRobotoItalic.toStdString().c_str());
    PoDoFo::PdfFont* pFontRegBOLDITALIC =
        Document.CreateFontSubset("Roboto Bold Italic", true, true, false, PoDoFo::PdfEncodingFactory::GlobalWinAnsiEncodingInstance(),
                                  CheminRobotoBoldItalic.toStdString().c_str());
    if (pFontMono == NULL || pFontMonoBOLD == NULL || pFontMonoBOLDITALIC == NULL || pFontMonoITALIC == NULL) {
        EnvoiLogVersMainWindows(QString("PoDoFo n'a pu charger les polices Monotypes"));
        return 0;
    }
    if (pFontReg == NULL || pFontRegBOLD == NULL || pFontRegBOLDITALIC == NULL || pFontRegITALIC == NULL) {
        EnvoiLogVersMainWindows(QString("PoDoFo n'a pu charger les polices Regular"));
        return 0;
    }
    double PageWidth = Painter.GetPage()->GetPageSize().GetWidth();
    Q_UNUSED(PageWidth);
    double PageHeight = Painter.GetPage()->GetPageSize().GetHeight();

    for (qsizetype lArg = 0; lArg < vecCommandeList.size(); lArg++) {
        /* -------------- Preload des ints -------------- */
        int ValRouge = RetourneCleInt(vecCommandeList[lArg].mVecCommande, "rouge");
        if (ValRouge == -1)
            ValRouge = ArrayFromREEMAKER.REErouge;
        if (ValRouge == -2)
            ValRouge = ArrayFromREEMAKER.REErougeAccent;
        if (ValRouge == INT16_MAX)
            ValRouge = 0;
        int valVert = RetourneCleInt(vecCommandeList[lArg].mVecCommande, "vert");
        if (valVert == -1)
            valVert = ArrayFromREEMAKER.REEvert;
        if (valVert == -2)
            valVert = ArrayFromREEMAKER.REEvertAccent;
        if (valVert == INT16_MAX)
            valVert = 0;
        int valBleu = RetourneCleInt(vecCommandeList[lArg].mVecCommande, "bleu");
        if (valBleu == -1)
            valBleu = ArrayFromREEMAKER.REEbleu;
        if (valBleu == -2)
            valBleu = ArrayFromREEMAKER.REEbleuAccent;
        if (valBleu == INT16_MAX)
            valBleu = 0;
        int ValRemplisRouge = RetourneCleInt(vecCommandeList[lArg].mVecCommande, "remplisrouge");
        if (ValRemplisRouge == -1)
            ValRemplisRouge = ArrayFromREEMAKER.REErouge;
        if (ValRemplisRouge == -2)
            ValRemplisRouge = ArrayFromREEMAKER.REErougeAccent;
        if (ValRemplisRouge == INT16_MAX)
            ValRemplisRouge = 0;
        int valRemplisVert = RetourneCleInt(vecCommandeList[lArg].mVecCommande, "remplisvert");
        if (valRemplisVert == -1)
            valRemplisVert = ArrayFromREEMAKER.REEvert;
        if (valRemplisVert == -2)
            valRemplisVert = ArrayFromREEMAKER.REEvertAccent;
        if (valRemplisVert == INT16_MAX)
            valRemplisVert = 0;
        int valRemplisBleu = RetourneCleInt(vecCommandeList[lArg].mVecCommande, "remplisbleu");
        if (valRemplisBleu == -1)
            valRemplisBleu = ArrayFromREEMAKER.REEbleu;
        if (valRemplisBleu == -2)
            valRemplisBleu = ArrayFromREEMAKER.REEbleuAccent;
        if (valRemplisBleu == INT16_MAX)
            valRemplisBleu = 0;
        int valAlignementI = RetourneCleInt(vecCommandeList[lArg].mVecCommande, "alignement");
        if (valAlignementI == INT16_MAX)
            valAlignementI = 0;
        int valAlignementLargeurI = RetourneCleInt(vecCommandeList[lArg].mVecCommande, "alignlargeur");
        if (valAlignementLargeurI == INT16_MAX)
            valAlignementLargeurI = 0;  // Defaut gauche
        int valAlignementHauteurI = RetourneCleInt(vecCommandeList[lArg].mVecCommande, "alignhauteur");
        if (valAlignementHauteurI == INT16_MAX)
            valAlignementHauteurI = 1;  // Defaut milieu
        int ValMax = RetourneCleInt(vecCommandeList[lArg].mVecCommande, "max");
        if (ValMax == INT16_MAX)
            ValMax = 4096;
        int ValSplit = RetourneCleInt(vecCommandeList[lArg].mVecCommande, "split");
        if (ValSplit == INT16_MAX)
            ValSplit = 0;
        int ValGColonne = RetourneCleInt(vecCommandeList[lArg].mVecCommande, "nombrecolonne");
        if (ValGColonne == INT16_MAX)
            ValGColonne = 0;
        int ValGLigne = RetourneCleInt(vecCommandeList[lArg].mVecCommande, "nombreligne");
        if (ValGLigne == INT16_MAX)
            ValGLigne = 0;
        /* -------------- Preload des doubles -------------- */
        double valEpaisseur = RetourneCleDouble(vecCommandeList[lArg].mVecCommande, "epaisseur");
        if (valEpaisseur == (double)INT16_MAX)
            valEpaisseur = 1.0;
        double valDebutX = RetourneCleDouble(vecCommandeList[lArg].mVecCommande, "debutx");
        if (valDebutX == (double)INT16_MAX)
            valDebutX = 0;
        double valDebutY = RetourneCleDouble(vecCommandeList[lArg].mVecCommande, "debuty");
        if (valDebutY == (double)INT16_MAX)
            valDebutY = 0;
        double valFinX = RetourneCleDouble(vecCommandeList[lArg].mVecCommande, "finx");
        if (valFinX == (double)INT16_MAX)
            valFinX = 0;
        double valFinY = RetourneCleDouble(vecCommandeList[lArg].mVecCommande, "finy");
        if (valFinY == (double)INT16_MAX)
            valFinY = 0;
        double valDebutX1 = RetourneCleDouble(vecCommandeList[lArg].mVecCommande, "debutx1");
        if (valDebutX1 == (double)INT16_MAX)
            valDebutX1 = 0.0;
        double valDebutY1 = RetourneCleDouble(vecCommandeList[lArg].mVecCommande, "debuty1");
        if (valDebutY1 == (double)INT16_MAX)
            valDebutY1 = 0.0;
        double valDebutX2 = RetourneCleDouble(vecCommandeList[lArg].mVecCommande, "debutx2");
        if (valDebutX2 == (double)INT16_MAX)
            valDebutX2 = 0.0;
        double valDebutY2 = RetourneCleDouble(vecCommandeList[lArg].mVecCommande, "debuty2");
        if (valDebutY2 == (double)INT16_MAX)
            valDebutY2 = 0.0;
        double valLargeur = RetourneCleDouble(vecCommandeList[lArg].mVecCommande, "largeur");
        if (valLargeur == (double)INT16_MAX)
            valLargeur = 100.0;
        double valHauteur = RetourneCleDouble(vecCommandeList[lArg].mVecCommande, "hauteur");
        if (valHauteur == (double)INT16_MAX)
            valHauteur = 20.0;
        double valTaillePolice = RetourneCleDouble(vecCommandeList[lArg].mVecCommande, "taillepolice");
        if (valTaillePolice == (double)INT16_MAX)
            valTaillePolice = 10.0;
        /* -------------- Preload des chaines de textes -------------- */
        QString valTexte = RetourneCleStr(vecCommandeList[lArg].mVecCommande, "texte");
        if (valTexte == "<!--CleNonTrouve-->")
            valTexte = " ";
        if (vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTE ||
            vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEQUESTION)
            valTexte = QString(valTexte).replace("\n", " ");  // On supprime les miltilignes si présentes
        QString valQuestion     = RetourneCleStr(vecCommandeList[lArg].mVecCommande, "question");
        QString valQuestionAide = RetourneCleStr(vecCommandeList[lArg].mVecCommande, "aidequestion");
        if (valQuestionAide == "<!--CleNonTrouve-->")
            valQuestionAide = " ";
        QString valDefautquestion = "";
        if (valDefautquestion == "<!--CleNonTrouve-->")
            valDefautquestion = " ";
        for (qsizetype iReponse = 0; iReponse < ListeQuestion.size(); iReponse++)
            if (ListeQuestion[iReponse].IndexQuestion == lArg) {
                if (ListeQuestion[iReponse].EstCheckbox) {
                    valDefautquestion = ListeQuestion[iReponse].CheckboxValue ? "oui" : "non";
                } else
                    valDefautquestion = ListeQuestion[iReponse].DefautQuestion.trimmed();
                break;
            }
        /* -------------- Preload des flags -------------- */
        bool bGras       = RetourneCleBool(vecCommandeList[lArg].mVecCommande, "gras");
        bool bItalic     = RetourneCleBool(vecCommandeList[lArg].mVecCommande, "italic");
        bool bGrasItalic = RetourneCleBool(vecCommandeList[lArg].mVecCommande, "grasitalic");
        bool bMono       = RetourneCleBool(vecCommandeList[lArg].mVecCommande, "monospace");
        if (vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTE ||
            vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEQUESTION ||
            vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNE ||
            vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNEQUESTION) {
            if (bMono) {  // Monospace
                if (bGras) {
                    pFontMonoBOLD->SetFontSize(
                        (float)GetMaxFontSize(pFontMonoBOLD, valLargeur, valHauteur, valTaillePolice,
                                              (vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNEQUESTION ||
                                               vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEQUESTION)
                                                  ? valDefautquestion
                                                  : valTexte,
                                              (vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNEQUESTION ||
                                               vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNE)
                                                  ? 1.1
                                                  : 1.0));
                    Painter.SetFont(pFontMonoBOLD);
                } else if (bItalic) {
                    pFontMonoITALIC->SetFontSize(
                        (float)GetMaxFontSize(pFontMonoITALIC, valLargeur, valHauteur, valTaillePolice,
                                              (vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNEQUESTION ||
                                               vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEQUESTION)
                                                  ? valDefautquestion
                                                  : valTexte,
                                              (vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNEQUESTION ||
                                               vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNE)
                                                  ? 1.1
                                                  : 1.0));
                    Painter.SetFont(pFontMonoITALIC);
                } else if (bGrasItalic) {
                    pFontMonoBOLDITALIC->SetFontSize(
                        (float)GetMaxFontSize(pFontMonoBOLDITALIC, valLargeur, valHauteur, valTaillePolice,
                                              (vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNEQUESTION ||
                                               vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEQUESTION)
                                                  ? valDefautquestion
                                                  : valTexte,
                                              (vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNEQUESTION ||
                                               vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNE)
                                                  ? 1.1
                                                  : 1.0));
                    Painter.SetFont(pFontMonoBOLDITALIC);
                } else {
                    pFontMono->SetFontSize(
                        (float)GetMaxFontSize(pFontMono, valLargeur, valHauteur, valTaillePolice,
                                              (vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNEQUESTION ||
                                               vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEQUESTION)
                                                  ? valDefautquestion
                                                  : valTexte,
                                              (vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNEQUESTION ||
                                               vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNE)
                                                  ? 1.1
                                                  : 1.0));
                    Painter.SetFont(pFontMono);
                }
            } else {  // Regular
                if (bGras) {
                    pFontRegBOLD->SetFontSize(
                        (float)GetMaxFontSize(pFontRegBOLD, valLargeur, valHauteur, valTaillePolice,
                                              (vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNEQUESTION ||
                                               vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEQUESTION)
                                                  ? valDefautquestion
                                                  : valTexte,
                                              (vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNEQUESTION ||
                                               vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNE)
                                                  ? 1.1
                                                  : 1.0));
                    Painter.SetFont(pFontRegBOLD);
                } else if (bItalic) {
                    pFontRegITALIC->SetFontSize(
                        (float)GetMaxFontSize(pFontRegITALIC, valLargeur, valHauteur, valTaillePolice,
                                              (vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNEQUESTION ||
                                               vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEQUESTION)
                                                  ? valDefautquestion
                                                  : valTexte,
                                              (vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNEQUESTION ||
                                               vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNE)
                                                  ? 1.1
                                                  : 1.0));
                    Painter.SetFont(pFontRegITALIC);
                } else if (bGrasItalic) {
                    pFontRegBOLDITALIC->SetFontSize(
                        (float)GetMaxFontSize(pFontRegBOLDITALIC, valLargeur, valHauteur, valTaillePolice,
                                              (vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNEQUESTION ||
                                               vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEQUESTION)
                                                  ? valDefautquestion
                                                  : valTexte,
                                              (vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNEQUESTION ||
                                               vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNE)
                                                  ? 1.1
                                                  : 1.0));
                    Painter.SetFont(pFontRegBOLDITALIC);
                } else {
                    pFontReg->SetFontSize(
                        (float)GetMaxFontSize(pFontReg, valLargeur, valHauteur, valTaillePolice,
                                              (vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNEQUESTION ||
                                               vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEQUESTION)
                                                  ? valDefautquestion
                                                  : valTexte,
                                              (vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNEQUESTION ||
                                               vecCommandeList[lArg].mTypeCommande == TypeCommande::DESSINETEXTEMULTILIGNE)
                                                  ? 1.1
                                                  : 1.0));
                    Painter.SetFont(pFontReg);
                }
            }
        }
        /* -------------- Preload des alignements -------------- */
        PoDoFo::EPdfAlignment mAlignLargeur         = PoDoFo::EPdfAlignment::ePdfAlignment_Left;
        PoDoFo::EPdfVerticalAlignment mAlignHauteur = PoDoFo::EPdfVerticalAlignment::ePdfVerticalAlignment_Top;
        if (valAlignementLargeurI == 0)
            mAlignLargeur = PoDoFo::EPdfAlignment::ePdfAlignment_Left;
        if (valAlignementLargeurI == 1)
            mAlignLargeur = PoDoFo::EPdfAlignment::ePdfAlignment_Center;
        if (valAlignementLargeurI == 2)
            mAlignLargeur = PoDoFo::EPdfAlignment::ePdfAlignment_Right;
        if (valAlignementHauteurI == 0)
            mAlignHauteur = PoDoFo::EPdfVerticalAlignment::ePdfVerticalAlignment_Top;
        if (valAlignementHauteurI == 1)
            mAlignHauteur = PoDoFo::EPdfVerticalAlignment::ePdfVerticalAlignment_Center;
        if (valAlignementHauteurI == 2)
            mAlignHauteur = PoDoFo::EPdfVerticalAlignment::ePdfVerticalAlignment_Bottom;
        if (ValSplit > 1) {
            mAlignLargeur = PoDoFo::EPdfAlignment::ePdfAlignment_Center;
            mAlignHauteur = PoDoFo::EPdfVerticalAlignment::ePdfVerticalAlignment_Center;
        }
        /* -------------- Preload du Painter -------------- */
        Painter.SetStrokeWidth(valEpaisseur);
        /* -------------- Switch avec les différentes actions -------------- */
        switch (vecCommandeList[lArg].mTypeCommande) {
            case TypeCommande::DESSINELIGNE: {
                {
                    qDebug() << "INFO: DESSINELIGNE ... ";
                    Painter.SetStrokingColor((double)(ValRouge / 255.0), (double)(valVert / 255.0),
                                             (double)(valBleu / 255.0));  // Couleur ligne
                    Painter.DrawLine(valDebutX, PageHeight - valDebutY, valFinX, PageHeight - valFinY);
                    Painter.Stroke();
                    qDebug() << " effectué";
                }
            } break;
            case TypeCommande::DESSINERECTANGLEVIDE: {
                {
                    qDebug() << "INFO: DESSINERECTANGLEVIDE ... ";
                    Painter.SetStrokingColor((double)(ValRouge / 255.0), (double)(valVert / 255.0),
                                             (double)(valBleu / 255.0));  // Couleur ligne
                    Painter.Rectangle(valDebutX, PageHeight - valDebutY - valHauteur, valLargeur, valHauteur);
                    Painter.Stroke();
                    qDebug() << " effectué";
                }
            } break;
            case TypeCommande::DESSINERECTANGLEGRILLE: {
                {
                    qDebug() << "INFO: DESSINERECTANGLEGRILLE ... ";
                    Painter.SetStrokingColor((double)(ValRouge / 255.0), (double)(valVert / 255.0),
                                             (double)(valBleu / 255.0));  // Couleur ligne
                    Painter.Rectangle(valDebutX, PageHeight - valDebutY - valHauteur, valLargeur, valHauteur);
                    double GapColonne = valLargeur / ValGColonne;
                    double GapLigne   = valHauteur / ValGLigne;
                    for (qsizetype i = 1; i < ValGColonne; i++)
                        Painter.Rectangle(valDebutX, PageHeight - valDebutY - valHauteur, i * GapColonne, valHauteur);
                    for (qsizetype i = 0; i < ValGLigne; i++) {
                        Painter.Rectangle(valDebutX, PageHeight - valDebutY - valHauteur, valLargeur, i * GapLigne);
                    }
                    Painter.Stroke();
                    qDebug() << " effectué";
                }
            } break;
            case TypeCommande::DESSINERECTANGLEREMPLIS: {
                {
                    qDebug() << "INFO: DESSINERECTANGLEREMPLIS ...";
                    Painter.SetStrokingColor((double)(ValRouge / 255.0), (double)(valVert / 255.0),
                                             (double)(valBleu / 255.0));  // Couleur ligne
                    Painter.SetColor((double)(ValRemplisRouge / 255.0), (double)(valRemplisVert / 255.0),
                                     (double)(valRemplisBleu / 255.0));  // Couleur ligne
                    Painter.Rectangle(valDebutX, PageHeight - valDebutY - valHauteur, valLargeur, valHauteur);
                    Painter.FillAndStroke();
                    qDebug() << " effectué";
                }
            } break;
            case TypeCommande::DESSINETEXTE: {
                {
                    qDebug() << "INFO: DESSINETEXTE ... ";
                    Painter.SetColor((double)(ValRouge / 255.0), (double)(valVert / 255.0),
                                     (double)(valBleu / 255.0));  // Couleur ligne
                    if (ValSplit < 2) {
                        PoDoFo::PdfString mUtf8(reinterpret_cast<const PoDoFo::pdf_utf8*>(valTexte.toUtf8().constData()));
                        Painter.DrawMultiLineText(PoDoFo::PdfRect(valDebutX, PageHeight - valDebutY - valHauteur, valLargeur, valHauteur), mUtf8,
                                                  mAlignLargeur, mAlignHauteur, false);
                    } else {
                        double _gap = valLargeur / ValSplit;
                        for (qsizetype i = 0; i < ValSplit; i++) {
                            PoDoFo::PdfString mUtf8(reinterpret_cast<const PoDoFo::pdf_utf8*>(QString(valTexte.mid(i, 1)).toUtf8().constData()));
                            Painter.DrawMultiLineText(PoDoFo::PdfRect(valDebutX + (i * _gap), PageHeight - valDebutY - valHauteur, _gap, valHauteur),
                                                      mUtf8, mAlignLargeur, mAlignHauteur, false);
                        }
                    }
                    qDebug() << " effectué";
                }
            } break;
            case TypeCommande::DESSINETEXTEMULTILIGNE: {
                {
                    qDebug() << "INFO: DESSINETEXTEMULTILIGNE ... ";
                    Painter.SetColor((double)(ValRouge / 255.0), (double)(valVert / 255.0),
                                     (double)(valBleu / 255.0));  // Couleur ligne
                    PoDoFo::PdfString mUtf8(reinterpret_cast<const PoDoFo::pdf_utf8*>(valTexte.toUtf8().constData()));
                    Painter.DrawMultiLineText(PoDoFo::PdfRect(valDebutX, PageHeight - valDebutY - valHauteur, valLargeur, valHauteur), mUtf8,
                                              mAlignLargeur, mAlignHauteur);
                    qDebug() << " effectué";
                }
            } break;
            case TypeCommande::DESSINETEXTEQUESTION: {
                {
                    qDebug() << "INFO: DESSINETEXTEQUESTION ... ";

                    Painter.SetColor((double)(ValRouge / 255.0), (double)(valVert / 255.0),
                                     (double)(valBleu / 255.0));  // Couleur ligne
                    if (ValSplit < 2) {
                        PoDoFo::PdfString mUtf8(reinterpret_cast<const PoDoFo::pdf_utf8*>(valDefautquestion.toUtf8().constData()));
                        Painter.DrawMultiLineText(PoDoFo::PdfRect(valDebutX, PageHeight - valDebutY - valHauteur, valLargeur, valHauteur), mUtf8,
                                                  mAlignLargeur, mAlignHauteur, false);
                    } else {
                        double _gap = valLargeur / ValSplit;
                        for (qsizetype i = 0; i < ValSplit; i++) {
                            PoDoFo::PdfString mUtf8(
                                reinterpret_cast<const PoDoFo::pdf_utf8*>(QString(valDefautquestion.mid(i, 1)).toUtf8().constData()));
                            Painter.DrawMultiLineText(PoDoFo::PdfRect(valDebutX + (i * _gap), PageHeight - valDebutY - valHauteur, _gap, valHauteur),
                                                      mUtf8, mAlignLargeur, mAlignHauteur, false);
                        }
                    }
                    qDebug() << " effectué";
                }
            } break;
            case TypeCommande::DESSINETEXTEMULTILIGNEQUESTION: {
                {
                    qDebug() << "INFO: DESSINETEXTEMULTILIGNEQUESTION ...";
                    Painter.SetColor((double)(ValRouge / 255.0), (double)(valVert / 255.0),
                                     (double)(valBleu / 255.0));  // Couleur ligne
                    PoDoFo::PdfString mUtf8(reinterpret_cast<const PoDoFo::pdf_utf8*>(valDefautquestion.toUtf8().constData()));
                    Painter.DrawMultiLineText(PoDoFo::PdfRect(valDebutX, PageHeight - valDebutY - valHauteur, valLargeur, valHauteur), mUtf8,
                                              mAlignLargeur, mAlignHauteur);
                    qDebug() << " effectué";
                }
            } break;
            case TypeCommande::INSEREIMAGE: {
                QString valChemin = RetourneCleStr(vecCommandeList[lArg].mVecCommande, "chemin");
                {
                    qDebug() << "INFO: INSEREIMAGE ... ";
                    try {
                        QString CheminImage = BaseModelePath + valChemin;
                        if (QFile::exists(CheminImage)) {
                            PoDoFo::PdfImage image(&Document);
                            image.LoadFromFile(CheminImage.toStdString().c_str());
                            double mScaleW = (valLargeur / image.GetWidth());
                            double mScaleH = (valHauteur / image.GetHeight());
                            Painter.DrawImage(valDebutX, PageHeight - valDebutY - valHauteur, &image, mScaleW, mScaleH);
                        }
                        qDebug() << " effectué";
                    } catch (const std::exception&) {
                        qDebug() << " non effectué, erreur de chargement de l'image " << valChemin;
                    } catch (const PoDoFo::PdfError&) {
                        qDebug() << " non effectué, erreur de chargement de l'image " << valChemin;
                    }
                }
            } break;
            case TypeCommande::DESSINECHECKBOX: {
                {
                    qDebug() << "INFO: DESSINECHECKBOX ... ";
                    Painter.SetStrokingColor((double)(ValRouge / 255.0), (double)(valVert / 255.0),
                                             (double)(valBleu / 255.0));  // Couleur ligne
                    Painter.DrawLine(valDebutX, PageHeight - valDebutY, valDebutX + valLargeur, PageHeight - valDebutY - valHauteur);
                    Painter.Stroke();
                    Painter.DrawLine(valDebutX, PageHeight - valDebutY - valHauteur, valDebutX + valLargeur, PageHeight - valDebutY);
                    Painter.Stroke();
                    Painter.Rectangle(valDebutX, PageHeight - valDebutY - valHauteur, valLargeur, valHauteur);
                    Painter.Stroke();
                    qDebug() << " effectué";
                }
            } break;
            case TypeCommande::DESSINECHECKBOXQUESTION: {
                {
                    qDebug() << "INFO: DESSINECHECKBOXQUESTION ... ";
                    Painter.SetStrokingColor((double)(ValRouge / 255.0), (double)(valVert / 255.0),
                                             (double)(valBleu / 255.0));  // Couleur ligne
                    Painter.SetStrokeWidth(valEpaisseur);
                    if (valDefautquestion == "oui") {
                        Painter.DrawLine(valDebutX, PageHeight - valDebutY, valDebutX + valLargeur, PageHeight - valDebutY - valHauteur);
                        Painter.Stroke();
                        Painter.DrawLine(valDebutX, PageHeight - valDebutY - valHauteur, valDebutX + valLargeur, PageHeight - valDebutY);
                        Painter.Stroke();
                    }
                    Painter.Rectangle(valDebutX, PageHeight - valDebutY - valHauteur, valLargeur, valHauteur);
                    Painter.Stroke();
                    qDebug() << " effectué";
                }
            } break;
            case TypeCommande::DESSINEMULTICHECKBOXQUESTION: {
                {
                    qDebug() << "INFO: DESSINECHECKBOXQUESTION ... ";
                    Painter.SetStrokingColor((double)(ValRouge / 255.0), (double)(valVert / 255.0),
                                             (double)(valBleu / 255.0));  // Couleur ligne
                    Painter.Rectangle(valDebutX1, PageHeight - valDebutY1 - valHauteur, valLargeur, valHauteur);
                    Painter.Rectangle(valDebutX2, PageHeight - valDebutY2 - valHauteur, valLargeur, valHauteur);
                    if (valDefautquestion == "oui") {
                        Painter.DrawLine(valDebutX1, PageHeight - valDebutY1, valDebutX1 + valLargeur, PageHeight - valDebutY1 - valHauteur);
                        Painter.Stroke();
                        Painter.DrawLine(valDebutX1, PageHeight - valDebutY1 - valHauteur, valDebutX1 + valLargeur, PageHeight - valDebutY1);
                        Painter.Stroke();
                    } else {
                        Painter.DrawLine(valDebutX2, PageHeight - valDebutY2, valDebutX2 + valLargeur, PageHeight - valDebutY2 - valHauteur);
                        Painter.Stroke();
                        Painter.DrawLine(valDebutX2, PageHeight - valDebutY2 - valHauteur, valDebutX2 + valLargeur, PageHeight - valDebutY2);
                        Painter.Stroke();
                    }
                    Painter.Stroke();
                    qDebug() << " effectué";
                }
            } break;
            case TypeCommande::PAGESUIVANTE: {
                Painter.FinishPage();
                PoDoFo::PdfPage* pPage = Document.InsertPage(PoDoFo::PdfRect(0.0, 0.0, 595.0, 842.0), NombrePageCree);
                Painter.SetPage(pPage);
                NombrePageCree++;
            } break;
            default:
                break;
        }
    }
    EnvoiLogVersMainWindows(QString("Page de garde créée avec %1 page(s)").arg(QString::number(NombrePageCree)));
    return NombrePageCree;
}

/** ItemCount:
    Retourne le nombre de bloc en mémoire

    @return int: Nombre de bloc en mémoire
*/
int PDGHelper::ItemCount() {
    return vecCommandeList.size();
}

/** ItemQuestionCount:
    Retourne le nombre de bloc questions

    @return int: Nombre de bloc question en mémoire
*/
int PDGHelper::ItemQuestionCount() {
    return ListeQuestion.size();
}

/** ClearList:
    Efface la liste de question

    @return aucun
*/
void PDGHelper::ClearList() {
    vecCommandeList.clear();
    ListeQuestion.clear();
}
/** GetMaxFontSize:
    Permet de définit la taille de police adéquate si le texte dépasse de sa
   zone définie

    @param Police
    @param rectWidth
    @param rectHeight
    @param mTaillePolice
    @param Texte
    @param CorrectionEspace : Coeef de majoration, défaut 1.0
    @return double taille de la police
*/
double PDGHelper::GetMaxFontSize(PoDoFo::PdfFont*& Police,
                                 double rectWidth,
                                 double rectHeight,
                                 double mTaillePolice,
                                 QString Texte,
                                 double CorrectionEspace) {
    double lTaillePolice = mTaillePolice;
    while (true) {
        Police->SetFontSize((float)lTaillePolice);
        double h = Police->GetFontMetrics()->GetLineSpacing() /*+
                                                                 std::abs(Police->GetFontMetrics()->GetDescent())*/
            ;
        PoDoFo::PdfString utf8Mesure(reinterpret_cast<const PoDoFo::pdf_utf8*>(Texte.toUtf8().constData()));
        double w                = Police->GetFontMetrics()->StringWidth(utf8Mesure);
        double SurfaceTexte     = h * w * CorrectionEspace;
        double SurfaceRectangle = rectWidth * rectHeight;
        if (SurfaceTexte < SurfaceRectangle)
            break;
        lTaillePolice -= 0.1;
    }
    return lTaillePolice;
}

/** RetourneCleStr:
    Retourne la valeur QSTRING de la clé, si non trouvé, retourne
   "<!--CleNonTrouve-->"

    @param lVecKey Vecteur avec les clés
    @param Clé à retourner
    @return QString
*/
QString PDGHelper::RetourneCleStr(QVector<CmdKeys>& lVecKey, QString Cle) {
    for (qsizetype i = 0; i < lVecKey.size(); i++) {
        if (lVecKey[i].Keys == Cle) {
            QString mVal = lVecKey[i].Valeur;
            if (mVal.mid(0, 1) == "\"")
                mVal = mVal.mid(1);
            if (mVal.mid(mVal.length() - 1, 1) == "\"")
                mVal = mVal.mid(0, mVal.length() - 1);
            mVal = QString(mVal).replace("{RetourLigne}", "\n");
            mVal = QString(mVal).replace("{Site}", ArrayFromREEMAKER.ReferenceSite);
            mVal = QString(mVal).replace("{NumeroTranche}", ArrayFromREEMAKER.NumeroTranche);
            mVal = QString(mVal).replace("{ReferenceREE}", ArrayFromREEMAKER.ReferenceREE);
            mVal = QString(mVal).replace("{IndiceREE}", ArrayFromREEMAKER.IndiceREE);
            return mVal;
            break;
        }
    }
    return "<!--CleNonTrouve-->";
}

/** RetourneCleDouble:
    Retourne la valeur DOUBLE de la clé, si non trouvé, retourne
   (double)INT16_MAX

    @param lVecKey Vecteur avec les clés
    @param Clé à retourner
    @return double
*/
double PDGHelper::RetourneCleDouble(QVector<CmdKeys>& lVecKey, QString Cle) {
    for (qsizetype i = 0; i < lVecKey.size(); i++) {
        if (lVecKey[i].Keys == Cle) {
            return (double)lVecKey[i].Valeur.toDouble();
            break;
        }
    }
    return (double)INT16_MAX;
}

/** RetourneCleInt:
    Retourne la valeur INT de la clé, si non trouvé, retourne INT16_MAX

    @param lVecKey Vecteur avec les clés
    @param Clé à retourner
    @return int
*/
int PDGHelper::RetourneCleInt(QVector<CmdKeys>& lVecKey, QString Cle) {
    for (qsizetype i = 0; i < lVecKey.size(); i++) {
        if (lVecKey[i].Keys == Cle) {
            return lVecKey[i].Valeur.toInt();
            break;
        }
    }
    return INT16_MAX;
}

/** RetourneCleBool:
    Retourne la valeur bool de la clé, si non trouvé, retourne false

    @param lVecKey Vecteur avec les clés
    @param Clé à retourner
    @return bool
*/
bool PDGHelper::RetourneCleBool(QVector<CmdKeys>& lVecKey, QString Cle) {
    for (qsizetype i = 0; i < lVecKey.size(); i++) {
        if (lVecKey[i].Keys == Cle) {
            return true;
            break;
        }
    }
    return false;
}

void PDGHelper::EnvoiLogVersMainWindows(QString Message) {
    EnvoiLogMessage(Message);
}
