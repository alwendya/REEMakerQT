#include "imageutils.h"

#include "ImageUtils.h"
#include <QBuffer>
#include <QDir>
#include <QFileInfo>
#include <QPainter>
#include <QTransform>
#include <QtMath>
#include <algorithm>

static inline quint8
clampByte(int v)
{
    return static_cast<quint8>(std::min(255, std::max(0, v)));
}

static inline double
clamp01(double x)
{
    return std::min(1.0, std::max(0.0, x));
}

ImageUtils::ImageUtils(QObject* parent)
  : QObject(parent)
{
}

ImageInfo
ImageUtils::probe(const QString& path, QString* error)
{
    ImageInfo info;
    QImageReader reader(path);
    reader.setAutoTransform(true);

    if (!reader.canRead()) {
        if (error)
            *error = reader.errorString();
        return info;
    }
    info.size = reader.size();
    info.format = reader.format();

    // Tentative d’identifier format sans lire tout le pixel data
    // On lira l’image pour récupérer alpha/depth si besoin (ci-dessous).
    // Métadonnées texte (PNG, TIFF…)
    const auto keys = reader.textKeys();
    for (const auto& k : keys) {
        info.textKeys.insert(k, reader.text(k));
    }

    // Pour obtenir des infos complètes (alpha, depth, dpm), on lit l’image (léger sur JPEG/PNG).
    QImage img = reader.read();
    if (img.isNull()) {
        if (error)
            *error = reader.errorString();
        return info;
    }

    info.hasAlpha = img.hasAlphaChannel();
    info.depth = img.depth();
    info.dotsPerMeterX = img.dotsPerMeterX();
    info.dotsPerMeterY = img.dotsPerMeterY();

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    if (img.colorSpace().isValid()) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        if (img.colorSpace() == QColorSpace::SRgb)
            info.colorSpaceName = "sRGB";
        else
            info.colorSpaceName = "Profile";
#else
        info.colorSpaceName = "Profile";
#endif
    } else {
        info.colorSpaceName = "Unknown";
    }
#else
    info.colorSpaceName = "Unknown";
#endif

    return info;
}

QImage
ImageUtils::load(const QString& path, ImageInfo* outInfo, QString* error)
{
    QImageReader reader(path);
    reader.setAutoTransform(true);
    QImage img = reader.read();
    if (img.isNull()) {
        if (error)
            *error = reader.errorString();
        return {};
    }
    if (outInfo) {
        *outInfo = probe(path); // réutilise la logique ci-dessus
    }
    return img;
}

bool
ImageUtils::save(const QImage& img,
                 const QString& path,
                 const QByteArray& format,
                 int quality,
                 const QMap<QString, QString>& textKeys,
                 QString* error)
{
    QByteArray fmt = format;
    if (fmt.isEmpty()) {
        // déduire depuis l’extension
        fmt = QFileInfo(path).suffix().toUpper().toLatin1();
    }
    QImageWriter writer(path, fmt);
    if (quality >= 0)
        writer.setQuality(quality);
    for (auto it = textKeys.constBegin(); it != textKeys.constEnd(); ++it) {
        writer.setText(it.key(), it.value());
    }
    if (!writer.write(img)) {
        if (error)
            *error = writer.errorString();
        return false;
    }
    return true;
}

QImage
ImageUtils::resizeFit(const QImage& src, const QSize& target, Qt::AspectRatioMode arm, Qt::TransformationMode tm)
{
    if (src.isNull() || !target.isValid())
        return {};
    return src.scaled(target, arm, tm);
}

QImage
ImageUtils::crop(const QImage& src, const QRect& rect)
{
    if (src.isNull() || rect.isEmpty())
        return {};
    QRect r = rect.intersected(QRect(QPoint(0, 0), src.size()));
    if (r.isEmpty())
        return {};
    return src.copy(r);
}

QImage
ImageUtils::rotate(const QImage& src, qreal degrees, const QColor& bg, Qt::TransformationMode tm)
{
    if (src.isNull())
        return {};
    QTransform t;
    t.rotate(degrees);
    QImage rotated = src.convertToFormat(QImage::Format_ARGB32_Premultiplied).transformed(t, tm);

    if (bg.alpha() == 0) {
        return rotated;
    } else {
        QImage out(rotated.size(), QImage::Format_ARGB32_Premultiplied);
        out.fill(bg);
        QPainter p(&out);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        p.drawImage(0, 0, rotated);
        p.end();
        return out;
    }
}

QImage
ImageUtils::rotate90(const QImage& src)
{
    return src.isNull() ? QImage() : src.transformed(QTransform().rotate(90));
}
QImage
ImageUtils::rotate180(const QImage& src)
{
    return src.isNull() ? QImage() : src.transformed(QTransform().rotate(180));
}
QImage
ImageUtils::rotate270(const QImage& src)
{
    return src.isNull() ? QImage() : src.transformed(QTransform().rotate(270));
}

QImage
ImageUtils::flipH(const QImage& src)
{
    if (src.isNull())
        return {};
    return src.flipped(Qt::Horizontal);
}

QImage
ImageUtils::flipV(const QImage& src)
{
    if (src.isNull())
        return {};
    return src.flipped(Qt::Vertical);
}

QImage
ImageUtils::toGrayscale(const QImage& src)
{
    if (src.isNull())
        return {};
    return src.convertToFormat(QImage::Format_Grayscale8);
}

QImage
ImageUtils::toSepia(const QImage& src, qreal intensity)
{
    if (src.isNull())
        return {};
    intensity = std::max(0.0, std::min(1.0, intensity));
    QImage img = src.convertToFormat(QImage::Format_ARGB32);
    const int w = img.width();
    const int h = img.height();

    for (int y = 0; y < h; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(img.scanLine(y));
        for (int x = 0; x < w; ++x) {
            QColor c = QColor::fromRgb(line[x]);
            int r = c.red(), g = c.green(), b = c.blue(), a = c.alpha();
            int tr = std::min(255, static_cast<int>(0.393 * r + 0.769 * g + 0.189 * b));
            int tg = std::min(255, static_cast<int>(0.349 * r + 0.686 * g + 0.168 * b));
            int tb = std::min(255, static_cast<int>(0.272 * r + 0.534 * g + 0.131 * b));
            // mix original ↔ sepia
            int nr = static_cast<int>((1.0 - intensity) * r + intensity * tr);
            int ng = static_cast<int>((1.0 - intensity) * g + intensity * tg);
            int nb = static_cast<int>((1.0 - intensity) * b + intensity * tb);
            line[x] = qRgba(clampByte(nr), clampByte(ng), clampByte(nb), a);
        }
    }
    return img;
}

QImage
ImageUtils::adjustBCG(const QImage& src, int brightness, int contrast, qreal gamma)
{
    if (src.isNull())
        return {};
    brightness = std::max(-100, std::min(100, brightness));
    contrast = std::max(-100, std::min(100, contrast));
    gamma = std::max(0.1, std::min(5.0, gamma));

    // Pré-calcul LUT 0..255
    int bShift = static_cast<int>(brightness * 255 / 100.0);
    double c255 = contrast * 255.0 / 100.0;
    double f = (259.0 * (c255 + 255.0)) / (255.0 * (259.0 - c255)); // contraste

    quint8 LUT[256];
    for (int v = 0; v < 256; ++v) {
        double vc = f * (v - 128.0) + 128.0 + bShift;
        vc = std::min(255.0, std::max(0.0, vc));
        double vn = vc / 255.0;
        double vg = std::pow(vn, 1.0 / gamma) * 255.0;
        LUT[v] = clampByte(static_cast<int>(vg + 0.5));
    }

    QImage img = src.convertToFormat(QImage::Format_ARGB32);
    const int w = img.width();
    const int h = img.height();

    for (int y = 0; y < h; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(img.scanLine(y));
        for (int x = 0; x < w; ++x) {
            QRgb px = line[x];
            int a = qAlpha(px);
            int r = LUT[qRed(px)];
            int g = LUT[qGreen(px)];
            int b = LUT[qBlue(px)];
            line[x] = qRgba(r, g, b, a);
        }
    }
    return img;
}

QImage
ImageUtils::adjustSaturation(const QImage& src, int saturation)
{
    if (src.isNull())
        return {};
    saturation = std::max(-100, std::min(100, saturation));
    double factor = 1.0 + saturation / 100.0;

    QImage img = src.convertToFormat(QImage::Format_ARGB32);
    const int w = img.width();
    const int h = img.height();

    for (int y = 0; y < h; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(img.scanLine(y));
        for (int x = 0; x < w; ++x) {
            QColor c = QColor::fromRgb(line[x]);
            float hF, sF, lF;
            c.getHslF(&hF, &sF, &lF);
            sF = clamp01(sF * factor);
            c.setHslF(hF, sF, lF, c.alphaF());
            line[x] = c.rgba();
        }
    }
    return img;
}

QImage
ImageUtils::overlayText(const QImage& base,
                        const QString& text,
                        const PositionTexte& position,
                        const QFont& font,
                        const QColor& color,
                        qreal opacity,
                        int padding,
                        const QColor& shadow)
{
    if (base.isNull())
        return {};

    QImage out = base.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QPainter p(&out);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setFont(font);
    p.setOpacity(1.0);

    QFontMetrics fm(font);
    QRect textRect = fm.boundingRect(text);
    int imageWidth = out.width();
    int imageHeight = out.height();

    int dpiX = out.dotsPerMeterX() / 39.37;
    double multiplier = (dpiX / 72.0);

    textRect = QRect(0, 0, (textRect.width() * multiplier) + 10, (textRect.height() * multiplier) + 10);

    // Calcul de la position de dessin (drawRect) en fonction de l'énumération
    int x = 0, y = 0;

    // Calcul des coordonnées x
    switch (position) {
        case PositionTexte::HautGauche:
        case PositionTexte::BasGauche:
            x = padding;
            break;
        case PositionTexte::Centre:
        case PositionTexte::HautMilieu:
        case PositionTexte::BasMilieu:
            x = (imageWidth - textRect.width()) / 2;
            break;
        case PositionTexte::HautDroite:
        case PositionTexte::BasDroite:
            x = imageWidth - textRect.width() - padding;
            break;
    }

    // Calcul des coordonnées y
    switch (position) {
        case PositionTexte::HautGauche:
        case PositionTexte::HautMilieu:
        case PositionTexte::HautDroite:
            y = padding; // On ajoute le padding
            break;
        case PositionTexte::Centre:
            y = (imageHeight - textRect.height()) / 2;
            break;
        case PositionTexte::BasGauche:
        case PositionTexte::BasMilieu:
        case PositionTexte::BasDroite:
            y = imageHeight - padding - textRect.height();
            break;
    }

    // Si on utilise le padding pour tous les bords, le calcul du centre est plus complexe
    // Ici, le padding est ajouté à l'extérieur du texte, donc le centre est calculé par rapport aux dimensions de l'image

    // Ajustement pour la position du centre si la hauteur est trop grande par rapport à l'image
    if (position == PositionTexte::Centre && textRect.height() + 2 * padding > imageHeight) {
        // Si le texte avec padding dépasse la hauteur de l'image, on le tronque ou on le centre au mieux
        // Ici, on le centre par rapport à ce qui reste.
        y = (imageHeight - textRect.height()) / 2;
    }
    if (position == PositionTexte::Centre && textRect.width() + 2 * padding > imageWidth) {
        x = (imageWidth - textRect.width()) / 2;
    }

    QRect drawRect(QPoint(x, y), textRect.size());

    int flags = 0;
    switch (position) {
        case PositionTexte::HautGauche:
            flags = Qt::AlignLeft | Qt::AlignTop;
            break;
        case PositionTexte::HautDroite:
            flags = Qt::AlignRight | Qt::AlignTop;
            break;
        case PositionTexte::HautMilieu:
            flags = Qt::AlignHCenter | Qt::AlignTop;
            break;
        case PositionTexte::Centre:
            flags = Qt::AlignHCenter | Qt::AlignVCenter;
            break;
        case PositionTexte::BasGauche:
            flags = Qt::AlignLeft | Qt::AlignBottom;
            break;
        case PositionTexte::BasDroite:
            flags = Qt::AlignRight | Qt::AlignBottom;
            break;
        case PositionTexte::BasMilieu:
            flags = Qt::AlignHCenter | Qt::AlignBottom;
            break;
    }

    // Ombre (optionnelle)
    if (shadow.alpha() > 0) {
        p.setPen(shadow);
        p.setBrush(Qt::NoBrush);
        QPoint shadowOffset(2, 2);
        p.drawText(drawRect.translated(shadowOffset), flags, text);
    }

    p.setPen(color);
    p.setOpacity(opacity);
    p.drawText(drawRect, flags, text);
    p.end();
    return out;
}

QImage
ImageUtils::overlayImage(const QImage& base, const QImage& watermark, const QPoint& topLeft, qreal opacity, const QSize& scaleTo)
{
    if (base.isNull() || watermark.isNull())
        return {};
    QImage out = base.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QImage wm = watermark.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    if (scaleTo.isValid()) {
        wm = wm.scaled(scaleTo, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    QPainter p(&out);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);
    p.setOpacity(opacity);
    p.drawImage(topLeft, wm);
    p.end();
    return out;
}

QImage
ImageUtils::convertFormat(const QImage& src, QImage::Format f)
{
    if (src.isNull())
        return {};
    return src.convertToFormat(f);
}

QImage
ImageUtils::convertToSRGB(const QImage& src)
{
    if (src.isNull())
        return {};
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    if (src.colorSpace().isValid() && src.colorSpace() != QColorSpace::SRgb) {
        return src.convertedToColorSpace(QColorSpace::SRgb);
    }
#endif
    return src;
}

QVector<quint32>
ImageUtils::histogramGray(const QImage& src)
{
    QVector<quint32> hist(256, 0u);
    if (src.isNull())
        return hist;

    QImage gray = src.convertToFormat(QImage::Format_Grayscale8);
    const int w = gray.width();
    const int h = gray.height();
    for (int y = 0; y < h; ++y) {
        const uchar* line = gray.constScanLine(y);
        for (int x = 0; x < w; ++x) {
            hist[line[x]]++;
        }
    }
    return hist;
}

QImage
ImageUtils::equalizeHistogramGray(const QImage& src)
{
    if (src.isNull())
        return {};
    QImage gray = src.convertToFormat(QImage::Format_Grayscale8);
    QVector<quint32> hist = histogramGray(gray);

    // CDF
    QVector<double> cdf(256, 0.0);
    quint64 total = static_cast<quint64>(gray.width()) * static_cast<quint64>(gray.height());
    quint64 cum = 0;
    for (int i = 0; i < 256; ++i) {
        cum += hist[i];
        cdf[i] = total ? static_cast<double>(cum) / static_cast<double>(total) : 0.0;
    }

    // LUT
    quint8 LUT[256];
    for (int i = 0; i < 256; ++i) {
        LUT[i] = clampByte(static_cast<int>(cdf[i] * 255.0 + 0.5));
    }

    QImage out(gray.size(), QImage::Format_Grayscale8);
    for (int y = 0; y < gray.height(); ++y) {
        const uchar* srcLine = gray.constScanLine(y);
        uchar* dstLine = out.scanLine(y);
        for (int x = 0; x < gray.width(); ++x) {
            dstLine[x] = LUT[srcLine[x]];
        }
    }
    return out;
}

bool
ImageUtils::compressToMaxBytes(const QImage& img,
                               const QString& outPath,
                               qint64 maxBytes,
                               QByteArray formatHint,
                               int minQuality,
                               int maxQuality,
                               QString* error)
{
    if (img.isNull()) {
        if (error)
            *error = "Image vide";
        return false;
    }
    if (maxBytes <= 0) {
        if (error)
            *error = "maxBytes invalide";
        return false;
    }

    minQuality = std::max(0, minQuality);
    maxQuality = std::min(100, maxQuality);
    if (minQuality > maxQuality)
        std::swap(minQuality, maxQuality);

    int bestQ = -1;
    int lo = minQuality, hi = maxQuality;
    QByteArray bestData;

    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        QImageWriter writer(&buffer, formatHint.isEmpty() ? QByteArray("JPEG") : formatHint);
        writer.setQuality(mid);
        if (!writer.write(img)) {
            if (error)
                *error = writer.errorString();
            return false;
        }
        QByteArray data = buffer.data();
        if (data.size() <= maxBytes) {
            bestQ = mid;
            bestData = std::move(data);
            lo = mid + 1; // tenter meilleure qualité
        } else {
            hi = mid - 1; // réduire la qualité
        }
    }

    if (bestQ < 0) {
        if (error)
            *error = "Impossible d'atteindre la taille cible, même à qualité minimale.";
        return false;
    }

    QFile f(outPath);
    if (!f.open(QIODevice::WriteOnly)) {
        if (error)
            *error = QString("Échec ouverture: %1").arg(outPath);
        return false;
    }
    f.write(bestData);
    f.close();
    return true;
}

QStringList
ImageUtils::batchProcess(const QStringList& inputPaths,
                         const QString& outputDir,
                         const std::function<QImage(const QImage&)>& pipeline,
                         QByteArray formatHint,
                         int quality)
{
    QStringList errors;
    QDir().mkpath(outputDir);

    for (const QString& inPath : inputPaths) {
        ImageInfo info;
        QString err;
        QImage img = load(inPath, &info, &err);
        if (img.isNull()) {
            errors << QString("Lecture '%1': %2").arg(inPath, err);
            continue;
        }

        QImage out = pipeline ? pipeline(img) : img;
        QFileInfo fi(inPath);
        QString outExt = !formatHint.isEmpty() ? QString::fromLatin1(formatHint).toLower() : fi.suffix().toLower();
        QString outPath = QDir(outputDir).filePath(fi.completeBaseName() + "." + outExt);

        if (!save(out, outPath, formatHint, quality, {}, &err)) {
            errors << QString("Écriture '%1': %2").arg(outPath, err);
        }
    }
    return errors;
}
