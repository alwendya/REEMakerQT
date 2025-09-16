/* -------------------------------------------------------------------------- */
/*      REEMaker 5 -- Grégory WENTZEL (c) 2023   Code sous licence GPL3       */
/* -------------------------------------------------------------------------- */

#include "blocQuestion.h"
/** blocQuestion:
    Créer un bloc en prenant en entrée les paramètres suivants:

    @param QWidget *parent
    @param ItemDefinition mIDEF
    @param bool Sombre
    @return aucun
*/
blocQuestion::blocQuestion(QWidget* parent, ItemDefinition mIDEF)
  : QWidget{ parent }
{
    validUPPER = new MyValidatorUPPER(parent);
    validLOWER = new MyValidatorLOWER(parent);
    validDECIMAL = new MyValidatorDECIMAL(parent);

    mItemLineEdit = new QLineEdit("");
    mItemTextEdit = new QTextEdit("");
    mItemCheckbox = new QCheckBox();

    mItemDef = new ItemDefinition();
    mItemDef->Aide = mIDEF.Aide;
    mItemDef->EstObligatoire = mIDEF.EstObligatoire;
    mItemDef->EtatCoche = mIDEF.EtatCoche;
    mItemDef->IndexControle = mIDEF.IndexControle;
    mItemDef->NomControle = mIDEF.NomControle;
    mItemDef->Question = mIDEF.Question;
    mItemDef->TypeDeBloc = mIDEF.TypeDeBloc;
    mItemDef->Maximum = mIDEF.Maximum;
    mItemDef->Reponse = mIDEF.Reponse;
    NomVariable = mIDEF.NomVariable;
    this->setObjectName(mItemDef->NomControle);
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    QLabel* LabelObligatoire = new QLabel(mIDEF.EstObligatoire ? "X" : "");
    if (mIDEF.EstObligatoire) {
        LabelObligatoire->setToolTip("Ce champ est à renseigner obligatoirement");
        LabelObligatoire->setPixmap(QPixmap(":/IconeWarning32").scaled(16, 16, Qt::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation));
    }
    LabelObligatoire->setAlignment(Qt::AlignCenter);
    LabelObligatoire->setFixedWidth(32);
    layout->addWidget(LabelObligatoire);
    QLabel* LabelTexte = new QLabel(mIDEF.Question);
    LabelTexte->setWordWrap(true);
    LabelTexte->setFixedWidth(350);
    if (mIDEF.EstObligatoire) {
        auto mFont = LabelObligatoire->font();
        mFont.setBold(true);
        LabelTexte->setFont(mFont);
    }
    layout->addWidget(LabelTexte);
    QLabel* LabelAide = new QLabel();
    if (mIDEF.Aide.length() > 0)
        LabelAide->setPixmap(QPixmap(":/IconeHelp32").scaled(16, 16, Qt::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation));
    LabelAide->setAlignment(Qt::AlignCenter);
    LabelAide->setToolTip(mIDEF.Aide);
    LabelAide->setToolTipDuration(0);
    LabelAide->setFixedWidth(32);
    layout->addWidget(LabelAide);

    switch (mIDEF.TypeDeBloc) {
        case Bloc_Texte_Simple: {
            if (mItemDef->Maximum > 0) {
                mItemLineEdit->setMaxLength(mItemDef->Maximum);
            }
            mItemLineEdit->setText(mIDEF.Reponse);
            if (mIDEF.TexteMajuscule)
                mItemLineEdit->setValidator(validUPPER);
            if (mIDEF.TexteMinuscule)
                mItemLineEdit->setValidator(validLOWER);
            if (mIDEF.TexteDecimal)
                mItemLineEdit->setValidator(validDECIMAL);
            if (mIDEF.Aide != "")
                mItemLineEdit->setStatusTip(mIDEF.Aide);
            mItemLineEdit->installEventFilter(mSingleLineFilter);
            layout->addWidget(mItemLineEdit);
        } break;
        case Bloc_Texte_Multiligne: {
            mItemTextEdit->setText(mIDEF.Reponse);
            mMultiLineFilter->FiltreMinuscule = mIDEF.TexteMinuscule;
            mMultiLineFilter->FiltreMajuscule = mIDEF.TexteMajuscule;
            mMultiLineFilter->LimiteMax = (mItemDef->Maximum > 0);
            mMultiLineFilter->ValLimite = mItemDef->Maximum;
            mItemTextEdit->installEventFilter(mMultiLineFilter);
            mItemTextEdit->setTabChangesFocus(true);
            mItemTextEdit->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
            if (mIDEF.Aide != "")
                mItemTextEdit->setStatusTip(mIDEF.Aide);
            layout->addWidget(mItemTextEdit);
        } break;
        case Bloc_Case_Coche: {
            if (mIDEF.Aide != "")
                mItemCheckbox->setStatusTip(mIDEF.Aide);
            mItemCheckbox->setChecked(mIDEF.EtatCoche);
            mItemCheckbox->installEventFilter(mCheckBoxFilter);
            layout->addWidget(mItemCheckbox);
        } break;
        default:
            break;
    }
}

/** RetourneDonnee:
    Retourne le contenu de ce bloc

    @return blocQuestion::ItemDefinition *
*/
blocQuestion::ItemDefinition*
blocQuestion::RetourneDonnee()
{
    switch (mItemDef->TypeDeBloc) {
        case Bloc_Texte_Simple:
            mItemDef->Reponse = mItemLineEdit->text();
            break;
        case Bloc_Texte_Multiligne:
            mItemDef->Reponse = mItemTextEdit->toPlainText();
            break;
        case Bloc_Case_Coche:
            mItemDef->EtatCoche = mItemCheckbox->isChecked();
            break;
        default:
            break;
    }
    mItemDef->NomVariable = NomVariable;
    return mItemDef;
}
