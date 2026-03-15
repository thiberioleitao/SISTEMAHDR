#include "RedeHidrologicaTesteManual.h"

#include "CalculadoraAcumulacaoHidrologica.h"
#include "Canal.h"
#include "MetodoRacionalTransformacao.h"

#include <memory>
#include <QStringList>

bool RedeHidrologicaTesteManual::criarElementosExemplo(RedeHidrologica* rede, QString* erro)
{
    if (erro) erro->clear();
    if (!rede) {
        if (erro) *erro = "Instância de rede não informada.";
        return false;
    }

    QString erroLocal;

    // Canais/bueiros/exutórios sem conexão inicial (idJusante vazio).
    if (!rede->adicionarElemento("D04.1-ACO", QString(), TipoElementoRede::Canal, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("D04.2-ACO", QString(), TipoElementoRede::Canal, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("D04.3-ACO", QString(), TipoElementoRede::Canal, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("D04.4-ACO", QString(), TipoElementoRede::Canal, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("C04-ACO", QString(), TipoElementoRede::Bueiro, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    if (!rede->adicionarElemento("D05.1-ACO", QString(), TipoElementoRede::Canal, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("D05.2-ACO", QString(), TipoElementoRede::Canal, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("C03-ACO", QString(), TipoElementoRede::Bueiro, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    if (!rede->adicionarElemento("D06.1-ACO", QString(), TipoElementoRede::Canal, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    if (!rede->adicionarElemento("D03.1-ACO", QString(), TipoElementoRede::Canal, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("D02.1-ACO", QString(), TipoElementoRede::Canal, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("C01-ACO", QString(), TipoElementoRede::Bueiro, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    if (!rede->adicionarElemento("D01.1-ACO", QString(), TipoElementoRede::Canal, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("C02-ACO", QString(), TipoElementoRede::Bueiro, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    if (!rede->adicionarElemento("EX-B1", QString(), TipoElementoRede::Exutorio, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("EX-B3", QString(), TipoElementoRede::Exutorio, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("EX-B4", QString(), TipoElementoRede::Exutorio, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    // Sub-bacias também nascem sem conexão; conexão ocorre na etapa posterior.
    if (!rede->adicionarElemento("BACIA01-SB01", QString(), TipoElementoRede::BaciaContribuicao, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("BACIA01-SB02", QString(), TipoElementoRede::BaciaContribuicao, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("BACIA01-SB03", QString(), TipoElementoRede::BaciaContribuicao, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("BACIA01-SB04", QString(), TipoElementoRede::BaciaContribuicao, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("BACIA03-SB02", QString(), TipoElementoRede::BaciaContribuicao, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("BACIA03-SB01", QString(), TipoElementoRede::BaciaContribuicao, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("BACIA02-SB01", QString(), TipoElementoRede::BaciaContribuicao, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("BACIA01-SB05", QString(), TipoElementoRede::BaciaContribuicao, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("BACIA01-SB06", QString(), TipoElementoRede::BaciaContribuicao, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->adicionarElemento("BACIA01-SB07", QString(), TipoElementoRede::BaciaContribuicao, &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    return true;
}

bool RedeHidrologicaTesteManual::conectarElementosExemplo(RedeHidrologica* rede, QString* erro)
{
    if (erro) erro->clear();
    if (!rede) {
        if (erro) *erro = "Instância de rede não informada.";
        return false;
    }

    QString erroLocal;

    // Conexão de canais/dispositivos.
    if (!rede->definirJusanteElemento("D04.1-ACO", "D04.2-ACO", &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->definirJusanteElemento("D04.2-ACO", "D04.3-ACO", &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->definirJusanteElemento("D04.3-ACO", "D04.4-ACO", &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->definirJusanteElemento("D04.4-ACO", "C04-ACO", &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->definirJusanteElemento("C04-ACO", "D06.1-ACO", &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    if (!rede->definirJusanteElemento("D05.1-ACO", "D05.2-ACO", &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->definirJusanteElemento("D05.2-ACO", "C03-ACO", &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->definirJusanteElemento("C03-ACO", "EX-B1", &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    if (!rede->definirJusanteElemento("D06.1-ACO", "EX-B1", &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    if (!rede->definirJusanteElemento("D03.1-ACO", "C01-ACO", &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->definirJusanteElemento("D02.1-ACO", "C01-ACO", &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->definirJusanteElemento("C01-ACO", "EX-B3", &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    if (!rede->definirJusanteElemento("D01.1-ACO", "C02-ACO", &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->definirJusanteElemento("C02-ACO", "EX-B4", &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    // Conexão das sub-bacias.
    if (!rede->definirJusanteElemento("BACIA01-SB01", "D04.1-ACO", &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->definirJusanteElemento("BACIA01-SB02", "D04.2-ACO", &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->definirJusanteElemento("BACIA01-SB03", "D04.3-ACO", &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->definirJusanteElemento("BACIA01-SB04", "D04.4-ACO", &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->definirJusanteElemento("BACIA03-SB02", "D02.1-ACO", &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->definirJusanteElemento("BACIA03-SB01", "D03.1-ACO", &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->definirJusanteElemento("BACIA02-SB01", "D01.1-ACO", &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->definirJusanteElemento("BACIA01-SB05", "D05.1-ACO", &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->definirJusanteElemento("BACIA01-SB06", "D05.2-ACO", &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);
    if (!rede->definirJusanteElemento("BACIA01-SB07", "D06.1-ACO", &erroLocal)) return (erro ? (*erro = erroLocal, false) : false);

    return true;
}

bool RedeHidrologicaTesteManual::montarRedeExemplo(RedeHidrologica* rede, QString* erro)
{
    if (erro) erro->clear();
    if (!rede) {
        if (erro) *erro = "Instância de rede não informada.";
        return false;
    }

    rede->limpar();

    if (!criarElementosExemplo(rede, erro)) {
        return false;
    }

    if (!conectarElementosExemplo(rede, erro)) {
        return false;
    }

    QString erroLocal;

    // Associação dos dados das sub-bacias após criação e conexão topológica.
    const std::shared_ptr<MetodoRacionalTransformacao> modeloRacional = std::make_shared<MetodoRacionalTransformacao>();

    auto associarBacia = [&](const QString& id,
                             const QString& nome,
                             double areaKm2,
                             double declividade,
                             double comprimentoKm,
                             double c10) {
        BaciaContribuicao bacia(nome, areaKm2, declividade, comprimentoKm, c10);
        bacia.setModeloTransformacao(modeloRacional);
        return rede->associarBaciaAoElemento(id, bacia, &erroLocal);
    };

    if (!associarBacia("BACIA01-SB01", "BACIA 01-SB01", 5445.88 / 1e6, 16.00 / 100.0, 74.69 / 1000.0, 0.50)) return (erro ? (*erro = erroLocal, false) : false);
    if (!associarBacia("BACIA01-SB02", "BACIA 01-SB02", 1244.39 / 1e6, 12.18 / 100.0, 53.99 / 1000.0, 0.50)) return (erro ? (*erro = erroLocal, false) : false);
    if (!associarBacia("BACIA01-SB03", "BACIA 01-SB03", 10347.64 / 1e6, 12.18 / 100.0, 140.97 / 1000.0, 0.50)) return (erro ? (*erro = erroLocal, false) : false);
    if (!associarBacia("BACIA01-SB04", "BACIA 01-SB04", 12175.37 / 1e6, 12.18 / 100.0, 134.71 / 1000.0, 0.50)) return (erro ? (*erro = erroLocal, false) : false);
    if (!associarBacia("BACIA03-SB02", "BACIA 03-SB02", 1338.67 / 1e6, 11.78 / 100.0, 38.78 / 1000.0, 0.50)) return (erro ? (*erro = erroLocal, false) : false);
    if (!associarBacia("BACIA03-SB01", "BACIA 03-SB01", 24281.00 / 1e6, 11.20 / 100.0, 244.00 / 1000.0, 0.50)) return (erro ? (*erro = erroLocal, false) : false);
    if (!associarBacia("BACIA02-SB01", "BACIA 02-SB01", 7855.00 / 1e6, 9.80 / 100.0, 168.89 / 1000.0, 0.50)) return (erro ? (*erro = erroLocal, false) : false);
    if (!associarBacia("BACIA01-SB05", "BACIA 01-SB05", 2043.70 / 1e6, 19.20 / 100.0, 54.45 / 1000.0, 0.50)) return (erro ? (*erro = erroLocal, false) : false);
    if (!associarBacia("BACIA01-SB06", "BACIA 01-SB06", 11729.73 / 1e6, 18.00 / 100.0, 117.00 / 1000.0, 0.50)) return (erro ? (*erro = erroLocal, false) : false);
    if (!associarBacia("BACIA01-SB07", "BACIA 01-SB07", 2854.25 / 1e6, 8.00 / 100.0, 59.97 / 1000.0, 0.50)) return (erro ? (*erro = erroLocal, false) : false);

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

    const double intensidadeChuvaBrutaMmH = 120.0;

    // Vazão local das bacias com base no modelo associado.
    QMap<QString, double> vazaoLocalPorElemento;
    for (const ElementoRedeHidrologica& e : rede.elementos()) {
        if (e.tipo != TipoElementoRede::BaciaContribuicao) continue;

        if (const BaciaContribuicao* b = rede.baciaPorId(e.id)) {
            vazaoLocalPorElemento.insert(e.id, b->calcularVazaoProjetoMetodoRacional(intensidadeChuvaBrutaMmH));
        }
    }

    // Vazão acumulada na rede (inclui canais e demais elementos topológicos).
    QString erroVazao;
    const QMap<QString, double> vazaoAcumulada = CalculadoraAcumulacaoHidrologica::calcularVazaoAcumuladaPorElemento(
        rede,
        vazaoLocalPorElemento,
        &erroVazao);

    if (!erroVazao.isEmpty()) {
        if (erro) *erro = erroVazao;
        return QString();
    }

    // Objetos Canal criados no teste (por ID), para validar propriedades dependentes da rede.
    QMap<QString, Canal> canaisPorId;
    for (const ElementoRedeHidrologica& e : rede.elementos()) {
        if (e.tipo == TipoElementoRede::Canal) {
            canaisPorId.insert(e.id, Canal(e.id));
        }
    }

    QStringList linhas;
    linhas << "=== Rede de teste manual ===";
    linhas << "Elemento destino: EX-B1";
    linhas << QString("L_talvegue_total(EX-B1): %1 km").arg(lTotalExB1, 0, 'f', 3);
    linhas << QString("Intensidade de teste: %1 mm/h").arg(intensidadeChuvaBrutaMmH, 0, 'f', 1);
    linhas << "";
    linhas << "Comprimento total por elemento:";

    for (auto it = comprimentosTotais.cbegin(); it != comprimentosTotais.cend(); ++it) {
        linhas << QString("- %1: %2 km").arg(it.key()).arg(it.value(), 0, 'f', 3);
    }

    linhas << "";
    linhas << "Vazão local das bacias (m3/s):";
    for (auto it = vazaoLocalPorElemento.cbegin(); it != vazaoLocalPorElemento.cend(); ++it) {
        linhas << QString("- %1: %2").arg(it.key()).arg(it.value(), 0, 'f', 3);
    }

    linhas << "";
    linhas << "Vazão acumulada nos canais (m3/s):";
    for (auto it = canaisPorId.cbegin(); it != canaisPorId.cend(); ++it) {
        linhas << QString("- %1: %2").arg(it.key()).arg(vazaoAcumulada.value(it.key(), 0.0), 0, 'f', 3);
    }

    return linhas.join('\n');
}
