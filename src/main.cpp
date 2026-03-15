#include <QApplication>
#include <QDebug>

#include "domain/RedeHidrologicaTesteManual.h"
#include "ui/MainWindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Teste manual da rede hidrológica (domínio), exibido no Output.
    QString erroTeste;
    const QString relatorioTeste = RedeHidrologicaTesteManual::gerarRelatorioExemplo(&erroTeste);
    if (!erroTeste.isEmpty()) {
        qDebug().noquote() << "[TesteManual][ERRO]" << erroTeste;
    }
    else {
        qDebug().noquote() << relatorioTeste;
    }

    //MainWindow janelaPrincipal;
    //janelaPrincipal.show();

    return app.exec();
}
