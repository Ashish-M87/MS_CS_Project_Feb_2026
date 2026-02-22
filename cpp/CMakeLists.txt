# ── cpp/CMakeLists.txt ────────────────────────────────────────────────────────
#
# Two targets:
#   1. ExpenseTrackerLib  — static library (model + backend logic)
#   2. ExpenseTracker     — the GUI executable
#
# KEY FIX: every header that contains Q_OBJECT must be listed as a source
# in the target that owns it.  AUTOMOC only scans files that are listed as
# sources in a target (or that live in the same directory as a listed source).
# Because our headers are in include/ (not src/), CMake will not find them
# automatically — listing them explicitly is what generates the moc_*.cpp
# files that the linker needs for the vtable symbols.
#
# Without this you get:
#   undefined reference to `vtable for MainWindow`   (and friends)
# ─────────────────────────────────────────────────────────────────────────────

# ── 1. Backend / Model static library ────────────────────────────────────────
add_library(ExpenseTrackerLib STATIC
    # Implementation files — repositories, model, and reusable widgets
    src/expense_repository.cpp
    src/expense_table_model.cpp
    src/pie_chart_widget.cpp
    src/summary_widget.cpp

    # Q_OBJECT headers — must be listed so AUTOMOC runs moc on them
    include/expense_table_model.h
    include/pie_chart_widget.h
    include/summary_widget.h
)

target_include_directories(ExpenseTrackerLib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ExpenseTrackerLib
    PUBLIC
        Qt6::Widgets
)

# ── 2. GUI Executable ─────────────────────────────────────────────────────────
#
# qt_add_executable (not plain add_executable) is required on Windows with
# MinGW/Qt because Qt provides its own WinMain entry point via
# Qt6EntryPoint.  Using add_executable causes an unresolved __imp___argc
# symbol at link time.  qt_add_executable handles this automatically.
qt_add_executable(ExpenseTracker
    # Source files — windows and dialogs
    src/main.cpp
    src/main_window.cpp
    src/add_expense_dialog.cpp

    # Q_OBJECT headers — AUTOMOC needs these listed to generate moc_*.cpp
    include/main_window.h
    include/add_expense_dialog.h

    # .ui files — AUTOUIC generates ui_main_window.h / ui_add_expense_dialog.h
    ${CMAKE_SOURCE_DIR}/shared/ui/main_window.ui
    ${CMAKE_SOURCE_DIR}/shared/ui/add_expense_dialog.ui
    ${CMAKE_SOURCE_DIR}/shared/ui/summary_panel.ui
)

target_link_libraries(ExpenseTracker
    PRIVATE
        ExpenseTrackerLib
)

# ── Platform settings ─────────────────────────────────────────────────────────

# Keep WIN32_EXECUTABLE FALSE so the console window is visible during
# development; flip to TRUE for a pure-GUI release build.
set_target_properties(ExpenseTracker PROPERTIES
    WIN32_EXECUTABLE FALSE
)

if(APPLE)
    set_target_properties(ExpenseTracker PROPERTIES
        MACOSX_BUNDLE TRUE
    )
endif()

# ── Post-build: copy shared resources next to the binary ─────────────────────
add_custom_command(TARGET ExpenseTracker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:ExpenseTracker>/../shared/resources
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/shared/resources/expense_theme.qss
        $<TARGET_FILE_DIR:ExpenseTracker>/../shared/resources/expense_theme.qss
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:ExpenseTracker>/../shared/data
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/shared/data/expenses.json
        $<TARGET_FILE_DIR:ExpenseTracker>/../shared/data/expenses.json
    COMMENT "Copying shared resources next to the binary"
)

# ── Configuration summary ─────────────────────────────────────────────────────
message(STATUS "")
message(STATUS "── Expense Tracker C++ ─────────────────────────────────────────")
message(STATUS "  Build type  : ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Qt version  : ${Qt6_VERSION}")
message(STATUS "  Binary dir  : ${CMAKE_BINARY_DIR}")
message(STATUS "")
message(STATUS "  Targets:")
message(STATUS "    ExpenseTrackerLib  (static library)")
message(STATUS "    ExpenseTracker     (GUI executable)")
message(STATUS "")
message(STATUS "  To build:")
message(STATUS "    cmake --build . --config Release --parallel")
message(STATUS "")
message(STATUS "  To run (Windows):")
message(STATUS "    .\\build\\cpp\\ExpenseTracker.exe")
message(STATUS "────────────────────────────────────────────────────────────────")
message(STATUS "")
