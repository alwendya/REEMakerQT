#ifndef BLUROVERLAY_H
#define BLUROVERLAY_H

#pragma once

#include <QBitArray>
#include <QByteArray>
#include <QDialogButtonBox>
#include <QElapsedTimer>
#include <QIcon>
#include <QImage>
#include <QPointer>
#include <QStyle>
#include <QTimer>
#include <QVector>
#include <QWidget>

/* =========================
 *   Modal custom + flou
 * ========================= */

class BlurModalPanel; // forward

// ---- Modèle de bouton ----
struct ModalButton
{
    int id; // valeur retournée
    QString text;
    QDialogButtonBox::ButtonRole role = QDialogButtonBox::AcceptRole;
    bool isDefault = false;
    bool isEscape = false;
};

// ---- Configuration du modal ----
struct ModalConfig
{
    QString title;
    QString message; // Rich text accepté
    QIcon icon;
    QSize iconSize = QSize(48, 48);

    QPixmap preview; // optionnel
    QSize previewMaxSize = QSize(480, 270);

    QVector<ModalButton> buttons; // au moins 1
    int defaultButtonId = -1;
    int escapeButtonId = -1;

    int panelMinWidth = 420;
    int panelMaxWidth = 720;

    bool enableFade = true;
    int fadeDurationMs = 160;         // utilisé par le fade "maison"
    bool clickOutsideToClose = false; // si true + escape défini, clic dehors => escape
};

// ---- Overlay qui floute + affiche un modal ----
class OverlayBlurWidget : public QWidget
{
    Q_OBJECT
  public:
    explicit OverlayBlurWidget(QWidget* parent, qreal blurRadius = 18.0, qreal downscale = 0.5, bool InstallEvent = true);

    void refreshSnapshot();                 // capture + flou
    int execModal(const ModalConfig& cfg);  // API synchrone
    void showModal(const ModalConfig& cfg); // API asynchrone

    // Helpers
    static QIcon iconFromStandard(QStyle::StandardPixmap sp, QWidget* ref);
    static ModalButton makeButton(int id,
                                  const QString& text,
                                  QDialogButtonBox::ButtonRole role = QDialogButtonBox::AcceptRole,
                                  bool isDefault = false,
                                  bool isEscape = false);

    // Lance un fade-in/out du VOILE uniquement (sans panel interne) :
    void beginOverlayVeilFadeIn(int fadeDurationMs = 160);
    void beginOverlayVeilFadeOutAndDelete(int fadeDurationMs = 160);

    void setBaseVeilOpacity(int a);
    int baseVeilOpacity() const;

  signals:
    void finished(int id); // pour showModal()
    void fadeOutDone();    // interne pour execModal()

  protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

  private slots:
    // Slots internes
    void onPanelRequestClose(int buttonId);
    void onAnimTick();

  private:
    // opacité max du voile (0..255), identique à l’ancienne constante 100
    int m_baseVeilOpacity = 100;

    // — données du flou de fond —
    QImage m_blurred;
    qreal m_radius;
    qreal m_downscale;
    bool m_InstallEventFilter;

    // — panel modal —
    QPointer<BlurModalPanel> m_panel;
    ModalConfig m_cfg;

    // — gestion exec() —
    int m_execResult = -1;
    bool m_inExec = false;

    // — animation "maison" —
    enum AnimKind
    {
        AnimNone,
        AnimFadeIn,
        AnimFadeOut
    };
    AnimKind m_animKind = AnimNone;
    qreal m_overlayAlpha = 0.0; // 0..1 (voile)
    QTimer* m_animTimer = nullptr;
    QElapsedTimer m_animClock;
    int m_animDurationMs = 160;
    QRect m_panelStartGeom;
    QRect m_panelTargetGeom;
    int m_resultOnFadeOut = -1;
    bool m_emitSignalOnFadeOut = false;

    // utilitaires
    void ensureFullSize();
    void centerPanel();

    static QImage toArgb32(const QPixmap& pix);
    static QImage blurWithGraphicsEffect(const QImage& src, qreal radius);

    void startFadeIn();
    void startFadeOut(int resultId, bool emitSignal);
};

/* =========================
 *   Panneau modal interne
 * ========================= */

#include <QFrame>

class BlurModalPanel : public QFrame
{
    Q_OBJECT
  public:
    explicit BlurModalPanel(OverlayBlurWidget* overlay, const ModalConfig& cfg);

  signals:
    void requestClose(int buttonId); // id du bouton pressé

  protected:
    void keyPressEvent(QKeyEvent*) override;

  private slots:
    void onButtonClicked(class QAbstractButton* button);

  private:
    int m_resultId = -1;
    int m_escapeId = -1;
    int m_defaultId = -1;

    class QLabel* m_iconLabel = nullptr;
    class QLabel* m_titleLabel = nullptr;
    class QLabel* m_messageLabel = nullptr;
    class QLabel* m_previewLabel = nullptr;
    class QDialogButtonBox* m_buttonBox = nullptr;

    void buildUi(const ModalConfig& cfg);
    void wireButtons(const ModalConfig& cfg);
    void applyDefaultAndEscape(const ModalConfig& cfg);
    static bool isRich(const QString& s);
};

/* =========================
 *   ProgressOverlay (non-modale)
 * ========================= */

class QLabel;
class QProgressBar;

/**
 * @brief Overlay léger non-modale pour afficher une progression avec texte.
 * - Déterminée: maximum > 0 -> affiche le pourcentage (%p%)
 * - Indéterminée: maximum == 0 -> barre "busy"
 * - Texte modifiable en temps réel
 * - Par défaut : ne bloque pas l’input (clics passent à travers)
 * - Fade in/out "maison" + option **flou de fond** à la place du voile
 */
class ProgressOverlay : public QWidget
{
    Q_OBJECT
  public:
    explicit ProgressOverlay(QWidget* parent, bool center = true, bool blockInput = false, int fadeDurationMs = 140);
    ~ProgressOverlay() override = default;

    // Helpers
    static ProgressOverlay* showDeterminate(QWidget* parent,
                                            const QString& text,
                                            int maximum,
                                            bool center = true,
                                            bool blockInput = false,
                                            int fadeDurationMs = 140);
    static ProgressOverlay* showIndeterminate(QWidget* parent,
                                              const QString& text,
                                              bool center = true,
                                              bool blockInput = false,
                                              int fadeDurationMs = 140);

    // Positionnement
    void centerInParent();
    void anchorBottomRight(int margin = 16);

    // Contrôle de la progression
    void setMaximum(int maximum); // 0 => indéterminé
    void setValue(int value);
    void setText(const QString& text);

    int maximum() const;
    int value() const;

    // Flou de fond (au lieu du voile)
    void enableBackdropBlur(bool enable, qreal radius = 12.0, qreal downscale = 0.5);
    void refreshBackdropSnapshot(); // recapture le fond (utile si l'UI change)

    // Fermer/Nettoyer (avec fade)
    void finish(const QString& finalText = QString(), int msBeforeFadeOut = 0);
    void cancelAndClose(); // fermeture immédiate (sans fade)

  protected:
    void paintEvent(QPaintEvent*) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void mousePressEvent(QMouseEvent* e) override;

  private slots:
    void onAnimTick();
    void onFinishTimeout();

  private:
    QWidget* m_panel = nullptr;
    QLabel* m_label = nullptr;
    QProgressBar* m_bar = nullptr;

    bool m_centered = true;
    bool m_blockInput = false;
    int m_margin = 16;

    // Animation
    enum AnimKind
    {
        AnimNone,
        AnimFadeIn,
        AnimFadeOut
    };
    AnimKind m_animKind = AnimNone;
    qreal m_alpha = 0.0; // 0..1 (intensité voile/fond)
    QTimer m_animTimer;
    QElapsedTimer m_animClock;
    int m_animDurationMs = 140;
    QRect m_startGeom;
    QRect m_targetGeom;

    // Finish différé
    QTimer m_finishTimer;

    // Flou de fond
    bool m_useBlur = false;
    qreal m_blurRadius = 12.0;
    qreal m_blurDownscale = 0.5;
    QImage m_blurred;

    void buildUi();
    void reposition();

    // animation helpers
    void startFadeIn();
    void startFadeOut();

    // blur helpers
    void makeBlurSnapshot(); // appelé par enableBackdropBlur/refreshBackdropSnapshot
};

/* =========================
 *   ToastOverlay (non-modale)
 * ========================= */

/**
 * @brief Petit "toast" informatif non-modale, auto‑fermeture, fade "maison".
 * Dessine lui-même (pas de widgets enfants) pour un rendu alpha propre et simple.
 */
class ToastOverlay : public QWidget
{
    Q_OBJECT
  public:
    enum Corner
    {
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        BottomCenter
    };

    static ToastOverlay* showToast(QWidget* parent,
                                   const QString& text,
                                   int durationMs = 3000,
                                   Corner corner = BottomRight,
                                   int margin = 16,
                                   int maxTextWidth = 420,
                                   int fadeDurationMs = 140);

  protected:
    void paintEvent(QPaintEvent*) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

  private slots:
    void onAnimTick();
    void onAutoClose();

  private:
    explicit ToastOverlay(QWidget* parent);

    // config
    QString m_text;
    int m_durationMs = 3000;
    Corner m_corner = BottomRight;
    int m_margin = 16;
    int m_maxTextWidth = 420;

    // anim
    int m_fadeDurationMs = 140;
    enum AnimKind
    {
        AnimNone,
        AnimFadeIn,
        AnimHold,
        AnimFadeOut
    };
    AnimKind m_animKind = AnimNone;
    qreal m_alpha = 0.0;
    QTimer m_animTimer;
    QElapsedTimer m_animClock;

    QTimer m_holdTimer; // durée d'affichage avant fade-out

    // layout
    QRect m_panelRect; // rect calculé du toast
    void recomputePanelRect();

    // helpers
    void startFadeIn();
    void startFadeOut();
    void positionPanel();
};

/* =========================
 *   Formulaire Site/REE/Tranches
 * ========================= */

struct SiteInfoData
{
    QString nomSite;
    QString nomREE;
    QString indiceREE;
    QBitArray tranches; // taille 10 (Tr.0 .. Tr.9)
    SiteInfoData()
      : tranches(10, false)
    {
    }
};

// Panel interne
class FormSiteInfoPanel : public QFrame
{
    Q_OBJECT
  public:
    explicit FormSiteInfoPanel(QWidget* parent = nullptr);
    void setLeadText(const QString& lead);
    void setFromData(const SiteInfoData& d);
    SiteInfoData currentData() const;

  signals:
    void accepted(const SiteInfoData& d);
    void canceled();

  private slots:
    void onAccept();
    void onCancel();

  private:
    class QLabel* m_leadLabel = nullptr;
    class QLineEdit* m_siteEdit = nullptr;
    class QLineEdit* m_reeEdit = nullptr;
    class QLineEdit* m_idxEdit = nullptr;
    QVector<class QCheckBox*> m_checks;
    class QPushButton* m_btnOk = nullptr;
    class QPushButton* m_btnCancel = nullptr;

    void buildUi();
};

// Helper haut-niveau (synchrone) : affiche l’overlay modal avec ce formulaire
int
execSiteInfoModal(QWidget* parent,
                  /*OUT*/ SiteInfoData& out,
                  const SiteInfoData* defaults = nullptr,
                  const QString& title = QObject::tr("Informations Site / REE"),
                  int fadeDurationMs = 160);

#endif // BLUROVERLAY_H
