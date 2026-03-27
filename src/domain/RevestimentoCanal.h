#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

/**
 * @brief Representa um tipo padronizado de revestimento hidraulico para canais.
 */
class RevestimentoCanal
{
public:
    /**
     * @brief Cria um revestimento vazio.
     */
    RevestimentoCanal() = default;

    /**
     * @brief Cria um revestimento com os limites hidraulicos usados nas verificacoes.
     * @param nome Nome padronizado do revestimento.
     * @param coeficienteManning Coeficiente de Manning associado ao material.
     * @param velocidadeMaximaAdmissivelMps Velocidade maxima admissivel para o revestimento.
     * @param tensaoCisalhamentoMaximaAdmissivelPa Tensao de cisalhamento maxima admissivel.
     */
    RevestimentoCanal(const QString& nome,
                      double coeficienteManning,
                      double velocidadeMaximaAdmissivelMps,
                      double tensaoCisalhamentoMaximaAdmissivelPa);

    /**
     * @brief Cria um revestimento legando o terceiro parametro como espessura.
     * @param material Nome do revestimento.
     * @param coeficienteManning Coeficiente de Manning associado ao material.
     * @param espessura Espessura armazenada por compatibilidade com codigo legado.
     */
    RevestimentoCanal(const QString& material,
                      double coeficienteManning,
                      double espessura);

    /**
     * @brief Retorna o nome padronizado do revestimento.
     * @return Nome do revestimento.
     */
    const QString& nome() const;

    /**
     * @brief Atualiza o nome padronizado do revestimento.
     * @param valor Novo nome do revestimento.
     */
    void setNome(const QString& valor);

    /**
     * @brief Retorna o nome do revestimento em compatibilidade com a API anterior.
     * @return Nome do revestimento.
     */
    const QString& material() const;

    /**
     * @brief Atualiza o nome do revestimento em compatibilidade com a API anterior.
     * @param valor Novo nome do revestimento.
     */
    void setMaterial(const QString& valor);

    /**
     * @brief Retorna o coeficiente de Manning do revestimento.
     * @return Coeficiente de Manning nao negativo.
     */
    double coeficienteManning() const;

    /**
     * @brief Atualiza o coeficiente de Manning do revestimento.
     * @param valor Novo coeficiente.
     */
    void setCoeficienteManning(double valor);

    /**
     * @brief Retorna a velocidade maxima admissivel do revestimento.
     * @return Velocidade limite em m/s.
     */
    double velocidadeMaximaAdmissivelMps() const;

    /**
     * @brief Atualiza a velocidade maxima admissivel do revestimento.
     * @param valor Novo limite em m/s.
     */
    void setVelocidadeMaximaAdmissivelMps(double valor);

    /**
     * @brief Retorna a tensao de cisalhamento maxima admissivel do revestimento.
     * @return Tensao limite em Pascal.
     */
    double tensaoCisalhamentoMaximaAdmissivelPa() const;

    /**
     * @brief Atualiza a tensao de cisalhamento maxima admissivel do revestimento.
     * @param valor Novo limite em Pascal.
     */
    void setTensaoCisalhamentoMaximaAdmissivelPa(double valor);

    /**
     * @brief Retorna a espessura armazenada por compatibilidade.
     * @return Espessura nao negativa.
     */
    double espessura() const;

    /**
     * @brief Atualiza a espessura armazenada por compatibilidade.
     * @param valor Nova espessura.
     */
    void setEspessura(double valor);

    /**
     * @brief Indica se o revestimento possui um nome preenchido.
     * @return true quando o revestimento esta identificado.
     */
    bool ehValido() const;

    /**
     * @brief Retorna a lista padrao de revestimentos oferecida pela aplicacao.
     * @return Vetor com revestimentos prontos para selecao.
     */
    static QVector<RevestimentoCanal> revestimentosPadrao();

    /**
     * @brief Retorna os nomes padrao usados no preenchimento de combobox.
     * @return Lista de nomes padronizados.
     */
    static QStringList nomesPadrao();

    /**
     * @brief Procura um revestimento padrao pelo nome informado.
     * @param nome Nome do revestimento desejado.
     * @return Revestimento encontrado ou um default seguro.
     */
    static RevestimentoCanal revestimentoPorNome(const QString& nome);

    /**
     * @brief Retorna um revestimento seguro para dados antigos sem informacao.
     * @return Revestimento padrao adotado pelo sistema.
     */
    static RevestimentoCanal revestimentoPadraoSeguro();

private:
    QString m_nome;
    double m_coeficienteManning = 0.0;
    double m_velocidadeMaximaAdmissivelMps = 0.0;
    double m_tensaoCisalhamentoMaximaAdmissivelPa = 0.0;
    double m_espessura = 0.0;
};
