#include "DelegadoRevestimentoCanalComboBox.h"

#include "ui/modelos/ModeloTabelaSecoesTransversais.h"

#include <QComboBox>

DelegadoRevestimentoCanalComboBox::DelegadoRevestimentoCanalComboBox(ModeloTabelaSecoesTransversais* modeloSecoes,
                                                                     QObject* parent)
    : QStyledItemDelegate(parent)
    , m_modeloSecoes(modeloSecoes)
{
}

QWidget* DelegadoRevestimentoCanalComboBox::createEditor(QWidget* parent,
                                                         const QStyleOptionViewItem&,
                                                         const QModelIndex&) const
{
    auto* combo = new QComboBox(parent);

    if (m_modeloSecoes) {
        combo->addItems(m_modeloSecoes->nomesRevestimentosDisponiveis());
    }

    return combo;
}

void DelegadoRevestimentoCanalComboBox::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* combo = qobject_cast<QComboBox*>(editor);
    if (!combo) return;

    const QString valorAtual = index.data(Qt::EditRole).toString();
    const int indice = combo->findText(valorAtual);
    combo->setCurrentIndex(indice >= 0 ? indice : 0);
}

void DelegadoRevestimentoCanalComboBox::setModelData(QWidget* editor,
                                                     QAbstractItemModel* model,
                                                     const QModelIndex& index) const
{
    auto* combo = qobject_cast<QComboBox*>(editor);
    if (!combo || !model) return;

    model->setData(index, combo->currentText(), Qt::EditRole);
}
