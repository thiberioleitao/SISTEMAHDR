#include <QApplication>
#include <QFont>

#include "ui/MainWindow.h"

/**
 * @brief Ponto de entrada da aplicação desktop baseada em Qt Widgets.
 * @param argc Quantidade de argumentos repassados pela linha de comando.
 * @param argv Vetor com os argumentos recebidos pela aplicação.
 * @return Código de encerramento retornado pelo loop principal do Qt.
 */
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("SISTEMAHDR");
    app.setOrganizationName("SISTEMAHDR");

    // Define uma tipografia consistente para toda a interface.
    QFont fonteAplicacao("Segoe UI", 10);
    fonteAplicacao.setStyleStrategy(QFont::PreferAntialias);
    app.setFont(fonteAplicacao);

    MainWindow janelaPrincipal;
    janelaPrincipal.show();

    return app.exec();
}
