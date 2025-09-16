/* -------------------------------------------------------------------------- */
/*      REEMaker 5 -- Grégory WENTZEL (c) 2023   Code sous licence GPL3       */
/* -------------------------------------------------------------------------- */

#ifndef BLOCQUESTION_H
#define BLOCQUESTION_H

#include <QApplication>
#include <QCheckBox>
#include <QDateTime>
#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QTextEdit>
#include <QValidator>
#include <QWidget>

/** blocQuestion:
    Class du bloc question pour intérroger l'utilisateur avec les bonnes valeurs
*/
class blocQuestion : public QWidget
{
    Q_OBJECT

  public:
    enum TypeBloc : int
    {
        Bloc_Texte_Simple = 0,
        Bloc_Texte_Multiligne = 2,
        Bloc_Case_Coche = 4
    };
    struct ItemDefinition
    {
        QString NomControle;
        qint64 IndexControle;
        TypeBloc TypeDeBloc;
        QString Question;
        QString Aide;
        QString Reponse;
        QString NomVariable;
        bool EstObligatoire;
        bool EtatCoche;
        bool TexteMajuscule;
        bool TexteMinuscule;
        bool TexteDecimal;
        qint64 Maximum;
    };
    explicit blocQuestion(QWidget* parent = nullptr, ItemDefinition mIDEF = ItemDefinition());
    ItemDefinition* RetourneDonnee();
  private slots:

  signals:

  private:
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

    class MyValidatorLOWER : public QValidator
    {
      public:
        MyValidatorLOWER(QObject* parent = nullptr)
          : QValidator(parent)
        {
        }
        State validate(QString& input, int&) const override
        {
            input = input.toUpper();
            return QValidator::Acceptable;
        }
    };

    class MyValidatorDECIMAL : public QValidator
    {
      public:
        MyValidatorDECIMAL(QObject* parent = nullptr)
          : QValidator(parent)
        {
        }
        State validate(QString& input, int&) const override
        {
            QString result;
            for (const QChar c : std::as_const(input)) {
                if (c.isDigit() || c == '.') {
                    result.append(c);
                }
            }
            input = result;
            return QValidator::Acceptable;
        }
    };

    class MultiLineFilter : public QObject
    {
        // Q_OBJECT
      public:
        bool FiltreMajuscule;
        bool FiltreMinuscule;
        bool LimiteMax;
        qint64 ValLimite;

      protected:
        bool eventFilter(QObject* obj, QEvent* event) override
        {
            if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
                QTextEdit* mObject = qobject_cast<QTextEdit*>(obj);
                QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
                // Ici on laisse passer les codes claviers de bases
                if ((keyEvent->key() == Qt::Key_Shift) || (keyEvent->key() == Qt::Key_Backspace) || (keyEvent->key() == Qt::Key_Alt) ||
                    (keyEvent->key() == Qt::Key_Delete) || (keyEvent->key() == Qt::Key_Up) || (keyEvent->key() == Qt::Key_Down) ||
                    (keyEvent->key() == Qt::Key_Left) || (keyEvent->key() == Qt::Key_Right))
                    return QObject::eventFilter(obj, event);
                QTextCursor c = mObject->textCursor();
                int current = c.position();
                if (LimiteMax) {
                    if (mObject->toPlainText().length() > ValLimite) {
                        mObject->setText(mObject->toPlainText().mid(0, ValLimite));
                        current = ValLimite - 1;
                    }
                }
                if (FiltreMajuscule)
                    mObject->setText(mObject->toPlainText().toUpper());
                if (FiltreMinuscule)
                    mObject->setText(mObject->toPlainText().toLower());

                c = mObject->textCursor();
                c.setPosition(current);
                c.setPosition(current, QTextCursor::KeepAnchor);
                mObject->setTextCursor(c);

                return QObject::eventFilter(obj, event); /*true;*/
            } else if (event->type() == QEvent::FocusIn) {
                QListWidget* mListWidget = qobject_cast<QListWidget*>(obj->parent()->parent()->parent());
                auto NomBlocQuestion = obj->parent()->objectName();
                if (mListWidget->count() > 1)
                    for (int i = 1 /*HEADER*/; i < mListWidget->count(); ++i) {
                        auto item = mListWidget->item(i);
                        auto itemWidget = qobject_cast<blocQuestion*>(mListWidget->itemWidget(item));
                        if (itemWidget->objectName() == NomBlocQuestion) {
                            mListWidget->scrollToItem(item, QAbstractItemView::EnsureVisible);
                            break;
                        }
                    }
                return QObject::eventFilter(obj, event); /*true;*/
            } else {
                // standard event processing
                return QObject::eventFilter(obj, event);
            }
        }
    };
    class SingleLineFilter : public QObject
    {
        // Q_OBJECT
      public:
      protected:
        bool eventFilter(QObject* obj, QEvent* event) override
        {
            if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
                //            QTextEdit* mObject = qobject_cast<QTextEdit*>(obj);
                //            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
                return QObject::eventFilter(obj, event); /*true;*/
            } else if (event->type() == QEvent::FocusIn) {
                QListWidget* mListWidget = qobject_cast<QListWidget*>(obj->parent()->parent()->parent());
                auto NomBlocQuestion = obj->parent()->objectName();
                if (mListWidget->count() > 1)
                    for (int i = 1 /*HEADER*/; i < mListWidget->count(); ++i) {
                        auto item = mListWidget->item(i);
                        auto itemWidget = qobject_cast<blocQuestion*>(mListWidget->itemWidget(item));
                        if (itemWidget->objectName() == NomBlocQuestion) {
                            mListWidget->scrollToItem(item, QAbstractItemView::EnsureVisible);
                            break;
                        }
                    }
                return QObject::eventFilter(obj, event); /*true;*/
            } else {
                // standard event processing
                return QObject::eventFilter(obj, event);
            }
        }
    };
    class CheckBoxFilter : public QObject
    {
        // Q_OBJECT
      public:
      protected:
        bool eventFilter(QObject* obj, QEvent* event) override
        {
            if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
                //            QTextEdit* mObject = qobject_cast<QTextEdit*>(obj);
                //            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
                return QObject::eventFilter(obj, event); /*true;*/
            } else if (event->type() == QEvent::FocusIn) {
                QListWidget* mListWidget = qobject_cast<QListWidget*>(obj->parent()->parent()->parent());
                auto NomBlocQuestion = obj->parent()->objectName();
                if (mListWidget->count() > 1)
                    for (int i = 1 /*HEADER*/; i < mListWidget->count(); ++i) {
                        auto item = mListWidget->item(i);
                        auto itemWidget = qobject_cast<blocQuestion*>(mListWidget->itemWidget(item));
                        if (itemWidget->objectName() == NomBlocQuestion) {
                            mListWidget->scrollToItem(item, QAbstractItemView::EnsureVisible);
                            break;
                        }
                    }
                return QObject::eventFilter(obj, event); /*true;*/
            } else {
                // standard event processing
                return QObject::eventFilter(obj, event);
            }
        }
    };
    MultiLineFilter* mMultiLineFilter = new MultiLineFilter();
    SingleLineFilter* mSingleLineFilter = new SingleLineFilter();
    CheckBoxFilter* mCheckBoxFilter = new CheckBoxFilter();
    ItemDefinition* mItemDef;
    QLineEdit* mItemLineEdit;
    QTextEdit* mItemTextEdit;
    QCheckBox* mItemCheckbox;
    MyValidatorUPPER* validUPPER;
    MyValidatorLOWER* validLOWER;
    MyValidatorDECIMAL* validDECIMAL;
    QString NomVariable;
};

#endif // BLOCQUESTION_H
