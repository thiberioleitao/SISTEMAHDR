#include "MainWindow.h"

#include "CanalTrapezoidalWidget.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("SISTEMAHDR - Canal Trapezoidal");

    m_widgetCanalTrapezoidal = new CanalTrapezoidalWidget(this);
    setCentralWidget(m_widgetCanalTrapezoidal);
    resize(980, 520);
}
