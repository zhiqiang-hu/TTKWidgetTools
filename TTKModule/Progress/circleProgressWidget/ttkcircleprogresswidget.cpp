#include "ttkcircleprogresswidget.h"

#include <QPainter>
#include <QPixmapCache>
#include <QPropertyAnimation>

TTKCircleProgressWidget::TTKCircleProgressWidget(QWidget *parent)
    : QWidget(parent),
      m_color(110, 190, 235),
      m_value(0),
      m_maximum(0),
      m_visibleValue(0),
      m_innerRadius(0.6f),
      m_outerRadius(1.0f),
      m_infiniteAnimationValue(0.0f)
{
    m_valueAnimation = new QPropertyAnimation(this, "visibleValue");
    m_infiniteAnimation = new QPropertyAnimation(this, "infiniteAnimationValue");

    m_infiniteAnimation->setLoopCount(-1);
    m_infiniteAnimation->setDuration(TTK_DN_S2MS);
    m_infiniteAnimation->setStartValue(0.0);
    m_infiniteAnimation->setEndValue(1.0);
    m_infiniteAnimation->start();
}

TTKCircleProgressWidget::~TTKCircleProgressWidget()
{
    delete m_valueAnimation;
    delete m_infiniteAnimation;
}

int TTKCircleProgressWidget::value() const
{
    return m_value;
}

int TTKCircleProgressWidget::maximum() const
{
    return m_maximum;
}

qreal TTKCircleProgressWidget::innerRadius() const
{
    return m_innerRadius;
}

qreal TTKCircleProgressWidget::outerRadius() const
{
    return m_outerRadius;
}

QColor TTKCircleProgressWidget::color() const
{
    return m_color;
}

QSize TTKCircleProgressWidget::sizeHint() const
{
    return QSize(200, 200);
}

void TTKCircleProgressWidget::setValue(int value)
{
    if(value < 0)
    {
        value = 0;
    }

    if(m_value != value)
    {
        m_valueAnimation->stop();
        m_valueAnimation->setEndValue(value);
        m_valueAnimation->setDuration(250);
        m_valueAnimation->start();

        m_value = value;
        Q_EMIT valueChanged(value);
    }
}

void TTKCircleProgressWidget::setMaximum(int maximum)
{
    if(maximum < 0)
    {
        maximum = 0;
    }

    if(m_maximum != maximum)
    {
        m_maximum = maximum;
        update();
        Q_EMIT maximumChanged(maximum);

        if(m_maximum == 0)
        {
            m_infiniteAnimation->start();
        }
        else
        {
            m_infiniteAnimation->stop();
        }
    }
}

void TTKCircleProgressWidget::setInnerRadius(qreal innerRadius)
{
    if(innerRadius > 1.0)
    {
        innerRadius = 1.0;
    }
    else if(innerRadius < 0.0)
    {
        innerRadius = 0.0;
    }

    if(m_innerRadius != innerRadius)
    {
        m_innerRadius = innerRadius;
        update();
    }
}

void TTKCircleProgressWidget::setOuterRadius(qreal outerRadius)
{
    if(outerRadius > 1.0)
    {
        outerRadius = 1.0;
    }
    else if(outerRadius < 0.0)
    {
        outerRadius = 0.0;
    }

    if(m_outerRadius != outerRadius)
    {
        m_outerRadius = outerRadius;
        update();
    }
}

void TTKCircleProgressWidget::setColor(const QColor &color)
{
    if(color != m_color)
    {
        m_color = color;
        update();
    }
}

static QRectF squared(QRectF rect)
{
    if(rect.width() > rect.height())
    {
        const qreal diff = rect.width() - rect.height();
        return rect.adjusted(diff / 2, 0, -diff / 2, 0);
    }
    else
    {
        const qreal diff = rect.height() - rect.width();
        return rect.adjusted(0, diff / 2, 0, -diff / 2);
    }
}

void TTKCircleProgressWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPixmap pix;
#if TTK_QT_VERSION_CHECK(5,13,0)
    if(!QPixmapCache::find(key(), &pix))
#else
    if(!QPixmapCache::find(key(), pix))
#endif
    {
        pix = generatePixmap();
        QPixmapCache::insert(key(), pix);
    }

    QPainter painter(this);
    painter.drawPixmap(0.5 * (width() - pix.width()), 0.5 * (height() - pix.height()), pix);
}

void TTKCircleProgressWidget::setInfiniteAnimationValue(qreal value)
{
    m_infiniteAnimationValue = value;
    update();
}

void TTKCircleProgressWidget::setVisibleValue(int value)
{
    if(m_visibleValue != value)
    {
        m_visibleValue = value;
        update();
    }
}

QString TTKCircleProgressWidget::key() const
{
    return QString("%1,%2,%3,%4,%5,%6,%7,%8")
            .arg(m_infiniteAnimationValue)
            .arg(m_visibleValue)
            .arg(m_maximum)
            .arg(m_innerRadius)
            .arg(m_outerRadius)
            .arg(width())
            .arg(height())
            .arg(m_color.rgb());
}

QPixmap TTKCircleProgressWidget::generatePixmap() const
{
    QPixmap pix(squared(rect()).size().toSize());
    pix.fill(QColor(0, 0, 0, 0));

    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF rect = pix.rect().adjusted(1, 1, -1, 1);
    const qreal margin = rect.width()*(1.0 - m_outerRadius) / 2.0;
    rect.adjust(margin, margin, -margin, -margin);
    const qreal innerRadius = m_innerRadius * rect.width() / 2.0;

    painter.setBrush(QColor(225, 225, 225));
    painter.setPen(QColor(225, 225, 225));
    painter.drawPie(rect, 0, 360 * 16);

    painter.setBrush(m_color);
    painter.setPen(m_color);

    if(m_maximum == 0)
    {
        const int startAngle = -m_infiniteAnimationValue * 360 * 16;
        const int spanAngle = 0.15 * 360 * 16;
        painter.drawPie(rect, startAngle, spanAngle);
    }
    else
    {
        const int value = qMin(m_visibleValue, m_maximum);
        const int startAngle = 90 * 16;
        const int spanAngle = -qreal(value) * 360 * 16 / m_maximum;
        painter.drawPie(rect, startAngle, spanAngle);
    }

    painter.setBrush(QColor(255, 255, 255));
    painter.setPen(QColor(0, 0, 0, 60));
    painter.drawEllipse(rect.center(), innerRadius, innerRadius);
    painter.drawArc(rect, 0, 360 * 16);
    return pix;
}

qreal TTKCircleProgressWidget::infiniteAnimationValue() const
{
    return m_infiniteAnimationValue;
}

int TTKCircleProgressWidget::visibleValue() const
{
    return m_visibleValue;
}
