#include "summary_widget.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QFrame>
#include <QScrollArea>
#include <QFont>

SummaryWidget::SummaryWidget(QWidget* parent)
    : QWidget(parent)
    , m_overallTotal(0.0)
{
    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(8, 8, 8, 8);
    outer->setSpacing(4);

    m_userLabel = new QLabel("User: —", this);
    QFont uf = m_userLabel->font();
    uf.setBold(true);
    m_userLabel->setFont(uf);
    outer->addWidget(m_userLabel);

    m_totalLabel = new QLabel("Total: $0.00", this);
    outer->addWidget(m_totalLabel);

    QFrame* sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setFrameShadow(QFrame::Sunken);
    outer->addWidget(sep);

    QLabel* catHeading = new QLabel("By Category:", this);
    QFont ch = catHeading->font();
    ch.setBold(true);
    catHeading->setFont(ch);
    outer->addWidget(catHeading);

    // Scrollable area for the per-category label list
    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setMinimumHeight(60);
    scroll->setMaximumHeight(160);

    QWidget* container = new QWidget();
    m_categoryLayout = new QVBoxLayout(container);
    m_categoryLayout->setContentsMargins(0, 0, 0, 0);
    m_categoryLayout->setSpacing(2);
    m_categoryLayout->addStretch();
    scroll->setWidget(container);
    outer->addWidget(scroll);

    QFrame* sep2 = new QFrame(this);
    sep2->setFrameShape(QFrame::HLine);
    sep2->setFrameShadow(QFrame::Sunken);
    outer->addWidget(sep2);

    QLabel* pieHeading = new QLabel("Spending Breakdown:", this);
    QFont ph = pieHeading->font();
    ph.setBold(true);
    pieHeading->setFont(ph);
    outer->addWidget(pieHeading);

    m_pieChart = new PieChartWidget(this);
    outer->addWidget(m_pieChart, 1);   // stretch = 1, takes remaining space
}

void SummaryWidget::refreshData(const QString& userName,
                                 const std::map<QString, double>& categoryTotals,
                                 double overallTotal)
{
    m_categoryTotals = categoryTotals;
    m_overallTotal   = overallTotal;

    m_userLabel->setText(QString("User: %1").arg(userName));
    m_totalLabel->setText(QString("Total: $%1").arg(overallTotal, 0, 'f', 2));

    buildCategoryLabels();
    m_pieChart->setData(categoryTotals);
}

static void clearLayout(QVBoxLayout* layout)
{
    // Remove every widget except the trailing stretch (last item)
    while (layout->count() > 1)
    {
        QLayoutItem* item = layout->takeAt(0);
        if (QWidget* w = item->widget())
            w->deleteLater();
        delete item;
    }
}

void SummaryWidget::buildCategoryLabels()
{
    clearLayout(m_categoryLayout);

    if (m_categoryTotals.empty())
    {
        QLabel* none = new QLabel("  (no expenses)", m_categoryLayout->parentWidget());
        m_categoryLayout->insertWidget(0, none);
        return;
    }

    int idx = 0;
    std::map<QString, double>::const_iterator it;
    for (it = m_categoryTotals.begin(); it != m_categoryTotals.end(); ++it)
    {
        double pct = (m_overallTotal > 0.0) ? (it->second / m_overallTotal * 100.0) : 0.0;
        QString text = QString("  %1:  $%2  (%3%)")
                           .arg(it->first)
                           .arg(it->second, 0, 'f', 2)
                           .arg(pct, 0, 'f', 1);
        QLabel* lbl = new QLabel(text, m_categoryLayout->parentWidget());
        m_categoryLayout->insertWidget(idx++, lbl);
    }
}
