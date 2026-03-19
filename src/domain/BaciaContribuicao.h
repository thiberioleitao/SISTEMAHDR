#pragma once

#include <QString>

class BaciaContribuicao
{
public:
    BaciaContribuicao() = default;
    explicit BaciaContribuicao(const QString& id);

    const QString& id() const;
    void setID(const QString& id);

    const QString& idProprio() const;
    void setIdProprio(const QString& id);

    const QString& idJusante() const;
    void setIdJusante(const QString& idJusante);

    const QString& nome() const;
    void setNome(const QString& nome);

    double areaKm2() const;
    void setAreaKm2(double valor);

    double areaM2() const;
    void setAreaM2(double valor);

    double declividadeMedia() const;
    void setDeclividadeMedia(double valor);

    double comprimentoTalveguePrincipalKm() const;
    void setComprimentoTalveguePrincipalKm(double valor);

    double C_10() const;
    void setC_10(double valor);

    // Método racional local: Q (m³/s) = 0,278 * C * i(mm/h) * A(km²)
    double calcularContribuicaoRacional(double intensidadeChuvaMmH) const;

    // Tc local com propagação de maior Tc já calculado em montante.
    double tempoConcentracaoKirpichModificado(double maiorTempoConcentracaoMontanteMin = 0.0) const;

private:
    double m_areaKm2 = 0.0;
    double m_declividadeMedia = 0.0;
    double m_comprimentoTalveguePrincipalKm = 0.0;
    double m_C_10 = 0.0;
    mutable double m_vazaoProjeto = 0.0;
    QString m_id;
    QString m_idJusante;
};
