#ifndef IMAGEUTILS_H
#define IMAGEUTILS_H

#pragma once

#include <QColor>
#include <QColorSpace>
#include <QFont>
#include <QImage>
#include <QImageReader>
#include <QImageWriter>
#include <QMap>
#include <QObject>
#include <QRect>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QVector>
#include <functional>

struct ImageInfo
{
    QSize size;
    QByteArray format; // "JPEG", "PNG", etc.
    bool hasAlpha = false;
    int depth = 0;             // bits per pixel
    qreal dotsPerMeterX = 0.0; // DPI = dpm * 0.0254
    qreal dotsPerMeterY = 0.0;
    QString colorSpaceName;          // heuristique
    QMap<QString, QString> textKeys; // métadonnées texte lisibles par QImageReader
};

class ImageUtils : public QObject
{
    Q_OBJECT
  public:
    explicit ImageUtils(QObject* parent = nullptr);

    enum class PositionTexte
    {
        HautGauche,
        HautMilieu,
        HautDroite,
        Centre,
        BasGauche,
        BasMilieu,
        BasDroite
    };

    // Chargement / sauvegarde
    static QImage load(const QString& path, ImageInfo* outInfo = nullptr, QString* error = nullptr);
    static bool save(const QImage& img,
                     const QString& path,
                     const QByteArray& format = QByteArray(),
                     int quality = -1,
                     const QMap<QString, QString>& textKeys = {},
                     QString* error = nullptr);

    static ImageInfo probe(const QString& path, QString* error = nullptr);

    // Transformations géométriques
    static QImage resizeFit(const QImage& src,
                            const QSize& target,
                            Qt::AspectRatioMode arm = Qt::KeepAspectRatio,
                            Qt::TransformationMode tm = Qt::SmoothTransformation);
    static QImage crop(const QImage& src, const QRect& rect);
    static QImage rotate(const QImage& src, qreal degrees, const QColor& bg = Qt::transparent, Qt::TransformationMode tm = Qt::SmoothTransformation);
    static QImage rotate90(const QImage& src);
    static QImage rotate180(const QImage& src);
    static QImage rotate270(const QImage& src);
    static QImage flipH(const QImage& src);
    static QImage flipV(const QImage& src);

    // Conversions & ajustements
    static QImage toGrayscale(const QImage& src);
    static QImage toSepia(const QImage& src, qreal intensity = 1.0); // 0..1
    static QImage adjustBCG(const QImage& src, int brightness /*-100..100*/, int contrast /*-100..100*/, qreal gamma /*0.1..5.0*/);
    static QImage adjustSaturation(const QImage& src, int saturation /*-100..100*/);

    static QImage overlayText(const QImage& base,
                              const QString& text,
                              const PositionTexte& position,
                              const QFont& font,
                              const QColor& color = QColor(255, 255, 255),
                              qreal opacity = 0.7,
                              int padding = 0,
                              const QColor& shadow = QColor(0, 0, 0, 128));
    static QImage overlayImage(const QImage& base,
                               const QImage& watermark,
                               const QPoint& topLeft,
                               qreal opacity = 0.5,
                               const QSize& scaleTo = QSize());

    static QImage convertFormat(const QImage& src, QImage::Format f);
    static QImage convertToSRGB(const QImage& src);

    // Histogrammes
    static QVector<quint32> histogramGray(const QImage& src);
    static QImage equalizeHistogramGray(const QImage& src);

    // Compression pilotée par taille
    static bool compressToMaxBytes(const QImage& img,
                                   const QString& outPath,
                                   qint64 maxBytes,
                                   QByteArray formatHint = "JPEG",
                                   int minQuality = 10,
                                   int maxQuality = 95,
                                   QString* error = nullptr);

    // Batch (pipeline)
    static QStringList batchProcess(const QStringList& inputPaths,
                                    const QString& outputDir,
                                    const std::function<QImage(const QImage&)>& pipeline,
                                    QByteArray formatHint = QByteArray(),
                                    int quality = -1);
};

#endif // IMAGEUTILS_H
