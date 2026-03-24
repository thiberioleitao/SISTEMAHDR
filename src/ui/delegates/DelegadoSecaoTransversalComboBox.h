#pragma once

#include <QPointer>
#include <QStyledItemDelegate>

class ModeloTabelaSecoesTransversais;

class DelegadoSecaoTransversalComboBox : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit DelegadoSecaoTransversalComboBox(ModeloTabelaSecoesTransversais* modeloSecoes,
                                              QObject* parent = nullptr);

    QWidget* createEditor(QWidget* parent,
                          const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;

    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor,
                      QAbstractItemModel* model,
                      const QModelIndex& index) const override;

private:
    QPointer<ModeloTabelaSecoesTransversais> m_modeloSecoes;
};
