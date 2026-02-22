cmake_minimum_required(VERSION 3.22)

project(ExpenseTracker
    VERSION 1.0.0
    DESCRIPTION "Cross-language Expense Tracker — C++ / Qt6 implementation"
    LANGUAGES CXX
)

# ── C++ standard ──────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)   # keep it strict standard C++11, no GNU extensions

# ── Qt automation flags ───────────────────────────────────────────────────────
# These three flags tell CMake to automatically run Qt's code-generation tools
# on any source file that needs them:
#   AUTOMOC  — runs moc on headers containing Q_OBJECT (signals and slots)
#   AUTOUIC  — runs uic on .ui files to generate ui_*.h headers
#   AUTORCC  — runs rcc on .qrc resource files (if we add any later)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Tell AUTOUIC where to find the shared .ui files
# Without this, uic would only look in the same directory as the .cpp file
set(CMAKE_AUTOUIC_SEARCH_PATHS
    "${CMAKE_SOURCE_DIR}/shared/ui"
)

# ── Find Qt6 ─────────────────────────────────────────────────────────────────
# You need Qt6 installed. Common paths:
#   Windows:  -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2019_64"
#   macOS:    -DCMAKE_PREFIX_PATH="/usr/local/opt/qt6"   (Homebrew)
#   Linux:    usually found automatically if installed via package manager
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# ── Add the C++ sub-project ───────────────────────────────────────────────────
add_subdirectory(cpp)
