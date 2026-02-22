from __future__ import annotations
import json
from collections import defaultdict
from datetime import date, datetime
from pathlib import Path
from typing import Any



class ExpenseManager:
    """Owns expense records and all validation/filtering logic."""

    def __init__(self) -> None:
        # Canonical in-memory store. UI modules treat this as source of truth.
        self.expenses: list[dict[str, Any]] = []

    def validate_non_empty_string(self, value: Any, field_name: str) -> str:
        if not isinstance(value, str):
            raise ValueError(f"{field_name} must be a string")
        cleaned = value.strip()
        if not cleaned:
            raise ValueError(f"{field_name} is required")
        return cleaned

    def normalize_date(self, value: Any, field_name: str = "date") -> str:
        if isinstance(value, date):
            return value.isoformat()
        if not isinstance(value, str):
            raise ValueError(f"{field_name} must be in YYYY-MM-DD format")
        cleaned = value.strip()
        try:
            parsed = datetime.strptime(cleaned, "%Y-%m-%d").date()
        except ValueError as exc:
            raise ValueError(f"{field_name} must be in YYYY-MM-DD format") from exc
        return parsed.isoformat()

    def normalize_amount(self, value: Any) -> float:
        try:
            amount = float(value)
        except (TypeError, ValueError) as exc:
            raise ValueError("amount must be a number") from exc
        if amount < 0:
            raise ValueError("amount must be non-negative")
        return amount

    def validate_expense(
        self,
        *,
        user: Any,
        expense_date: Any,
        category: Any,
        description: Any,
        amount: Any,
    ) -> dict[str, Any]:
        return {
            "user": self.validate_non_empty_string(user, "user"),
            "date": self.normalize_date(expense_date, "date"),
            "category": self.validate_non_empty_string(category, "category"),
            "description": self.validate_non_empty_string(description, "description"),
            "amount": self.normalize_amount(amount),
        }

    def add_expense(
        self,
        *,
        user: str,
        expense_date: str,
        category: str,
        description: str,
        amount: float,
    ) -> dict[str, Any]:
        expense = self.validate_expense(
            user=user,
            expense_date=expense_date,
            category=category,
            description=description,
            amount=amount,
        )
        self.expenses.append(expense)
        return expense

    def edit_expense(
        self,
        index: int,
        *,
        user: str,
        expense_date: str,
        category: str,
        description: str,
        amount: float,
    ) -> dict[str, Any]:
        if not 0 <= index < len(self.expenses):
            raise ValueError("expense index out of range")
        updated = self.validate_expense(
            user=user,
            expense_date=expense_date,
            category=category,
            description=description,
            amount=amount,
        )
        self.expenses[index] = updated
        return updated

    def delete_expense(self, index: int) -> dict[str, Any]:
        if not 0 <= index < len(self.expenses):
            raise ValueError("expense index out of range")
        return self.expenses.pop(index)

    def parse_optional_date(self, value: Any, field_name: str) -> date | None:
        if value is None:
            return None
        if isinstance(value, str) and not value.strip():
            return None
        normalized = self.normalize_date(value, field_name)
        return datetime.strptime(normalized, "%Y-%m-%d").date()

    def filter_expenses(
        self,
        *,
        from_date: str | date | None = None,
        to_date: str | date | None = None,
        category: str | None = None,
        user: str | None = None,
    ) -> list[dict[str, Any]]:
        """Return a new filtered list without mutating self.expenses."""
        from_dt = self.parse_optional_date(from_date, "from_date")
        to_dt = self.parse_optional_date(to_date, "to_date")
        if from_dt and to_dt and from_dt > to_dt:
            raise ValueError("from_date cannot be after to_date")

        category_filter = category.strip().lower() if isinstance(category, str) else ""
        user_filter = user.strip() if isinstance(user, str) else ""

        filtered: list[dict[str, Any]] = []
        for expense in self.expenses:
            # Dates are stored as ISO strings for JSON portability.
            expense_date = datetime.strptime(expense["date"], "%Y-%m-%d").date()
            if from_dt and expense_date < from_dt:
                continue
            if to_dt and expense_date > to_dt:
                continue
            if category_filter and expense["category"].strip().lower() != category_filter:
                continue
            if user_filter and expense["user"] != user_filter:
                continue
            filtered.append(expense)
        return filtered

    def monthly_total(
        self,
        year: int,
        month: int,
        category: str | None = None,
        user: str | None = None,
    ) -> float:
        """Monthly rollup helper kept for non-UI uses and smoke tests."""
        if month < 1 or month > 12:
            raise ValueError("month must be between 1 and 12")

        category_totals: defaultdict[str, float] = defaultdict(float)
        monthly_totals: defaultdict[tuple[int, int], float] = defaultdict(float)

        user_filter = user.strip() if isinstance(user, str) else ""

        for expense in self.expenses:
            if user_filter and expense["user"] != user_filter:
                continue
            expense_date = datetime.strptime(expense["date"], "%Y-%m-%d").date()
            ym_key = (expense_date.year, expense_date.month)
            monthly_totals[ym_key] += float(expense["amount"])
            if ym_key == (year, month):
                category_totals[expense["category"].strip().lower()] += float(expense["amount"])

        if category and category.strip():
            return round(category_totals[category.strip().lower()], 2)
        return round(monthly_totals[(year, month)], 2)

    def categories(self, *, user: str | None = None) -> list[str]:
        user_filter = user.strip() if isinstance(user, str) else ""
        values = {
            expense["category"]
            for expense in self.expenses
            if not user_filter or expense["user"] == user_filter
        }
        return sorted(values)

    def save_to_json(self, file_path: str | Path) -> None:
        """Persist the full expense list to disk."""
        target = Path(file_path)
        target.parent.mkdir(parents=True, exist_ok=True)
        with target.open("w", encoding="utf-8") as handle:
            json.dump(self.expenses, handle, indent=2)

    def load_from_json(self, file_path: str | Path, *, merge: bool = False) -> None:
        """Load and validate expenses from disk before using them in memory."""
        target = Path(file_path)
        if not target.exists():
            return
        with target.open("r", encoding="utf-8") as handle:
            data = json.load(handle)
        if not isinstance(data, list):
            raise ValueError("JSON data must be a list of expenses")

        validated: list[dict[str, Any]] = []
        for idx, item in enumerate(data):
            if not isinstance(item, dict):
                raise ValueError(f"Invalid expense at index {idx}")
            validated.append(
                self.validate_expense(
                    user=item.get("user"),
                    expense_date=item.get("date"),
                    category=item.get("category"),
                    description=item.get("description"),
                    amount=item.get("amount"),
                )
            )

        if merge:
            self.expenses.extend(validated)
        else:
            self.expenses = validated