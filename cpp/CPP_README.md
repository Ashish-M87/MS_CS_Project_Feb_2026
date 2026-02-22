# Expense Tracker — C++ / Qt6

Cross-Language Application Development — Group Project  
**Parthasarathi Ponnapalli** (C++) · **Ashish Mahajan** (Python)

---

## Project Structure

```
ExpenseTracker/
├── CMakeLists.txt               ← Root CMake configuration
├── cpp/
│   ├── CMakeLists.txt           ← C++ targets (lib + executable)
│   ├── include/
│   │   ├── expense_record.h     ← Data struct
│   │   ├── expense_repository.h ← STL-based model / storage
│   │   ├── expense_table_model.h← Qt MVC table model
│   │   ├── add_expense_dialog.h ← Add / edit dialog
│   │   ├── summary_widget.h     ← Summary panel + bar chart
│   │   └── main_window.h        ← Main window controller
│   └── src/
│       ├── main.cpp      # Entry point — QApplication + MainWindow on the stack
│       ├── main_window.cpp
│       ├── expense_repository.cpp
│       ├── expense_table_model.cpp # Qt MVC model — bridges data to QTableView
│       ├── add_expense_dialog.cpp
│       └── summary_widget.cpp
├── shared/
│   ├── ui/
│   │   ├── main_window.ui       ← Shared Qt Designer layout
│   │   └── add_expense_dialog.ui
│   ├── resources/
│   │   └── expense_theme.qss    ← Shared stylesheet
│   └── data/
│       └── expenses.json        ← Shared data file (read/written by both apps)
├── docs/
|   └── deliverable1.docx
└── build.bat                   ← Build Script to Create an Executable 
```

---

## Architecture

This application follows the **Model-View-Controller (MVC)** pattern as implemented by Qt.
```
┌─────────────────────────────────────────────────────────────┐
│                        MainWindow                           │
│                    (Controller layer)                       │
│  ┌──────────────────┐         ┌───────────────────────────┐ │
│  │  Toolbar         │         │  Filter bar               │ │
│  │  + Add           │         │  dateFrom / dateTo        │ │
│  │  + Edit          │         │  category dropdown        │ │
│  │  + Delete        │         │  Clear Filters            │ │
│  │  + Export CSV    │         └───────────────────────────┘ │
│  │  + User switcher │                                       │
│  └──────────────────┘                                       │
│                                                             │
│  ┌──────────────────────────┐  ┌──────────────────────────┐ │
│  │  QTableView              │  │  SummaryWidget           │ │
│  │  (View layer)            │  │  - User name             │ │
│  │  ↑                       │  │  - Total spend           │ │
│  │  ExpenseTableModel       │  │  - Per-category list     │ │
│  │  (Model bridge)          │  │  - PieChartWidget        │ │
│  │  ↑                       │  └──────────────────────────┘ │
│  │  QSortFilterProxyModel   │                               │
│  └──────────────────────────┘                               │
│                    ↑  refresh()                             │
│           ExpenseRepository                                 │
│           (Data layer — private std::vector)                │
│                    ↑                                        │
│           expenses.json  (disk)                             │
└─────────────────────────────────────────────────────────────┘
```

## Prerequisites

| Tool | Version |
|------|---------|
| CMake | ≥ 3.22 |
| Qt6 | ≥ 6.4 (Widgets module required) |
| C++ compiler | GCC 11 / Clang 14 / MSVC 2022 |

### Install Qt6

- **Windows** — [Qt Online Installer](https://www.qt.io/download)  
  Select: Qt 6.x → Desktop → 64-bit
- **macOS** — `brew install qt6`
- **Ubuntu/Debian** — `sudo apt install qt6-base-dev`

---

## Build Instructions

### 1. Configure

```bash
# From the expense_tracker/ root directory

# Linux / macOS
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Windows (adjust Qt path to your install)
cmake -S . -B build  -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/mingw_64"
```

### 2. Build

```bash
cmake --build build --config Release --parallel
```

### 3. Run

```bash
# Linux / macOS
./build/cpp/ExpenseTracker

# Windows
.\build\cpp\ExpenseTracker.exe
```

---

## Data File Location

The app stores `expenses.json` in the OS-standard per-user application data directory:

| Platform | Path |
|---|---|
| **Windows** | `%LOCALAPPDATA%\ExpenseTracker\ExpenseTracker\expenses.json` |
| **macOS** | `~/Library/Application Support/ExpenseTracker/expenses.json` |
| **Linux** | `~/.local/share/ExpenseTracker/expenses.json` |

On **first run only**, if `shared/data/expenses.json` exists, it is copied to this location as seed data. Subsequent runs always use the user-data path.

This is resolved using Qt's `QStandardPaths::AppLocalDataLocation` — the only path that works reliably across all platforms and build configurations.

## Git Branches

| Branch | Owner | Contents |
|--------|-------|---------|
| `cplusplus` | Parthasarathi | C++ implementation (this folder) |
| `python` | Ashish | Python / PySide6 implementation |
| `main` | Both | Merged final state after Day 3 |
