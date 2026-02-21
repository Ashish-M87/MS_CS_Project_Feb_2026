from __future__ import annotations
import sys
from pathlib import Path
from PySide6.QtWidgets import QApplication
from main_window import MainWindow



def main() -> int:
    app = QApplication(sys.argv)
    qss_path = Path(__file__).resolve().parent.parent / "shared" / "resources" / "expense_theme.qss"
    if qss_path.exists():
        app.setStyleSheet(qss_path.read_text(encoding="utf-8"))

    window = MainWindow()
    window.resize(1400, 820)
    window.show()
    return app.exec()

if __name__ == "__main__":
    raise SystemExit(main())