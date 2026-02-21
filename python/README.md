# Expense Tracker (Python / PySide6) - AM. 

This folder contains the Python implementation of the Expense Tracker.
It uses shared Qt Designer files from `../shared/ui` and shared data at `../shared/data/expenses.json`.

## Folder layout

- `python/main.py`: app entry point
- `python/main_window.py`: main window wiring (toolbar, filters, table, summary panel, export)
- `python/add_expense_dialog.py`: add/edit dialog controller wired to `shared/ui/add_expense_dialog.ui`
- `python/expense_manager.py`: business logic + validation + filtering + JSON save/load
- `python/table_model.py`: `QAbstractTableModel` used by the table view
- `python/requirements.txt`: runtime dependencies
- `python/build_executable.sh`: local executable build script (PyInstaller)

## Data source

The app reads/writes only:
- `shared/data/expenses.json`

## Run locally

From project root (`MS_CS_Project_Feb_2026`):

```bash
python3 -m venv .venv

source .venv/bin/activate
pip install -r python/requirements.txt
python python/main.py
```

## Build executable (macOS/Linux)

This creates a PyInstaller `onedir` app under `dist/ExpenseTracker`.

```bash
source .venv/bin/activate
pip install -r python/requirements-dev.txt
bash python/build_executable.sh
```

Output:

- `dist/ExpenseTracker/ExpenseTracker` (executable)
- includes bundled `shared/ui`, `shared/resources`, and `shared/data`

## Notes

- Qt Charts are optional at runtime. If unavailable, the summary panel shows a chart placeholder message.
- UI file references:
  - `shared/ui/main_window.ui` -> handled by `python/main_window.py`
  - `shared/ui/add_expense_dialog.ui` -> handled by `python/add_expense_dialog.py`