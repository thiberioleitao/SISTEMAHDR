#include "DelegadoSecaoTransversalComboBox.h"

#include "ui/modelos/ModeloTabelaSecoesTransversais.h"

#include <QComboBox>

DelegadoSecaoTransversalComboBox::DelegadoSecaoTransversalComboBox(ModeloTabelaSecoesTransversais* modeloSecoes,
                                                                   QObject* parent)
    : QStyledItemDelegate(parent)
    , m_modeloSecoes(modeloSecoes)
{
}

QWidget* DelegadoSecaoTransversalComboBox::createEditor(QWidget* parent,
                                                        const QStyleOptionViewItem&,
                                                        const QModelIndex&) const
{
    auto* combo = new QComboBox(parent);

    if (m_modeloSecoes) {
        const int totalLinhas = m_modeloSecoes->rowCount();
        for (int linha = 0; linha < totalLinhas; ++linha) {
            const QString idSecao = m_modeloSecoes->index(linha, ModeloTabelaSecoesTransversais::ColunaNomenclatura)
                                       .data(Qt::DisplayRole)
                                       .toString();
            if (!idSecao.trimmed().isEmpty()) {
                combo->addItem(idSecao.trimmed());
            }
        }
    }

    return combo;
}

void DelegadoSecaoTransversalComboBox::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* combo = qobject_cast<QComboBox*>(editor);
    if (!combo) return;

    const QString valorAtual = index.data(Qt::EditRole).toString();
    const int indice = combo->findText(valorAtual);
    combo->setCurrentIndex(indice >= 0 ? indice : 0);
}

void DelegadoSecaoTransversalComboBox::setModelData(QWidget* editor,
                                                    QAbstractItemModel* model,
                                                    const QModelIndex& index) const
{
    auto* combo = qobject_cast<QComboBox*>(editor);
    if (!combo || !model) return;

    model->setData(index, combo->currentText(), Qt::EditRole);
}
