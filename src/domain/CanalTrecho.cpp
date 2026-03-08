#include "CanalTrecho.h"

#include <algorithm>

CanalTrecho::CanalTrecho(const QString& nome,
                         double comprimento,
                         double larguraFundo,
                         double alturaLaminaProjeto,
                         double taludeLateral,
                         double declividadeFundo,
                         double coeficienteManning)
    : m_nome(nome)
    , m_comprimento(std::max(0.0, comprimento))
    , m_secao(larguraFundo, taludeLateral)
    , m_alturaLaminaProjeto(std::max(0.0, alturaLaminaProjeto))
    , m_declividadeFundo(std::max(0.0, declividadeFundo))
    , m_revestimento(QString(), coeficienteManning, 0.0)
{
}

CanalTrecho::CanalTrecho(const QString& nome,
                         double comprimento,
                         const SecaoTransversalTrapezoidal& secao,
                         double declividadeFundo,
                         const RevestimentoCanal& revestimento,
                         double alturaLaminaProjeto)
    : m_nome(nome)
    , m_comprimento(std::max(0.0, comprimento))
    , m_secao(secao)
    , m_alturaLaminaProjeto(std::max(0.0, alturaLaminaProjeto))
    , m_declividadeFundo(std::max(0.0, declividadeFundo))
    , m_revestimento(revestimento)
{
}

const QString& CanalTrecho::nome() const
{
    return m_nome;
}

void CanalTrecho::setNome(const QString& nome)
{
    m_nome = nome;
}

double CanalTrecho::comprimento() const
{
    return m_comprimento;
}

void CanalTrecho::setComprimento(double valor)
{
    m_comprimento = std::max(0.0, valor);
}

double CanalTrecho::larguraFundo() const
{
    return m_secao.larguraFundo();
}

void CanalTrecho::setLarguraFundo(double valor)
{
    m_secao.setLarguraFundo(valor);
}

double CanalTrecho::alturaLaminaProjeto() const
{
    return m_alturaLaminaProjeto;
}

void CanalTrecho::setAlturaLaminaProjeto(double valor)
{
    m_alturaLaminaProjeto = std::max(0.0, valor);
}

double CanalTrecho::taludeLateral() const
{
    return m_secao.taludeLateral();
}

void CanalTrecho::setTaludeLateral(double valor)
{
    m_secao.setTaludeLateral(valor);
}

double CanalTrecho::declividadeFundo() const
{
    return m_declividadeFundo;
}

void CanalTrecho::setDeclividadeFundo(double valor)
{
    m_declividadeFundo = std::max(0.0, valor);
}

double CanalTrecho::coeficienteManning() const
{
    return m_revestimento.coeficienteManning();
}

void CanalTrecho::setCoeficienteManning(double valor)
{
    m_revestimento.setCoeficienteManning(valor);
}

const SecaoTransversalTrapezoidal& CanalTrecho::secaoTransversal() const
{
    return m_secao;
}

void CanalTrecho::setSecaoTransversal(const SecaoTransversalTrapezoidal& secao)
{
    m_secao = secao;
}

const RevestimentoCanal& CanalTrecho::revestimento() const
{
    return m_revestimento;
}

void CanalTrecho::setRevestimento(const RevestimentoCanal& revestimento)
{
    m_revestimento = revestimento;
}

double CanalTrecho::larguraSuperficial(double alturaLamina) const
{
    return m_secao.larguraSuperficial(alturaLamina);
}

double CanalTrecho::areaMolhada(double alturaLamina) const
{
    return m_secao.areaMolhada(alturaLamina);
}

double CanalTrecho::perimetroMolhado(double alturaLamina) const
{
    return m_secao.perimetroMolhado(alturaLamina);
}

double CanalTrecho::raioHidraulico(double alturaLamina) const
{
    return m_secao.raioHidraulico(alturaLamina);
}

double CanalTrecho::velocidadeManning(double alturaLamina) const
{
    return CalculoHidraulicoCanal::velocidadeManning(
        m_secao,
        alturaLamina,
        m_declividadeFundo,
        coeficienteManning());
}

double CanalTrecho::vazaoManning(double alturaLamina) const
{
    return CalculoHidraulicoCanal::vazaoManning(
        m_secao,
        alturaLamina,
        m_declividadeFundo,
        coeficienteManning());
}

double CanalTrecho::vazaoProjeto() const
{
    return CalculoHidraulicoCanal::vazaoProjeto(
        m_secao,
        m_alturaLaminaProjeto,
        m_declividadeFundo,
        coeficienteManning());
}
