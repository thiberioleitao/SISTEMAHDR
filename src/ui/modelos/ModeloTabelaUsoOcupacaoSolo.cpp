#include "ModeloTabelaUsoOcupacaoSolo.h"

#include <algorithm>

ModeloTabelaUsoOcupacaoSolo::ModeloTabelaUsoOcupacaoSolo(QObject* parent)
    : QAbstractTableModel(parent)
{
    carregarPadrao();
}

int ModeloTabelaUsoOcupacaoSolo::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return m_tiposUsoOcupacao.size();
}

int ModeloTabelaUsoOcupacaoSolo::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return TotalColunas;
}

QVariant ModeloTabelaUsoOcupacaoSolo::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return QVariant();
    if (index.row() < 0 || index.row() >= m_tiposUsoOcupacao.size()) return QVariant();

    const TipoUsoOcupacaoSolo& tipo = m_tiposUsoOcupacao.at(index.row());

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case ColunaId: return tipo.id();
        case ColunaNManning: return QString::number(tipo.nManning(), 'f', 4);
        case ColunaCRunoff: return QString::number(tipo.cRunoff(), 'f', 3);
        case ColunaCurveNumber: return QString::number(tipo.curveNumber(), 'f', 2);
        case ColunaFatorR: return QString::number(tipo.fatorR(), 'f', 2);
        case ColunaFatorK: return QString::number(tipo.fatorK(), 'f', 4);
        case ColunaFatorC: return QString::number(tipo.fatorC(), 'f', 4);
        case ColunaFatorP: return QString::number(tipo.fatorP(), 'f', 4);
        case ColunaDensidade: return QString::number(tipo.densidadeKgM3(), 'f', 2);
        default: return QVariant();
        }
    }

    if (role == Qt::TextAlignmentRole) {
        if (index.column() >= ColunaNManning) {
            return Qt::AlignCenter;
        }
    }

    return QVariant();
}

QVariant ModeloTabelaUsoOcupacaoSolo::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case ColunaId: return "ID";
        case ColunaNManning: return "n_Manning";
        case ColunaCRunoff: return "C (runoff)";
        case ColunaCurveNumber: return "CN (curve number)";
        case ColunaFatorR: return "R (MJ.mm/ha.h.ano)";
        case ColunaFatorK: return "K (Mg.ha.h/(ha.MJ.mm))";
        case ColunaFatorC: return "C";
        case ColunaFatorP: return "P";
        case ColunaDensidade: return "rho (kg/m3)";
        default: return QVariant();
        }
    }

    return section + 1;
}

Qt::ItemFlags ModeloTabelaUsoOcupacaoSolo::flags(const QModelIndex& index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

bool ModeloTabelaUsoOcupacaoSolo::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::EditRole) return false;
    if (!index.isValid()) return false;
    if (index.row() < 0 || index.row() >= m_tiposUsoOcupacao.size()) return false;

    TipoUsoOcupacaoSolo& tipo = m_tiposUsoOcupacao[index.row()];

    switch (index.column()) {
    case ColunaId: {
        const QString idNovo = value.toString().trimmed();
        if (idNovo.isEmpty()) return false;
        if (existeId(idNovo, index.row())) return false;
        tipo.setId(idNovo);
        break;
    }
    case ColunaNManning:
        tipo.setNManning(std::max(0.0, value.toDouble()));
        break;
    case ColunaCRunoff:
        tipo.setCRunoff(std::max(0.0, value.toDouble()));
        break;
    case ColunaCurveNumber:
        tipo.setCurveNumber(std::max(0.0, value.toDouble()));
        break;
    case ColunaFatorR:
        tipo.setFatorR(std::max(0.0, value.toDouble()));
        break;
    case ColunaFatorK:
        tipo.setFatorK(std::max(0.0, value.toDouble()));
        break;
    case ColunaFatorC:
        tipo.setFatorC(std::max(0.0, value.toDouble()));
        break;
    case ColunaFatorP:
        tipo.setFatorP(std::max(0.0, value.toDouble()));
        break;
    case ColunaDensidade:
        tipo.setDensidadeKgM3(std::max(0.0, value.toDouble()));
        break;
    default:
        return false;
    }

    emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
    emit dadosAlterados();
    return true;
}

const QVector<TipoUsoOcupacaoSolo>& ModeloTabelaUsoOcupacaoSolo::tiposUsoOcupacao() const
{
    return m_tiposUsoOcupacao;
}

void ModeloTabelaUsoOcupacaoSolo::definirTiposUsoOcupacao(const QVector<TipoUsoOcupacaoSolo>& tipos)
{
    beginResetModel();
    m_tiposUsoOcupacao = tipos;
    endResetModel();
    emit dadosAlterados();
}

void ModeloTabelaUsoOcupacaoSolo::carregarPadrao()
{
    beginResetModel();

    TipoUsoOcupacaoSolo residencial("Residencial");
    residencial.setNManning(0.0150);
    residencial.setCRunoff(0.60);
    residencial.setCurveNumber(85.0);
    residencial.setFatorR(0.0);
    residencial.setFatorK(0.0);
    residencial.setFatorC(0.0);
    residencial.setFatorP(0.0);
    residencial.setDensidadeKgM3(1000.0);

    TipoUsoOcupacaoSolo comercial("Comercial");
    comercial.setNManning(0.0140);
    comercial.setCRunoff(0.80);
    comercial.setCurveNumber(92.0);
    comercial.setFatorR(0.0);
    comercial.setFatorK(0.0);
    comercial.setFatorC(0.0);
    comercial.setFatorP(0.0);
    comercial.setDensidadeKgM3(1000.0);

    TipoUsoOcupacaoSolo areaVerde("Area verde");
    areaVerde.setNManning(0.0300);
    areaVerde.setCRunoff(0.30);
    areaVerde.setCurveNumber(70.0);
    areaVerde.setFatorR(0.0);
    areaVerde.setFatorK(0.0);
    areaVerde.setFatorC(0.0);
    areaVerde.setFatorP(0.0);
    areaVerde.setDensidadeKgM3(1000.0);

    m_tiposUsoOcupacao = { residencial, comercial, areaVerde };

    endResetModel();
}

bool ModeloTabelaUsoOcupacaoSolo::existeId(const QString& id, int linhaIgnorada) const
{
    const QString idNormalizado = id.trimmed();
    for (int i = 0; i < m_tiposUsoOcupacao.size(); ++i) {
        if (i == linhaIgnorada) continue;
        if (m_tiposUsoOcupacao.at(i).id().compare(idNormalizado, Qt::CaseInsensitive) == 0) {
            return true;
        }
    }

    return false;
}

const TipoUsoOcupacaoSolo* ModeloTabelaUsoOcupacaoSolo::tipoPorId(const QString& id) const
{
    const QString idNormalizado = id.trimmed();
    for (const TipoUsoOcupacaoSolo& tipo : m_tiposUsoOcupacao) {
        if (tipo.id().compare(idNormalizado, Qt::CaseInsensitive) == 0) {
            return &tipo;
        }
    }

    return nullptr;
}

QStringList ModeloTabelaUsoOcupacaoSolo::idsDisponiveis() const
{
    QStringList ids;
    ids.reserve(m_tiposUsoOcupacao.size());

    for (const TipoUsoOcupacaoSolo& tipo : m_tiposUsoOcupacao) {
        if (!tipo.id().trimmed().isEmpty()) {
            ids.append(tipo.id().trimmed());
        }
    }

    return ids;
}
