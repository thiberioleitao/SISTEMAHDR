#include "BaciaContribuicao.h"

#include "ModeloInfiltracao.h"
#include "ModeloTransformacaoChuvaVazao.h"

#include <algorithm>

BaciaContribuicao::BaciaContribuicao(double areaKm2,
                                     double declividadeMedia,
                                     double comprimentoTalveguePrincipalKm)
    : m_areaKm2(std::max(0.0, areaKm2))
    , m_declividadeMedia(std::max(0.0, declividadeMedia))
    , m_comprimentoTalveguePrincipalKm(std::max(0.0, comprimentoTalveguePrincipalKm))
{
}

double BaciaContribuicao::areaKm2() const
{
    return m_areaKm2;
}

void BaciaContribuicao::setAreaKm2(double valor)
{
    m_areaKm2 = std::max(0.0, valor);
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

const std::shared_ptr<ModeloInfiltracao>& BaciaContribuicao::modeloInfiltracao() const
{
    return m_modeloInfiltracao;
}

void BaciaContribuicao::setModeloInfiltracao(const std::shared_ptr<ModeloInfiltracao>& modelo)
{
    m_modeloInfiltracao = modelo;
}

const std::shared_ptr<ModeloTransformacaoChuvaVazao>& BaciaContribuicao::modeloTransformacao() const
{
    return m_modeloTransformacao;
}

void BaciaContribuicao::setModeloTransformacao(const std::shared_ptr<ModeloTransformacaoChuvaVazao>& modelo)
{
    m_modeloTransformacao = modelo;
}

double BaciaContribuicao::calcularVazaoProjeto(double intensidadeChuvaBrutaMmH) const
{
    double intensidadeEfetiva = std::max(0.0, intensidadeChuvaBrutaMmH);

    if (m_modeloInfiltracao) {
        intensidadeEfetiva = std::max(0.0, m_modeloInfiltracao->intensidadeEfetivaMmH(*this, intensidadeEfetiva));
    }

    if (!m_modeloTransformacao) return 0.0;

    return std::max(0.0, m_modeloTransformacao->vazaoPicoM3s(*this, intensidadeEfetiva));
}
