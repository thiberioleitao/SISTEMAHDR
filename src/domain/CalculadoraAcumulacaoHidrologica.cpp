#include "CalculadoraAcumulacaoHidrologica.h"

#include <QQueue>

#include <algorithm>
#include <cmath>

namespace
{
struct EstruturaTopologica
{
    QMap<QString, int> indicePorId;
    QMap<QString, int> grauEntrada;
    QVector<QString> ordemTopologica;
};

bool montarEstruturaTopologica(const QVector<ElementoRedeHidrologica>& elementos,
                               EstruturaTopologica* estrutura,
                               QString* erro)
{
    if (!estrutura) return false;
    estrutura->indicePorId.clear();
    estrutura->grauEntrada.clear();
    estrutura->ordemTopologica.clear();

    for (int i = 0; i < elementos.size(); ++i) {
        const ElementoRedeHidrologica& e = elementos.at(i);
        estrutura->indicePorId.insert(e.id, i);
        estrutura->grauEntrada.insert(e.id, 0);
    }

    for (const ElementoRedeHidrologica& e : elementos) {
        if (e.idJusante.isEmpty()) continue;

        if (!estrutura->indicePorId.contains(e.idJusante)) {
            if (erro) *erro = "ID jusante inexistente na rede: " + e.idJusante;
            return false;
        }

        estrutura->grauEntrada[e.idJusante] += 1;
    }

    QQueue<QString> fila;
    for (auto it = estrutura->grauEntrada.cbegin(); it != estrutura->grauEntrada.cend(); ++it) {
        if (it.value() == 0) fila.enqueue(it.key());
    }

    while (!fila.isEmpty()) {
        const QString idAtual = fila.dequeue();
        estrutura->ordemTopologica.append(idAtual);

        const ElementoRedeHidrologica& eAtual = elementos.at(estrutura->indicePorId.value(idAtual));
        if (!eAtual.idJusante.isEmpty()) {
            estrutura->grauEntrada[eAtual.idJusante] -= 1;
            if (estrutura->grauEntrada.value(eAtual.idJusante) == 0) {
                fila.enqueue(eAtual.idJusante);
            }
        }
    }

    if (estrutura->ordemTopologica.size() != elementos.size()) {
        if (erro) *erro = "A rede possui ciclo ou estrutura inválida para acumulação de vazão.";
        return false;
    }

    return true;
}

double tempoConcentracaoPadraoMin(double comprimentoTalvegue,
                                  double declividadeTalvegue)
{
    const double l = std::max(1e-6, comprimentoTalvegue);
    const double s = std::max(1e-6, declividadeTalvegue);

    // Fórmula tipo Kirpich (aprox.) para manter fallback funcional.
    return 57.0 * std::pow(l, 0.77) * std::pow(s, -0.385);
}

struct AcumuladorAreasInterno
{
    double areaTotal = 0.0;
    double somaCvezesArea = 0.0;
    double somaComprimentoVezesArea = 0.0;
    double somaDeclividadeVezesArea = 0.0;
};

bool validarParametrosLocais(const QString& id,
                             const ParametrosHidrologicosLocais& p,
                             MetodologiaAcumulacaoVazao metodologia,
                             QString* erro)
{
    if (p.area < 0.0 || p.coeficienteEscoamento < 0.0 || p.comprimentoTalvegue < 0.0 || p.declividadeTalvegue < 0.0 || p.vazaoLocal < 0.0) {
        if (erro) *erro = "Parâmetros hidrológicos inválidos (valor negativo) para o elemento: " + id;
        return false;
    }

    if (metodologia == MetodologiaAcumulacaoVazao::SomaDasAreas && p.coeficienteEscoamento > 1.0) {
        if (erro) *erro = "Coeficiente de escoamento C > 1 no elemento: " + id;
        return false;
    }

    return true;
}

bool obterComprimentoLocalTalvegue(const RedeHidrologica& rede,
                                   const ElementoRedeHidrologica& elemento,
                                   const QMap<QString, double>& comprimentoTalvegueLocalPorElemento,
                                   double* comprimentoLocal,
                                   QString* erro)
{
    if (!comprimentoLocal) return false;

    if (comprimentoTalvegueLocalPorElemento.contains(elemento.id)) {
        const double valor = comprimentoTalvegueLocalPorElemento.value(elemento.id);
        if (valor < 0.0) {
            if (erro) *erro = "Comprimento de talvegue local negativo no elemento: " + elemento.id;
            return false;
        }
        *comprimentoLocal = valor;
        return true;
    }

    if (elemento.tipo == TipoElementoRede::BaciaContribuicao) {
        const BaciaContribuicao* bacia = rede.baciaPorId(elemento.id);
        if (!bacia) {
            if (erro) *erro = "Elemento do tipo bacia sem vínculo válido: " + elemento.id;
            return false;
        }

        const double valor = bacia->comprimentoTalveguePrincipalKm();
        if (valor < 0.0) {
            if (erro) *erro = "Comprimento de talvegue da bacia inválido (negativo): " + elemento.id;
            return false;
        }

        *comprimentoLocal = valor;
        return true;
    }

    *comprimentoLocal = 0.0;
    return true;
}
}

QMap<QString, double> CalculadoraAcumulacaoHidrologica::contribuicaoBasePorElemento(const RedeHidrologica& rede)
{
    QMap<QString, double> contribuicao;
    for (const ElementoRedeHidrologica& e : rede.elementos()) {
        if (e.tipo == TipoElementoRede::BaciaContribuicao) {
            contribuicao.insert(e.id, 0.0);
        }
    }
    return contribuicao;
}

double CalculadoraAcumulacaoHidrologica::calcularComprimentoTotalTalvegueAteElemento(
    const RedeHidrologica& rede,
    const QString& idElementoDestino,
    const QMap<QString, double>& comprimentoTalvegueLocalPorElemento,
    QString* erro,
    QMap<QString, double>* comprimentoTotalPorElementoSaida)
{
    if (erro) erro->clear();
    if (comprimentoTotalPorElementoSaida) comprimentoTotalPorElementoSaida->clear();

    const QString idDestino = idElementoDestino.trimmed();
    if (idDestino.isEmpty()) {
        if (erro) *erro = "ID de destino não informado para cálculo do comprimento total de talvegue.";
        return 0.0;
    }

    if (!rede.existeId(idDestino)) {
        if (erro) *erro = "ID de destino inexistente na rede: " + idDestino;
        return 0.0;
    }

    const QVector<ElementoRedeHidrologica>& elementos = rede.elementos();
    if (elementos.isEmpty()) {
        if (erro) *erro = "A rede está vazia.";
        return 0.0;
    }

    EstruturaTopologica estrutura;
    if (!montarEstruturaTopologica(elementos, &estrutura, erro)) {
        return 0.0;
    }

    QMap<QString, double> comprimentoTotalPorElemento;
    for (const QString& id : estrutura.indicePorId.keys()) {
        comprimentoTotalPorElemento.insert(id, 0.0);
    }

    // Critério hidrológico: caminho contribuinte principal (máximo acumulado), não soma de ramos.
    for (const QString& idAtual : estrutura.ordemTopologica) {
        const ElementoRedeHidrologica& eAtual = elementos.at(estrutura.indicePorId.value(idAtual));

        double comprimentoLocal = 0.0;
        if (!obterComprimentoLocalTalvegue(rede, eAtual, comprimentoTalvegueLocalPorElemento, &comprimentoLocal, erro)) {
            return 0.0;
        }

        double maiorMontante = 0.0;
        for (const ElementoRedeHidrologica& e : elementos) {
            if (e.idJusante == idAtual) {
                maiorMontante = std::max(maiorMontante, comprimentoTotalPorElemento.value(e.id, 0.0));
            }
        }

        comprimentoTotalPorElemento[idAtual] = maiorMontante + comprimentoLocal;
    }

    if (comprimentoTotalPorElementoSaida) {
        *comprimentoTotalPorElementoSaida = comprimentoTotalPorElemento;
    }

    return std::max(0.0, comprimentoTotalPorElemento.value(idDestino, 0.0));
}

QMap<QString, double> CalculadoraAcumulacaoHidrologica::calcularVazaoAcumulada(
    const RedeHidrologica& rede,
    MetodologiaAcumulacaoVazao metodologia,
    const QMap<QString, ParametrosHidrologicosLocais>& parametrosLocais,
    const std::function<double(double)>& funcaoIntensidadeIDF,
    const std::function<double(double, double)>& funcaoTempoConcentracao,
    QString* erro,
    QMap<QString, ParametrosAcumuladosBacia>* parametrosAcumuladosSaida)
{
    if (erro) erro->clear();
    if (parametrosAcumuladosSaida) parametrosAcumuladosSaida->clear();

    QMap<QString, double> vazaoAcumuladaPorElemento;
    const QVector<ElementoRedeHidrologica>& elementos = rede.elementos();
    if (elementos.isEmpty()) {
        return vazaoAcumuladaPorElemento;
    }

    EstruturaTopologica estrutura;
    if (!montarEstruturaTopologica(elementos, &estrutura, erro)) {
        return QMap<QString, double>();
    }

    for (const QString& id : estrutura.indicePorId.keys()) {
        vazaoAcumuladaPorElemento.insert(id, 0.0);
    }

    if (metodologia == MetodologiaAcumulacaoVazao::SomaDasVazoes) {
        QMap<QString, double> vazaoEntradaAcumulada;
        for (const QString& id : estrutura.indicePorId.keys()) {
            vazaoEntradaAcumulada.insert(id, 0.0);
        }

        for (const QString& idAtual : estrutura.ordemTopologica) {
            const ElementoRedeHidrologica& eAtual = elementos.at(estrutura.indicePorId.value(idAtual));
            const ParametrosHidrologicosLocais pLocal = parametrosLocais.value(idAtual, ParametrosHidrologicosLocais());

            if (!validarParametrosLocais(idAtual, pLocal, metodologia, erro)) {
                return QMap<QString, double>();
            }

            const double vazaoSaida = vazaoEntradaAcumulada.value(idAtual) + std::max(0.0, pLocal.vazaoLocal);
            vazaoAcumuladaPorElemento[idAtual] = vazaoSaida;

            if (!eAtual.idJusante.isEmpty()) {
                vazaoEntradaAcumulada[eAtual.idJusante] = vazaoEntradaAcumulada.value(eAtual.idJusante) + vazaoSaida;
            }
        }

        return vazaoAcumuladaPorElemento;
    }

    // Divergência da metodologia: recalculamos Q com a bacia acumulada no nó.
    if (!funcaoIntensidadeIDF) {
        if (erro) *erro = "Metodologia SomaDasAreas requer uma função de intensidade IDF.";
        return QMap<QString, double>();
    }

    QMap<QString, AcumuladorAreasInterno> acumuladoEntradaPorElemento;
    for (const QString& id : estrutura.indicePorId.keys()) {
        acumuladoEntradaPorElemento.insert(id, AcumuladorAreasInterno());
    }

    for (const QString& idAtual : estrutura.ordemTopologica) {
        const ElementoRedeHidrologica& eAtual = elementos.at(estrutura.indicePorId.value(idAtual));
        const ParametrosHidrologicosLocais pLocal = parametrosLocais.value(idAtual, ParametrosHidrologicosLocais());

        if (!validarParametrosLocais(idAtual, pLocal, metodologia, erro)) {
            return QMap<QString, double>();
        }

        AcumuladorAreasInterno acumulado = acumuladoEntradaPorElemento.value(idAtual);

        const double areaLocal = std::max(0.0, pLocal.area);
        acumulado.areaTotal += areaLocal;
        acumulado.somaCvezesArea += std::max(0.0, pLocal.coeficienteEscoamento) * areaLocal;
        acumulado.somaComprimentoVezesArea += std::max(0.0, pLocal.comprimentoTalvegue) * areaLocal;
        acumulado.somaDeclividadeVezesArea += std::max(0.0, pLocal.declividadeTalvegue) * areaLocal;

        ParametrosAcumuladosBacia pa;
        pa.areaTotal = acumulado.areaTotal;
        pa.somaCvezesArea = acumulado.somaCvezesArea;

        if (pa.areaTotal > 0.0) {
            pa.comprimentoRepresentativo = acumulado.somaComprimentoVezesArea / pa.areaTotal;
            pa.declividadeRepresentativa = acumulado.somaDeclividadeVezesArea / pa.areaTotal;
        }

        const auto funcaoTc = funcaoTempoConcentracao ? funcaoTempoConcentracao : tempoConcentracaoPadraoMin;
        pa.tempoConcentracao = std::max(0.0, funcaoTc(pa.comprimentoRepresentativo, pa.declividadeRepresentativa));
        pa.intensidade = std::max(0.0, funcaoIntensidadeIDF(pa.tempoConcentracao));

        const double cPonderado = (pa.areaTotal > 0.0) ? (pa.somaCvezesArea / pa.areaTotal) : 0.0;
        pa.vazaoPico = (cPonderado * pa.intensidade * pa.areaTotal) / 3.60;

        vazaoAcumuladaPorElemento[idAtual] = pa.vazaoPico;
        if (parametrosAcumuladosSaida) {
            parametrosAcumuladosSaida->insert(idAtual, pa);
        }

        if (!eAtual.idJusante.isEmpty()) {
            AcumuladorAreasInterno destino = acumuladoEntradaPorElemento.value(eAtual.idJusante);
            destino.areaTotal += acumulado.areaTotal;
            destino.somaCvezesArea += acumulado.somaCvezesArea;
            destino.somaComprimentoVezesArea += acumulado.somaComprimentoVezesArea;
            destino.somaDeclividadeVezesArea += acumulado.somaDeclividadeVezesArea;
            acumuladoEntradaPorElemento[eAtual.idJusante] = destino;
        }
    }

    return vazaoAcumuladaPorElemento;
}

QMap<QString, double> CalculadoraAcumulacaoHidrologica::calcularVazaoAcumuladaPorElemento(
    const RedeHidrologica& rede,
    const QMap<QString, double>& contribuicaoPorElemento,
    QString* erro)
{
    QMap<QString, ParametrosHidrologicosLocais> parametros;
    for (auto it = contribuicaoPorElemento.cbegin(); it != contribuicaoPorElemento.cend(); ++it) {
        ParametrosHidrologicosLocais p;
        p.vazaoLocal = std::max(0.0, it.value());
        parametros.insert(it.key(), p);
    }

    return calcularVazaoAcumulada(
        rede,
        MetodologiaAcumulacaoVazao::SomaDasVazoes,
        parametros,
        std::function<double(double)>(),
        std::function<double(double, double)>(),
        erro,
        nullptr);
}
