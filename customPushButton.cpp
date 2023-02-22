#include "customPushButton.h"
#include <QPainter>
#include <QDebug>
///
/// \brief customPushButton::customPushButton   Création d'un PushButton avec l'image insérée de la bonne manière
/// \param parent
///
customPushButton::customPushButton(QWidget* parent) : QPushButton(parent)
{
    m_color = QColor(0,0,255,255);
    m_pixmap = QPixmap ("://ColorPicker16");
}

///
/// \brief customPushButton::~customPushButton  Déchargement du PushButton
///
customPushButton::~customPushButton()
{
}

///
/// \brief customPushButton::sizeHint   Surcharge du redimensionnement initial
/// \return
///
QSize customPushButton::sizeHint() const
{
    const auto parentHint = QPushButton::sizeHint();
    // add margins here if needed
    return QSize(parentHint.width() + m_pixmap.width(), std::max(parentHint.height(), m_pixmap.height()));
}

///
/// \brief customPushButton::paintEvent     Surcharge de la fonction Paint pour afficher correctement l'icone
/// \param e
///
void customPushButton::paintEvent(QPaintEvent* e)
{
    QPushButton::paintEvent(e);
    if (!m_pixmap.isNull())
    {
        const int y = (height() - m_pixmap.height()) / 2; // add margin if needed

        QPixmap *pix = new QPixmap(40,m_pixmap.height());
        QPainter *paint = new QPainter(pix);
        paint->fillRect(0,0,40,m_pixmap.height(),QBrush(m_color));
        paint->setPen(*(new QColor(0,0,0,255)));
        paint->drawRect(0,0,40-1,m_pixmap.height()-2+1);//(1 for margin arrangment)
        QPainter painter(this);
        painter.drawPixmap(5, y, m_pixmap); // softcoded horizontal margin
        painter.drawPixmap(25, y, *pix); // softcoded horizontal margin
    }
}

///
/// \brief customPushButton::setColor   Permet de régler la couleur en aperçu
/// \param color
///
void customPushButton::setColor(QColor color)
{
    m_color = color;
    this->repaint();
}
///
/// \brief customPushButton::getColor   Permet de récupérer la couleur en aperçu
/// \return
///
QColor customPushButton::getColor()
{
    return m_color;
}
