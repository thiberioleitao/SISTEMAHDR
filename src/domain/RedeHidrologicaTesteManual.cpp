#include "RedeHidrologicaTesteManual.h"

#include "Canal.h"
#include "Bueiro.h"
#include "IDF.h"

#include <algorithm>
#include <cmath>
#include <QQueue>
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
    const IDF idf(279.63, 0.00, 0.524, 0.00);
    const double tempoRetornoAnos = 50.0;

    auto idsBaciasContribuintes = [&](const QString& idElemento) {
        QVector<QString> ids = rede.idsMontanteDoElemento(idElemento);
        if (rede.tipoDoElemento(idElemento) == TipoElementoRede::BaciaContribuicao) {
            ids.append(idElemento);
        }
        return ids;
    };

    struct DetalhesTcAcumulado
    {
        QMap<QString, double> comprimentoCriticoPorId;
        QMap<QString, double> declividadeEquivalentePorId;
        QMap<QString, double> comprimentoViaMontantePorCanal;
        QMap<QString, double> comprimentoViaBaciaLocalPorCanal;
        QMap<QString, double> declividadeViaMontantePorCanal;
        QMap<QString, double> declividadeViaBaciaLocalPorCanal;
        QMap<QString, double> comprimentoMontanteNaoBaciaPorCanal;
        QMap<QString, double> declividadeMontanteNaoBaciaPorCanal;
        QMap<QString, double> comprimentoBaciaLocalPorCanal;
        QMap<QString, double> declividadeBaciaLocalPorCanal;
        QMap<QString, double> declividadeCanalPorCanal;
        QMap<QString, QString> caminhoSelecionadoPorCanal;
    };

    auto calcularDetalhesTcAcumulado = [&]() {
        DetalhesTcAcumulado detalhes;

        const QVector<QString> ids = rede.idsElementos();
        QMap<QString, QString> jusantePorId;
        QMap<QString, int> grauEntrada;

        for (const QString& id : ids) {
            jusantePorId[id] = rede.idJusanteDoElemento(id);
            grauEntrada[id] = 0;
        }

        for (const QString& id : ids) {
            const QString idJusante = jusantePorId.value(id);
            if (idJusante.isEmpty()) continue;
            if (!grauEntrada.contains(idJusante)) continue;
            grauEntrada[idJusante] += 1;
        }

        QQueue<QString> fila;
        for (auto it = grauEntrada.cbegin(); it != grauEntrada.cend(); ++it) {
            if (it.value() == 0) fila.enqueue(it.key());
        }

        QVector<QString> ordemTopologica;
        while (!fila.isEmpty()) {
            const QString idAtual = fila.dequeue();
            ordemTopologica.append(idAtual);

            const QString idJusante = jusantePorId.value(idAtual);
            if (idJusante.isEmpty()) continue;

            grauEntrada[idJusante] -= 1;
            if (grauEntrada.value(idJusante) == 0) fila.enqueue(idJusante);
        }

        QMap<QString, QVector<QString>> idsMontanteDiretoPorId;
        for (const QString& id : ids) {
            idsMontanteDiretoPorId.insert(id, {});
        }
        for (const QString& id : ids) {
            const QString idJusante = jusantePorId.value(id);
            if (!idJusante.isEmpty() && idsMontanteDiretoPorId.contains(idJusante)) {
                idsMontanteDiretoPorId[idJusante].append(id);
            }
        }

        for (const QString& idAtual : ordemTopologica) {
            const QVector<QString> idsMontanteDireto = idsMontanteDiretoPorId.value(idAtual);

            double maiorComprimentoMontante = 0.0;
            double declividadeDoMaiorMontante = 0.0;
            for (const QString& idMontante : idsMontanteDireto) {
                const double comprimentoMontante = detalhes.comprimentoCriticoPorId.value(idMontante, 0.0);
                if (comprimentoMontante >= maiorComprimentoMontante) {
                    maiorComprimentoMontante = comprimentoMontante;
                    declividadeDoMaiorMontante = detalhes.declividadeEquivalentePorId.value(idMontante, 0.0);
                }
            }

            // Para Lc1, desconsidera bacias locais diretas (elas entram em Lc2).
            double maiorComprimentoMontanteNaoBacia = 0.0;
            double declividadeDoMaiorMontanteNaoBacia = 0.0;
            for (const QString& idMontante : idsMontanteDireto) {
                if (rede.tipoDoElemento(idMontante) == TipoElementoRede::BaciaContribuicao) continue;

                const double comprimentoMontante = detalhes.comprimentoCriticoPorId.value(idMontante, 0.0);
                if (comprimentoMontante >= maiorComprimentoMontanteNaoBacia) {
                    maiorComprimentoMontanteNaoBacia = comprimentoMontante;
                    declividadeDoMaiorMontanteNaoBacia = detalhes.declividadeEquivalentePorId.value(idMontante, 0.0);
                }
            }

            double comprimentoCriticoKm = maiorComprimentoMontante;
            double declividadeEquivalenteCritica = declividadeDoMaiorMontante;

            if (rede.tipoDoElemento(idAtual) == TipoElementoRede::BaciaContribuicao) {
                const BaciaContribuicao* bacia = rede.baciaPorId(idAtual);
                if (bacia) {
                    const double comprimentoBaciaKm = std::max(0.0, bacia->comprimentoTalveguePrincipalKm());
                    const double declividadeBacia = std::max(0.0, bacia->declividadeMedia());

                    if (comprimentoBaciaKm >= comprimentoCriticoKm) {
                        comprimentoCriticoKm = comprimentoBaciaKm;
                        declividadeEquivalenteCritica = declividadeBacia;
                    }
                }
            }
            else if (rede.tipoDoElemento(idAtual) == TipoElementoRede::Canal) {
                const Canal* canal = rede.canalPorId(idAtual);
                if (canal) {
                    const double comprimentoCanalKm = std::max(0.0, canal->comprimento()) / 1000.0;
                    const double declividadeCanal = std::max(0.0, (canal->declividadeMinima() + canal->declividadeMaxima()) * 0.5);

                    detalhes.comprimentoMontanteNaoBaciaPorCanal[idAtual] = maiorComprimentoMontanteNaoBacia;
                    detalhes.declividadeMontanteNaoBaciaPorCanal[idAtual] = declividadeDoMaiorMontanteNaoBacia;
                    detalhes.declividadeCanalPorCanal[idAtual] = declividadeCanal;

                    const double comprimentoViaMontante = maiorComprimentoMontanteNaoBacia + comprimentoCanalKm;
                    const double declividadeViaMontante = (comprimentoViaMontante > 0.0)
                                                            ? ((declividadeDoMaiorMontanteNaoBacia * maiorComprimentoMontanteNaoBacia)
                                                               + (declividadeCanal * comprimentoCanalKm))
                                                                / comprimentoViaMontante
                                                            : 0.0;

                    double maiorComprimentoBaciaLocal = 0.0;
                    double declividadeBaciaLocal = 0.0;
                    for (const QString& idMontante : idsMontanteDireto) {
                        if (rede.tipoDoElemento(idMontante) != TipoElementoRede::BaciaContribuicao) continue;

                        const BaciaContribuicao* baciaLocal = rede.baciaPorId(idMontante);
                        if (!baciaLocal) continue;

                        const double comprimentoBaciaKm = std::max(0.0, baciaLocal->comprimentoTalveguePrincipalKm());
                        if (comprimentoBaciaKm >= maiorComprimentoBaciaLocal) {
                            maiorComprimentoBaciaLocal = comprimentoBaciaKm;
                            declividadeBaciaLocal = std::max(0.0, baciaLocal->declividadeMedia());
                        }
                    }

                    const double comprimentoViaBaciaLocal = maiorComprimentoBaciaLocal + (comprimentoCanalKm * 0.5);

                    // Para Seq no caminho de sub-bacia, usa o comprimento total do canal.
                    const double comprimentoBaseSeqViaBaciaLocal = maiorComprimentoBaciaLocal + comprimentoCanalKm;
                    const double declividadeViaBaciaLocal = (comprimentoBaseSeqViaBaciaLocal > 0.0)
                                                              ? ((declividadeBaciaLocal * maiorComprimentoBaciaLocal)
                                                                 + (declividadeCanal * comprimentoCanalKm))
                                                                  / comprimentoBaseSeqViaBaciaLocal
                                                              : 0.0;

                    detalhes.comprimentoViaMontantePorCanal[idAtual] = comprimentoViaMontante;
                    detalhes.comprimentoViaBaciaLocalPorCanal[idAtual] = comprimentoViaBaciaLocal;
                    detalhes.declividadeViaMontantePorCanal[idAtual] = declividadeViaMontante;
                    detalhes.declividadeViaBaciaLocalPorCanal[idAtual] = declividadeViaBaciaLocal;
                    detalhes.comprimentoBaciaLocalPorCanal[idAtual] = maiorComprimentoBaciaLocal;
                    detalhes.declividadeBaciaLocalPorCanal[idAtual] = declividadeBaciaLocal;

                    if (comprimentoViaBaciaLocal > comprimentoViaMontante) {
                        comprimentoCriticoKm = comprimentoViaBaciaLocal;
                        declividadeEquivalenteCritica = declividadeViaBaciaLocal;
                        detalhes.caminhoSelecionadoPorCanal[idAtual] = "Lc2";
                    }
                    else {
                        comprimentoCriticoKm = comprimentoViaMontante;
                        declividadeEquivalenteCritica = declividadeViaMontante;
                        detalhes.caminhoSelecionadoPorCanal[idAtual] = "Lc1";
                    }
                }
            }

            detalhes.comprimentoCriticoPorId[idAtual] = std::max(0.0, comprimentoCriticoKm);
            detalhes.declividadeEquivalentePorId[idAtual] = std::max(0.0, declividadeEquivalenteCritica);
        }

        return detalhes;
    };

    const DetalhesTcAcumulado detalhesTcAcumulado = calcularDetalhesTcAcumulado();

    auto formatarValor = [&](double valor, int casasDecimais) {
        if (!std::isfinite(valor)) return QString("n/d");
        return QString::number(valor, 'f', casasDecimais);
    };

    auto descreverSecaoCanal = [&](const Canal& canal) {
        if (canal.tipoSecao() == TipoSecaoCanal::Semicircular) {
            const double diametroM = std::max(0.0, canal.secaoSemicircular().diametro());
            return QString("Semicircular(D=%1 m)").arg(diametroM, 0, 'f', 2);
        }

        const double larguraFundoM = std::max(0.0, canal.secaoTransversal().larguraFundo());
        const double talude = std::max(0.0, canal.secaoTransversal().taludeLateral());
        return QString("Trapezoidal(b=%1 m; m=%2)")
            .arg(larguraFundoM, 0, 'f', 2)
            .arg(talude, 0, 'f', 2);
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

        const double tcCanalMin = rede.tempoConcentracaoKirpichModificadoAreaTotal(id);
        const double intensidadeCanalMmH = idf.intensidadeMmH(tempoRetornoAnos, tcCanalMin);
        const double qCanalM3s = std::max(0.0, vazaoAcumuladaTR50M3s.value(id, 0.0));
        const double cMedio = rede.coeficienteEscoamentoMedioPonderado(id);
        const double areaTotalKm2 = rede.areaAcumuladaTotalContribuinte(id);
        // Ltotal do relatório hidrológico deve seguir o caminho crítico do Tc.
        const double lTotalTalvegueKm = std::max(0.0, detalhesTcAcumulado.comprimentoCriticoPorId.value(id, 0.0));
        const double areaTotalM2 = std::max(0.0, areaTotalKm2) * 1e6;
        const double lTotalTalvegueM = std::max(0.0, lTotalTalvegueKm) * 1000.0;
        const double declividadeEquivalente = detalhesTcAcumulado.declividadeEquivalentePorId.value(id, 0.0);
        const double comprimentoViaMontanteKm = detalhesTcAcumulado.comprimentoViaMontantePorCanal.value(id, 0.0);
        const double comprimentoViaBaciaLocalKm = detalhesTcAcumulado.comprimentoViaBaciaLocalPorCanal.value(id, 0.0);
        const double declividadeViaMontante = detalhesTcAcumulado.declividadeViaMontantePorCanal.value(id, 0.0);
        const double declividadeViaBaciaLocal = detalhesTcAcumulado.declividadeViaBaciaLocalPorCanal.value(id, 0.0);
        const double comprimentoMontanteNaoBaciaKm = detalhesTcAcumulado.comprimentoMontanteNaoBaciaPorCanal.value(id, 0.0);
        const double declividadeMontanteNaoBacia = detalhesTcAcumulado.declividadeMontanteNaoBaciaPorCanal.value(id, 0.0);
        const double comprimentoBaciaLocalKm = detalhesTcAcumulado.comprimentoBaciaLocalPorCanal.value(id, 0.0);
        const double declividadeBaciaLocal = detalhesTcAcumulado.declividadeBaciaLocalPorCanal.value(id, 0.0);
        const double declividadeCanal = detalhesTcAcumulado.declividadeCanalPorCanal.value(id, 0.0);
        const double declividadeFinalCanal = std::max(0.0, canal->declividadeFinal());
        const QString caminhoSelecionado = detalhesTcAcumulado.caminhoSelecionadoPorCanal.value(id, "-");
        const double comprimentoViaMontanteM = std::max(0.0, comprimentoViaMontanteKm) * 1000.0;
        const double comprimentoViaBaciaLocalM = std::max(0.0, comprimentoViaBaciaLocalKm) * 1000.0;

        const double laminaProjetoM = canal->alturaLaminaParaVazaoProjeto(qCanalM3s);

        Canal canalSMin = *canal;
        canalSMin.setDeclividadeFinal(canal->declividadeMinima());
        const double laminaSMinM = canalSMin.alturaLaminaParaVazaoProjeto(qCanalM3s);

        Canal canalSMax = *canal;
        canalSMax.setDeclividadeFinal(canal->declividadeMaxima());
        const double laminaSMaxM = canalSMax.alturaLaminaParaVazaoProjeto(qCanalM3s);

        QString linhaCanal = QString("- %1: Tc=%2 min | i=%3 mm/h | Q=%4 m3/s | Cmed=%5 | A_total=%6 km2 | Ltotal=%7 km | Seq=%8 (%9%%) | A_total=%10 m2 | Ltotal=%11 m | Lc1(Lmont+Lcanal)=%12 m | Lc2(LSB+Lcanal/2)=%13 m | Lmont=%14 m | Smont=%15%% | LSB=%16 m | SSB=%17%% | Scanal=%18%% | Sfinal=%19%% | S_Lc1=%20%% | S_Lc2=%21%% | Caminho=%22 | h_proj=%23 m | h_smin=%24 m | h_smax=%25 m")
                                .arg(id)
                                .arg(tcCanalMin, 0, 'f', 2)
                                .arg(intensidadeCanalMmH, 0, 'f', 2)
                                .arg(qCanalM3s, 0, 'f', 3)
                                .arg(cMedio, 0, 'f', 3)
                                .arg(areaTotalKm2, 0, 'f', 3)
                                .arg(lTotalTalvegueKm, 0, 'f', 3)
                                .arg(declividadeEquivalente, 0, 'f', 5)
                                .arg(declividadeEquivalente * 100.0, 0, 'f', 3)
                                .arg(areaTotalM2, 0, 'f', 2)
                                .arg(lTotalTalvegueM, 0, 'f', 2)
                                .arg(comprimentoViaMontanteM, 0, 'f', 2)
                                .arg(comprimentoViaBaciaLocalM, 0, 'f', 2)
                                .arg(comprimentoMontanteNaoBaciaKm * 1000.0, 0, 'f', 2)
                                .arg(declividadeMontanteNaoBacia * 100.0, 0, 'f', 3)
                                .arg(comprimentoBaciaLocalKm * 1000.0, 0, 'f', 2)
                                .arg(declividadeBaciaLocal * 100.0, 0, 'f', 3)
                                .arg(declividadeCanal * 100.0, 0, 'f', 3)
                                .arg(declividadeFinalCanal * 100.0, 0, 'f', 3)
                                .arg(declividadeViaMontante * 100.0, 0, 'f', 3)
                                .arg(declividadeViaBaciaLocal * 100.0, 0, 'f', 3)
                                .arg(caminhoSelecionado)
                                .arg(formatarValor(laminaProjetoM, 3))
                                .arg(formatarValor(laminaSMinM, 3))
                                .arg(formatarValor(laminaSMaxM, 3));

        linhaCanal += QString(" | Secao=%1 | n=%2")
                          .arg(descreverSecaoCanal(*canal))
                          .arg(canal->coeficienteManning(), 0, 'f', 3);

        linhas << linhaCanal;
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
