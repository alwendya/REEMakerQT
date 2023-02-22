#ifndef CUSTOMPUSHBUTTON_H
#define CUSTOMPUSHBUTTON_H

#include <QObject>
#include <QWidget>
#include <QPushButton>

class customPushButton : public QPushButton
{
public:
    explicit customPushButton(QWidget* parent = nullptr);
    virtual ~customPushButton();

    //void setPixmap(const QPixmap& pixmap);
    void setColor(QColor color);
    QColor getColor();

    virtual QSize sizeHint() const override;

protected:
    virtual void paintEvent(QPaintEvent* e) override;

private:
    QPixmap m_pixmap;
    QColor m_color;

};

#endif // CUSTOMPUSHBUTTON_H
