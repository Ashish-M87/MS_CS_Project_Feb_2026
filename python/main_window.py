from __future__ import annotations

from datetime import datetime
from pathlib import Path

from PySide6.QtCore import QFile
from PySide6.QtUiTools import QUiLoader
from PySide6.QtWidgets import (
    QComboBox,
    QDialog,
    QLineEdit,
    QMainWindow,
    QMessageBox,
    QPushButton,
    QTableView,
    QLabel,
)

from add_edit_dialog import AddEditDialog
from expense_manager import ExpenseManager
from table_model import ExpenseTableModel


class MainWindow(QMainWindow):
    def __init__(self) -> None:
        super().__init__()
        ui_path = Path(__file__).resolve().parent.parent / "shared" / "ui" / "main_window.ui"
        loader = QUiLoader()
        ui_file = QFile(str(ui_path))
        if not ui_file.open(QFile.ReadOnly):
            raise RuntimeError(f"Unable to open UI file: {ui_path}")
        self.ui = loader.load(ui_file, self)
        ui_file.close()
        if self.ui is None:
            raise RuntimeError(f"Unable to load UI file: {ui_path}")

        self.setCentralWidget(self.ui)
        self.setWindowTitle("Expense Tracker - Python")

        self.manager = ExpenseManager()
        self.data_path = Path(__file__).resolve().parent.parent / "shared" / "data" / "expenses.json"
        self.load_expenses()

        self.userComboBox = self.findChild(QComboBox, "userComboBox")
        self.fromDateLineEdit = self.findChild(QLineEdit, "fromDateLineEdit")
        self.toDateLineEdit = self.findChild(QLineEdit, "toDateLineEdit")
        self.categoryFilterLineEdit = self.findChild(QLineEdit, "categoryFilterLineEdit")
        self.monthLineEdit = self.findChild(QLineEdit, "monthLineEdit")
        self.summaryCategoryLineEdit = self.findChild(QLineEdit, "summaryCategoryLineEdit")
        self.summaryResultLabel = self.findChild(QLabel, "summaryResultLabel")

        self.addButton = self.findChild(QPushButton, "addButton")
        self.editButton = self.findChild(QPushButton, "editButton")
        self.deleteButton = self.findChild(QPushButton, "deleteButton")
        self.applyFilterButton = self.findChild(QPushButton, "applyFilterButton")
        self.summaryButton = self.findChild(QPushButton, "summaryButton")

        self.tableView = self.findChild(QTableView, "expenseTableView")
        if (
            self.userComboBox is None
            or self.fromDateLineEdit is None
            or self.toDateLineEdit is None
            or self.categoryFilterLineEdit is None
            or self.monthLineEdit is None
            or self.summaryCategoryLineEdit is None
            or self.summaryResultLabel is None
            or self.addButton is None
            or self.editButton is None
            or self.deleteButton is None
            or self.applyFilterButton is None
            or self.summaryButton is None
            or self.tableView is None
        ):
            raise RuntimeError("main_window.ui is missing one or more required widgets")

        self.current_view: list[dict] = []
        self.table_model = ExpenseTableModel([])
        self.tableView.setModel(self.table_model)
        self.userComboBox.setEditable(True)

        self.set_defaults()
        self.connect_signals()
        self.refresh_user_dropdown()
        self.refresh_table()

    def load_expenses(self) -> None:
        self.manager.load_from_json(self.data_path)

    def set_defaults(self) -> None:
        self.monthLineEdit.setText(datetime.now().strftime("%Y-%m"))

    def connect_signals(self) -> None:
        self.addButton.clicked.connect(self.on_add)
        self.editButton.clicked.connect(self.on_edit)
        self.deleteButton.clicked.connect(self.on_delete)
        self.applyFilterButton.clicked.connect(self.refresh_table)
        self.summaryButton.clicked.connect(self.on_summary)
        self.userComboBox.currentTextChanged.connect(self.refresh_table)
        self.userComboBox.editTextChanged.connect(self.refresh_table)

    def current_user(self) -> str:
        return self.userComboBox.currentText().strip()

    def refresh_user_dropdown(self) -> None:
        current = self.current_user()
        users = sorted({expense["user"] for expense in self.manager.expenses})
        self.userComboBox.blockSignals(True)
        self.userComboBox.clear()
        self.userComboBox.addItem("")
        for user in users:
            self.userComboBox.addItem(user)
        self.userComboBox.setCurrentText(current)
        self.userComboBox.blockSignals(False)

    def require_current_user(self) -> str:
        user = self.current_user()
        if not user:
            raise ValueError("current user is required")
        return user

    def selected_expense_and_index(self) -> tuple[dict, int] | tuple[None, None]:
        selected = self.tableView.selectionModel().selectedRows()
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
            user_filter = self.current_user()
            if not user_filter:
                self.current_view = []
                self.table_model.set_expenses(self.current_view)
                return
            self.current_view = self.manager.filter_expenses(
                from_date=self.fromDateLineEdit.text().strip() or None,
                to_date=self.toDateLineEdit.text().strip() or None,
                category=self.categoryFilterLineEdit.text().strip() or None,
                user=user_filter or None,
            )
            self.table_model.set_expenses(self.current_view)
            self.tableView.resizeColumnsToContents()
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

    def on_summary(self) -> None:
        month_text = self.monthLineEdit.text().strip()
        try:
            month_date = datetime.strptime(month_text, "%Y-%m")
            total = self.manager.monthly_total(
                month_date.year,
                month_date.month,
                category=self.summaryCategoryLineEdit.text().strip() or None,
                user=self.current_user() or None,
            )
            self.summaryResultLabel.setText(f"Total: {total:.2f}")
        except ValueError as exc:
            QMessageBox.warning(self, "Summary Error", str(exc))
