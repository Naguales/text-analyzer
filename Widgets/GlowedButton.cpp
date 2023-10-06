#include "GlowedButton.h"
#include "CommonData.h"

CGlowedButton::CGlowedButton(const QString& text, const QColor& glowColor, QWidget* parent)
    : QPushButton(text, parent)
{
    m_scaleFactor = logicalDpiX() / CommonData::LogicalDpiRefValue;

    m_shadowEffect = new QGraphicsDropShadowEffect(this);
    m_shadowEffect->setColor(glowColor);
    m_shadowEffect->setOffset(0, 0);
    m_shadowEffect->setBlurRadius(qRound(16 * m_scaleFactor));

    // Hide shadow effect at normal state.
    m_shadowEffect->setEnabled(false);
    setGraphicsEffect(m_shadowEffect);
}

void CGlowedButton::enterEvent(QEvent* event)
{
    QPushButton::enterEvent(event);

    if (isEnabled()) {
        m_shadowEffect->setEnabled(true);
    }
}

void CGlowedButton::leaveEvent(QEvent* event)
{
    QPushButton::leaveEvent(event);

    if (isEnabled()) {
        m_shadowEffect->setEnabled(false);
    }
}
