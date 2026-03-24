#pragma once

#include <QStyledItemDelegate>

class DelegadoGeometriaSecaoComboBox : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit DelegadoGeometriaSecaoComboBox(QObject* parent = nullptr);

    QWidget* createEditor(QWidget* parent,
                          const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;

    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor,
                      QAbstractItemModel* model,
                      const QModelIndex& index) const override;
};
