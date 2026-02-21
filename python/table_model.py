from __future__ import annotations
from datetime import datetime
from typing import Any
from PySide6.QtCore import QAbstractTableModel, QModelIndex, Qt



class ExpenseTableModel(QAbstractTableModel):
    HEADERS = ["Date", "Amount", "Category", "Description"]

    def __init__(self, expenses: list[dict[str, Any]] | None = None) -> None:
        super().__init__()
        self._expenses: list[dict[str, Any]] = expenses or []

    def set_expenses(self, expenses: list[dict[str, Any]]) -> None:
        self.beginResetModel()
        self._expenses = expenses
        self.endResetModel()

    def rowCount(self, parent: QModelIndex = QModelIndex()) -> int:
        if parent.isValid():
            return 0
        return len(self._expenses)

    def columnCount(self, parent: QModelIndex = QModelIndex()) -> int:
        if parent.isValid():
            return 0
        return len(self.HEADERS)

    def headerData(
        self,
        section: int,
        orientation: Qt.Orientation,
        role: int = Qt.DisplayRole,
    ) -> str | None:
        if role != Qt.DisplayRole:
            return None
        if orientation == Qt.Horizontal and 0 <= section < len(self.HEADERS):
            return self.HEADERS[section]
        return str(section + 1)

    def data(self, index: QModelIndex, role: int = Qt.DisplayRole) -> Any:
        if not index.isValid() or not 0 <= index.row() < len(self._expenses):
            return None

        expense = self._expenses[index.row()]
        column = index.column()

        if role == Qt.DisplayRole:
            if column == 0:
                try:
                    return datetime.strptime(expense["date"], "%Y-%m-%d").strftime("%d %b %Y")
                except ValueError:
                    return expense["date"]
            if column == 1:
                return f"${float(expense['amount']):.2f}"
            if column == 2:
                return expense["category"]
            if column == 3:
                return expense["description"]

        if role == Qt.TextAlignmentRole and column == 1:
            return Qt.AlignRight | Qt.AlignVCenter

        return None