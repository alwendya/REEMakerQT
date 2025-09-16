#include "customlistitemwidget.h"
#include <QFileInfo>
#include <QHBoxLayout>
#include <QStyle>
#include <QToolTip>

CustomListItemWidget::CustomListItemWidget(const QString& fullPath, QWidget* parent)
  : QWidget(parent)
  , m_fullPath(fullPath)
{
    // 1. Créez l'icône en fonction du type de fichier
    iconLabel = new QLabel(this);
    QPixmap iconPixmap;

    QFileInfo fileInfo(fullPath);
    m_extensionText = fileInfo.suffix().toLower();

    if (m_extensionText == "pdf") {
        // Chargez votre icône PDF depuis les ressources Qt
        // Assurez-vous que ':ressources/icons/pdf_icon.png' est bien défini
        // dans votre fichier .qrc
        iconPixmap = QPixmap(":/PDFFile24");
        m_extension = typeFichier::PDF;
    } else if (m_extensionText == "jpg" || m_extensionText == "jpeg" || m_extensionText == "png" || m_extensionText == "bmp" ||
               m_extensionText == "tiff" || m_extensionText == "webp") {
        // Chargez votre icône d'image depuis les ressources Qt
        iconPixmap = QPixmap(":/Icone32/picture.png");
        m_extension = typeFichier::Image;
    } else {
        // Utilisez une icône par défaut si le type n'est pas reconnu
        // Vous pouvez utiliser une icône générique du système si vous voulez
        iconPixmap = QIcon::fromTheme("document-generic").pixmap(32, 32); // Ex: Icône générique de document
        m_extension = typeFichier::Autre;
    }

    if (!iconPixmap.isNull()) {
        iconLabel->setPixmap(iconPixmap.scaled(24, 24, Qt::KeepAspectRatio,
                                               Qt::SmoothTransformation)); // Taille de l'icône
        iconLabel->setFixedSize(QSize(24, 24));                            // Définit la taille fixe du QLabel pour l'icône
    }

    // Crée les autres widgets (inchangé)
    fullPathLabel = new QLabel(fullPath);
    fullPathLabel->hide(); // Rend le QLabel invisible

    compactPathLabel = new QLabel(getCompactFileName(fullPath));
    compactPathLabel->setToolTip(fullPath); // Affiche le nom de fichier complet au survol

    fromLabel = new QLabel("À partir de:");
    startSpinBox = new QSpinBox();
    startSpinBox->setMinimum(1);
    startSpinBox->setMaximum(99999);

    toLabel = new QLabel("à:");
    endSpinBox = new QSpinBox();
    endSpinBox->setMinimum(1);
    endSpinBox->setMaximum(99990);

    // Met en place le layout horizontal
    QHBoxLayout* layout = new QHBoxLayout(this);

    // Ajoute l'icône en premier
    layout->addWidget(iconLabel);
    layout->addSpacing(5); // Un petit espacement après l'icône

    // Ajoute le reste des widgets existants
    layout->addWidget(compactPathLabel);
    layout->addSpacing(20);
    // --- Ajout du QSpacerItem horizontal ---
    layout->addStretch();
    layout->addWidget(fromLabel);
    layout->addWidget(startSpinBox);
    layout->addWidget(toLabel);
    layout->addWidget(endSpinBox);
    if (m_extension == typeFichier::Autre || m_extension == typeFichier::Image) {
        fromLabel->setVisible(false);
        startSpinBox->setVisible(false);
        toLabel->setVisible(false);
        endSpinBox->setVisible(false);
    }

    // Ce layout gérera la disposition de tous les widgets.
}

CustomListItemWidget::typeFichier
CustomListItemWidget::getExtension() const
{
    return m_extension;
}

QString
CustomListItemWidget::getExtensionText() const
{
    return m_extensionText;
}

QString
CustomListItemWidget::getCompactFileName(const QString& fullPath, int maxLength)
{
    QFileInfo fileInfo(fullPath);
    QString fileName = fileInfo.fileName(); // Récupère le nom du fichier

    if (fileName.length() > maxLength) {
        // Calcule le nombre de caractères à garder de chaque côté
        int keepFromStart = maxLength / 2;
        int keepFromEnd = maxLength - keepFromStart - 3; // -3 pour les points de suspension "..."

        // Assure qu'on ne prenne pas plus de caractères qu'il n'y en a dans le nom de fichier
        if (keepFromStart + keepFromEnd > fileName.length()) {
            keepFromStart = fileName.length() / 2;
            keepFromEnd = fileName.length() - keepFromStart - 3;
            if (keepFromEnd < 0)
                keepFromEnd = 0; // Cas où le nom est trop court pour les ..., 3 points
        }

        QString startPart = fileName.left(keepFromStart);
        QString endPart = fileName.right(keepFromEnd);

        return startPart + "..." + endPart;
    } else {
        // Si le nom du fichier est plus court que maxLength, retourne le nom complet
        return fileName;
    }
}

QString
CustomListItemWidget::getFullPath() const
{
    return m_fullPath;
}

int
CustomListItemWidget::getStartPage() const
{
    return startSpinBox->value();
}

int
CustomListItemWidget::getEndPage() const
{
    return endSpinBox->value();
}

int
CustomListItemWidget::getEndPageMax() const
{
    return endSpinBox->maximum();
}
void
CustomListItemWidget::setStartPage(int valeur)
{
    startSpinBox->setValue(valeur);
}

void
CustomListItemWidget::setEndPage(int valeur)
{
    endSpinBox->setValue(valeur);
}
void
CustomListItemWidget::setStartPageMax(int valeur)
{
    startSpinBox->setMaximum(valeur);
}

void
CustomListItemWidget::setEndPageMax(int valeur)
{
    endSpinBox->setMaximum(valeur);
}
