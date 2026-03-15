#include "Canal.h"

#include "CalculadoraAcumulacaoHidrologica.h"
#include "RedeHidrologica.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <QSet>

Canal::Canal(const QString& nome)
    : m_nome(nome)
{
}

double Canal::velocidadeManning(const SecaoTransversalTrapezoidal& secao,
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

double Canal::vazaoManning(const SecaoTransversalTrapezoidal& secao,
                           double alturaLamina,
                           double declividadeFundo,
                           double coeficienteManning)
{
    const double y = std::max(0.0, alturaLamina);
    const double area = secao.areaMolhada(y);
    const double velocity = velocidadeManning(secao, y, declividadeFundo, coeficienteManning);
    return area * velocity;
}

double Canal::laminaParaVazao(const SecaoTransversalTrapezoidal& secao,
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

const QString& Canal::nome() const
{
    return m_nome;
}

void Canal::setNome(const QString& nome)
{
    m_nome = nome;
}

int Canal::quantidadeTrechos() const
{
    return m_trechos.size();
}

bool Canal::vazio() const
{
    return m_trechos.isEmpty();
}

const QVector<CanalTrecho>& Canal::trechos() const
{
    return m_trechos;
}

bool Canal::adicionarTrecho(const CanalTrecho& trecho, double)
{
    m_trechos.append(trecho);
    return true;
}

bool Canal::removerTrecho(int indice)
{
    if (indice < 0 || indice >= m_trechos.size()) return false;
    m_trechos.removeAt(indice);
    return true;
}

void Canal::limpar()
{
    m_trechos.clear();
}


double Canal::declividadeMedia() const
{
    if (m_trechos.isEmpty()) return 0.0;

    double soma = 0.0;
    for (const CanalTrecho& trecho : m_trechos) {
        soma += trecho.declividadeFundo();
    }

    return soma / static_cast<double>(m_trechos.size());
}

double Canal::comprimentoTotal() const
{
    double soma = 0.0;
    for (const CanalTrecho& trecho : m_trechos) {
        soma += trecho.comprimento();
    }
    return soma;
}

double Canal::areaAcumuladaTotalContribuinte(const RedeHidrologica& rede,
                                             QString* erro) const
{
    if (erro) erro->clear();

    const QString idAtual = m_nome.trimmed();
    if (idAtual.isEmpty()) {
        if (erro) *erro = "Nome do canal vazio: use um nome único para representar o ID topológico.";
        return 0.0;
    }

    if (!rede.existeId(idAtual)) {
        if (erro) *erro = "Nome/ID do canal não encontrado na rede: " + idAtual;
        return 0.0;
    }

    QString erroMontante;
    const QVector<QString> idsMontanteLista = rede.idsMontanteDoElemento(idAtual, &erroMontante);
    if (!erroMontante.isEmpty()) {
        if (erro) *erro = erroMontante;
        return 0.0;
    }

    QSet<QString> idsMontante;
    for (const QString& id : idsMontanteLista) {
        idsMontante.insert(id);
    }

    // Soma apenas áreas de elementos do tipo bacia, via mapa id->bacia na rede.
    double areaTotal = 0.0;
    for (const ElementoRedeHidrologica& e : rede.elementos()) {
        if (e.tipo == TipoElementoRede::BaciaContribuicao && idsMontante.contains(e.id)) {
            if (const BaciaContribuicao* b = rede.baciaPorId(e.id)) {
                areaTotal += std::max(0.0, b->areaKm2());
            }
        }
    }

    return areaTotal;
}

double Canal::coeficienteEscoamentoMedioPonderado(const RedeHidrologica& rede,
                                                  QString* erro) const
{
    if (erro) erro->clear();

    const QString idAtual = m_nome.trimmed();
    if (idAtual.isEmpty()) {
        if (erro) *erro = "Nome do canal vazio: use um nome único para representar o ID topológico.";
        return 0.0;
    }

    if (!rede.existeId(idAtual)) {
        if (erro) *erro = "Nome/ID do canal não encontrado na rede: " + idAtual;
        return 0.0;
    }

    QString erroMontante;
    const QVector<QString> idsMontanteLista = rede.idsMontanteDoElemento(idAtual, &erroMontante);
    if (!erroMontante.isEmpty()) {
        if (erro) *erro = erroMontante;
        return 0.0;
    }

    QSet<QString> idsMontante;
    for (const QString& id : idsMontanteLista) {
        idsMontante.insert(id);
    }

    double somaArea = 0.0;
    double somaCvezesArea = 0.0;

    for (const ElementoRedeHidrologica& e : rede.elementos()) {
        if (e.tipo != TipoElementoRede::BaciaContribuicao || !idsMontante.contains(e.id)) continue;

        const BaciaContribuicao* bacia = rede.baciaPorId(e.id);
        if (!bacia) continue;

        const double area = std::max(0.0, bacia->areaKm2());
        if (area <= 0.0) continue;

        const double c = std::max(0.0, bacia->C_10());
        somaArea += area;
        somaCvezesArea += c * area;
    }

    if (somaArea <= 0.0) {
        if (erro) *erro = "Não há bacias válidas para calcular C_10 médio ponderado.";
        return 0.0;
    }

    return somaCvezesArea / somaArea;
}
