#include "blocQuestion.h"
///
/// \brief blocQuestion::blocQuestion   Créer un bloc en prenant en entrée les paramètres suivants :
/// \param parent   Handle du contrôle parent
/// \param mIDEF    ItemDefinition qui va remplir le contrôle
/// \param Sombre   Le contrôle doit il être dessiné avec une apparence sombre
///
blocQuestion::blocQuestion(QWidget *parent, ItemDefinition mIDEF, bool Sombre)
    : QWidget{parent}
{
    validUPPER = new MyValidatorUPPER(parent);
    validLOWER = new MyValidatorLOWER(parent);
    validDECIMAL = new MyValidatorDECIMAL(parent);

    mItemLineEdit = new QLineEdit("");
    mItemTextEdit = new QTextEdit("");
    mItemCheckbox = new QCheckBox();

    mItemDef = new ItemDefinition();
    mItemDef->Aide =mIDEF.Aide;
    mItemDef->EstObligatoire = mIDEF.EstObligatoire;
    mItemDef->EtatCoche = mIDEF.EtatCoche;
    mItemDef->IndexControle = mIDEF.IndexControle;
    mItemDef->NomControle = mIDEF.NomControle;
    mItemDef->Question = mIDEF.Question;
    mItemDef->TypeDeBloc = mIDEF.TypeDeBloc;
    mItemDef->Maximum = mIDEF.Maximum;
    mItemDef->Reponse = mIDEF.Reponse;
    this->setObjectName(mItemDef->NomControle);
    QHBoxLayout *layout = new QHBoxLayout( this );
    layout->setContentsMargins(2,2,2,2);
    QLabel *LabelObligatoire = new QLabel(mIDEF.EstObligatoire?"X":"");
    if (mIDEF.EstObligatoire)
    {
        LabelObligatoire->setToolTip("Ce champ est à renseigner obligatoirement");
        LabelObligatoire->setPixmap(QPixmap(":/IconeWarning32").scaled(16,16,Qt::KeepAspectRatio,Qt::TransformationMode::SmoothTransformation));
    }
    LabelObligatoire->setAlignment(Qt::AlignCenter);
    LabelObligatoire->setFixedWidth(32);
    layout->addWidget(LabelObligatoire);
    QLabel *LabelTexte = new QLabel(mIDEF.Question);
    LabelTexte->setFixedWidth(350);
    if (mIDEF.EstObligatoire)
    {
        auto mFont = LabelObligatoire->font();
        mFont.setBold(true);
        LabelTexte->setFont(mFont);
    }
    layout->addWidget(LabelTexte);
    QLabel *LabelAide = new QLabel();
    if (mIDEF.Aide.length()>0)
        LabelAide->setPixmap(QPixmap(":/IconeHelp32").scaled(16,16,Qt::KeepAspectRatio,Qt::TransformationMode::SmoothTransformation));
    LabelAide->setAlignment(Qt::AlignCenter);
    LabelAide->setToolTip(mIDEF.Aide);
    LabelAide->setToolTipDuration(0);
    LabelAide->setFixedWidth(32);
    layout->addWidget(LabelAide);

    switch (mIDEF.TypeDeBloc) {
    case Bloc_Texte_Simple:
    {
        if (mItemDef->Maximum > 0)
        {
            mItemLineEdit->setMaxLength(mItemDef->Maximum);
        }
        mItemLineEdit->setText(mIDEF.Reponse);
        if (Sombre)
            mItemLineEdit->setStyleSheet("border: 1px solid white; background: DimGray;");
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
    }
        break;
    case Bloc_Texte_Multiligne:
    {
        mItemTextEdit->setText(mIDEF.Reponse);
        mMultiLineFilter->FiltreMinuscule = mIDEF.TexteMinuscule;
        mMultiLineFilter->FiltreMajuscule = mIDEF.TexteMajuscule;
        mMultiLineFilter->LimiteMax = (mItemDef->Maximum > 0);
        mMultiLineFilter->ValLimite = mItemDef->Maximum;
        mItemTextEdit->installEventFilter(mMultiLineFilter);
        mItemTextEdit->setTabChangesFocus(true);
        mItemTextEdit->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
        if (Sombre)
            mItemTextEdit->setStyleSheet("border: 1px solid white; background: DimGray;");
        if (mIDEF.Aide != "")
            mItemTextEdit->setStatusTip(mIDEF.Aide);
        layout->addWidget(mItemTextEdit);
    }
        break;
    case Bloc_Case_Coche:
    {
        if (mIDEF.Aide != "")
            mItemCheckbox->setStatusTip(mIDEF.Aide);
        mItemCheckbox->setChecked(mIDEF.EtatCoche);
        mItemCheckbox->installEventFilter(mCheckBoxFilter);
        layout->addWidget(mItemCheckbox);
    }
        break;
    default:
        break;
    }
}

///
/// \brief blocQuestion::RetourneDonnee     Retourne le contenu de ce bloc
/// \return
///
blocQuestion::ItemDefinition* blocQuestion::RetourneDonnee()
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
    return mItemDef;
}
