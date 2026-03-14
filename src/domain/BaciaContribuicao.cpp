#include "BaciaContribuicao.h"

#include "ModeloInfiltracao.h"
#include "ModeloTransformacaoChuvaVazao.h"

#include <algorithm>

BaciaContribuicao::BaciaContribuicao(const QString& nome,
                                     double areaKm2,
                                     double declividadeMedia,
                                     double comprimentoTalveguePrincipalKm,
                                     double C_10)
    : m_nome(nome)
    , m_areaKm2(std::max(0.0, areaKm2))
    , m_declividadeMedia(std::max(0.0, declividadeMedia))
    , m_comprimentoTalveguePrincipalKm(std::max(0.0, comprimentoTalveguePrincipalKm))
    , m_C_10(std::max(0.0, C_10))
{
}

const QString& BaciaContribuicao::nome() const
{
    return m_nome;
}

void BaciaContribuicao::setNome(const QString& nome)
{
    m_nome = nome;
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

double BaciaContribuicao::C_10() const
{
    return m_C_10;
}

void BaciaContribuicao::setC_10(double valor)
{
    m_C_10 = std::max(0.0, valor);
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
