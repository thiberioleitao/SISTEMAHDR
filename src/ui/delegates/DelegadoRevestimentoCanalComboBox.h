#pragma once

#include <QPointer>
#include <QStyledItemDelegate>

class ModeloTabelaSecoesTransversais;

/**
 * @brief Delegate que exibe os revestimentos padrao em uma combobox.
 */
class DelegadoRevestimentoCanalComboBox : public QStyledItemDelegate
{
    Q_OBJECT

public:
    /**
     * @brief Cria o delegate vinculado ao modelo de secoes.
     * @param modeloSecoes Modelo consultado para listar os revestimentos.
     * @param parent Objeto pai do delegate.
     */
    explicit DelegadoRevestimentoCanalComboBox(ModeloTabelaSecoesTransversais* modeloSecoes,
                                               QObject* parent = nullptr);

    /**
     * @brief Cria o editor combobox para a celula selecionada.
     * @param parent Widget pai do editor.
     * @param option Opcao de estilo da view.
     * @param index Celula em edicao.
     * @return Widget editor configurado.
     */
    QWidget* createEditor(QWidget* parent,
                          const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;

    /**
     * @brief Sincroniza o valor atual da celula com a combobox.
     * @param editor Widget editor criado pelo delegate.
     * @param index Celula em edicao.
     */
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;

    /**
     * @brief Persiste no modelo o revestimento escolhido na combobox.
     * @param editor Widget editor criado pelo delegate.
     * @param model Modelo Qt que recebera o novo valor.
     * @param index Celula editada.
     */
    void setModelData(QWidget* editor,
                      QAbstractItemModel* model,
                      const QModelIndex& index) const override;

private:
    QPointer<ModeloTabelaSecoesTransversais> m_modeloSecoes;
};
