from __future__ import annotations

from pathlib import Path
from typing import Any

from PySide6.QtWidgets import QVBoxLayout
from PySide6.QtWidgets import QDialog, QDialogButtonBox, QMessageBox
from PySide6.QtUiTools import QUiLoader
from PySide6.QtCore import QFile
from PySide6.QtWidgets import QLineEdit, QDoubleSpinBox


class AddEditDialog(QDialog):
    def __init__(self, parent=None, expense: dict[str, Any] | None = None) -> None:
        super().__init__(parent)
        ui_path = Path(__file__).resolve().parent.parent / "shared" / "ui" / "add_edit_dialog.ui"
        loader = QUiLoader()
        ui_file = QFile(str(ui_path))
        if not ui_file.open(QFile.ReadOnly):
            raise RuntimeError(f"Unable to open UI file: {ui_path}")
        self.ui = loader.load(ui_file, self)
        ui_file.close()
        if self.ui is None:
            raise RuntimeError(f"Unable to load UI file: {ui_path}")

        self.setWindowTitle("Add Expense" if expense is None else "Edit Expense")
        dialog_layout = QVBoxLayout(self)
        dialog_layout.addWidget(self.ui)

        self.dateLineEdit = self.ui.findChild(QLineEdit, "dateLineEdit")
        self.categoryLineEdit = self.ui.findChild(QLineEdit, "categoryLineEdit")
        self.descriptionLineEdit = self.ui.findChild(QLineEdit, "descriptionLineEdit")
        self.amountDoubleSpinBox = self.ui.findChild(QDoubleSpinBox, "amountDoubleSpinBox")
        self.buttonBox = self.ui.findChild(QDialogButtonBox, "buttonBox")
        if (
            self.dateLineEdit is None
            or self.categoryLineEdit is None
            or self.descriptionLineEdit is None
            or self.amountDoubleSpinBox is None
            or self.buttonBox is None
        ):
            raise RuntimeError("add_edit_dialog.ui is missing one or more required widgets")

        self.buttonBox.accepted.connect(self._on_accept)
        self.buttonBox.rejected.connect(self.reject)

        if expense is not None:
            self.dateLineEdit.setText(expense["date"])
            self.categoryLineEdit.setText(expense["category"])
            self.descriptionLineEdit.setText(expense["description"])
            self.amountDoubleSpinBox.setValue(float(expense["amount"]))

    def _on_accept(self) -> None:
        if not self.dateLineEdit.text().strip():
            QMessageBox.warning(self, "Validation Error", "date is required")
            return
        if not self.categoryLineEdit.text().strip():
            QMessageBox.warning(self, "Validation Error", "category is required")
            return
        if not self.descriptionLineEdit.text().strip():
            QMessageBox.warning(self, "Validation Error", "description is required")
            return
        self.accept()

    def get_values(self) -> dict[str, Any]:
        return {
            "date": self.dateLineEdit.text().strip(),
            "category": self.categoryLineEdit.text().strip(),
            "description": self.descriptionLineEdit.text().strip(),
            "amount": float(self.amountDoubleSpinBox.value()),
        }
