from expense_manager import ExpenseManager

m = ExpenseManager()
m.add_expense(user="ashish", expense_date="2026-02-20", category="Food", description="Lunch", amount=12.5)
m.add_expense(user="ashish", expense_date="2026-02-21", category="Gas", description="Fuel", amount=40)

assert len(m.filter_expenses(user="ashish")) == 2
assert m.monthly_total(2026, 2, user="ashish") == 52.5
assert m.monthly_total(2026, 2, category="Food", user="ashish") == 12.5
print("OK")
