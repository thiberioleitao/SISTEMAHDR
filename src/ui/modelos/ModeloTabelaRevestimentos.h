#pragma once

#include "domain/RevestimentoCanal.h"

#include <QAbstractTableModel>
#include <QVector>

/**
 * @brief Modelo Qt que expõe o catálogo editável de revestimentos hidráulicos.
 */
class ModeloTabelaRevestimentos : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Coluna
    {
        ColunaNome = 0,
        ColunaCoeficienteManning,
        ColunaVelocidadeMaxima,
        ColunaTensaoCisalhamentoMaxima,
        ColunaEspessura,
        TotalColunas
    };

    /**
     * @brief Cria o modelo com o catálogo padrão de revestimentos.
     * @param parent Objeto pai do modelo.
     */
    explicit ModeloTabelaRevestimentos(QObject* parent = nullptr);

    /**
     * @brief Retorna a quantidade de linhas do catálogo.
     * @param parent Índice pai do Qt.
     * @return Total de revestimentos disponíveis.
     */
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    /**
     * @brief Retorna a quantidade de colunas da tabela.
     * @param parent Índice pai do Qt.
     * @return Quantidade de propriedades expostas por revestimento.
     */
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    /**
     * @brief Retorna o valor solicitado pela view.
     * @param index Célula consultada.
     * @param role Papel Qt do dado solicitado.
     * @return Valor correspondente para exibição, edição ou alinhamento.
     */
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Informa o cabeçalho das colunas e linhas.
     * @param section Índice da seção consultada.
     * @param orientation Orientação do cabeçalho.
     * @param role Papel Qt solicitado.
     * @return Texto do cabeçalho correspondente.
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    /**
     * @brief Define as células editáveis do catálogo.
     * @param index Célula consultada.
     * @return Flags Qt compatíveis com a célula.
     */
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    /**
     * @brief Atualiza um parâmetro editável do revestimento.
     * @param index Célula alterada.
     * @param value Novo valor informado.
     * @param role Papel Qt da alteração.
     * @return true quando a alteração foi aplicada.
     */
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    /**
     * @brief Restaura o catálogo padrão do domínio.
     */
    void carregarPadrao();

    /**
     * @brief Substitui o catálogo atual.
     * @param revestimentos Novo conjunto de revestimentos.
     */
    void definirRevestimentos(const QVector<RevestimentoCanal>& revestimentos);

    /**
     * @brief Retorna o catálogo carregado no modelo.
     * @return Vetor com os revestimentos atuais.
     */
    const QVector<RevestimentoCanal>& revestimentos() const;

    /**
     * @brief Procura um revestimento pelo nome.
     * @param nome Nome procurado.
     * @return Ponteiro constante para o revestimento, ou nulo quando não encontrado.
     */
    const RevestimentoCanal* revestimentoPorNome(const QString& nome) const;

    /**
     * @brief Retorna os nomes disponíveis para combo boxes.
     * @return Lista de nomes do catálogo atual.
     */
    QStringList nomesDisponiveis() const;

signals:
    void dadosAlterados();

private:
    QVector<RevestimentoCanal> m_revestimentos;
};
