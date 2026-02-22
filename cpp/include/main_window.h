#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "expense_repository.h"
#include "expense_table_model.h"
#include "summary_widget.h"

#include <QMainWindow>
#include <QSortFilterProxyModel>
#include <QComboBox>
#include <QLabel>

namespace Ui { class MainWindow; }

/*
 * MainWindow — the Controller.  It owns everything and wires UI to data.
 *
 * Memory ownership to explain:
 *
 *   m_ui, m_repo, m_model:
 *     Created with 'new' in the constructor.
 *     Deleted with 'delete' in the destructor.
 *     This is the C++ equivalent of Python creating and forgetting — except
 *     in C++ YOU decide when the memory is freed.
 *
 *   m_proxy, m_summary, m_userCombo:
 *     Also created with 'new' but given 'this' as parent.
 *     Qt's parent-child system deletes them automatically when this window
 *     closes.  We do NOT delete them manually.
 *
 * Python comparison:
 *     class MainWindow:
 *         def __init__(self):
 *             self.repo  = ExpenseRepository(path)  # GC handles cleanup
 *             self.model = ExpenseTableModel()
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = NULL);
    ~MainWindow();

private slots:
    void onAdd();
    void onEdit();
    void onDelete();
    void onExportCsv();
    void onFilterChanged();
    void onClearFilters();
    void onUserChanged(int index);
    void onManageUsers();

private:
    void setupToolbar();
    void setupFilterBar();
    void setupTableView();
    void refresh();              // apply filters and update table + summary
    int  selectedId() const;     // id of the highlighted row, or -1

    // Owned with new/delete
    Ui::MainWindow*    m_ui;
    ExpenseRepository* m_repo;
    ExpenseTableModel* m_model;

    // Qt parent-child ownership (deleted automatically)
    QSortFilterProxyModel* m_proxy;
    SummaryWidget*         m_summary;
    QComboBox*             m_userCombo;

    UserRecord m_currentUser;

    static const QString DATA_FILE;
};

#endif // MAIN_WINDOW_H
