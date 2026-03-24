#include "DelegadoGeometriaSecaoComboBox.h"

#include <QComboBox>

DelegadoGeometriaSecaoComboBox::DelegadoGeometriaSecaoComboBox(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

QWidget* DelegadoGeometriaSecaoComboBox::createEditor(QWidget* parent,
                                                       const QStyleOptionViewItem&,
                                                       const QModelIndex&) const
{
    auto* combo = new QComboBox(parent);
    combo->addItem("Trapezoidal");
    combo->addItem("Semicircular");
    return combo;
}

void DelegadoGeometriaSecaoComboBox::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* combo = qobject_cast<QComboBox*>(editor);
    if (!combo) return;

    const QString valorAtual = index.data(Qt::EditRole).toString();
    const int indice = combo->findText(valorAtual, Qt::MatchFixedString);
    combo->setCurrentIndex(indice >= 0 ? indice : 0);
}

void DelegadoGeometriaSecaoComboBox::setModelData(QWidget* editor,
                                                   QAbstractItemModel* model,
                                                   const QModelIndex& index) const
{
    auto* combo = qobject_cast<QComboBox*>(editor);
    if (!combo || !model) return;

    model->setData(index, combo->currentText(), Qt::EditRole);
}
