#ifndef SUMMARY_WIDGET_H
#define SUMMARY_WIDGET_H

#include "pie_chart_widget.h"
#include <QWidget>
#include <map>
#include <QString>

class QLabel;
class QVBoxLayout;

class SummaryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SummaryWidget(QWidget* parent = NULL);

    // Pass the FILTERED records' totals — summary always matches the table
    void refreshData(const QString& userName,
                     const std::map<QString, double>& categoryTotals,
                     double overallTotal);

private:
    void buildCategoryLabels();

    QLabel*         m_userLabel;
    QLabel*         m_totalLabel;
    QVBoxLayout*    m_categoryLayout;
    PieChartWidget* m_pieChart;

    std::map<QString, double> m_categoryTotals;
    double                    m_overallTotal;
};

#endif // SUMMARY_WIDGET_H
