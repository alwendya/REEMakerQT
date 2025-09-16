#include "imageconverter.h"

using namespace PoDoFo;

ImageConverter::ImageConverter(QWidget* parent)
{
    m_Parent = parent;
    // Initialisation des options de choix
    s_smallerImageOption = FitOption::Ask;
    s_largerImageOption = FitOption::Ask;
}

double
ImageConverter::pixelsToPdfUnits(double pixels, double dpi)
{
    // 1 pouce = 72 points
    // points = (pixels / pixels_par_pouce) * points_par_pouce
    return (pixels / dpi) * 72.0;
}

bool
ImageConverter::convertImageToPdf(const QString& imagePath, const QString& outputPath)
{
    std::string utf8Image = imagePath.toUtf8().toStdString();
    std::string utf8OutPath = outputPath.toUtf8().toStdString();
    try {
        bool IsPortrait = false;
        {
            QImage image(imagePath);
            if (image.isNull()) {
                qDebug() << "Erreur: Impossible de charger l'image ou format "
                            "incorrect.";
                return false;
            }
            IsPortrait = (image.height() > image.width());
        }

        PdfMemDocument document;
        PdfPage& page = document.GetPages().CreatePage(PdfPageSize::A4);

        // Page A4 pour référence, orienté en portrait ou paysage
        auto rectPage = IsPortrait ? page.GetRect() : PoDoFo::Rect(0, 0, page.GetRect().Height, page.GetRect().Width);
        auto podofoImage = document.CreateImage();
        podofoImage->Load(utf8Image);
        QSize imageQSize(pixelsToPdfUnits(podofoImage->GetWidth()), pixelsToPdfUnits(podofoImage->GetHeight()));
        QSize pageQSize(rectPage.Width, rectPage.Height);
        PdfPainter painter;

        bool fitToPage = shouldFitImage(imageQSize, pageQSize, imagePath);
        // Defaut 96 dpi
        if (fitToPage) {
            // L'image doit être redimensionnée pour tenir dans la page
            double Echelle =
              !IsPortrait ? (double)pageQSize.width() / (double)imageQSize.width() : (double)pageQSize.height() / (double)imageQSize.height();
            page.SetRect(Rect(0, 0, imageQSize.width() * Echelle, imageQSize.height() * Echelle));
            painter.SetCanvas(page);
            painter.DrawImage(*podofoImage, 0, 0, Echelle, Echelle);
            painter.FinishDrawing();
        } else {
            // L'image est laissée à sa taille originale
            page.SetRect(Rect(0, 0, imageQSize.width(), imageQSize.height()));
            painter.SetCanvas(page);
            painter.DrawImage(*podofoImage, 0, 0);
            painter.FinishDrawing();
        }

        document.Save(utf8OutPath);
        return true;

    } catch (const PdfError& e) {
        qDebug() << "Erreur PoDoFo: " << e.what();
        return false;
    }
}

bool
ImageConverter::shouldFitImage(const QSize& imageSize, const QSize& pageSize, const QString& imagePath)
{
    bool isLarger = imageSize.width() > pageSize.width() || imageSize.height() > pageSize.height();

    FitOption* currentOption = isLarger ? &s_largerImageOption : &s_smallerImageOption;

    if (*currentOption == FitOption::AlwaysFit) {
        return true;
    }
    if (*currentOption == FitOption::AlwaysOriginal) {
        return false;
    }

    // Charge l'image pour le dialogue
    QImage image(imagePath);
    if (image.isNull()) {
        qDebug() << "Erreur: Impossible de charger l'image pour le dialogue.";
        return false;
    }

    QString message;
    if (isLarger) {
        message = "L'image est plus grande qu'une page A4. Que voulez-vous faire ?";
    } else {
        message = "L'image est plus petite qu'une page A4. Que voulez-vous faire ?";
    }

    {
        // Crée l'overlay (flou + modal custom)
        auto* overlay = new OverlayBlurWidget(m_Parent, 4.0, 0.5);

        // Charge une image pour la preview
        QPixmap preview(imagePath);

        ModalConfig cfg;
        cfg.title = "Redimensionnement";
        cfg.message = message;
        cfg.icon = OverlayBlurWidget::iconFromStandard(QStyle::SP_MessageBoxInformation, m_Parent);
        cfg.preview = preview;                // <= ICI la preview
        cfg.previewMaxSize = QSize(600, 450); // bornage (optionnel)
        cfg.buttons = { OverlayBlurWidget::makeButton(0, "Adapter à la page", QDialogButtonBox::ActionRole, true, true),
                        OverlayBlurWidget::makeButton(1, "Taille originale", QDialogButtonBox::ActionRole, false, false),
                        OverlayBlurWidget::makeButton(2, "Toujours adapter à la page", QDialogButtonBox::ActionRole, false, false),
                        OverlayBlurWidget::makeButton(3, "Toujours en taille originale", QDialogButtonBox::ActionRole, false, false) };
        cfg.clickOutsideToClose = false;
        cfg.escapeButtonId = -1;
        int result = overlay->execModal(cfg);
        if (result == 0) {
            // Adapter à la page
            m_fitToPage = true;
            m_alwaysApply = false;
        }
        if (result == 1) {
            // Taille originale
            m_fitToPage = false;
            m_alwaysApply = false;
        }
        if (result == 2) {
            // Toujours adapter à la page
            m_fitToPage = true;
            m_alwaysApply = true;
        }
        if (result == 3) {
            // Toujours en taille originale
            m_fitToPage = false;
            m_alwaysApply = true;
        }
        if (result >= 0 && result <= 3) {
            if (m_alwaysApply) {
                *currentOption = m_fitToPage ? FitOption::AlwaysFit : FitOption::AlwaysOriginal;
            }
            return true;
        }
    }

    return false; // L'utilisateur a fermé le dialogue
}
