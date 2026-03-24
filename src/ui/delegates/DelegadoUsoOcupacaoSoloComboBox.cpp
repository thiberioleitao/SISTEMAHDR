#include "DelegadoUsoOcupacaoSoloComboBox.h"

#include "ui/modelos/ModeloTabelaUsoOcupacaoSolo.h"

#include <QComboBox>

DelegadoUsoOcupacaoSoloComboBox::DelegadoUsoOcupacaoSoloComboBox(ModeloTabelaUsoOcupacaoSolo* modeloUsoOcupacao,
                                                                 QObject* parent)
    : QStyledItemDelegate(parent)
    , m_modeloUsoOcupacao(modeloUsoOcupacao)
{
}

QWidget* DelegadoUsoOcupacaoSoloComboBox::createEditor(QWidget* parent,
                                                        const QStyleOptionViewItem&,
                                                        const QModelIndex&) const
{
    auto* combo = new QComboBox(parent);

    if (m_modeloUsoOcupacao) {
        combo->addItems(m_modeloUsoOcupacao->idsDisponiveis());
    }

    return combo;
}

void DelegadoUsoOcupacaoSoloComboBox::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* combo = qobject_cast<QComboBox*>(editor);
    if (!combo) return;

    const QString valorAtual = index.data(Qt::EditRole).toString();
    const int indice = combo->findText(valorAtual);
    combo->setCurrentIndex(indice >= 0 ? indice : 0);
}

void DelegadoUsoOcupacaoSoloComboBox::setModelData(QWidget* editor,
                                                    QAbstractItemModel* model,
                                                    const QModelIndex& index) const
{
    auto* combo = qobject_cast<QComboBox*>(editor);
    if (!combo || !model) return;

    model->setData(index, combo->currentText(), Qt::EditRole);
}
