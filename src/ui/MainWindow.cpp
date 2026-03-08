#include "MainWindow.h"

#include <QLabel>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("SISTEMAHDR - Hello Qt");

    auto* lbl = new QLabel("Hello World - Qt + CMake + Visual Studio", this);
    lbl->setAlignment(Qt::AlignCenter);

    setCentralWidget(lbl);
    resize(500, 300);
}
