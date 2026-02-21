from __future__ import annotations
from pathlib import Path
from typing import Any
from PySide6.QtCore import QFile, QDate
from PySide6.QtUiTools import QUiLoader
from PySide6.QtWidgets import (
    QDateEdit,
    QDialog,
    QDoubleSpinBox,
    QLineEdit,
    QMessageBox,
    QPushButton,
    QVBoxLayout,
    QWidget,
)



class AddEditDialog(QDialog):
    def __init__(self, parent=None, expense: dict[str, Any] | None = None) -> None:
        super().__init__(parent)
        ui_path = Path(__file__).resolve().parent.parent / "shared" / "ui" / "add_expense_dialog.ui"
        loader = QUiLoader()
        ui_file = QFile(str(ui_path))
        if not ui_file.open(QFile.ReadOnly):
            raise RuntimeError(f"Unable to open UI file: {ui_path}")
        dialog_widget = loader.load(ui_file)
        ui_file.close()
        if dialog_widget is None:
            raise RuntimeError(f"Unable to load UI file: {ui_path}")
        if not isinstance(dialog_widget, QWidget):
            raise RuntimeError(f"Unexpected root widget in UI file: {ui_path}")

        host_layout = QVBoxLayout(self)
        host_layout.setContentsMargins(0, 0, 0, 0)
        host_layout.addWidget(dialog_widget)

        self.dateEdit = dialog_widget.findChild(QDateEdit, "dateEdit")
        self.amountSpinBox = dialog_widget.findChild(QDoubleSpinBox, "amountSpinBox")
        self.categoryLineEdit = dialog_widget.findChild(QLineEdit, "categoryLineEdit")
        self.descriptionLineEdit = dialog_widget.findChild(QLineEdit, "descriptionLineEdit")
        self.saveButton = dialog_widget.findChild(QPushButton, "saveButton")
        self.cancelButton = dialog_widget.findChild(QPushButton, "cancelButton")

        if (
            self.dateEdit is None
            or self.amountSpinBox is None
            or self.categoryLineEdit is None
            or self.descriptionLineEdit is None
            or self.saveButton is None
            or self.cancelButton is None
        ):
            raise RuntimeError("add_expense_dialog.ui is missing one or more required widgets")

        self.setWindowTitle("Add Expense" if expense is None else "Edit Expense")
        self.saveButton.clicked.connect(self.on_save)
        self.cancelButton.clicked.connect(self.reject)

        self.dateEdit.setDate(QDate.currentDate())
        if expense is not None:
            y, m, d = expense["date"].split("-")
            self.dateEdit.setDate(QDate(int(y), int(m), int(d)))
            self.amountSpinBox.setValue(float(expense["amount"]))
            self.categoryLineEdit.setText(expense["category"])
            self.descriptionLineEdit.setText(expense["description"])

    def on_save(self) -> None:
        if not self.categoryLineEdit.text().strip():
            QMessageBox.warning(self, "Validation Error", "category is required")
            return
        if not self.descriptionLineEdit.text().strip():
            QMessageBox.warning(self, "Validation Error", "description is required")
            return
        self.accept()

    def get_values(self) -> dict[str, Any]:
        return {
            "date": self.dateEdit.date().toString("yyyy-MM-dd"),
            "category": self.categoryLineEdit.text().strip(),
            "description": self.descriptionLineEdit.text().strip(),
            "amount": float(self.amountSpinBox.value()),
        }
