#include <QApplication>

#include "ui/MainWindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    MainWindow janelaPrincipal;
    janelaPrincipal.show();

    return app.exec();
}
