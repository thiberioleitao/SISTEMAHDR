#include "Bueiro.h"

#include "RegistroElementosRede.h"
#include "RedeHidrologica.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <QHash>
#include <QQueue>
#include <QSet>

namespace
{
QHash<const Bueiro*, RedeHidrologica*> g_redePorBueiro;
}

Bueiro::Bueiro(const QString& nome)
    : m_nome(nome.trimmed())
{
}

RedeHidrologica* Bueiro::rede()
{
    return g_redePorBueiro.value(this, nullptr);
}

const RedeHidrologica* Bueiro::rede() const
{
    return g_redePorBueiro.value(this, nullptr);
}

void Bueiro::setRede(RedeHidrologica* rede)
{
    if (rede) {
        g_redePorBueiro.insert(this, rede);
    }
    else {
        g_redePorBueiro.remove(this);
    }
}

const QString& Bueiro::idProprio() const
{
    return m_nome;
}

void Bueiro::setIdProprio(const QString& id)
{
    m_nome = id.trimmed();
}

const QString& Bueiro::idJusante() const
{
    return m_idJusante;
}

void Bueiro::setIdJusante(const QString& idJusante)
{
    const QString idNormalizado = idJusante.trimmed();
    if (idNormalizado == m_nome) return;
    m_idJusante = idNormalizado;
}

double Bueiro::calcularComprimentoTotalTalvegueAteElemento() const
{
    if (!this->rede()) return 0.0;
    const RedeHidrologica& rede = *this->rede();

    const QString idDestino = idProprio().trimmed();
    if (idDestino.isEmpty()) return 0.0;
    if (!rede.existeId(idDestino)) return 0.0;

    QVector<ElementoTopologicoInterno> elementos;
    const QVector<QString> ids = rede.idsElementos();
    elementos.reserve(ids.size());
    for (const QString& id : ids) {
        ElementoTopologicoInterno e;
        e.id = id;
        e.idJusante = rede.idJusanteDoElemento(id);
        e.tipo = rede.tipoDoElemento(id);
        elementos.append(e);
    }
    if (elementos.isEmpty()) return 0.0;

    EstruturaTopologica estrutura;
    estrutura.indicePorId.clear();
    estrutura.grauEntrada.clear();
    estrutura.ordemTopologica.clear();

    for (int i = 0; i < elementos.size(); ++i) {
        const ElementoTopologicoInterno& e = elementos.at(i);
        estrutura.indicePorId.insert(e.id, i);
        estrutura.grauEntrada.insert(e.id, 0);
    }

    for (const ElementoTopologicoInterno& e : elementos) {
        if (e.idJusante.isEmpty()) continue;
        if (!estrutura.indicePorId.contains(e.idJusante)) return 0.0;
        estrutura.grauEntrada[e.idJusante] += 1;
    }

    QQueue<QString> fila;
    for (auto it = estrutura.grauEntrada.cbegin(); it != estrutura.grauEntrada.cend(); ++it) {
        if (it.value() == 0) fila.enqueue(it.key());
    }

    while (!fila.isEmpty()) {
        const QString idAtual = fila.dequeue();
        estrutura.ordemTopologica.append(idAtual);

        const ElementoTopologicoInterno& eAtual = elementos.at(estrutura.indicePorId.value(idAtual));
        if (!eAtual.idJusante.isEmpty()) {
            estrutura.grauEntrada[eAtual.idJusante] -= 1;
            if (estrutura.grauEntrada.value(eAtual.idJusante) == 0) {
                fila.enqueue(eAtual.idJusante);
            }
        }
    }

    if (estrutura.ordemTopologica.size() != elementos.size()) return 0.0;

    QMap<QString, double> comprimentoTotalPorElemento;
    for (const QString& id : estrutura.indicePorId.keys()) {
        comprimentoTotalPorElemento.insert(id, 0.0);
    }

    for (const QString& idAtual : estrutura.ordemTopologica) {
        const ElementoTopologicoInterno& eAtual = elementos.at(estrutura.indicePorId.value(idAtual));

        double comprimentoLocal = 0.0;
        if (eAtual.tipo == TipoElementoRede::BaciaContribuicao) {
            const BaciaContribuicao* bacia = rede.baciaPorId(eAtual.id);
            if (!bacia) return 0.0;

            const double valor = bacia->comprimentoTalveguePrincipalKm();
            if (valor < 0.0) return 0.0;
            comprimentoLocal = valor;
        }

        double maiorMontante = 0.0;
        for (const ElementoTopologicoInterno& e : elementos) {
            if (e.idJusante == idAtual) {
                maiorMontante = std::max(maiorMontante, comprimentoTotalPorElemento.value(e.id, 0.0));
            }
        }

        comprimentoTotalPorElemento[idAtual] = maiorMontante + comprimentoLocal;
    }

    return std::max(0.0, comprimentoTotalPorElemento.value(idDestino, 0.0));
}

QMap<QString, double> Bueiro::calcularVazaoAcumuladaPorElemento(
    const QMap<QString, double>& contribuicaoPorElemento) const
{
    if (!this->rede()) return QMap<QString, double>();
    const RedeHidrologica& rede = *this->rede();

    QMap<QString, double> vazaoAcumuladaPorElemento;
    QVector<ElementoTopologicoInterno> elementos;
    const QVector<QString> ids = rede.idsElementos();
    elementos.reserve(ids.size());
    for (const QString& id : ids) {
        ElementoTopologicoInterno e;
        e.id = id;
        e.idJusante = rede.idJusanteDoElemento(id);
        e.tipo = rede.tipoDoElemento(id);
        elementos.append(e);
    }
    if (elementos.isEmpty()) return vazaoAcumuladaPorElemento;

    EstruturaTopologica estrutura;
    estrutura.indicePorId.clear();
    estrutura.grauEntrada.clear();
    estrutura.ordemTopologica.clear();

    for (int i = 0; i < elementos.size(); ++i) {
        const ElementoTopologicoInterno& e = elementos.at(i);
        estrutura.indicePorId.insert(e.id, i);
        estrutura.grauEntrada.insert(e.id, 0);
    }

    for (const ElementoTopologicoInterno& e : elementos) {
        if (e.idJusante.isEmpty()) continue;
        if (!estrutura.indicePorId.contains(e.idJusante)) return QMap<QString, double>();
        estrutura.grauEntrada[e.idJusante] += 1;
    }

    QQueue<QString> fila;
    for (auto it = estrutura.grauEntrada.cbegin(); it != estrutura.grauEntrada.cend(); ++it) {
        if (it.value() == 0) fila.enqueue(it.key());
    }

    while (!fila.isEmpty()) {
        const QString idAtual = fila.dequeue();
        estrutura.ordemTopologica.append(idAtual);

        const ElementoTopologicoInterno& eAtual = elementos.at(estrutura.indicePorId.value(idAtual));
        if (!eAtual.idJusante.isEmpty()) {
            estrutura.grauEntrada[eAtual.idJusante] -= 1;
            if (estrutura.grauEntrada.value(eAtual.idJusante) == 0) {
                fila.enqueue(eAtual.idJusante);
            }
        }
    }

    if (estrutura.ordemTopologica.size() != elementos.size()) return QMap<QString, double>();

    QMap<QString, double> vazaoEntradaAcumulada;
    for (const QString& id : estrutura.indicePorId.keys()) {
        vazaoEntradaAcumulada.insert(id, 0.0);
        vazaoAcumuladaPorElemento.insert(id, 0.0);
    }

    for (const QString& idAtual : estrutura.ordemTopologica) {
        const ElementoTopologicoInterno& eAtual = elementos.at(estrutura.indicePorId.value(idAtual));
        const double vazaoLocal = std::max(0.0, contribuicaoPorElemento.value(idAtual, 0.0));
        const double vazaoSaida = vazaoEntradaAcumulada.value(idAtual) + vazaoLocal;
        vazaoAcumuladaPorElemento[idAtual] = vazaoSaida;

        if (!eAtual.idJusante.isEmpty()) {
            vazaoEntradaAcumulada[eAtual.idJusante] = vazaoEntradaAcumulada.value(eAtual.idJusante) + vazaoSaida;
        }
    }

    return vazaoAcumuladaPorElemento;
}

double Bueiro::velocidadeManning(const SecaoTransversalTrapezoidal& secao,
                                double alturaLamina,
                                double declividadeFundo,
                                double coeficienteManning)
{
    const double y = std::max(0.0, alturaLamina);
    const double s = std::max(0.0, declividadeFundo);
    const double n = std::max(0.0, coeficienteManning);

    if (n <= 0.0 || s <= 0.0) return 0.0;

    const double r = secao.raioHidraulico(y);
    if (r <= 0.0) return 0.0;

    return (1.0 / n) * std::pow(r, 2.0 / 3.0) * std::sqrt(s);
}

double Bueiro::vazaoManning(const SecaoTransversalTrapezoidal& secao,
                           double alturaLamina,
                           double declividadeFundo,
                           double coeficienteManning)
{
    const double y = std::max(0.0, alturaLamina);
    const double area = secao.areaMolhada(y);
    const double velocity = velocidadeManning(secao, y, declividadeFundo, coeficienteManning);
    return area * velocity;
}

double Bueiro::laminaParaVazao(const SecaoTransversalTrapezoidal& secao,
                              double vazaoDesejada,
                              double declividadeFundo,
                              double coeficienteManning,
                              double tolerancia,
                              int maxIteracoes,
                              double alturaMaximaBusca)
{
    if (vazaoDesejada <= 0.0) return 0.0;
    if (declividadeFundo <= 0.0 || coeficienteManning <= 0.0) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    const double tol = std::max(1e-12, tolerancia);
    const int maxIt = std::max(1, maxIteracoes);
    const double yMaxFisico = std::max(1e-6, alturaMaximaBusca);

    const auto f = [&](double y) {
        return vazaoManning(secao, y, declividadeFundo, coeficienteManning) - vazaoDesejada;
    };

    double y = 0.5 * yMaxFisico;
    for (int i = 0; i < maxIt; ++i) {
        const double fy = f(y);
        if (std::abs(fy) <= tol) return y;

        const double h = std::max(1e-8, 1e-6 * std::max(1.0, y));
        const double yMenos = std::max(0.0, y - h);
        const double yMais = std::min(yMaxFisico, y + h);
        const double intervalo = yMais - yMenos;
        if (intervalo <= 0.0) break;

        const double dFy = (f(yMais) - f(yMenos)) / intervalo;
        if (!std::isfinite(dFy) || std::abs(dFy) < 1e-14) break;

        const double yNovo = y - (fy / dFy);
        if (!std::isfinite(yNovo)) break;

        y = std::clamp(yNovo, 0.0, yMaxFisico);
    }

    double yMin = 0.0;
    double yMax = yMaxFisico;
    double fMin = f(yMin);
    double fMax = f(yMax);

    if (fMin * fMax > 0.0) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    for (int i = 0; i < maxIt; ++i) {
        const double yMid = 0.5 * (yMin + yMax);
        const double fMid = f(yMid);

        if (std::abs(fMid) <= tol || std::abs(yMax - yMin) <= tol) {
            return yMid;
        }

        if (fMin * fMid <= 0.0) {
            yMax = yMid;
            fMax = fMid;
        }
        else {
            yMin = yMid;
            fMin = fMid;
        }
    }

    return 0.5 * (yMin + yMax);
}

const QString& Bueiro::nome() const
{
    return m_nome;
}

void Bueiro::setNome(const QString& nome)
{
    m_nome = nome.trimmed();
}

int Bueiro::quantidadeTrechos() const
{
    return m_trechos.size();
}

bool Bueiro::vazio() const
{
    return m_trechos.isEmpty();
}

const QVector<Canal>& Bueiro::trechos() const
{
    return m_trechos;
}

bool Bueiro::adicionarTrecho(const Canal& trecho, double)
{
    m_trechos.append(trecho);
    return true;
}

bool Bueiro::removerTrecho(int indice)
{
    if (indice < 0 || indice >= m_trechos.size()) return false;
    m_trechos.removeAt(indice);
    return true;
}

void Bueiro::limpar()
{
    m_trechos.clear();
}


double Bueiro::declividadeMedia() const
{
    if (m_trechos.isEmpty()) return 0.0;

    double soma = 0.0;
    for (const Canal& trecho : m_trechos) {
        soma += trecho.declividadeFinal();
    }

    return soma / static_cast<double>(m_trechos.size());
}

double Bueiro::comprimentoTotal() const
{
    double soma = 0.0;
    for (const Canal& trecho : m_trechos) {
        soma += trecho.comprimento();
    }
    return soma;
}



double Bueiro::areaAcumuladaTotalContribuinte() const
{
    if (!this->rede()) return 0.0;
    const RedeHidrologica& rede = *this->rede();

    const QString idAtual = idProprio().trimmed();
    if (idAtual.isEmpty()) return 0.0;

    if (!rede.existeId(idAtual)) return 0.0;

    const QVector<QString> idsMontanteLista = rede.idsMontanteDoElemento(idAtual);

    QSet<QString> idsMontante;
    for (const QString& id : idsMontanteLista) {
        idsMontante.insert(id);
    }

    // Soma apenas áreas de elementos do tipo bacia, via mapa id->bacia na rede.
    double areaTotal = 0.0;
    for (const QString& id : rede.idsElementos()) {
        if (rede.tipoDoElemento(id) == TipoElementoRede::BaciaContribuicao && idsMontante.contains(id)) {
            if (const BaciaContribuicao* b = rede.baciaPorId(id)) {
                areaTotal += std::max(0.0, b->areaKm2());
            }
        }
    }

    return areaTotal;
}

double Bueiro::coeficienteEscoamentoMedioPonderado() const
{
    if (!this->rede()) return 0.0;
    const RedeHidrologica& rede = *this->rede();

    const QString idAtual = idProprio().trimmed();
    if (idAtual.isEmpty()) return 0.0;

    if (!rede.existeId(idAtual)) return 0.0;

    const QVector<QString> idsMontanteLista = rede.idsMontanteDoElemento(idAtual);

    QSet<QString> idsMontante;
    for (const QString& id : idsMontanteLista) {
        idsMontante.insert(id);
    }

    double somaArea = 0.0;
    double somaCvezesArea = 0.0;

    for (const QString& id : rede.idsElementos()) {
        if (rede.tipoDoElemento(id) != TipoElementoRede::BaciaContribuicao || !idsMontante.contains(id)) continue;

        const BaciaContribuicao* bacia = rede.baciaPorId(id);
        if (!bacia) continue;

        const double area = std::max(0.0, bacia->areaKm2());
        if (area <= 0.0) continue;

        const double c = std::max(0.0, bacia->C_10());
        somaArea += area;
        somaCvezesArea += c * area;
    }

    if (somaArea <= 0.0) return 0.0;

    return somaCvezesArea / somaArea;
}

double Bueiro::vazaoAcumuladaTotal() const
{
    return m_vazaoAcumuladaTotal;
}

bool Bueiro::calcularVazaoAcumuladaTotal()
{
    if (!this->rede()) return false;
    const RedeHidrologica& rede = *this->rede();

    const QString idAtual = idProprio().trimmed();
    if (idAtual.isEmpty()) return false;

    if (!rede.existeId(idAtual)) return false;

    QMap<QString, double> vazaoLocalPorElemento;
    for (const QString& id : rede.idsElementos()) {
        if (rede.tipoDoElemento(id) != TipoElementoRede::BaciaContribuicao) continue;

        const BaciaContribuicao* bacia = rede.baciaPorId(id);
        if (!bacia) continue;

        vazaoLocalPorElemento.insert(id, bacia->calcularContribuicaoRacional(rede.intensidadeChuvaProjetoMmH()));
    }

    const QMap<QString, double> vazaoAcumulada = calcularVazaoAcumuladaPorElemento(vazaoLocalPorElemento);

    if (!vazaoAcumulada.contains(idAtual)) return false;

    const double contribuicaoDireta = rede.contribuicaoBaciasParaElemento(idAtual, rede.intensidadeChuvaProjetoMmH());
    m_vazaoAcumuladaTotal = std::max(0.0, vazaoAcumulada.value(idAtual, 0.0));
    m_vazaoContribuicao = std::max(0.0, contribuicaoDireta);
    m_vazaoMontante = std::max(0.0, m_vazaoAcumuladaTotal - m_vazaoContribuicao);

    return true;
}
