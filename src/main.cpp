#include <QApplication>
#include <QDebug>

#include "domain/RedeHidrologicaTesteManual.h"
#include "ui/MainWindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Teste manual da rede hidrológica (domínio), exibido no Output.
    const QString relatorioTeste = RedeHidrologicaTesteManual::gerarRelatorioExemplo();
    if (!relatorioTeste.isEmpty()) {
        qDebug().noquote() << relatorioTeste;
    }

    //MainWindow janelaPrincipal;
    //janelaPrincipal.show();

    return app.exec();
}
