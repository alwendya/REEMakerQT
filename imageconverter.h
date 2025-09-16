#ifndef IMAGECONVERTER_H
#define IMAGECONVERTER_H

#include <QApplication>
#include <QDebug>
#include <QDialog>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSize>
#include <QString>
#include <QVBoxLayout>

#include <podofo/podofo.h>

#include "OverlayWidget.h"

// Les énumérations sont inchangées
enum class ImageSizeOption
{
    FitToPage,
    OriginalSize
};

enum class FitOption
{
    Ask,
    AlwaysFit,
    AlwaysOriginal
};

class ImageConverter
{
  public:
    ImageConverter(QWidget* parent = nullptr);
    bool convertImageToPdf(const QString& imagePath, const QString& outputPath);

  private:
    ImageSizeOption getImageHandlingOption(const QSize& imageSize, const QSize& pageSize);
    bool shouldFitImage(const QSize& imageSize, const QSize& pageSize, const QString& imagePath);
    double pixelsToPdfUnits(double pixels, double dpi = 72.0);

    // Variables pour mémoriser les choix utilisateur
    FitOption s_smallerImageOption;
    FitOption s_largerImageOption;

    bool m_fitToPage;
    bool m_alwaysApply;

    QWidget* m_Parent;
};

#endif // IMAGECONVERTER_H
