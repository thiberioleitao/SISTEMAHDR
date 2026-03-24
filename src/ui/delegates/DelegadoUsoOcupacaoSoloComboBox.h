#pragma once

#include <QPointer>
#include <QStyledItemDelegate>

class ModeloTabelaUsoOcupacaoSolo;

class DelegadoUsoOcupacaoSoloComboBox : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit DelegadoUsoOcupacaoSoloComboBox(ModeloTabelaUsoOcupacaoSolo* modeloUsoOcupacao,
                                             QObject* parent = nullptr);

    QWidget* createEditor(QWidget* parent,
                          const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;

    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor,
                      QAbstractItemModel* model,
                      const QModelIndex& index) const override;

private:
    QPointer<ModeloTabelaUsoOcupacaoSolo> m_modeloUsoOcupacao;
};
