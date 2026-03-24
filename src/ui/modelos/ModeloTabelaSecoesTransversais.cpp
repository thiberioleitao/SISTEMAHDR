#include "ModeloTabelaSecoesTransversais.h"

namespace
{
bool ehGeometriaSemicircular(const QString& geometria)
{
    return geometria.trimmed().compare("Semicircular", Qt::CaseInsensitive) == 0;
}

QString normalizarGeometria(const QString& geometria)
{
    const QString texto = geometria.trimmed();
    if (texto.compare("Semicircular", Qt::CaseInsensitive) == 0) {
        return "Semicircular";
    }
    return "Trapezoidal";
}
}

ModeloTabelaSecoesTransversais::ModeloTabelaSecoesTransversais(QObject* parent)
    : QAbstractTableModel(parent)
{
    carregarPadrao();
}

int ModeloTabelaSecoesTransversais::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return m_registros.size();
}

int ModeloTabelaSecoesTransversais::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return TotalColunas;
}

QVariant ModeloTabelaSecoesTransversais::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return QVariant();
    if (index.row() < 0 || index.row() >= m_registros.size()) return QVariant();

    const RegistroSecaoTransversal& registro = m_registros.at(index.row());

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case ColunaNomenclatura: return registro.nomenclatura;
        case ColunaGeometria: return registro.geometria;
        case ColunaBaseMenor: return registro.baseMenorM;
        case ColunaTalude: return registro.talude;
        case ColunaDiametro: return registro.diametroM;
        default: return QVariant();
        }
    }

    if (role == Qt::TextAlignmentRole) {
        if (index.column() >= ColunaBaseMenor) {
            return Qt::AlignCenter;
        }
    }

    return QVariant();
}

QVariant ModeloTabelaSecoesTransversais::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case ColunaNomenclatura: return "Nomenclatura";
        case ColunaGeometria: return "Geometria";
        case ColunaBaseMenor: return "b_menor (m)";
        case ColunaTalude: return "Talude / m";
        case ColunaDiametro: return "D (m)";
        default: return QVariant();
        }
    }

    return section + 1;
}

Qt::ItemFlags ModeloTabelaSecoesTransversais::flags(const QModelIndex& index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;

    Qt::ItemFlags flagsBase = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (index.row() < 0 || index.row() >= m_registros.size()) return flagsBase;

    const RegistroSecaoTransversal& registro = m_registros.at(index.row());
    const bool semicircular = ehGeometriaSemicircular(registro.geometria);

    if (index.column() == ColunaDiametro && !semicircular) {
        return flagsBase;
    }

    if ((index.column() == ColunaBaseMenor || index.column() == ColunaTalude) && semicircular) {
        return flagsBase;
    }

    return flagsBase | Qt::ItemIsEditable;
}

bool ModeloTabelaSecoesTransversais::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::EditRole) return false;
    if (!index.isValid()) return false;
    if (index.row() < 0 || index.row() >= m_registros.size()) return false;

    RegistroSecaoTransversal& registro = m_registros[index.row()];

    switch (index.column()) {
    case ColunaNomenclatura:
        registro.nomenclatura = value.toString().trimmed();
        break;
    case ColunaGeometria: {
        registro.geometria = normalizarGeometria(value.toString());

        // Ajusta campos compatíveis com a geometria selecionada.
        if (ehGeometriaSemicircular(registro.geometria)) {
            registro.baseMenorM = "-";
            registro.talude = "-";
            if (registro.diametroM.trimmed().isEmpty() || registro.diametroM.trimmed() == "-") {
                registro.diametroM = "0.50";
            }
        }
        else {
            if (registro.baseMenorM.trimmed().isEmpty() || registro.baseMenorM.trimmed() == "-") {
                registro.baseMenorM = "0.50";
            }
            if (registro.talude.trimmed().isEmpty() || registro.talude.trimmed() == "-") {
                registro.talude = "1.00";
            }
            registro.diametroM = "-";
        }
        break;
    }
    case ColunaBaseMenor:
        if (ehGeometriaSemicircular(registro.geometria)) return false;
        registro.baseMenorM = value.toString().trimmed();
        break;
    case ColunaTalude:
        if (ehGeometriaSemicircular(registro.geometria)) return false;
        registro.talude = value.toString().trimmed();
        break;
    case ColunaDiametro:
        if (!ehGeometriaSemicircular(registro.geometria)) return false;
        registro.diametroM = value.toString().trimmed();
        break;
    default: return false;
    }

    if (index.column() == ColunaGeometria) {
        const QModelIndex inicioLinha = this->index(index.row(), 0);
        const QModelIndex fimLinha = this->index(index.row(), TotalColunas - 1);
        emit dataChanged(inicioLinha, fimLinha, { Qt::DisplayRole, Qt::EditRole });
    }
    else {
        emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
    }

    return true;
}

void ModeloTabelaSecoesTransversais::carregarPadrao()
{
    beginResetModel();
    m_registros = {
        {"VTD-1", "Trapezoidal", "0.30", "1.00", "-"},
        {"VTD-2", "Trapezoidal", "0.50", "1.00", "-"},
        {"CTD-1", "Trapezoidal", "0.50", "1.00", "-"},
        {"CSD-1", "Semicircular", "-", "-", "0.40"},
        {"CSD-2", "Semicircular", "-", "-", "0.50"},
        {"CSD-3", "Semicircular", "-", "-", "0.60"},
        {"CSD-4", "Semicircular", "-", "-", "0.80"},
        {"CSD-5", "Semicircular", "-", "-", "1.00"}
    };
    endResetModel();
}

const QVector<RegistroSecaoTransversal>& ModeloTabelaSecoesTransversais::registros() const
{
    return m_registros;
}

void ModeloTabelaSecoesTransversais::definirRegistros(const QVector<RegistroSecaoTransversal>& registros)
{
    beginResetModel();
    m_registros = registros;
    endResetModel();
}
