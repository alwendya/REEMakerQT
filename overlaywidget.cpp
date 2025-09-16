#include "OverlayWidget.h"

#include <QAbstractButton>
#include <QApplication>
#include <QCheckBox>
#include <QEvent>
#include <QEventLoop>
#include <QGraphicsBlurEffect>
#include <QGraphicsOpacityEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScreen>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>
#include <cmath> // pour std::pow
#include <cmath> // ease#include <QMouseEvent>
#include <cmath>

namespace {
static qreal
easeOutCubic(qreal t)
{
    return 1.0 - std::pow(1.0 - t, 3.0);
}
static QSize
scaledSizeKeepingAspect(const QSize& src, const QSize& bound)
{
    if (src.isEmpty() || bound.isEmpty())
        return QSize();
    QSize s = src;
    s.scale(bound, Qt::KeepAspectRatio);
    return s;
}
static QFont
titleFont(QWidget* w)
{
    QFont f = w->font();
    f.setPointSizeF(f.pointSizeF() + 2);
    f.setBold(true);
    return f;
}
static QImage
toArgb32(const QPixmap& pix)
{
    QImage img = pix.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
    img.setDevicePixelRatio(pix.devicePixelRatio());
    return img;
}
static QImage
blurWithGraphicsEffect(const QImage& src, qreal radius)
{
    if (radius <= 0.0)
        return src;
    QGraphicsScene scene;
    QGraphicsPixmapItem item(QPixmap::fromImage(src));
    QGraphicsBlurEffect blur;
    blur.setBlurRadius(radius);
    item.setGraphicsEffect(&blur);
    scene.addItem(&item);

    QImage res(src.size(), QImage::Format_ARGB32_Premultiplied);
    res.setDevicePixelRatio(src.devicePixelRatio());
    res.fill(Qt::transparent);
    QPainter p(&res);
    scene.render(&p);
    return res;
}
} // namespace

// class HoldVeil : public QWidget
// {
//   public:
//     explicit HoldVeil(QWidget* refTopLevel, int alpha)
//       : QWidget(nullptr)
//       , m_alpha(qBound(0, alpha, 255))
//     {
//         // Fenêtre top-level "tool", sans décoration, translucide, insensible aux clics
//         setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
//         setAttribute(Qt::WA_TranslucentBackground);
//         setAttribute(Qt::WA_TransparentForMouseEvents, true);
//         setAttribute(Qt::WA_NoSystemBackground);

//         // Recouvre la fenêtre de référence (en coordonnées écran)
//         QWidget* tlw = refTopLevel ? refTopLevel->window() : nullptr;
//         if (tlw) {
//             const QRect gr = tlw->frameGeometry(); // incluant la bordure fenêtrée
//             setGeometry(gr);
//         }
//         // Au cas où refTopLevel est nul, on n’affiche rien
//     }

//   protected:
//     void paintEvent(QPaintEvent*) override
//     {
//         QPainter p(this);
//         p.fillRect(rect(), QColor(0, 0, 0, m_alpha));
//     }

//   private:
//     int m_alpha = 100;
// };

// namespace {
// class PanelResizer : public QObject
// {
//   public:
//     PanelResizer(QWidget* parentWindow, OverlayBlurWidget* overlay, QWidget* panel)
//       : QObject(overlay)
//       , m_parent(parentWindow)
//       , m_overlay(overlay)
//       , m_panel(panel)
//     {
//         // Reposition rapide (throttle ~60 FPS)
//         m_reposThrottle.setSingleShot(true);
//         m_reposThrottle.setInterval(16);
//         QObject::connect(&m_reposThrottle, &QTimer::timeout, this, &PanelResizer::reposition);

//         // Recapture blur à la fin du resize (debounce)
//         m_captureDebounce.setSingleShot(true);
//         m_captureDebounce.setInterval(150); // ajuste entre 120 et 200 ms selon préférence
//         QObject::connect(&m_captureDebounce, &QTimer::timeout, this, &PanelResizer::recaptureBlurOnce);
//     }

//     bool eventFilter(QObject* watched, QEvent* e) override
//     {
//         if (watched == m_parent && (e->type() == QEvent::Resize || e->type() == QEvent::Move)) {
//             // Reposition immédiat (voile reste présent)
//             m_reposThrottle.start();
//             // Programme une recapture à la fin du drag
//             m_captureDebounce.start();
//         }
//         return QObject::eventFilter(watched, e);
//     }

//   private slots:
//     void reposition()
//     {
//         if (!m_parent || !m_overlay || !m_panel)
//             return;

//         // 1) Étendre l’overlay
//         m_overlay->setGeometry(m_parent->rect());

//         // 2) Recentrer le panel (identique à centerPanel())
//         const int margin = 24;
//         QSize pref = m_panel->sizeHint();
//         pref.setWidth(qBound(420, pref.width(), 720));
//         QRect r = m_overlay->rect().adjusted(margin, margin, -margin, -margin);
//         QSize sz(qMin(pref.width(), r.width()), qMin(qMax(pref.height(), 180), r.height()));
//         QPoint topLeft = r.center() - QPoint(sz.width() / 2, sz.height() / 2);
//         m_panel->setGeometry(QRect(topLeft, sz));

//         // Force un repaint pour garder le voile
//         m_overlay->update();
//     }

//     void recaptureBlurOnce()
//     {
//         if (!m_parent || !m_overlay)
//             return;

//         const bool wasVisible = m_overlay->isVisible();

//         // 1) Crée un voile top-level temporaire au-dessus de la fenêtre
//         //    en utilisant la même opacité de voile que l’overlay
//         int alpha = 100;
//         // si tu as exposé OverlayBlurWidget::baseVeilOpacity(), récupère-le :
//         // alpha = m_overlay->baseVeilOpacity();

//         HoldVeil* veil = new HoldVeil(m_parent, alpha);
//         veil->show();
//         veil->raise();
//         qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

//         // 2) Masquer l’overlay pour ne pas l’auto-capturer
//         if (wasVisible) {
//             m_overlay->hide();
//             qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
//         }

//         // 3) Rafraîchir le snapshot flouté
//         m_overlay->refreshSnapshot();

//         // 4) Réafficher overlay et retirer le voile temporaire
//         if (wasVisible) {
//             m_overlay->show();
//             m_overlay->raise();
//         }
//         veil->deleteLater();
//     }

//   private:
//     QPointer<QWidget> m_parent;
//     QPointer<OverlayBlurWidget> m_overlay;
//     QPointer<QWidget> m_panel;

//     QTimer m_reposThrottle;
//     QTimer m_captureDebounce;
// };
// } // namespace

namespace {
class PanelResizer : public QObject
{
  public:
    PanelResizer(QWidget* parentWindow, OverlayBlurWidget* overlay, QWidget* panel)
      : QObject(overlay)
      , m_parent(parentWindow)
      , m_overlay(overlay)
      , m_panel(panel)
    {
        m_throttle.setSingleShot(true);
        m_throttle.setInterval(16); // ~60 FPS
        QObject::connect(&m_throttle, &QTimer::timeout, this, [this] { recenter(); });
    }

    bool eventFilter(QObject* watched, QEvent* e) override
    {
        if (watched == m_parent && (e->type() == QEvent::Resize || e->type() == QEvent::Move)) {
            m_throttle.start();
        }
        return QObject::eventFilter(watched, e);
    }

  private:
    void recenter()
    {
        if (!m_parent || !m_overlay || !m_panel)
            return;

        // 1) Étendre l’overlay sur la fenêtre
        m_overlay->setGeometry(m_parent->rect());

        // 2) Recentrer le panel (logique identique à centerPanel())
        const int margin = 24;
        QSize pref = m_panel->sizeHint();
        if (pref.width() < 420)
            pref.setWidth(420);
        if (pref.width() > 720)
            pref.setWidth(720);
        QRect r = m_overlay->rect().adjusted(margin, margin, -margin, -margin);
        QSize sz(qMin(pref.width(), r.width()), qMin(qMax(pref.height(), 180), r.height()));
        QPoint topLeft = r.center() - QPoint(sz.width() / 2, sz.height() / 2);
        m_panel->setGeometry(QRect(topLeft, sz));

        // 3) Rafraîchir le flou (évite de capturer l’overlay lui-même)
        const bool wasVisible = m_overlay->isVisible();
        if (wasVisible) {
            m_overlay->hide();
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        }
        m_overlay->refreshSnapshot();
        if (wasVisible) {
            m_overlay->show();
            m_overlay->raise();
        }
    }

    QPointer<QWidget> m_parent;
    QPointer<OverlayBlurWidget> m_overlay;
    QPointer<QWidget> m_panel;
    QTimer m_throttle;
};
} // namespace

/* =========================
 *   OverlayBlurWidget
 * ========================= */

OverlayBlurWidget::OverlayBlurWidget(QWidget* parent, qreal blurRadius, qreal downscale, bool InstallEvent)
  : QWidget(parent)
  , m_radius(blurRadius)
  , m_downscale(downscale)
  , m_InstallEventFilter(InstallEvent)
{
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setAutoFillBackground(false);
    setFocusPolicy(Qt::NoFocus); // laisser le panel capter Enter/Esc
    setVisible(false);
    ensureFullSize();

    // Timer d’animation
    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(16); // ~60 FPS
    connect(m_animTimer, &QTimer::timeout, this, &OverlayBlurWidget::onAnimTick);
    // suivre le parent
    if (parent && InstallEvent)
        parent->installEventFilter(this);
}

void
OverlayBlurWidget::ensureFullSize()
{
    if (parentWidget()) {
        setGeometry(parentWidget()->rect());
    }
    raise();
}

void
OverlayBlurWidget::resizeEvent(QResizeEvent*)
{
    ensureFullSize();
    centerPanel();
}

void
OverlayBlurWidget::paintEvent(QPaintEvent*)
{
    if (m_blurred.isNull())
        return;
    QPainter p(this);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);
    p.drawImage(rect(), m_blurred);

    // voile sombre avec alpha animé
    // const int baseVeil = 100; // opacité max du voile
    // const int a = int(baseVeil * qBound<qreal>(0.0, m_overlayAlpha, 1.0));
    // p.fillRect(rect(), QColor(0, 0, 0, a));
    const int a = int(m_baseVeilOpacity * qBound<qreal>(0.0, m_overlayAlpha, 1.0));
    p.fillRect(rect(), QColor(0, 0, 0, a));
}

void
OverlayBlurWidget::mousePressEvent(QMouseEvent* e)
{
    // Clic à l'intérieur du panel -> laisser passer au panel
    if (m_panel && m_panel->geometry().contains(e->pos())) {
        e->ignore();
        return;
    }
    // Clic en dehors -> fermer uniquement si configuré
    if (m_cfg.clickOutsideToClose && m_cfg.escapeButtonId != -1) {
        startFadeOut(m_cfg.escapeButtonId, !m_inExec);
        e->accept();
        return;
    }
    e->accept(); // bloquer l'UI derrière
}

void
OverlayBlurWidget::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape && m_cfg.escapeButtonId != -1) {
        startFadeOut(m_cfg.escapeButtonId, !m_inExec);
        return;
    }
    QWidget::keyPressEvent(e);
}

QIcon
OverlayBlurWidget::iconFromStandard(QStyle::StandardPixmap sp, QWidget* ref)
{
    QStyle* st = ref ? ref->style() : QApplication::style();
    return st->standardIcon(sp, nullptr, ref);
}

ModalButton
OverlayBlurWidget::makeButton(int id, const QString& text, QDialogButtonBox::ButtonRole role, bool isDefault, bool isEscape)
{
    ModalButton b;
    b.id = id;
    b.text = text;
    b.role = role;
    b.isDefault = isDefault;
    b.isEscape = isEscape;
    return b;
}

QImage
OverlayBlurWidget::toArgb32(const QPixmap& pix)
{
    return ::toArgb32(pix);
}

QImage
OverlayBlurWidget::blurWithGraphicsEffect(const QImage& src, qreal radius)
{
    return ::blurWithGraphicsEffect(src, radius);
}

void
OverlayBlurWidget::refreshSnapshot()
{
    if (!parentWidget())
        return;
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents); // finaliser l’UI

    QPixmap pix = parentWidget()->grab();
    if (pix.isNull() || pix.width() == 0 || pix.height() == 0) {
        m_blurred = QImage();
        setVisible(false);
        return;
    }

    QImage src = toArgb32(pix);
    QImage scaled = src;

    if (m_downscale > 0.0 && m_downscale < 1.0) {
        const QSizeF s = src.size() / src.devicePixelRatio();
        QSize target = (s * m_downscale).toSize();
        if (target.width() > 0 && target.height() > 0) {
            scaled = src.scaled(target, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            scaled.setDevicePixelRatio(src.devicePixelRatio());
        }
    }

    m_blurred = blurWithGraphicsEffect(scaled, m_radius);
    setVisible(true);
    update();
}

void
OverlayBlurWidget::centerPanel()
{
    if (!m_panel)
        return;
    const int margin = 24;
    QSize pref = m_panel->sizeHint();
    if (pref.width() < m_cfg.panelMinWidth)
        pref.setWidth(m_cfg.panelMinWidth);
    if (pref.width() > m_cfg.panelMaxWidth)
        pref.setWidth(m_cfg.panelMaxWidth);
    QRect r = rect().adjusted(margin, margin, -margin, -margin);
    QSize sz(qMin(pref.width(), r.width()), qMin(qMax(pref.height(), 160), r.height()));
    QPoint topLeft = r.center() - QPoint(sz.width() / 2, sz.height() / 2);
    m_panel->setGeometry(QRect(topLeft, sz));
}

void
OverlayBlurWidget::startFadeIn()
{
    if (!m_cfg.enableFade) {
        m_overlayAlpha = 1.0;
        if (m_panel)
            centerPanel();
        update();
        return;
    }

    // Prépare slide léger depuis le haut
    if (m_panel) {
        centerPanel();
        m_panelTargetGeom = m_panel->geometry();
        m_panelStartGeom = m_panelTargetGeom.translated(0, -20);
        m_panel->setGeometry(m_panelStartGeom);
    }

    m_animKind = AnimFadeIn;
    m_overlayAlpha = 0.0;
    m_animDurationMs = (m_cfg.fadeDurationMs > 0 ? m_cfg.fadeDurationMs : 160);
    m_animClock.restart();
    m_animTimer->start();
}

void
OverlayBlurWidget::startFadeOut(int resultId, bool emitSignal)
{
    m_resultOnFadeOut = resultId;
    m_emitSignalOnFadeOut = emitSignal;

    if (!m_cfg.enableFade) {
        m_execResult = resultId;
        if (m_panel) {
            m_panel->hide();
            m_panel->deleteLater();
            m_panel = nullptr;
        }
        hide();
        if (m_inExec)
            emit fadeOutDone();
        else if (emitSignal) {
            emit finished(m_execResult);
        }
        deleteLater();
        return;
    }

    // slide inverse
    if (m_panel) {
        m_panelTargetGeom = m_panel->geometry();
        m_panelStartGeom = m_panelTargetGeom.translated(0, -20);
    }

    m_animKind = AnimFadeOut;
    m_animDurationMs = (m_cfg.fadeDurationMs > 0 ? m_cfg.fadeDurationMs : 160);
    m_animClock.restart();
    m_animTimer->start();
}

void
OverlayBlurWidget::onAnimTick()
{
    const qint64 elapsed = m_animClock.elapsed();
    qreal t = qBound<qreal>(0.0, elapsed / qreal(m_animDurationMs), 1.0);
    const qreal e = easeOutCubic(t);

    if (m_animKind == AnimFadeIn) {
        m_overlayAlpha = e;
        if (m_panel) {
            int y = m_panelStartGeom.y() + int((m_panelTargetGeom.y() - m_panelStartGeom.y()) * e);
            QRect r = m_panelTargetGeom;
            r.moveTop(y);
            m_panel->setGeometry(r);
        }
        update();
        if (t >= 1.0) {
            m_animTimer->stop();
            m_animKind = AnimNone;
            m_overlayAlpha = 1.0;
            if (m_panel)
                m_panel->setGeometry(m_panelTargetGeom);
            update();
        }
        return;
    }

    if (m_animKind == AnimFadeOut) {
        m_overlayAlpha = 1.0 - e;
        if (m_panel) {
            int y = m_panelTargetGeom.y() + int((m_panelStartGeom.y() - m_panelTargetGeom.y()) * e);
            QRect r = m_panelTargetGeom;
            r.moveTop(y);
            m_panel->setGeometry(r);
        }
        update();
        if (t >= 1.0) {
            m_animTimer->stop();
            m_animKind = AnimNone;

            m_execResult = m_resultOnFadeOut;
            if (m_panel) {
                m_panel->hide();
                m_panel->deleteLater();
                m_panel = nullptr;
            }
            hide();
            if (m_inExec)
                emit fadeOutDone();
            else if (m_emitSignalOnFadeOut)
                emit finished(m_execResult);
            deleteLater();
        }
        return;
    }
}

int
OverlayBlurWidget::execModal(const ModalConfig& cfg)
{
    m_inExec = true;
    m_execResult = -1;

    m_cfg = cfg;
    refreshSnapshot();

    m_panel = new BlurModalPanel(this, cfg);
    m_panel->setParent(this);
    m_panel->raise();
    m_panel->show();
    m_panel->setFocus();

    connect(m_panel, &BlurModalPanel::requestClose, this, &OverlayBlurWidget::onPanelRequestClose);

    startFadeIn();

    QEventLoop loop;
    connect(this, &OverlayBlurWidget::fadeOutDone, &loop, &QEventLoop::quit);
    loop.exec();

    m_inExec = false;
    // auto-delete dans fadeOut
    return m_execResult;
}

void
OverlayBlurWidget::showModal(const ModalConfig& cfg)
{
    m_inExec = false;
    m_execResult = -1;
    m_cfg = cfg;

    refreshSnapshot();

    m_panel = new BlurModalPanel(this, cfg);
    m_panel->setParent(this);
    m_panel->raise();
    m_panel->show();
    m_panel->setFocus();

    connect(m_panel, &BlurModalPanel::requestClose, this, &OverlayBlurWidget::onPanelRequestClose);

    startFadeIn();
}

void
OverlayBlurWidget::onPanelRequestClose(int buttonId)
{
    startFadeOut(buttonId, !m_inExec);
}

void
OverlayBlurWidget::beginOverlayVeilFadeIn(int fadeDurationMs)
{
    // Utilise exactement la même animation que execModal() (m_overlayAlpha + timer)
    m_cfg.enableFade = true;
    m_cfg.fadeDurationMs = (fadeDurationMs > 0 ? fadeDurationMs : 160);
    // Pas de panel interne -> startFadeIn() animera juste le voile
    startFadeIn();
}

void
OverlayBlurWidget::beginOverlayVeilFadeOutAndDelete(int fadeDurationMs)
{
    // Fait un fade-out du voile puis se supprime
    m_cfg.enableFade = true;
    m_cfg.fadeDurationMs = (fadeDurationMs > 0 ? fadeDurationMs : 160);
    // Pas de résultat/emit de signal nécessaire
    startFadeOut(/*resultId*/ 0, /*emitSignal*/ false);
}

void
OverlayBlurWidget::setBaseVeilOpacity(int a)
{
    m_baseVeilOpacity = qBound(0, a, 255);
    update();
}

int
OverlayBlurWidget::baseVeilOpacity() const
{
    return m_baseVeilOpacity;
}

bool
OverlayBlurWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == parentWidget()) {
        if (event->type() == QEvent::Resize || event->type() == QEvent::Move) {
            ensureFullSize();
            centerPanel();
        }
    }
    return QWidget::eventFilter(watched, event);
}

/* =========================
 *   BlurModalPanel
 * ========================= */

#include <QTextOption>

class QLabel;
class QPushButton;
class QDialogButtonBox;

#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>

bool
BlurModalPanel::isRich(const QString& s)
{
    return Qt::mightBeRichText(s);
}

BlurModalPanel::BlurModalPanel(OverlayBlurWidget* overlay, const ModalConfig& cfg)
  : QFrame(overlay)
{
    setObjectName("BlurModalPanel");
    setStyleSheet("#BlurModalPanel { background: rgba(30,30,30,220); border-radius: 12px; }"
                  "QLabel { color: white; }"
                  "QPushButton { min-height: 28px; padding: 6px 12px; }");
    setFrameShape(QFrame::NoFrame);
    setFocusPolicy(Qt::StrongFocus);

    // Build UI
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(20, 20, 20, 16);
    root->setSpacing(14);

    // Titre + icône
    QHBoxLayout* topRow = new QHBoxLayout;
    topRow->setSpacing(12);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setVisible(!cfg.icon.isNull());
    if (!cfg.icon.isNull()) {
        QPixmap px = cfg.icon.pixmap(cfg.iconSize);
        m_iconLabel->setPixmap(px);
        m_iconLabel->setFixedSize(cfg.iconSize);
    }

    m_titleLabel = new QLabel(cfg.title, this);
    m_titleLabel->setFont(titleFont(this));
    m_titleLabel->setWordWrap(true);

    QVBoxLayout* titleCol = new QVBoxLayout;
    titleCol->addWidget(m_titleLabel);
    titleCol->setSpacing(6);

    // Message
    m_messageLabel = new QLabel(this);
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setTextFormat(isRich(cfg.message) ? Qt::RichText : Qt::PlainText);
    m_messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_messageLabel->setText(cfg.message);
    titleCol->addWidget(m_messageLabel);

    topRow->addWidget(m_iconLabel, 0, Qt::AlignTop);
    topRow->addLayout(titleCol, 1);
    root->addLayout(topRow);

    // Preview optionnelle
    m_previewLabel = new QLabel(this);
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setVisible(!cfg.preview.isNull());
    m_previewLabel->setStyleSheet("QLabel { background: rgba(255,255,255,18); border-radius: 8px; }");

    if (!cfg.preview.isNull()) {
        QPixmap px = cfg.preview;
        QSize pr = px.size() / px.devicePixelRatio();
        QSize tgt = scaledSizeKeepingAspect(pr, cfg.previewMaxSize);
        if (!tgt.isEmpty()) {
            QPixmap scaled = px.scaled(tgt * px.devicePixelRatio(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            scaled.setDevicePixelRatio(px.devicePixelRatio());
            m_previewLabel->setPixmap(scaled);
            m_previewLabel->setMinimumSize(tgt);
            m_previewLabel->setMaximumSize(cfg.previewMaxSize);
        } else {
            m_previewLabel->setPixmap(px);
        }
        root->addWidget(m_previewLabel);
    }

    // Boutons
    m_buttonBox = new QDialogButtonBox(this);
    m_buttonBox->setCenterButtons(true);
    root->addWidget(m_buttonBox);

    // Wire + defaults
    wireButtons(cfg);
    applyDefaultAndEscape(cfg);

    connect(m_buttonBox, &QDialogButtonBox::clicked, this, &BlurModalPanel::onButtonClicked);
}

void
BlurModalPanel::wireButtons(const ModalConfig& cfg)
{
    for (const ModalButton& mb : cfg.buttons) {
        QPushButton* pb = new QPushButton(mb.text, this);
        pb->setAutoDefault(true);
        pb->setDefault(false);
        pb->setProperty("btn_id", mb.id);
        m_buttonBox->addButton(pb, mb.role);
        if (mb.isDefault)
            m_defaultId = mb.id;
        if (mb.isEscape)
            m_escapeId = mb.id;
    }
}

void
BlurModalPanel::applyDefaultAndEscape(const ModalConfig& cfg)
{
    if (m_defaultId == -1)
        m_defaultId = cfg.defaultButtonId;
    if (m_escapeId == -1)
        m_escapeId = cfg.escapeButtonId;

    if (m_defaultId != -1) {
        const QList<QAbstractButton*> buttons = m_buttonBox->buttons();
        for (QAbstractButton* b : buttons) {
            QPushButton* pb = qobject_cast<QPushButton*>(b);
            if (pb && pb->property("btn_id").toInt() == m_defaultId) {
                pb->setDefault(true);
                pb->setFocus();
                break;
            }
        }
    }
}

void
BlurModalPanel::onButtonClicked(QAbstractButton* button)
{
    if (!button)
        return;
    int buttonId = button->property("btn_id").toInt();
    m_resultId = buttonId;
    emit requestClose(buttonId);
}

void
BlurModalPanel::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape && m_escapeId != -1) {
        m_resultId = m_escapeId;
        emit requestClose(m_escapeId);
        return;
    }
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        if (m_defaultId != -1) {
            const QList<QAbstractButton*> buttons = m_buttonBox->buttons();
            for (QAbstractButton* b : buttons) {
                QPushButton* pb = qobject_cast<QPushButton*>(b);
                if (pb && pb->property("btn_id").toInt() == m_defaultId) {
                    pb->click();
                    return;
                }
            }
        } else {
            const QList<QAbstractButton*> buttons = m_buttonBox->buttons();
            for (QAbstractButton* b : buttons) {
                if (m_buttonBox->buttonRole(b) == QDialogButtonBox::AcceptRole) {
                    QPushButton* pb = qobject_cast<QPushButton*>(b);
                    if (pb) {
                        pb->click();
                        return;
                    }
                }
            }
        }
    }
    QFrame::keyPressEvent(e);
}

/* =========================
 *   ProgressOverlay (avec flou optionnel)
 * ========================= */

ProgressOverlay::ProgressOverlay(QWidget* parent, bool center, bool blockInput, int fadeDurationMs)
  : QWidget(parent)
  , m_centered(center)
  , m_blockInput(blockInput)
  , m_animDurationMs(fadeDurationMs > 0 ? fadeDurationMs : 140)
{
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setAutoFillBackground(false);

    if (!m_blockInput)
        setAttribute(Qt::WA_TransparentForMouseEvents, true);

    setVisible(true);
    if (parent)
        setGeometry(parent->rect());
    raise();

    // UI
    buildUi();
    reposition();

    // suivre le parent
    if (parent)
        parent->installEventFilter(this);

    // Timer anim
    m_animTimer.setInterval(16);
    connect(&m_animTimer, &QTimer::timeout, this, &ProgressOverlay::onAnimTick);

    // Timer finish différé
    m_finishTimer.setSingleShot(true);
    connect(&m_finishTimer, &QTimer::timeout, this, &ProgressOverlay::onFinishTimeout);

    // Fade in
    startFadeIn();
}

void
ProgressOverlay::buildUi()
{
    m_panel = new QWidget(this);
    m_panel->setObjectName("ProgressOverlayPanel");
    m_panel->setStyleSheet("#ProgressOverlayPanel { background: rgba(30,30,30,220); border-radius: 10px; }"
                           "QLabel { color: white; }");

    QVBoxLayout* lay = new QVBoxLayout(m_panel);
    lay->setContentsMargins(16, 16, 16, 16);
    lay->setSpacing(10);

    m_label = new QLabel(tr("Initialisation..."), m_panel);
    m_label->setWordWrap(true);

    m_bar = new QProgressBar(m_panel);
    m_bar->setMinimum(0);
    m_bar->setMaximum(0); // indéterminé par défaut
    m_bar->setTextVisible(true);
    m_bar->setFormat(QStringLiteral("%p%")); // en mode déterminé

    lay->addWidget(m_label);
    lay->addWidget(m_bar);

    m_panel->setMinimumWidth(360);
    m_panel->adjustSize();
    m_panel->show();
}

void
ProgressOverlay::reposition()
{
    if (!parentWidget())
        return;

    setGeometry(parentWidget()->rect());
    if (m_centered) {
        QSize sz = m_panel->sizeHint();
        if (sz.width() < 360)
            sz.setWidth(360);
        QRect r = rect().adjusted(m_margin, m_margin, -m_margin, -m_margin);
        QSize s(qMin(sz.width(), r.width()), qMin(sz.height(), r.height()));
        QPoint topLeft = r.center() - QPoint(s.width() / 2, s.height() / 2);
        m_panel->setGeometry(QRect(topLeft, s));
    } else {
        QSize s = m_panel->sizeHint();
        int x = width() - s.width() - m_margin;
        int y = height() - s.height() - m_margin;
        m_panel->setGeometry(QRect(QPoint(x, y), s));
    }
}

void
ProgressOverlay::centerInParent()
{
    m_centered = true;
    reposition();
}

void
ProgressOverlay::anchorBottomRight(int margin)
{
    m_centered = false;
    m_margin = qMax(0, margin);
    reposition();
}

void
ProgressOverlay::setMaximum(int maximum)
{
    if (maximum <= 0) {
        m_bar->setRange(0, 0);                // indéterminé
        m_bar->setFormat(QStringLiteral("")); // pas de % (le style gère l’animation)
    } else {
        m_bar->setRange(0, maximum);
        m_bar->setFormat(QStringLiteral("%p%"));
    }
}

void
ProgressOverlay::setValue(int value)
{
    if (m_bar->maximum() > 0)
        m_bar->setValue(value);
}

void
ProgressOverlay::setText(const QString& text)
{
    m_label->setText(text);
}

int
ProgressOverlay::maximum() const
{
    return m_bar->maximum();
}
int
ProgressOverlay::value() const
{
    return m_bar->value();
}

void
ProgressOverlay::enableBackdropBlur(bool enable, qreal radius, qreal downscale)
{
    m_useBlur = enable;
    m_blurRadius = radius;
    m_blurDownscale = downscale;

    if (m_useBlur) {
        makeBlurSnapshot();
    } else {
        m_blurred = QImage();
    }
    update();
}

void
ProgressOverlay::refreshBackdropSnapshot()
{
    if (!m_useBlur)
        return;
    makeBlurSnapshot();
    update();
}

void
ProgressOverlay::makeBlurSnapshot()
{
    if (!parentWidget())
        return;

    // Éviter de capturer l’overlay lui-même : masquer brièvement
    const bool wasVisible = isVisible();
    if (wasVisible) {
        hide();
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }

    QPixmap pix = parentWidget()->grab();
    if (!pix.isNull() && pix.width() > 0 && pix.height() > 0) {
        QImage src = toArgb32(pix);
        QImage scaled = src;
        if (m_blurDownscale > 0.0 && m_blurDownscale < 1.0) {
            const QSizeF s = src.size() / src.devicePixelRatio();
            QSize target = (s * m_blurDownscale).toSize();
            if (target.width() > 0 && target.height() > 0) {
                scaled = src.scaled(target, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                scaled.setDevicePixelRatio(src.devicePixelRatio());
            }
        }
        m_blurred = blurWithGraphicsEffect(scaled, m_blurRadius);
    } else {
        m_blurred = QImage();
    }

    if (wasVisible) {
        show();
        raise();
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }
}

void
ProgressOverlay::finish(const QString& finalText, int msBeforeFadeOut)
{
    if (!finalText.isEmpty())
        m_label->setText(finalText);
    if (msBeforeFadeOut <= 0)
        startFadeOut();
    else
        m_finishTimer.start(msBeforeFadeOut);
}

void
ProgressOverlay::cancelAndClose()
{
    deleteLater();
}

void
ProgressOverlay::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    if (m_useBlur && !m_blurred.isNull()) {
        // Fond flouté (modulé par alpha pour un fondu doux)
        p.setOpacity(m_alpha);
        p.drawImage(rect(), m_blurred);
        p.setOpacity(1.0);
        // Option : voile complémentaire très léger si besoin
        // int add = int(20 * m_alpha); p.fillRect(rect(), QColor(0,0,0,add));
    } else {
        // Voile léger animé
        const int maxA = 60;
        int a = int(maxA * qBound<qreal>(0.0, m_alpha, 1.0));
        p.fillRect(rect(), QColor(0, 0, 0, a));
    }
}

bool
ProgressOverlay::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == parentWidget()) {
        if (event->type() == QEvent::Resize || event->type() == QEvent::Move) {
            reposition();
            if (m_useBlur) {
                // Reprendre un snapshot pour suivre la fenêtre
                makeBlurSnapshot();
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void
ProgressOverlay::mousePressEvent(QMouseEvent* e)
{
    if (m_blockInput)
        e->accept();
    else
        e->ignore();
}

void
ProgressOverlay::startFadeIn()
{
    m_animKind = AnimFadeIn;
    m_alpha = 0.0;

    // slide doux depuis +12px (bas -> haut)
    if (m_panel) {
        reposition();
        m_targetGeom = m_panel->geometry();
        m_startGeom = m_targetGeom.translated(0, 12);
        m_panel->setGeometry(m_startGeom);
    }

    m_animClock.restart();
    m_animTimer.start();
}

void
ProgressOverlay::startFadeOut()
{
    m_animKind = AnimFadeOut;
    if (m_panel) {
        m_targetGeom = m_panel->geometry();
        m_startGeom = m_targetGeom.translated(0, 12);
    }
    m_animClock.restart();
    m_animTimer.start();
}

void
ProgressOverlay::onAnimTick()
{
    const qint64 elapsed = m_animClock.elapsed();
    qreal t = qBound<qreal>(0.0, elapsed / qreal(m_animDurationMs), 1.0);
    const qreal e = easeOutCubic(t);

    if (m_animKind == AnimFadeIn) {
        m_alpha = e;
        if (m_panel) {
            int y = m_startGeom.y() + int((m_targetGeom.y() - m_startGeom.y()) * e);
            QRect r = m_targetGeom;
            r.moveTop(y);
            m_panel->setGeometry(r);
        }
        update();
        if (t >= 1.0) {
            m_animTimer.stop();
            m_animKind = AnimNone;
            m_alpha = 1.0;
            if (m_panel)
                m_panel->setGeometry(m_targetGeom);
            update();
        }
        return;
    }

    if (m_animKind == AnimFadeOut) {
        m_alpha = 1.0 - e;
        if (m_panel) {
            int y = m_targetGeom.y() + int((m_startGeom.y() - m_targetGeom.y()) * e);
            QRect r = m_targetGeom;
            r.moveTop(y);
            m_panel->setGeometry(r);
        }
        update();
        if (t >= 1.0) {
            m_animTimer.stop();
            m_animKind = AnimNone;
            deleteLater();
        }
        return;
    }
}

void
ProgressOverlay::onFinishTimeout()
{
    startFadeOut();
}

/* ==== Helpers statiques ==== */

ProgressOverlay*
ProgressOverlay::showDeterminate(QWidget* parent, const QString& text, int maximum, bool center, bool blockInput, int fadeDurationMs)
{
    auto* po = new ProgressOverlay(parent, center, blockInput, fadeDurationMs);
    po->setText(text);
    po->setMaximum(qMax(1, maximum));
    po->setValue(0);
    po->show();
    po->raise();
    return po;
}

ProgressOverlay*
ProgressOverlay::showIndeterminate(QWidget* parent, const QString& text, bool center, bool blockInput, int fadeDurationMs)
{
    auto* po = new ProgressOverlay(parent, center, blockInput, fadeDurationMs);
    po->setText(text);
    po->setMaximum(0); // indéterminé
    po->show();
    po->raise();
    return po;
}

/* =========================
 *   ToastOverlay
 * ========================= */

ToastOverlay::ToastOverlay(QWidget* parent)
  : QWidget(parent)
{
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setAutoFillBackground(false);
    setAttribute(Qt::WA_TransparentForMouseEvents, true); // ne bloque pas l'UI

    if (parent)
        setGeometry(parent->rect());
    setVisible(true);
    raise();

    // Anim
    m_animTimer.setInterval(16);
    connect(&m_animTimer, &QTimer::timeout, this, &ToastOverlay::onAnimTick);

    m_holdTimer.setSingleShot(true);
    connect(&m_holdTimer, &QTimer::timeout, this, &ToastOverlay::onAutoClose);

    if (parent)
        parent->installEventFilter(this);
}

ToastOverlay*
ToastOverlay::showToast(QWidget* parent, const QString& text, int durationMs, Corner corner, int margin, int maxTextWidth, int fadeDurationMs)
{
    ToastOverlay* t = new ToastOverlay(parent);
    t->m_text = text;
    t->m_durationMs = qMax(0, durationMs);
    t->m_corner = corner;
    t->m_margin = qMax(0, margin);
    t->m_maxTextWidth = qMax(200, maxTextWidth);
    t->m_fadeDurationMs = qMax(60, fadeDurationMs);

    t->recomputePanelRect();
    t->positionPanel();
    t->startFadeIn();
    return t;
}

void
ToastOverlay::recomputePanelRect()
{
    if (!parentWidget())
        return;
    const QFont f = font();
    QFontMetrics fm(f);

    // calcul de bounding rect avec wrap
    const int padX = 14, padY = 10;
    QRect textRect(0, 0, m_maxTextWidth, 10000);
    textRect = fm.boundingRect(textRect, Qt::TextWordWrap, m_text);
    QSize panelSize(textRect.width() + 2 * padX, textRect.height() + 2 * padY);

    QRect r(0, 0, panelSize.width(), panelSize.height());
    m_panelRect = r;
}

void
ToastOverlay::positionPanel()
{
    if (!parentWidget())
        return;
    setGeometry(parentWidget()->rect());

    QRect area = rect().adjusted(m_margin, m_margin, -m_margin, -m_margin);
    QRect r = m_panelRect;

    switch (m_corner) {
        case TopLeft:
            r.moveTopLeft(area.topLeft());
            break;
        case TopRight:
            r.moveTopRight(area.topRight());
            break;
        case BottomLeft:
            r.moveBottomLeft(area.bottomLeft());
            break;
        case BottomRight:
            r.moveBottomRight(area.bottomRight());
            break;
        case BottomCenter:
            r.moveBottom(area.bottom());
            r.moveLeft(area.center().x() - r.width() / 2);
            break;
    }
    m_panelRect = r;
}

bool
ToastOverlay::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == parentWidget() && (event->type() == QEvent::Resize || event->type() == QEvent::Move)) {
        positionPanel();
    }
    return QWidget::eventFilter(watched, event);
}

void
ToastOverlay::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const qreal a = qBound<qreal>(0.0, m_alpha, 1.0);
    QColor base(30, 30, 30);
    QColor fill = QColor(base.red(), base.green(), base.blue(), int(220 * a));

    // ombre (2 passes simples)
    QColor shadow = QColor(0, 0, 0, int(90 * a));
    QRect sr = m_panelRect.adjusted(-3, -3, 3, 3);
    QPainterPath sp;
    sp.addRoundedRect(sr, 10, 10);
    p.fillPath(sp, shadow);

    // panneau
    QPainterPath path;
    path.addRoundedRect(m_panelRect, 10, 10);
    p.fillPath(path, fill);

    // texte
    p.setPen(QColor(255, 255, 255, int(255 * a)));
    const int padX = 14, padY = 10;
    QRect tr = m_panelRect.adjusted(padX, padY, -padX, -padY);
    p.drawText(tr, Qt::TextWordWrap, m_text);
}

void
ToastOverlay::startFadeIn()
{
    m_animKind = AnimFadeIn;
    m_alpha = 0.0;
    m_animClock.restart();
    m_animTimer.start();
}

void
ToastOverlay::startFadeOut()
{
    m_animKind = AnimFadeOut;
    m_animClock.restart();
    m_animTimer.start();
}

void
ToastOverlay::onAnimTick()
{
    const qint64 elapsed = m_animClock.elapsed();
    qreal t = qBound<qreal>(0.0, elapsed / qreal(m_fadeDurationMs), 1.0);
    const qreal e = easeOutCubic(t);

    if (m_animKind == AnimFadeIn) {
        m_alpha = e;
        update();
        if (t >= 1.0) {
            m_animTimer.stop();
            m_animKind = AnimHold;
            m_alpha = 1.0;
            update();
            if (m_durationMs > 0)
                m_holdTimer.start(m_durationMs);
        }
        return;
    }

    if (m_animKind == AnimFadeOut) {
        m_alpha = 1.0 - e;
        update();
        if (t >= 1.0) {
            m_animTimer.stop();
            m_animKind = AnimNone;
            deleteLater();
        }
        return;
    }
}

void
ToastOverlay::onAutoClose()
{
    startFadeOut();
}

/* =========================
 *   Formulaire Site/REE/Tranches
 * ========================= */

FormSiteInfoPanel::FormSiteInfoPanel(QWidget* parent)
  : QFrame(parent)
{
    buildUi();
}

void
FormSiteInfoPanel::buildUi()
{
    setObjectName("FormSiteInfoPanel");
    setStyleSheet(
      "#FormSiteInfoPanel { background: rgba(30,30,30,220); border-radius: 12px; }"
      "QLabel { color: white; }"
      "QLineEdit { color: white; background: rgba(255,255,255,20); border: 1px solid rgba(255,255,255,40); border-radius: 4px; padding: 4px; }"
      "QCheckBox { color: white; background: #000000; spacing: 8px; border: 1px solid rgba(255,255,255,40); border-radius: 2px; }");

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(20, 20, 20, 16);
    root->setSpacing(12);

    // Ligne 1
    m_leadLabel = new QLabel(tr("Merci de renseigner les différentes informations :"), this);
    m_leadLabel->setWordWrap(true);
    root->addWidget(m_leadLabel);

    // Ligne 2 : grille horizontale
    QWidget* row2 = new QWidget(this);
    QGridLayout* g = new QGridLayout(row2);
    g->setContentsMargins(0, 0, 0, 0);
    g->setHorizontalSpacing(10);
    g->setVerticalSpacing(6);

    QLabel* lSite = new QLabel(tr("Nom du site :"), row2);
    m_siteEdit = new QLineEdit(row2);

    QLabel* lRee = new QLabel(tr("Référence REE :"), row2);
    m_reeEdit = new QLineEdit(row2);

    QLabel* lIdx = new QLabel(tr("Indice REE :"), row2);
    m_idxEdit = new QLineEdit(row2);

    int r = 0;
    int c = 0;
    g->addWidget(lSite, r, c++);
    g->addWidget(m_siteEdit, r, c++);
    g->addWidget(lRee, r, c++);
    g->addWidget(m_reeEdit, r, c++);
    g->addWidget(lIdx, r, c++);
    g->addWidget(m_idxEdit, r, c++);

    g->setColumnStretch(1, 1);
    g->setColumnStretch(3, 1);
    g->setColumnStretch(5, 1);

    root->addWidget(row2);

    // Ligne 3 : checkboxes Tr. 0..9
    QWidget* row3 = new QWidget(this);
    QHBoxLayout* hb = new QHBoxLayout(row3);
    hb->setContentsMargins(0, 0, 0, 0);
    hb->setSpacing(8);

    QLabel* lTr = new QLabel(tr("Choix de la tranche :"), row3);
    hb->addWidget(lTr);

    m_checks.reserve(10);
    for (int i = 0; i < 10; ++i) {
        QCheckBox* cb = new QCheckBox(QStringLiteral("Tr. %1").arg(i), row3);
        m_checks.push_back(cb);
        hb->addWidget(cb);
    }
    hb->addStretch(1);
    root->addWidget(row3);

    // Boutons
    QWidget* rowB = new QWidget(this);
    QHBoxLayout* hbb = new QHBoxLayout(rowB);
    hbb->setContentsMargins(0, 0, 0, 0);
    hbb->setSpacing(10);

    hbb->addStretch(1);
    m_btnOk = new QPushButton(tr("Valider"), rowB);
    m_btnCancel = new QPushButton(tr("Annuler"), rowB);
    hbb->addWidget(m_btnOk);
    hbb->addWidget(m_btnCancel);

    root->addWidget(rowB);

    // Connexions
    connect(m_btnOk, &QPushButton::clicked, this, &FormSiteInfoPanel::onAccept);
    connect(m_btnCancel, &QPushButton::clicked, this, &FormSiteInfoPanel::onCancel);
}

void
FormSiteInfoPanel::setLeadText(const QString& lead)
{
    m_leadLabel->setText(lead);
}

void
FormSiteInfoPanel::setFromData(const SiteInfoData& d)
{
    m_siteEdit->setText(d.nomSite);
    m_reeEdit->setText(d.nomREE);
    m_idxEdit->setText(d.indiceREE);
    const int n = qMin(10, d.tranches.size());
    for (int i = 0; i < n; ++i) {
        m_checks[i]->setChecked(d.tranches.testBit(i));
    }
}

SiteInfoData
FormSiteInfoPanel::currentData() const
{
    SiteInfoData d;
    d.nomSite = m_siteEdit->text().trimmed();
    d.nomREE = m_reeEdit->text().trimmed();
    d.indiceREE = m_idxEdit->text().trimmed();
    for (int i = 0; i < 10; ++i)
        d.tranches.setBit(i, m_checks[i]->isChecked());
    return d;
}

void
FormSiteInfoPanel::onAccept()
{
    emit accepted(currentData());
}

void
FormSiteInfoPanel::onCancel()
{
    emit canceled();
}

/* ==== Helper : exécuter ce formulaire dans un overlay modal ==== */

int
execSiteInfoModal(QWidget* parent, /*OUT*/ SiteInfoData& out, const SiteInfoData* defaults, const QString& title, int fadeDurationMs)
{
    // 1) Overlay flouté + snapshot
    // A 1.0 : non flouté pour conserver l'overlay précédent
    auto* overlay = new OverlayBlurWidget(parent, /*blurRadius*/ 1.0, /*downscale*/ 0.5, false);
    overlay->refreshSnapshot();

    // Assure un voile/fade identique à execModal()
    overlay->setBaseVeilOpacity(100); // valeur par défaut de execModal()
    overlay->beginOverlayVeilFadeIn(fadeDurationMs);

    // 2) Panel composite (titre + formulaire)
    auto* panel = new QWidget(overlay);
    panel->setObjectName("FormCompositePanel");
    panel->setStyleSheet("#FormCompositePanel { background: rgba(30,30,30,220); border-radius: 12px; }"
                         "QLabel { color: white; }");

    auto* root = new QVBoxLayout(panel);
    root->setContentsMargins(20, 20, 20, 16);
    root->setSpacing(12);

    auto* titleLabel = new QLabel(title, panel);
    {
        QFont tf = titleLabel->font();
        tf.setBold(true);
        tf.setPointSizeF(tf.pointSizeF() + 2);
        titleLabel->setFont(tf);
        titleLabel->setWordWrap(true);
    }
    root->addWidget(titleLabel);

    auto* form = new FormSiteInfoPanel(panel);
    form->setLeadText(QObject::tr("Merci de renseigner les différentes informations :"));
    if (defaults)
        form->setFromData(*defaults);
    root->addWidget(form);

    // 3) Centrage du panel (lambda CORRECTE avec capture)
    auto centerPanel = [overlay, panel] {
        const int margin = 24;
        QSize pref = panel->sizeHint();
        if (pref.width() < 420)
            pref.setWidth(420);
        if (pref.width() > 720)
            pref.setWidth(720);
        QRect r = overlay->rect().adjusted(margin, margin, -margin, -margin);
        QSize sz(qMin(pref.width(), r.width()), qMin(qMax(pref.height(), 180), r.height()));
        QPoint topLeft = r.center() - QPoint(sz.width() / 2, sz.height() / 2);
        panel->setGeometry(QRect(topLeft, sz));
    };
    centerPanel();
    panel->show();
    panel->setFocus();

    // PATCH

    // Suivre les redimensionnements/déplacements de la fenêtre
    auto* resizer = new PanelResizer(parent, overlay, panel);
    if (parent)
        parent->installEventFilter(resizer);

    // Nettoyage auto
    QObject::connect(overlay, &QObject::destroyed, resizer, &QObject::deleteLater);
    QObject::connect(panel, &QObject::destroyed, resizer, &QObject::deleteLater);

    // 4) Animation d’entrée simple (slide depuis le haut)
    const int animMs = qMax(60, fadeDurationMs);
    QElapsedTimer clock;
    clock.start();

    QRect targetGeom = panel->geometry();
    QRect startGeom = targetGeom.translated(0, -20);
    panel->setGeometry(startGeom);

    QTimer* anim = new QTimer(panel);
    anim->setInterval(16); // ~60 FPS
    QObject::connect(anim, &QTimer::timeout, [panel, startGeom, targetGeom, anim, animMs, &clock] {
        qreal t = qBound<qreal>(0.0, clock.elapsed() / qreal(animMs), 1.0);
        qreal e = 1.0 - std::pow(1.0 - t, 3.0); // easeOutCubic
        QRect r = targetGeom;
        r.moveTop(startGeom.y() + int((targetGeom.y() - startGeom.y()) * e));
        panel->setGeometry(r);
        if (t >= 1.0) {
            anim->stop();
            anim->deleteLater();
        }
    });
    anim->start();

    // 5) Boucle locale modale + connexions (LAMBDA AVEC PARAMÈTRE)
    int result = -1;       // 1 = OK, 0 = Cancel
    SiteInfoData gathered; // données collectées
    QEventLoop loop;

    // IMPORTANT : la lambda reçoit bien "const SiteInfoData& data"
    QObject::connect(form, &FormSiteInfoPanel::accepted, &loop, [&](const SiteInfoData& data) {
        gathered = data;
        result = 1;
        loop.quit();
    });

    QObject::connect(form, &FormSiteInfoPanel::canceled, &loop, [&] {
        result = 0;
        loop.quit();
    });

    // (Optionnel) Si l’overlay est détruit d’une autre façon, ne reste pas bloqué :
    QObject::connect(overlay, &QObject::destroyed, &loop, &QEventLoop::quit);

    loop.exec(); // Attend 'accepted' ou 'canceled'

    // 6) Nettoyage

    // Nettoyage panel tout de suite
    panel->deleteLater();

    // Laisse l’overlay faire un fade-out identique à execModal(), puis auto-delete
    overlay->beginOverlayVeilFadeOutAndDelete(fadeDurationMs);

    // Recopie de la valeur si OK
    if (result == 1)
        out = gathered;
    return (result == 1 ? 1 : 0);
}
