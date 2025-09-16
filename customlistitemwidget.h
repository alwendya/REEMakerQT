#ifndef CUSTOMLISTITEMWIDGET_H
#define CUSTOMLISTITEMWIDGET_H

#include <QLabel>
#include <QSpinBox>
#include <QWidget>

class CustomListItemWidget : public QWidget
{
    Q_OBJECT

  public:
    enum typeFichier
    {
        Image,
        PDF,
        Autre
    };
    explicit CustomListItemWidget(const QString& fullPath, QWidget* parent = nullptr);

    // Retourne le nom de fichier complet
    QString getFullPath() const;
    typeFichier getExtension() const;
    QString getExtensionText() const;

    // Retourne les valeurs des SpinBox
    int getStartPage() const;
    int getEndPage() const;
    int getEndPageMax() const;
    // Retourne les valeurs des SpinBox
    void setStartPage(int valeur);
    void setEndPage(int valeur);
    void setStartPageMax(int valeur);
    void setEndPageMax(int valeur);

  private:
    QLabel* iconLabel; // Nouveau QLabel pour l'icône
    QLabel* fullPathLabel;
    QLabel* compactPathLabel;
    QLabel* fromLabel;
    QSpinBox* startSpinBox;
    QLabel* toLabel;
    QSpinBox* endSpinBox;

    QString m_fullPath;
    typeFichier m_extension;
    QString m_extensionText;

    QString getCompactFileName(const QString& fullPath, int maxLength = 60);
};

#endif // CUSTOMLISTITEMWIDGET_H
