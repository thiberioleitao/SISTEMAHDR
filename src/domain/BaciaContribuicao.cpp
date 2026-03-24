#include "BaciaContribuicao.h"
#include "HidrologiaUtils.h"

#include <algorithm>

BaciaContribuicao::BaciaContribuicao(const QString& id)
{
    setID(id);
}

const QString& BaciaContribuicao::id() const
{
    return m_id;
}

void BaciaContribuicao::setID(const QString& id)
{
    m_id = id.trimmed();
}

const QString& BaciaContribuicao::idProprio() const
{
    return m_id;
}

void BaciaContribuicao::setIdProprio(const QString& id)
{
    m_id = id.trimmed();
}

const QString& BaciaContribuicao::idJusante() const
{
    return m_idJusante;
}

void BaciaContribuicao::setIdJusante(const QString& idJusante)
{
    const QString idNormalizado = idJusante.trimmed();
    if (idNormalizado == m_id) return;
    m_idJusante = idNormalizado;
}

const QString& BaciaContribuicao::nome() const
{
    return m_id;
}

void BaciaContribuicao::setNome(const QString& nome)
{
    m_id = nome.trimmed();
}

double BaciaContribuicao::areaKm2() const
{
    return m_areaKm2;
}

void BaciaContribuicao::setAreaKm2(double valor)
{
    m_areaKm2 = std::max(0.0, valor);
}

double BaciaContribuicao::areaM2() const
{
    return std::max(0.0, m_areaKm2) * 1000000.0;
}

void BaciaContribuicao::setAreaM2(double valor)
{
    m_areaKm2 = std::max(0.0, valor) / 1000000.0;
}

double BaciaContribuicao::declividadeMedia() const
{
    return m_declividadeMedia;
}

void BaciaContribuicao::setDeclividadeMedia(double valor)
{
    m_declividadeMedia = std::max(0.0, valor);
}

double BaciaContribuicao::comprimentoTalveguePrincipalKm() const
{
    return m_comprimentoTalveguePrincipalKm;
}

void BaciaContribuicao::setComprimentoTalveguePrincipalKm(double valor)
{
    m_comprimentoTalveguePrincipalKm = std::max(0.0, valor);
}

double BaciaContribuicao::C_10() const
{
    return m_C_10;
}

void BaciaContribuicao::setC_10(double valor)
{
    m_C_10 = std::max(0.0, valor);
}

double BaciaContribuicao::calcularContribuicaoRacional(double intensidadeChuvaMmH) const
{
    // Fórmula racional local: Q = 0,278 * C * i * A
    const double c = std::max(0.0, m_C_10);
    const double i = std::max(0.0, intensidadeChuvaMmH);
    const double a = std::max(0.0, m_areaKm2);

    m_vazaoProjeto = 0.278 * c * i * a;
    return m_vazaoProjeto;
}

double BaciaContribuicao::tempoConcentracaoKirpichModificado(double maiorTempoConcentracaoMontanteMin) const
{
    const double comprimentoTalvegueKm = std::max(0.0, m_comprimentoTalveguePrincipalKm);
    const double declividadeTalvegue = std::max(0.0, m_declividadeMedia);

    return Hidrologia::calcularTcKirpichModificadoMin(
        comprimentoTalvegueKm,
        declividadeTalvegue,
        maiorTempoConcentracaoMontanteMin);
}
