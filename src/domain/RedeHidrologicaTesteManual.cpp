#include "RedeHidrologicaTesteManual.h"

#include "Canal.h"
#include "Bueiro.h"
#include "IDF.h"

#include <algorithm>
#include <cmath>
#include <QStringList>

bool RedeHidrologicaTesteManual::criarElementosExemplo(RedeHidrologica* rede)
{
    if (!rede) return false;

    // 1) Criar bacias somente com ID informado pelo usuário.
    auto criarBaciaSomenteComNome = [&](const QString& id) {
        const QString idNormalizado = id.trimmed();
        if (idNormalizado.isEmpty()) return false;
        if (rede->existeId(idNormalizado)) return false;

        BaciaContribuicao bacia(idNormalizado);
        return rede->adicionarBacia(bacia);
    };

    // 2) Alterar propriedades das bacias após o cadastro.
    auto definirPropriedadesBacia = [&](const QString& id,
                                        double areaKm2,
                                        double declividade,
                                        double comprimentoKm,
                                        double c10) {
        BaciaContribuicao* bacia = rede->baciaPorId(id);
        if (!bacia) return false;

        bacia->setAreaKm2(areaKm2);
        bacia->setDeclividadeMedia(declividade);
        bacia->setComprimentoTalveguePrincipalKm(comprimentoKm);
        bacia->setC_10(c10);
        return true;
    };

    // 3) Criar canais somente com ID informado pelo usuário.
    auto criarCanalSomenteComNome = [&](const QString& id) {
        const QString idNormalizado = id.trimmed();
        if (idNormalizado.isEmpty()) return false;
        if (rede->existeId(idNormalizado)) return false;
        return rede->adicionarCanal(Canal(idNormalizado));
    };

    // 4) Alterar propriedades dos canais após o cadastro.
    auto definirPropriedadesCanal = [&](const QString& id,
                                        double comprimentoM,
                                        double declividadeMinPercentual,
                                        double declividadeMaxPercentual,
                                        const QString& idSecao) {
        Canal* canal = rede->canalPorId(id);
        if (!canal) return false;

        canal->setComprimento(comprimentoM);
        canal->setDeclividadeMinima(declividadeMinPercentual / 100.0);
        canal->setDeclividadeMaxima(declividadeMaxPercentual / 100.0);
        canal->setDeclividadeFinal(declividadeMaxPercentual / 100.0);

        const QString secao = idSecao.trimmed().toUpper();

        if (secao == "VTD-1") {
            canal->setSecaoTransversal(SecaoTransversalTrapezoidal(0.30, 1.00));
            canal->setCoeficienteManning(0.025);
            return true;
        }
        if (secao == "VTD-2") {
            canal->setSecaoTransversal(SecaoTransversalTrapezoidal(0.50, 1.00));
            canal->setCoeficienteManning(0.025);
            return true;
        }
        if (secao == "CTD-1") {
            canal->setSecaoTransversal(SecaoTransversalTrapezoidal(0.50, 1.00));
            canal->setCoeficienteManning(0.025);
            return true;
        }
        if (secao == "CSD-1") {
            canal->setSecaoSemicircular(SecaoTransversalSemicircular(0.40));
            canal->setCoeficienteManning(0.015);
            return true;
        }
        if (secao == "CSD-2") {
            canal->setSecaoSemicircular(SecaoTransversalSemicircular(0.50));
            canal->setCoeficienteManning(0.015);
            return true;
        }
        if (secao == "CSD-3") {
            canal->setSecaoSemicircular(SecaoTransversalSemicircular(0.60));
            canal->setCoeficienteManning(0.015);
            return true;
        }
        if (secao == "CSD-4") {
            canal->setSecaoSemicircular(SecaoTransversalSemicircular(0.80));
            canal->setCoeficienteManning(0.015);
            return true;
        }
        if (secao == "CSD-5") {
            canal->setSecaoSemicircular(SecaoTransversalSemicircular(1.00));
            canal->setCoeficienteManning(0.015);
            return true;
        }

        return false;
    };

    auto adicionarBueiro = [&](const QString& id) {
        return rede->adicionarBueiro(Bueiro(id));
    };

    // 1) Criar bacias com base no nome.
    if (!criarBaciaSomenteComNome("BACIA01-SB01")) return false;
    if (!criarBaciaSomenteComNome("BACIA01-SB02")) return false;
    if (!criarBaciaSomenteComNome("BACIA01-SB03")) return false;
    if (!criarBaciaSomenteComNome("BACIA01-SB04")) return false;
    if (!criarBaciaSomenteComNome("BACIA03-SB02")) return false;
    if (!criarBaciaSomenteComNome("BACIA03-SB01")) return false;
    if (!criarBaciaSomenteComNome("BACIA02-SB01")) return false;
    if (!criarBaciaSomenteComNome("BACIA01-SB05")) return false;
    if (!criarBaciaSomenteComNome("BACIA01-SB06")) return false;
    if (!criarBaciaSomenteComNome("BACIA01-SB07")) return false;

    // 2) Definir propriedades das bacias após criar os elementos.
    if (!definirPropriedadesBacia("BACIA01-SB01", 5445.88 / 1e6, 16.00 / 100.0, 74.69 / 1000.0, 0.6)) return false;
    if (!definirPropriedadesBacia("BACIA01-SB02", 1244.39 / 1e6, 12.18 / 100.0, 53.99 / 1000.0, 0.6)) return false;
    if (!definirPropriedadesBacia("BACIA01-SB03", 10347.64 / 1e6, 12.18 / 100.0, 140.97 / 1000.0, 0.6)) return false;
    if (!definirPropriedadesBacia("BACIA01-SB04", 12175.37 / 1e6, 12.18 / 100.0, 134.71 / 1000.0, 0.6)) return false;
    if (!definirPropriedadesBacia("BACIA03-SB02", 1338.67 / 1e6, 11.78 / 100.0, 38.78 / 1000.0, 0.6)) return false;
    if (!definirPropriedadesBacia("BACIA03-SB01", 24281.00 / 1e6, 11.20 / 100.0, 244.00 / 1000.0, 0.6)) return false;
    if (!definirPropriedadesBacia("BACIA02-SB01", 7855.00 / 1e6, 9.80 / 100.0, 168.89 / 1000.0, 0.6)) return false;
    if (!definirPropriedadesBacia("BACIA01-SB05", 2043.70 / 1e6, 19.20 / 100.0, 54.45 / 1000.0, 0.6)) return false;
    if (!definirPropriedadesBacia("BACIA01-SB06", 11729.73 / 1e6, 18.00 / 100.0, 117.00 / 1000.0, 0.6)) return false;
    if (!definirPropriedadesBacia("BACIA01-SB07", 2854.25 / 1e6, 8.00 / 100.0, 59.97 / 1000.0, 0.6)) return false;

    // 3) Criar canais com base no nome.
    if (!criarCanalSomenteComNome("D04.1-ACO")) return false;
    if (!criarCanalSomenteComNome("D04.2-ACO")) return false;
    if (!criarCanalSomenteComNome("D04.3-ACO")) return false;
    if (!criarCanalSomenteComNome("D04.4-ACO")) return false;
    if (!criarCanalSomenteComNome("D05.1-ACO")) return false;
    if (!criarCanalSomenteComNome("D05.2-ACO")) return false;
    if (!criarCanalSomenteComNome("D06.1-ACO")) return false;
    if (!criarCanalSomenteComNome("D03.1-ACO")) return false;
    if (!criarCanalSomenteComNome("D02.1-ACO")) return false;
    if (!criarCanalSomenteComNome("D01.1-ACO")) return false;

    // 4) Definir propriedades dos canais após criar os elementos.
    if (!definirPropriedadesCanal("D04.1-ACO", 110.00, 2.85, 9.10, "CSD-1")) return false;
    if (!definirPropriedadesCanal("D04.2-ACO", 17.00, 9.10, 11.60, "CSD-1")) return false;
    if (!definirPropriedadesCanal("D04.3-ACO", 89.32, 3.50, 11.60, "CSD-3")) return false;
    if (!definirPropriedadesCanal("D04.4-ACO", 64.00, 4.00, 8.50, "CSD-4")) return false;
    if (!definirPropriedadesCanal("D05.1-ACO", 47.00, 3.00, 13.50, "CSD-1")) return false;
    if (!definirPropriedadesCanal("D05.2-ACO", 86.00, 9.60, 15.80, "CSD-2")) return false;
    if (!definirPropriedadesCanal("D06.1-ACO", 115.00, 5.40, 14.50, "CSD-4")) return false;
    if (!definirPropriedadesCanal("D03.1-ACO", 50.00, 3.00, 6.50, "CSD-4")) return false;
    if (!definirPropriedadesCanal("D02.1-ACO", 30.00, 3.50, 5.00, "VTD-1")) return false;
    if (!definirPropriedadesCanal("D01.1-ACO", 40.00, 1.00, 4.90, "CSD-3")) return false;

    if (!adicionarBueiro("C04-ACO")) return false;
    if (!adicionarBueiro("C03-ACO")) return false;
    if (!adicionarBueiro("C01-ACO")) return false;
    if (!adicionarBueiro("C02-ACO")) return false;

    if (!rede->adicionarExutorio("EX-B1")) return false;
    if (!rede->adicionarExutorio("EX-B3")) return false;
    if (!rede->adicionarExutorio("EX-B4")) return false;

    return true;
}

bool RedeHidrologicaTesteManual::conectarElementosExemplo(RedeHidrologica* rede)
{
    if (!rede) return false;

    auto conectar = [&](const QString& idMontante, const QString& idJusante) {
        return rede->definirJusanteElemento(idMontante, idJusante);
    };

    // Conexão de canais/dispositivos.
    if (!conectar("D04.1-ACO", "D04.2-ACO")) return false;
    if (!conectar("D04.2-ACO", "D04.3-ACO")) return false;
    if (!conectar("D04.3-ACO", "D04.4-ACO")) return false;
    if (!conectar("D04.4-ACO", "C04-ACO")) return false;
    if (!conectar("C04-ACO", "D06.1-ACO")) return false;

    if (!conectar("D05.1-ACO", "D05.2-ACO")) return false;
    if (!conectar("D05.2-ACO", "C03-ACO")) return false;
    if (!conectar("C03-ACO", "EX-B1")) return false;

    if (!conectar("D06.1-ACO", "EX-B1")) return false;

    if (!conectar("D03.1-ACO", "C01-ACO")) return false;
    if (!conectar("D02.1-ACO", "C01-ACO")) return false;
    if (!conectar("C01-ACO", "EX-B3")) return false;

    if (!conectar("D01.1-ACO", "C02-ACO")) return false;
    if (!conectar("C02-ACO", "EX-B4")) return false;

    // Conexão das sub-bacias.
    if (!conectar("BACIA01-SB01", "D04.1-ACO")) return false;
    if (!conectar("BACIA01-SB02", "D04.2-ACO")) return false;
    if (!conectar("BACIA01-SB03", "D04.3-ACO")) return false;
    if (!conectar("BACIA01-SB04", "D04.4-ACO")) return false;
    if (!conectar("BACIA03-SB02", "D02.1-ACO")) return false;
    if (!conectar("BACIA03-SB01", "D03.1-ACO")) return false;
    if (!conectar("BACIA02-SB01", "D01.1-ACO")) return false;
    if (!conectar("BACIA01-SB05", "D05.1-ACO")) return false;
    if (!conectar("BACIA01-SB06", "D05.2-ACO")) return false;
    if (!conectar("BACIA01-SB07", "D06.1-ACO")) return false;

    return true;
}

bool RedeHidrologicaTesteManual::montarRedeExemplo(RedeHidrologica* rede)
{
    if (!rede) return false;

    rede->limpar();

    if (!criarElementosExemplo(rede)) {
        return false;
    }

    if (!conectarElementosExemplo(rede)) {
        return false;
    }

    return true;
}

QString RedeHidrologicaTesteManual::gerarRelatorioExemplo()
{
    RedeHidrologica rede;
    if (!montarRedeExemplo(&rede)) {
        return QString();
    }

    const double lTotalExB1 = rede.calcularComprimentoTotalTalvegueAteElemento("EX-B1");

    // Parâmetros IDF para cálculo hidrológico.
    const IDF idf(1200.0, 0.20, 0.80, 10.0);
    const double tempoRetornoAnos = 50.0;

    auto idsBaciasContribuintes = [&](const QString& idElemento) {
        QVector<QString> ids = rede.idsMontanteDoElemento(idElemento);
        if (rede.tipoDoElemento(idElemento) == TipoElementoRede::BaciaContribuicao) {
            ids.append(idElemento);
        }
        return ids;
    };

    auto calcularTcTotalMin = [&](const QString& idElemento) {
        double tcTotalMin = 0.0;
        const QVector<QString> idsContribuintes = idsBaciasContribuintes(idElemento);

        for (const QString& id : idsContribuintes) {
            if (rede.tipoDoElemento(id) != TipoElementoRede::BaciaContribuicao) continue;

            tcTotalMin = std::max(tcTotalMin, rede.tempoConcentracaoKirpichModificadoAreaTotal(id));
        }

        return tcTotalMin;
    };

    auto formatarValor = [&](double valor, int casasDecimais) {
        if (!std::isfinite(valor)) return QString("n/d");
        return QString::number(valor, 'f', casasDecimais);
    };

    // 5) Calcular parâmetros acumulados (Tc, áreas e comprimentos) via rede.
    QMap<QString, double> vazaoBaciasTR50M3s;
    for (const QString& id : rede.idsElementos()) {
        if (rede.tipoDoElemento(id) != TipoElementoRede::BaciaContribuicao) continue;

        const BaciaContribuicao* bacia = rede.baciaPorId(id);
        if (!bacia) continue;

        const double tcBaciaMin = rede.tempoConcentracaoKirpichModificadoAreaTotal(id);
        const double intensidadeBaciaMmH = idf.intensidadeMmH(tempoRetornoAnos, tcBaciaMin);
        const double c = std::max(0.0, bacia->C_10());
        const double a = std::max(0.0, bacia->areaKm2());
        const double qBaciaM3s = std::max(0.0, 0.278 * c * intensidadeBaciaMmH * a);

        vazaoBaciasTR50M3s.insert(id, qBaciaM3s);
    }

    // 6) Calcular vazões acumuladas pelo método da soma das áreas.
    const QMap<QString, double> vazaoAcumuladaTR50M3s = rede.calcularVazaoAcumuladaPorElemento(vazaoBaciasTR50M3s);

    QStringList linhas;
    linhas << "=== Rede de teste manual ===";
    linhas << QString("L_talvegue_total(EX-B1): %1 km").arg(lTotalExB1, 0, 'f', 3);
    linhas << QString("TR de projeto: %1 anos").arg(tempoRetornoAnos, 0, 'f', 1);

    linhas << "";
    linhas << "Bacias - vazão de projeto (TR=50 anos):";
    for (const QString& id : rede.idsElementos()) {
        if (rede.tipoDoElemento(id) != TipoElementoRede::BaciaContribuicao) continue;

        const BaciaContribuicao* bacia = rede.baciaPorId(id);
        if (!bacia) continue;

        const double tcBaciaMin = rede.tempoConcentracaoKirpichModificadoAreaTotal(id);
        const double intensidadeBaciaMmH = idf.intensidadeMmH(tempoRetornoAnos, tcBaciaMin);
        const double qBaciaM3s = vazaoBaciasTR50M3s.value(id, 0.0);

        linhas << QString("- %1: Tc=%2 min | i=%3 mm/h | Q=%4 m3/s")
                     .arg(id)
                     .arg(tcBaciaMin, 0, 'f', 2)
                     .arg(intensidadeBaciaMmH, 0, 'f', 2)
                     .arg(qBaciaM3s, 0, 'f', 3);
    }

    linhas << "";
    linhas << "Bacias - colunas solicitadas:";
    for (const QString& id : rede.idsElementos()) {
        if (rede.tipoDoElemento(id) != TipoElementoRede::BaciaContribuicao) continue;

        const BaciaContribuicao* bacia = rede.baciaPorId(id);
        if (!bacia) continue;

        const double lTalvegueTotalM = rede.calcularComprimentoTotalTalvegueAteElemento(id) * 1000.0;
        const double areaTotalM2 = std::max(0.0, bacia->areaKm2()) * 1e6;
        const double declividadePercentual = std::max(0.0, bacia->declividadeMedia()) * 100.0;
        const double cRunoff = std::max(0.0, bacia->C_10());

        linhas << QString("- %1: A_total=%2 m2 | L_talvegue_total=%3 m | S=%4 %% | C(runoff)=%5")
                     .arg(id)
                     .arg(areaTotalM2, 0, 'f', 2)
                     .arg(lTalvegueTotalM, 0, 'f', 2)
                     .arg(declividadePercentual, 0, 'f', 2)
                     .arg(cRunoff, 0, 'f', 3);
    }

    linhas << "";
    linhas << "Canais - vazão acumulada e lâmina (TR=50 anos):";
    for (const QString& id : rede.idsElementos()) {
        if (rede.tipoDoElemento(id) != TipoElementoRede::Canal) continue;

        const Canal* canal = rede.canalPorId(id);
        if (!canal) continue;

        const double tcCanalMin = calcularTcTotalMin(id);
        const double intensidadeCanalMmH = idf.intensidadeMmH(tempoRetornoAnos, tcCanalMin);
        const double qCanalM3s = std::max(0.0, vazaoAcumuladaTR50M3s.value(id, 0.0));
        const double cMedio = rede.coeficienteEscoamentoMedioPonderado(id);
        const double areaTotalKm2 = rede.areaAcumuladaTotalContribuinte(id);
        const double lTotalTalvegueKm = rede.calcularComprimentoTotalTalvegueAteElemento(id);

        const double laminaProjetoM = canal->alturaLaminaParaVazaoProjeto(qCanalM3s);

        Canal canalSMin = *canal;
        canalSMin.setDeclividadeFinal(canal->declividadeMinima());
        const double laminaSMinM = canalSMin.alturaLaminaParaVazaoProjeto(qCanalM3s);

        Canal canalSMax = *canal;
        canalSMax.setDeclividadeFinal(canal->declividadeMaxima());
        const double laminaSMaxM = canalSMax.alturaLaminaParaVazaoProjeto(qCanalM3s);

        linhas << QString("- %1: Tc=%2 min | i=%3 mm/h | Q=%4 m3/s | Cmed=%5 | A_total=%6 km2 | Ltotal=%7 km | h_proj=%8 m | h_smin=%9 m | h_smax=%10 m")
                     .arg(id)
                     .arg(tcCanalMin, 0, 'f', 2)
                     .arg(intensidadeCanalMmH, 0, 'f', 2)
                     .arg(qCanalM3s, 0, 'f', 3)
                     .arg(cMedio, 0, 'f', 3)
                     .arg(areaTotalKm2, 0, 'f', 3)
                     .arg(lTotalTalvegueKm, 0, 'f', 3)
                     .arg(formatarValor(laminaProjetoM, 3))
                     .arg(formatarValor(laminaSMinM, 3))
                     .arg(formatarValor(laminaSMaxM, 3));
    }

    linhas << "";
    linhas << "Canais - colunas solicitadas:";
    for (const QString& id : rede.idsElementos()) {
        if (rede.tipoDoElemento(id) != TipoElementoRede::Canal) continue;

        const Canal* canal = rede.canalPorId(id);
        if (!canal) continue;

        const QString idJusante = rede.idJusanteDoElemento(id);
        const QVector<QString> idsMontante = rede.idsMontanteDoElemento(id);
        const QString idsMontanteTexto = idsMontante.isEmpty() ? QString("-") : idsMontante.join(", ");
        const double areaTotalM2 = std::max(0.0, rede.areaAcumuladaTotalContribuinte(id)) * 1e6;

        linhas << QString("- %1: ID-JUSANTE=%2 | IDS-MONTANTE=%3 | A_total=%4 m2")
                     .arg(id)
                     .arg(idJusante.isEmpty() ? QString("-") : idJusante)
                     .arg(idsMontanteTexto)
                     .arg(areaTotalM2, 0, 'f', 2);
    }

    return linhas.join('\n');
}
