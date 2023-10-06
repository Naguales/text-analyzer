#ifndef GLOWEDBUTTON_H
#define GLOWEDBUTTON_H

#include <QPushButton>
#include <QGraphicsDropShadowEffect>

class CGlowedButton : public QPushButton
{
    Q_OBJECT
     
public:
    //! @param glowColor with alpha component.
    CGlowedButton(const QString &text, const QColor& glowColor, QWidget* parent = nullptr);

protected:
    void enterEvent(QEvent*) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent*) Q_DECL_OVERRIDE;

private:
    QGraphicsDropShadowEffect* m_shadowEffect;

    double m_scaleFactor { 1.0 };
};

#endif // GLOWEDBUTTON_H
