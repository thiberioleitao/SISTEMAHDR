#pragma once

#include <QMainWindow>

class CanalTrapezoidalWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    CanalTrapezoidalWidget* m_widgetCanalTrapezoidal = nullptr;

    void atualizarDesenho();
};