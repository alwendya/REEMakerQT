/* -------------------------------------------------------------------------- */
/*      REEMaker 5 -- Grégory WENTZEL (c) 2023   Code sous licence GPL3       */
/* -------------------------------------------------------------------------- */

#include "customPushButton.h"
#include <QDebug>
#include <QPainter>

/** customPushButton:
    Création d'un PushButton avec l'image insérée de la bonne manière

    @param QWidget *parent
    @return Aucun
*/
customPushButton::customPushButton(QWidget* parent) : QPushButton(parent) {
    m_color  = QColor(0, 0, 255, 255);
    m_pixmap = QPixmap("://ColorPicker16");
}

/** ~customPushButton:
    Déchargement du PushButton

    @return Aucun
*/
customPushButton::~customPushButton() {}

/** sizeHint:
    Override du redimensionnement initial

    @return Aucun
*/
QSize customPushButton::sizeHint() const {
    const auto parentHint = QPushButton::sizeHint();
    // add margins here if needed
    return QSize(parentHint.width() + m_pixmap.width(), std::max(parentHint.height(), m_pixmap.height()));
}

/** paintEvent:
    Override de la fonction Paint pour afficher correctement l'icone

    @param QPaintEvent *e
    @return Aucun
*/
void customPushButton::paintEvent(QPaintEvent* e) {
    QPushButton::paintEvent(e);
    if (!m_pixmap.isNull()) {
        const int y = (height() - m_pixmap.height()) / 2;  // add margin if needed

        QPixmap* pix    = new QPixmap(40, m_pixmap.height());
        QPainter* paint = new QPainter(pix);
        paint->fillRect(0, 0, 40, m_pixmap.height(), QBrush(m_color));
        paint->setPen(*(new QColor(0, 0, 0, 255)));
        paint->drawRect(0, 0, 40 - 1,
                        m_pixmap.height() - 2 + 1);  //(1 for margin arrangment)
        QPainter painter(this);
        painter.drawPixmap(5, y, m_pixmap);  // softcoded horizontal margin
        painter.drawPixmap(25, y, *pix);     // softcoded horizontal margin
    }
}

/** setColor:
    Permet de régler la couleur en aperçu

    @param QColor color
    @return Aucun
*/
void customPushButton::setColor(QColor color) {
    m_color = color;
    this->repaint();
}

/** getColor:
    Permet de récupérer la couleur en aperçu

    @return QColor
*/
QColor customPushButton::getColor() {
    return m_color;
}
