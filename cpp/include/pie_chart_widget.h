#ifndef PIE_CHART_WIDGET_H
#define PIE_CHART_WIDGET_H

#include <QWidget>
#include <map>
#include <QString>

/*
 * PieChartWidget — draws a pie chart using Qt's QPainter.
 *
 * Demonstrates C++ inheritance:
 *   - Inherits from QWidget
 *   - Overrides paintEvent() — Qt calls this whenever the widget needs
 *     to redraw.  The 'override' keyword tells the compiler to verify the
 *     function signature matches the base class exactly.
 *
 * Python equivalent:
 *     class PieChartWidget(QWidget):
 *         def paintEvent(self, event): ...
 */
class PieChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PieChartWidget(QWidget* parent = NULL);

    void setData(const std::map<QString, double>& categoryTotals);

    QSize sizeHint()        const override { return QSize(200, 170); }
    QSize minimumSizeHint() const override { return QSize(160, 140); }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    std::map<QString, double> m_data;
    double                    m_total;   // initialised in constructor body

    static const QColor COLOURS[];
    static const int    COLOUR_COUNT;
};

#endif // PIE_CHART_WIDGET_H
