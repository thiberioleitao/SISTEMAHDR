#include "RedeHidrologicaTesteManual.h"

#include "CalculadoraAcumulacaoHidrologica.h"

#include <QStringList>

bool RedeHidrologicaTesteManual::montarRedeExemplo(RedeHidrologica* rede, QString* erro)
{
    if (erro) erro->clear();
    if (!rede) {
        if (erro) *erro = "Instância de rede não informada.";
        return false;
    }

    rede->limpar();

    QString erroLocal;

    // Topologia de canais e dispositivos (planilha DIMENSIONAMENTO - ACO - R0A).
    if (!rede->adicionarElemento("D04.1-ACO", "D04.2-ACO", TipoElementoRede::Canal, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("D04.2-ACO", "D04.3-ACO", TipoElementoRede::Canal, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("D04.3-ACO", "D04.4-ACO", TipoElementoRede::Canal, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("D04.4-ACO", "C04-ACO", TipoElementoRede::Canal, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("C04-ACO", "D06.1-ACO", TipoElementoRede::Bueiro, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    if (!rede->adicionarElemento("D05.1-ACO", "D05.2-ACO", TipoElementoRede::Canal, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("D05.2-ACO", "C03-ACO", TipoElementoRede::Canal, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("C03-ACO", "EX-B1", TipoElementoRede::Bueiro, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    if (!rede->adicionarElemento("D06.1-ACO", "EX-B1", TipoElementoRede::Canal, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    if (!rede->adicionarElemento("D03.1-ACO", "C01-ACO", TipoElementoRede::Canal, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("D02.1-ACO", "C01-ACO", TipoElementoRede::Canal, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("C01-ACO", "EX-B3", TipoElementoRede::Bueiro, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    if (!rede->adicionarElemento("D01.1-ACO", "C02-ACO", TipoElementoRede::Canal, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("C02-ACO", "EX-B4", TipoElementoRede::Bueiro, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    // Exutórios.
    if (!rede->adicionarElemento("EX-B1", QString(), TipoElementoRede::Outro, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("EX-B3", QString(), TipoElementoRede::Outro, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("EX-B4", QString(), TipoElementoRede::Outro, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    // Sub-bacias associadas por ID (A em km², L em km, S adimensional, C_10 adimensional).
    if (!rede->adicionarElemento("BACIA01-SB01", "D04.1-ACO", TipoElementoRede::BaciaContribuicao, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->associarBaciaAoElemento("BACIA01-SB01", BaciaContribuicao("BACIA 01-SB01", 5445.88 / 1e6, 16.00 / 100.0, 74.69 / 1000.0, 0.50), &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    if (!rede->adicionarElemento("BACIA01-SB02", "D04.2-ACO", TipoElementoRede::BaciaContribuicao, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->associarBaciaAoElemento("BACIA01-SB02", BaciaContribuicao("BACIA 01-SB02", 1244.39 / 1e6, 12.18 / 100.0, 53.99 / 1000.0, 0.50), &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    if (!rede->adicionarElemento("BACIA01-SB03", "D04.3-ACO", TipoElementoRede::BaciaContribuicao, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->associarBaciaAoElemento("BACIA01-SB03", BaciaContribuicao("BACIA 01-SB03", 10347.64 / 1e6, 12.18 / 100.0, 140.97 / 1000.0, 0.50), &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    if (!rede->adicionarElemento("BACIA01-SB04", "D04.4-ACO", TipoElementoRede::BaciaContribuicao, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->associarBaciaAoElemento("BACIA01-SB04", BaciaContribuicao("BACIA 01-SB04", 12175.37 / 1e6, 12.18 / 100.0, 134.71 / 1000.0, 0.50), &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    if (!rede->adicionarElemento("BACIA03-SB02", "D02.1-ACO", TipoElementoRede::BaciaContribuicao, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->associarBaciaAoElemento("BACIA03-SB02", BaciaContribuicao("BACIA 03-SB02", 1338.67 / 1e6, 11.78 / 100.0, 38.78 / 1000.0, 0.50), &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    if (!rede->adicionarElemento("BACIA03-SB01", "D03.1-ACO", TipoElementoRede::BaciaContribuicao, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->associarBaciaAoElemento("BACIA03-SB01", BaciaContribuicao("BACIA 03-SB01", 24281.00 / 1e6, 11.20 / 100.0, 244.00 / 1000.0, 0.50), &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    if (!rede->adicionarElemento("BACIA02-SB01", "D01.1-ACO", TipoElementoRede::BaciaContribuicao, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->associarBaciaAoElemento("BACIA02-SB01", BaciaContribuicao("BACIA 02-SB01", 7855.00 / 1e6, 9.80 / 100.0, 168.89 / 1000.0, 0.50), &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    if (!rede->adicionarElemento("BACIA01-SB05", "D05.1-ACO", TipoElementoRede::BaciaContribuicao, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->associarBaciaAoElemento("BACIA01-SB05", BaciaContribuicao("BACIA 01-SB05", 2043.70 / 1e6, 19.20 / 100.0, 54.45 / 1000.0, 0.50), &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    if (!rede->adicionarElemento("BACIA01-SB06", "D05.2-ACO", TipoElementoRede::BaciaContribuicao, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->associarBaciaAoElemento("BACIA01-SB06", BaciaContribuicao("BACIA 01-SB06", 11729.73 / 1e6, 18.00 / 100.0, 117.00 / 1000.0, 0.50), &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    if (!rede->adicionarElemento("BACIA01-SB07", "D06.1-ACO", TipoElementoRede::BaciaContribuicao, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->associarBaciaAoElemento("BACIA01-SB07", BaciaContribuicao("BACIA 01-SB07", 2854.25 / 1e6, 8.00 / 100.0, 59.97 / 1000.0, 0.50), &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    return true;
}

QString RedeHidrologicaTesteManual::gerarRelatorioExemplo(QString* erro)
{
    if (erro) erro->clear();

    RedeHidrologica rede;
    if (!montarRedeExemplo(&rede, erro)) {
        return QString();
    }

    QString erroCalc;
    QMap<QString, double> comprimentosTotais;
    const double lTotalExB1 = CalculadoraAcumulacaoHidrologica::calcularComprimentoTotalTalvegueAteElemento(
        rede,
        "EX-B1",
        QMap<QString, double>(),
        &erroCalc,
        &comprimentosTotais);

    if (!erroCalc.isEmpty()) {
        if (erro) *erro = erroCalc;
        return QString();
    }

    QStringList linhas;
    linhas << "=== Rede de teste manual ===";
    linhas << "Elemento destino: EX-B1";
    linhas << QString("L_talvegue_total(EX-B1): %1 km").arg(lTotalExB1, 0, 'f', 3);
    linhas << "";
    linhas << "Comprimento total por elemento:";

    for (auto it = comprimentosTotais.cbegin(); it != comprimentosTotais.cend(); ++it) {
        linhas << QString("- %1: %2 km").arg(it.key()).arg(it.value(), 0, 'f', 3);
    }

    return linhas.join('\n');
}
