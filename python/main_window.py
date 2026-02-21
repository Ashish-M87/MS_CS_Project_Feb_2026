from __future__ import annotations
import csv
from collections import defaultdict
from pathlib import Path
from typing import Any
from PySide6.QtCore import QFile, QDate
from PySide6.QtUiTools import QUiLoader
from PySide6.QtWidgets import (
    QComboBox,
    QDateEdit,
    QDialog,
    QFileDialog,
    QFrame,
    QHBoxLayout,
    QInputDialog,
    QLabel,
    QMainWindow,
    QMessageBox,
    QPushButton,
    QSizePolicy,
    QTableView,
    QTextEdit,
    QVBoxLayout,
    QWidget,
    QHeaderView,
)

try:
    from PySide6.QtCharts import QChart, QChartView, QPieSeries

    HAS_QT_CHARTS = True
except ImportError:
    HAS_QT_CHARTS = False

from add_expense_dialog import AddEditDialog
from expense_manager import ExpenseManager
from table_model import ExpenseTableModel



class MainWindow(QMainWindow):
    def __init__(self) -> None:
        super().__init__()
        self.load_main_ui()

        self.manager = ExpenseManager()
        self.data_path = Path(__file__).resolve().parent.parent / "shared" / "data" / "expenses.json"
        self.load_data()

        self.bind_or_create_widgets()

        self.current_view: list[dict[str, Any]] = []
        self.table_model = ExpenseTableModel([])
        self.expenseTableView.setModel(self.table_model)
        self.expenseTableView.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        self.expenseTableView.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        self.expenseTableView.verticalHeader().setSectionResizeMode(QHeaderView.ResizeToContents)

        splitter = self.findChild(QWidget, "mainSplitter")
        if splitter is not None and hasattr(splitter, "setStretchFactor"):
            splitter.setStretchFactor(0, 5)
            splitter.setStretchFactor(1, 2)

        self.setup_filter_defaults()
        self.connect_signals()
        self.refresh_user_dropdown()
        self.refresh_category_filter_dropdown()
        self.refresh_table()

    def load_main_ui(self) -> None:
        ui_path = Path(__file__).resolve().parent.parent / "shared" / "ui" / "main_window.ui"
        loader = QUiLoader()
        ui_file = QFile(str(ui_path))
        if not ui_file.open(QFile.ReadOnly):
            raise RuntimeError(f"Unable to open UI file: {ui_path}")
        loaded = loader.load(ui_file)
        ui_file.close()
        if loaded is None:
            raise RuntimeError(f"Unable to load UI file: {ui_path}")

        if isinstance(loaded, QMainWindow):
            central = loaded.centralWidget()
            if central is None:
                raise RuntimeError("main_window.ui has no central widget")
            central.setParent(self)
            self.setCentralWidget(central)
            if loaded.statusBar() is not None:
                loaded.statusBar().setParent(self)
                self.setStatusBar(loaded.statusBar())
        else:
            self.setCentralWidget(loaded)
        self.setWindowTitle("Expense Tracker")

    def _find_any(self, cls, *names: str):
        for name in names:
            widget = self.findChild(cls, name)
            if widget is not None:
                return widget
        return None

    def root_layout(self) -> QVBoxLayout:
        layout = self.centralWidget().layout()
        if not isinstance(layout, QVBoxLayout):
            raise RuntimeError("main_window.ui root layout must be QVBoxLayout")
        return layout

    def bind_or_create_widgets(self) -> None:
        self.fromDateEdit = self._find_any(QDateEdit, "fromDateEdit", "dateFromEdit")
        self.toDateEdit = self._find_any(QDateEdit, "toDateEdit", "dateToEdit")
        self.categoryFilterComboBox = self._find_any(QComboBox, "categoryFilterComboBox", "categoryCombo")
        self.clearFiltersButton = self._find_any(QPushButton, "clearFiltersButton", "clearFiltersBtn")
        self.expenseTableView = self._find_any(QTableView, "expenseTableView", "tableView")

        self.summaryContainerLayout = self._find_any(QVBoxLayout, "summaryContainerLayout")
        if self.summaryContainerLayout is None:
            raise RuntimeError("main_window.ui is missing summaryContainerLayout")

        if (
            self.fromDateEdit is None
            or self.toDateEdit is None
            or self.categoryFilterComboBox is None
            or self.clearFiltersButton is None
            or self.expenseTableView is None
        ):
            raise RuntimeError("main_window.ui filter/table widgets are missing")

        self.create_top_toolbar_if_missing()
        self.create_summary_panel_if_missing()

    def create_top_toolbar_if_missing(self) -> None:
        self.addButton = self.findChild(QPushButton, "addButton")
        self.editButton = self.findChild(QPushButton, "editButton")
        self.deleteButton = self.findChild(QPushButton, "deleteButton")
        self.exportCsvButton = self.findChild(QPushButton, "exportCsvButton")
        self.manageUsersButton = self.findChild(QPushButton, "manageUsersButton")
        self.userComboBox = self.findChild(QComboBox, "userComboBox")

        if all(
            widget is not None
            for widget in [
                self.addButton,
                self.editButton,
                self.deleteButton,
                self.exportCsvButton,
                self.manageUsersButton,
                self.userComboBox,
            ]
        ):
            return

        frame = QFrame(self)
        frame.setFrameShape(QFrame.StyledPanel)
        frame.setObjectName("dynamicToolbarFrame")
        frame.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        frame.setMinimumHeight(40)
        frame.setMaximumHeight(46)

        row = QHBoxLayout(frame)
        row.setContentsMargins(6, 4, 6, 4)
        row.setSpacing(8)

        self.addButton = QPushButton("Add Expense", frame)
        self.addButton.setObjectName("addButton")
        self.editButton = QPushButton("Edit", frame)
        self.editButton.setObjectName("editButton")
        self.deleteButton = QPushButton("Delete", frame)
        self.deleteButton.setObjectName("deleteButton")
        self.exportCsvButton = QPushButton("Export CSV", frame)
        self.exportCsvButton.setObjectName("exportCsvButton")

        user_label = QLabel("User:", frame)
        self.userComboBox = QComboBox(frame)
        self.userComboBox.setObjectName("userComboBox")
        self.userComboBox.setEditable(True)
        self.userComboBox.setMinimumWidth(180)

        self.manageUsersButton = QPushButton("Manage Users", frame)
        self.manageUsersButton.setObjectName("manageUsersButton")

        row.addWidget(self.addButton)
        row.addWidget(self.editButton)
        row.addWidget(self.deleteButton)
        row.addWidget(self.exportCsvButton)
        row.addWidget(user_label)
        row.addWidget(self.userComboBox)
        row.addWidget(self.manageUsersButton)
        row.addStretch(1)

        self.root_layout().insertWidget(0, frame)

    def create_summary_panel_if_missing(self) -> None:
        self.summaryUserLabel = self.findChild(QLabel, "summaryUserLabel")
        self.summaryTotalLabel = self.findChild(QLabel, "summaryTotalLabel")
        self.byCategoryText = self.findChild(QTextEdit, "byCategoryText")
        self.chartPlaceholderWidget = self.findChild(QWidget, "chartPlaceholderWidget")
        self.chartPlaceholderLayout = self.findChild(QVBoxLayout, "chartPlaceholderLayout")

        self.summaryMonthDateEdit = self.findChild(QDateEdit, "summaryMonthDateEdit")
        self.summaryCategoryComboBox = self.findChild(QComboBox, "summaryCategoryComboBox")
        self.summaryApplyButton = self.findChild(QPushButton, "summaryApplyButton")

        if (
            self.summaryUserLabel is not None
            and self.summaryTotalLabel is not None
            and self.byCategoryText is not None
            and self.chartPlaceholderWidget is not None
            and self.chartPlaceholderLayout is not None
        ):
            return

        self.summaryUserLabel = QLabel("User: -", self)
        self.summaryUserLabel.setObjectName("summaryUserLabel")
        self.summaryUserLabel.setStyleSheet("font-weight: bold;")

        self.summaryTotalLabel = QLabel("Total: $0.00", self)
        self.summaryTotalLabel.setObjectName("summaryTotalLabel")
        self.summaryTotalLabel.setStyleSheet("font-size: 12pt; font-weight: bold;")

        summary_filter_row = QHBoxLayout()
        self.summaryMonthDateEdit = QDateEdit(self)
        self.summaryMonthDateEdit.setObjectName("summaryMonthDateEdit")
        self.summaryMonthDateEdit.setCalendarPopup(True)
        self.summaryMonthDateEdit.setDisplayFormat("MMM yyyy")

        self.summaryCategoryComboBox = QComboBox(self)
        self.summaryCategoryComboBox.setObjectName("summaryCategoryComboBox")

        self.summaryApplyButton = QPushButton("Apply", self)
        self.summaryApplyButton.setObjectName("summaryApplyButton")

        summary_filter_row.addWidget(self.summaryMonthDateEdit)
        summary_filter_row.addWidget(self.summaryCategoryComboBox)
        summary_filter_row.addWidget(self.summaryApplyButton)

        by_category_label = QLabel("By Category:", self)
        by_category_label.setStyleSheet("font-weight: bold;")

        self.byCategoryText = QTextEdit(self)
        self.byCategoryText.setObjectName("byCategoryText")
        self.byCategoryText.setReadOnly(True)

        chart_title = QLabel("Spending Breakdown:", self)
        chart_title.setStyleSheet("font-weight: bold;")

        self.chartPlaceholderWidget = QWidget(self)
        self.chartPlaceholderWidget.setObjectName("chartPlaceholderWidget")
        self.chartPlaceholderLayout = QVBoxLayout(self.chartPlaceholderWidget)
        self.chartPlaceholderLayout.setObjectName("chartPlaceholderLayout")
        self.chartPlaceholderLayout.setContentsMargins(0, 0, 0, 0)

        self.summaryContainerLayout.addWidget(self.summaryUserLabel)
        self.summaryContainerLayout.addWidget(self.summaryTotalLabel)
        self.summaryContainerLayout.addLayout(summary_filter_row)
        self.summaryContainerLayout.addWidget(by_category_label)
        self.summaryContainerLayout.addWidget(self.byCategoryText)
        self.summaryContainerLayout.addWidget(chart_title)
        self.summaryContainerLayout.addWidget(self.chartPlaceholderWidget)

    def load_data(self) -> None:
        self.manager.load_from_json(self.data_path)

    def setup_filter_defaults(self) -> None:
        sentinel = QDate(1900, 1, 1)

        self.fromDateEdit.setMinimumDate(sentinel)
        self.fromDateEdit.setDate(sentinel)
        self.fromDateEdit.setSpecialValueText("Any")

        self.toDateEdit.setMinimumDate(sentinel)
        self.toDateEdit.setDate(sentinel)
        self.toDateEdit.setSpecialValueText("Any")

        self.categoryFilterComboBox.clear()
        self.categoryFilterComboBox.addItem("All Categories", "")

        self.summaryMonthDateEdit.setMinimumDate(sentinel)
        self.summaryMonthDateEdit.setDate(sentinel)
        self.summaryMonthDateEdit.setSpecialValueText("All Months")
        self.summaryCategoryComboBox.clear()
        self.summaryCategoryComboBox.addItem("All Categories", "")

    def connect_signals(self) -> None:
        self.addButton.clicked.connect(self.on_add)
        self.editButton.clicked.connect(self.on_edit)
        self.deleteButton.clicked.connect(self.on_delete)
        self.exportCsvButton.clicked.connect(self.on_export_csv)
        self.manageUsersButton.clicked.connect(self.on_manage_users)

        self.userComboBox.currentTextChanged.connect(self.refresh_table)
        self.userComboBox.editTextChanged.connect(self.refresh_table)
        self.fromDateEdit.dateChanged.connect(self.refresh_table)
        self.toDateEdit.dateChanged.connect(self.refresh_table)
        self.categoryFilterComboBox.currentIndexChanged.connect(self.refresh_table)
        self.clearFiltersButton.clicked.connect(self.on_clear_filters)

        self.summaryApplyButton.clicked.connect(self.refresh_summary_from_current_view)

    def current_user(self) -> str:
        return self.userComboBox.currentText().strip()

    def require_current_user(self) -> str:
        user = self.current_user()
        if not user:
            raise ValueError("current user is required")
        return user

    def refresh_user_dropdown(self) -> None:
        current = self.current_user()
        users = sorted({expense["user"] for expense in self.manager.expenses})

        self.userComboBox.blockSignals(True)
        self.userComboBox.clear()
        self.userComboBox.addItem("")
        for user in users:
            self.userComboBox.addItem(user)

        if current:
            self.userComboBox.setCurrentText(current)
        elif users:
            self.userComboBox.setCurrentText(users[0])
        self.userComboBox.blockSignals(False)

    def refresh_category_filter_dropdown(self) -> None:
        current = self.categoryFilterComboBox.currentData() or ""
        categories = self.manager.categories(user=self.current_user() or None)

        self.categoryFilterComboBox.blockSignals(True)
        self.categoryFilterComboBox.clear()
        self.categoryFilterComboBox.addItem("All Categories", "")
        for category in categories:
            self.categoryFilterComboBox.addItem(category, category)

        idx = self.categoryFilterComboBox.findData(current)
        self.categoryFilterComboBox.setCurrentIndex(idx if idx >= 0 else 0)
        self.categoryFilterComboBox.blockSignals(False)

    def refresh_summary_filter_dropdown(self, expenses: list[dict[str, Any]]) -> None:
        current = self.summaryCategoryComboBox.currentData() or ""
        categories = sorted({expense["category"].strip() for expense in expenses})

        self.summaryCategoryComboBox.blockSignals(True)
        self.summaryCategoryComboBox.clear()
        self.summaryCategoryComboBox.addItem("All Categories", "")
        for category in categories:
            self.summaryCategoryComboBox.addItem(category, category)

        idx = self.summaryCategoryComboBox.findData(current)
        self.summaryCategoryComboBox.setCurrentIndex(idx if idx >= 0 else 0)
        self.summaryCategoryComboBox.blockSignals(False)

    def optional_date_from_edit(self, edit: QDateEdit) -> str | None:
        if edit.date() == edit.minimumDate():
            return None
        return edit.date().toString("yyyy-MM-dd")

    def summary_filtered_expenses(self, expenses: list[dict[str, Any]]) -> list[dict[str, Any]]:
        filtered = list(expenses)

        month_date = self.summaryMonthDateEdit.date()
        if month_date != self.summaryMonthDateEdit.minimumDate():
            ym = month_date.toString("yyyy-MM")
            filtered = [expense for expense in filtered if expense["date"].startswith(ym)]

        category = self.summaryCategoryComboBox.currentData() or ""
        if category:
            filtered = [expense for expense in filtered if expense["category"].strip().lower() == category.lower()]

        return filtered

    def on_clear_filters(self) -> None:
        self.fromDateEdit.setDate(self.fromDateEdit.minimumDate())
        self.toDateEdit.setDate(self.toDateEdit.minimumDate())
        self.categoryFilterComboBox.setCurrentIndex(0)
        self.refresh_table()

    def selected_expense_and_index(self) -> tuple[dict[str, Any], int] | tuple[None, None]:
        selected = self.expenseTableView.selectionModel().selectedRows()
        if not selected:
            return None, None
        row = selected[0].row()
        if row < 0 or row >= len(self.current_view):
            return None, None

        expense = self.current_view[row]
        for idx, candidate in enumerate(self.manager.expenses):
            if candidate is expense:
                return expense, idx
        return None, None

    def refresh_table(self) -> None:
        try:
            user = self.current_user()
            if not user:
                self.current_view = []
                self.table_model.set_expenses(self.current_view)
                self.refresh_category_filter_dropdown()
                self.refresh_summary_filter_dropdown(self.current_view)
                self.update_summary_panel(self.current_view)
                return

            from_date = self.optional_date_from_edit(self.fromDateEdit)
            to_date = self.optional_date_from_edit(self.toDateEdit)
            category = self.categoryFilterComboBox.currentData() or None

            self.current_view = self.manager.filter_expenses(
                from_date=from_date,
                to_date=to_date,
                category=category,
                user=user,
            )
            self.table_model.set_expenses(self.current_view)

            self.refresh_category_filter_dropdown()
            self.refresh_summary_filter_dropdown(self.current_view)
            self.update_summary_panel(self.current_view)

            if self.statusBar() is not None:
                total = len(self.manager.filter_expenses(user=user))
                self.statusBar().showMessage(f"User: {user} | Showing {len(self.current_view)} of {total} records")
        except ValueError as exc:
            QMessageBox.warning(self, "Filter Error", str(exc))

    def on_add(self) -> None:
        try:
            user = self.require_current_user()
        except ValueError as exc:
            QMessageBox.warning(self, "Validation Error", str(exc))
            return

        dialog = AddEditDialog(self)
        if dialog.exec() == QDialog.DialogCode.Accepted:
            payload = dialog.get_values()
            try:
                self.manager.add_expense(
                    user=user,
                    expense_date=payload["date"],
                    category=payload["category"],
                    description=payload["description"],
                    amount=payload["amount"],
                )
                self.manager.save_to_json(self.data_path)
                self.refresh_user_dropdown()
                self.refresh_table()
            except ValueError as exc:
                QMessageBox.warning(self, "Validation Error", str(exc))

    def on_edit(self) -> None:
        expense, index = self.selected_expense_and_index()
        if expense is None or index is None:
            QMessageBox.information(self, "Edit Expense", "Select an expense to edit")
            return

        dialog = AddEditDialog(self, expense=expense)
        if dialog.exec() == QDialog.DialogCode.Accepted:
            payload = dialog.get_values()
            try:
                user = self.require_current_user()
                self.manager.edit_expense(
                    index,
                    user=user,
                    expense_date=payload["date"],
                    category=payload["category"],
                    description=payload["description"],
                    amount=payload["amount"],
                )
                self.manager.save_to_json(self.data_path)
                self.refresh_user_dropdown()
                self.refresh_table()
            except ValueError as exc:
                QMessageBox.warning(self, "Validation Error", str(exc))

    def on_delete(self) -> None:
        _expense, index = self.selected_expense_and_index()
        if index is None:
            QMessageBox.information(self, "Delete Expense", "Select an expense to delete")
            return

        try:
            self.manager.delete_expense(index)
            self.manager.save_to_json(self.data_path)
            self.refresh_user_dropdown()
            self.refresh_table()
        except ValueError as exc:
            QMessageBox.warning(self, "Delete Error", str(exc))

    def on_export_csv(self) -> None:
        if not self.current_view:
            QMessageBox.information(self, "Export CSV", "No expenses to export")
            return

        path, _ = QFileDialog.getSaveFileName(
            self,
            "Export CSV",
            str(Path(__file__).resolve().parent / "exports" / "expenses.csv"),
            "CSV Files (*.csv)",
        )
        if not path:
            return

        export_path = Path(path)
        export_path.parent.mkdir(parents=True, exist_ok=True)
        with export_path.open("w", newline="", encoding="utf-8") as handle:
            writer = csv.writer(handle)
            writer.writerow(["Date", "Amount", "Category", "Description"])
            for expense in self.current_view:
                writer.writerow(
                    [
                        expense["date"],
                        f"{float(expense['amount']):.2f}",
                        expense["category"],
                        expense["description"],
                    ]
                )

        QMessageBox.information(self, "Export CSV", f"Exported {len(self.current_view)} expenses")

    def on_manage_users(self) -> None:
        users = sorted({expense["user"] for expense in self.manager.expenses})
        if not users:
            QMessageBox.information(self, "Manage Users", "No users available")
            return

        current = self.current_user()
        current_index = users.index(current) if current in users else 0
        selected, ok = QInputDialog.getItem(
            self,
            "Manage Users",
            "Select user:",
            users,
            current_index,
            False,
        )
        if ok and selected:
            self.userComboBox.setCurrentText(selected)

    def refresh_summary_from_current_view(self) -> None:
        self.update_summary_panel(self.current_view)

    def update_summary_panel(self, expenses: list[dict[str, Any]]) -> None:
        summary_expenses = self.summary_filtered_expenses(expenses)
        user = self.current_user()
        self.summaryUserLabel.setText(f"User: {user or '-'}")

        total = sum(float(expense["amount"]) for expense in summary_expenses)
        self.summaryTotalLabel.setText(f"Total: ${total:.2f}")

        totals: defaultdict[str, float] = defaultdict(float)
        for expense in summary_expenses:
            totals[expense["category"].strip()] += float(expense["amount"])

        if total <= 0 or not totals:
            self.byCategoryText.setPlainText("No data available for the current selection.")
        else:
            lines = []
            for category, amount in sorted(totals.items(), key=lambda it: it[1], reverse=True):
                pct = (amount / total * 100.0) if total > 0 else 0.0
                lines.append(f"{category}: ${amount:.2f} ({pct:.1f}%)")
            self.byCategoryText.setPlainText("\n".join(lines))

        self.update_chart_placeholder(totals)

    def update_chart_placeholder(self, totals: defaultdict[str, float]) -> None:
        while self.chartPlaceholderLayout.count():
            item = self.chartPlaceholderLayout.takeAt(0)
            widget = item.widget()
            if widget is not None:
                widget.deleteLater()

        if not totals:
            self.chartPlaceholderLayout.addWidget(QLabel("No chart data"))
            return

        if HAS_QT_CHARTS:
            series = QPieSeries()
            for category, amount in sorted(totals.items(), key=lambda it: it[1], reverse=True):
                series.append(category, amount)

            chart = QChart()
            chart.addSeries(series)
            chart.legend().setVisible(True)

            view = QChartView(chart)
            view.setMinimumHeight(220)
            self.chartPlaceholderLayout.addWidget(view)
        else:
            self.chartPlaceholderLayout.addWidget(QLabel("Chart unavailable (QtCharts not installed)"))
