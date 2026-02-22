#include "pie_chart_widget.h"

#include <QPainter>
#include <QFontMetrics>

const QColor PieChartWidget::COLOURS[] = {
    QColor(0x4E, 0x79, 0xA7),   // steel blue
    QColor(0xF2, 0x8E, 0x2B),   // orange
    QColor(0x59, 0xA1, 0x4F),   // green
    QColor(0xE1, 0x57, 0x59),   // red
    QColor(0xB0, 0x7A, 0xA1),   // purple
    QColor(0xFF, 0xBE, 0x7D),   // peach
    QColor(0x76, 0xB7, 0xB2),   // teal
    QColor(0xFF, 0xD7, 0x00),   // yellow
};
const int PieChartWidget::COLOUR_COUNT = 8;

PieChartWidget::PieChartWidget(QWidget* parent)
    : QWidget(parent)
    , m_total(0.0)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void PieChartWidget::setData(const std::map<QString, double>& categoryTotals)
{
    m_data  = categoryTotals;
    m_total = 0.0;
    std::map<QString, double>::const_iterator it;
    for (it = m_data.begin(); it != m_data.end(); ++it)
        m_total += it->second;
    update();   // triggers a repaint
}

void PieChartWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int W = width();
    const int H = height();

    if (m_data.empty() || m_total <= 0.0)
    {
        p.setPen(Qt::gray);
        p.drawText(rect(), Qt::AlignCenter, "No data");
        return;
    }

    // Reserve bottom of widget for the legend, top for the pie disc
    const int LEGEND_ROW_H = 16;
    const int LEGEND_H     = (int)m_data.size() * LEGEND_ROW_H + 4;
    const int PIE_H        = H - LEGEND_H;
    const int DIAMETER     = qMin(W - 16, PIE_H - 8);
    if (DIAMETER < 20) return;

    const int  PIE_X = (W - DIAMETER) / 2;
    const int  PIE_Y = 4;
    QRectF pieRect(PIE_X, PIE_Y, DIAMETER, DIAMETER);

    // ── Draw pie slices ───────────────────────────────────────────────────────
    double startAngle = 90.0;   // 12-o'clock
    int    colIdx     = 0;

    std::map<QString, double>::const_iterator slice;
    for (slice = m_data.begin(); slice != m_data.end(); ++slice)
    {
        double sweepDeg = (slice->second / m_total) * 360.0;
        QColor col = COLOURS[colIdx % COLOUR_COUNT];

        p.setBrush(col);
        p.setPen(QPen(Qt::white, 1));
        p.drawPie(pieRect,
                  (int)(startAngle * 16),
                  (int)(-sweepDeg * 16));

        startAngle -= sweepDeg;
        ++colIdx;
    }

    // ── Draw legend ───────────────────────────────────────────────────────────
    int legendY = PIE_Y + DIAMETER + 6;
    colIdx = 0;

    // Reset brush so drawRect() only draws an outline (not a filled rect)
    p.setBrush(Qt::NoBrush);

    QFont f = p.font();
    f.setPointSize(8);
    p.setFont(f);
    QFontMetrics fm(f);

    std::map<QString, double>::const_iterator leg;
    for (leg = m_data.begin(); leg != m_data.end(); ++leg)
    {
        QColor col = COLOURS[colIdx % COLOUR_COUNT];
        double pct = (leg->second / m_total) * 100.0;

        p.fillRect(4, legendY + 2, 10, 10, col);                   // coloured square
        p.setPen(palette().color(QPalette::Dark));
        p.drawRect(4, legendY + 2, 10, 10);                         // border

        QString entry = QString("%1  %2%").arg(leg->first).arg(pct, 0, 'f', 1);
        entry = fm.elidedText(entry, Qt::ElideRight, W - 20);
        p.setPen(palette().color(QPalette::WindowText));
        p.drawText(18, legendY + 12, entry);

        legendY += LEGEND_ROW_H;
        ++colIdx;
    }
}
