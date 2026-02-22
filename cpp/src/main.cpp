#include "main_window.h"

#include <QApplication>
#include <QFile>
#include <QDir>

/*
 * main — program entry point, identical pattern to Python's:
 *     if __name__ == "__main__":
 *         app = QApplication()
 *         window = MainWindow()
 *         window.show()
 *         app.exec()
 *
 * Memory note:
 *   MainWindow window  — created ON THE STACK (no 'new').
 *   When main() returns, the stack frame is unwound and the destructor is
 *   called automatically.  This is stack-based RAII — no new, no delete.
 */
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("ExpenseTracker");
    app.setOrganizationName("ExpenseTracker");

    // Load stylesheet
    QFile qss(QCoreApplication::applicationDirPath() + "/../shared/resources/expense_theme.qss");
    if (qss.open(QIODevice::ReadOnly | QIODevice::Text))
        app.setStyleSheet(qss.readAll());

    // Ensure the data folder exists
    QDir dir(QCoreApplication::applicationDirPath() + "/../shared/data");
    if (!dir.exists()) dir.mkpath(".");

    MainWindow window;   // stack allocation — destructor called automatically on exit
    window.show();
    return app.exec();
}
