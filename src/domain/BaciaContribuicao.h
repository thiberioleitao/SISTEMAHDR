#pragma once

#include <memory>

#include <QString>

class ModeloInfiltracao;
class ModeloTransformacaoChuvaVazao;

class BaciaContribuicao
{
public:
    BaciaContribuicao();

    BaciaContribuicao(const QString& nome,
                      const QString& idJusante,
                      double areaKm2,
                      double declividadeMedia,
                      double comprimentoTalveguePrincipalKm);

    BaciaContribuicao(const QString& nome,
                      double areaKm2,
                      double declividadeMedia,
                      double comprimentoTalveguePrincipalKm);

    BaciaContribuicao(double areaKm2,
                      double declividadeMedia,
                      double comprimentoTalveguePrincipalKm);

    const QString& id() const;

    const QString& nome() const;
    void setNome(const QString& nome);

    const QString& idJusante() const;
    void setIdJusante(const QString& idJusante);

    double areaKm2() const;
    void setAreaKm2(double valor);

    double declividadeMedia() const;
    void setDeclividadeMedia(double valor);

    double comprimentoTalveguePrincipalKm() const;
    void setComprimentoTalveguePrincipalKm(double valor);

    const std::shared_ptr<ModeloInfiltracao>& modeloInfiltracao() const;
    void setModeloInfiltracao(const std::shared_ptr<ModeloInfiltracao>& modelo);

    const std::shared_ptr<ModeloTransformacaoChuvaVazao>& modeloTransformacao() const;
    void setModeloTransformacao(const std::shared_ptr<ModeloTransformacaoChuvaVazao>& modelo);

    double calcularVazaoProjeto(double intensidadeChuvaBrutaMmH) const;

private:
    static QString gerarIdAutomatico();

    QString m_id;
    QString m_nome;
    QString m_idJusante;
    double m_areaKm2 = 0.0;
    double m_declividadeMedia = 0.0;
    double m_comprimentoTalveguePrincipalKm = 0.0;

    std::shared_ptr<ModeloInfiltracao> m_modeloInfiltracao;
    std::shared_ptr<ModeloTransformacaoChuvaVazao> m_modeloTransformacao;
};
