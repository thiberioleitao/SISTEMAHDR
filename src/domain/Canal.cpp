#include "Canal.h"

#include <algorithm>
#include <cmath>
#include <limits>

Canal::Canal(const QString& id)
    : m_id(id.trimmed())
{
}

const QString& Canal::idProprio() const
{
    return m_id;
}

void Canal::setIdProprio(const QString& id)
{
    m_id = id.trimmed();
}

const QString& Canal::idJusante() const
{
    return m_idJusante;
}

void Canal::setIdJusante(const QString& idJusante)
{
    const QString idNormalizado = idJusante.trimmed();
    if (idNormalizado == m_id) return;
    m_idJusante = idNormalizado;
}

const QString& Canal::id() const
{
    return idProprio();
}

void Canal::setId(const QString& id)
{
    setIdProprio(id);
}

double Canal::comprimentoTotal() const
{
    return std::max(0.0, m_comprimento);
}

double Canal::comprimento() const
{
    return std::max(0.0, m_comprimento);
}

void Canal::setComprimento(double valor)
{
    m_comprimento = std::max(0.0, valor);
}

double Canal::larguraFundo() const
{
    if (m_tipoSecao == TipoSecaoCanal::Semicircular) {
        return m_secaoSemicircular.diametro();
    }
    return m_secao.larguraFundo();
}

void Canal::setLarguraFundo(double valor)
{
    m_secao.setLarguraFundo(valor);
    m_tipoSecao = TipoSecaoCanal::Trapezoidal;
}

double Canal::taludeLateral() const
{
    if (m_tipoSecao == TipoSecaoCanal::Semicircular) {
        return 0.0;
    }
    return m_secao.taludeLateral();
}

void Canal::setTaludeLateral(double valor)
{
    m_secao.setTaludeLateral(valor);
    m_tipoSecao = TipoSecaoCanal::Trapezoidal;
}

double Canal::declividadeFinal() const
{
    return std::max(0.0, m_declividadeFinal);
}

void Canal::setDeclividadeFinal(double valor)
{
    m_declividadeFinal = std::max(0.0, valor);
}

double Canal::declividadeMinima() const
{
    return std::max(0.0, m_declividadeMinima);
}

void Canal::setDeclividadeMinima(double valor)
{
    m_declividadeMinima = std::max(0.0, valor);
}

double Canal::declividadeMaxima() const
{
    return std::max(0.0, m_declividadeMaxima);
}

void Canal::setDeclividadeMaxima(double valor)
{
    m_declividadeMaxima = std::max(0.0, valor);
}

double Canal::coeficienteManning() const
{
    return std::max(0.0, m_revestimento.coeficienteManning());
}

void Canal::setCoeficienteManning(double valor)
{
    m_revestimento.setCoeficienteManning(valor);
}

const SecaoTransversalTrapezoidal& Canal::secaoTransversal() const
{
    return m_secao;
}

void Canal::setSecaoTransversal(const SecaoTransversalTrapezoidal& secao)
{
    m_secao = secao;
    m_tipoSecao = TipoSecaoCanal::Trapezoidal;
}

const SecaoTransversalSemicircular& Canal::secaoSemicircular() const
{
    return m_secaoSemicircular;
}

void Canal::setSecaoSemicircular(const SecaoTransversalSemicircular& secao)
{
    m_secaoSemicircular = secao;
    m_tipoSecao = TipoSecaoCanal::Semicircular;
}

TipoSecaoCanal Canal::tipoSecao() const
{
    return m_tipoSecao;
}

void Canal::setTipoSecao(TipoSecaoCanal tipo)
{
    m_tipoSecao = tipo;
}

const RevestimentoCanal& Canal::revestimento() const
{
    return m_revestimento;
}

void Canal::setRevestimento(const RevestimentoCanal& revestimento)
{
    m_revestimento = revestimento;
}

double Canal::larguraSuperficial(double alturaLamina) const
{
    if (m_tipoSecao == TipoSecaoCanal::Semicircular) {
        return m_secaoSemicircular.larguraSuperficial(alturaLamina);
    }
    return m_secao.larguraSuperficial(alturaLamina);
}

double Canal::areaMolhada(double alturaLamina) const
{
    if (m_tipoSecao == TipoSecaoCanal::Semicircular) {
        return m_secaoSemicircular.areaMolhada(alturaLamina);
    }
    return m_secao.areaMolhada(alturaLamina);
}

double Canal::perimetroMolhado(double alturaLamina) const
{
    if (m_tipoSecao == TipoSecaoCanal::Semicircular) {
        return m_secaoSemicircular.perimetroMolhado(alturaLamina);
    }
    return m_secao.perimetroMolhado(alturaLamina);
}

double Canal::raioHidraulico(double alturaLamina) const
{
    if (m_tipoSecao == TipoSecaoCanal::Semicircular) {
        return m_secaoSemicircular.raioHidraulico(alturaLamina);
    }
    return m_secao.raioHidraulico(alturaLamina);
}

double Canal::velocidadeManning(double alturaLamina) const
{
    const double y = std::max(0.0, alturaLamina);
    const double s = std::max(0.0, m_declividadeFinal);
    const double n = std::max(0.0, coeficienteManning());

    if (n <= 0.0 || s <= 0.0) return 0.0;

    const double r = raioHidraulico(y);
    if (r <= 0.0) return 0.0;

    return (1.0 / n) * std::pow(r, 2.0 / 3.0) * std::sqrt(s);
}

double Canal::vazaoManning(double alturaLamina) const
{
    const double y = std::max(0.0, alturaLamina);
    const double area = areaMolhada(y);
    return area * velocidadeManning(y);
}

double Canal::alturaLaminaParaVazaoProjeto(double vazaoProjeto,
                                           double alturaMaximaBusca,
                                           double tolerancia,
                                           int maxIteracoes) const
{
    if (vazaoProjeto <= 0.0) return 0.0;
    if (declividadeFinal() <= 0.0 || coeficienteManning() <= 0.0) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    const double tol = std::max(1e-12, tolerancia);
    const int maxIt = std::max(1, maxIteracoes);
    const double yMaxFisico = std::max(1e-6, alturaMaximaBusca);

    const auto f = [&](double y) {
        return vazaoManning(y) - vazaoProjeto;
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
