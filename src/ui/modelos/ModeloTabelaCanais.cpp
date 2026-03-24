#include "ModeloTabelaCanais.h"

#include <algorithm>

ModeloTabelaCanais::ModeloTabelaCanais(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int ModeloTabelaCanais::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return m_canais.size();
}

int ModeloTabelaCanais::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return TotalColunas;
}

QVariant ModeloTabelaCanais::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return QVariant();
    if (index.row() < 0 || index.row() >= m_canais.size()) return QVariant();

    const Canal& canal = m_canais.at(index.row());

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case ColunaId: return canal.id();
        case ColunaIdJusante: return canal.idJusante();
        case ColunaSecaoTransversal: return m_idsSecaoPorLinha.value(index.row(), QString("VTD-1"));
        case ColunaComprimento: return QString::number(canal.comprimento(), 'f', 2);
        case ColunaDeclividadeFinal: return QString::number(canal.declividadeFinal(), 'f', 4);
        case ColunaDeclividadeFinalPercentual: return QString::number(canal.declividadeFinal() * 100.0, 'f', 2);
        case ColunaDeclividadeMinima: return QString::number(canal.declividadeMinima(), 'f', 4);
        case ColunaDeclividadeMinimaPercentual: return QString::number(canal.declividadeMinima() * 100.0, 'f', 2);
        case ColunaDeclividadeMaxima: return QString::number(canal.declividadeMaxima(), 'f', 4);
        case ColunaDeclividadeMaximaPercentual: return QString::number(canal.declividadeMaxima() * 100.0, 'f', 2);
        default: return QVariant();
        }
    }

    if (role == Qt::TextAlignmentRole) {
        if (index.column() >= ColunaComprimento) {
            return Qt::AlignCenter;
        }
    }

    return QVariant();
}

QVariant ModeloTabelaCanais::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case ColunaId: return "ID";
        case ColunaIdJusante: return "ID Jusante";
        case ColunaSecaoTransversal: return "Se\u00E7\u00E3o transversal";
        case ColunaComprimento: return "Comprimento (m)";
        case ColunaDeclividadeFinal: return "Declividade Final (m/m)";
        case ColunaDeclividadeFinalPercentual: return "Declividade Final (%)";
        case ColunaDeclividadeMinima: return "Declividade Min. (m/m)";
        case ColunaDeclividadeMinimaPercentual: return "Declividade Min. (%)";
        case ColunaDeclividadeMaxima: return "Declividade Max. (m/m)";
        case ColunaDeclividadeMaximaPercentual: return "Declividade Max. (%)";
        default: return QVariant();
        }
    }

    return section + 1;
}

Qt::ItemFlags ModeloTabelaCanais::flags(const QModelIndex& index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

bool ModeloTabelaCanais::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::EditRole) return false;
    if (!index.isValid()) return false;
    if (index.row() < 0 || index.row() >= m_canais.size()) return false;

    Canal& canal = m_canais[index.row()];

    switch (index.column()) {
    case ColunaId: {
        const QString novoId = value.toString().trimmed();
        if (novoId.isEmpty()) return false;
        if (existeId(novoId, index.row())) return false;
        canal.setId(novoId);
        break;
    }
    case ColunaIdJusante:
        canal.setIdJusante(value.toString());
        break;
    case ColunaSecaoTransversal:
        m_idsSecaoPorLinha[index.row()] = value.toString().trimmed();
        break;
    case ColunaComprimento:
        canal.setComprimento(std::max(0.0, value.toDouble()));
        break;
    case ColunaDeclividadeFinal:
        canal.setDeclividadeFinal(std::max(0.0, value.toDouble()));
        break;
    case ColunaDeclividadeFinalPercentual:
        canal.setDeclividadeFinal(std::max(0.0, value.toDouble()) / 100.0);
        break;
    case ColunaDeclividadeMinima:
        canal.setDeclividadeMinima(std::max(0.0, value.toDouble()));
        break;
    case ColunaDeclividadeMinimaPercentual:
        canal.setDeclividadeMinima(std::max(0.0, value.toDouble()) / 100.0);
        break;
    case ColunaDeclividadeMaxima:
        canal.setDeclividadeMaxima(std::max(0.0, value.toDouble()));
        break;
    case ColunaDeclividadeMaximaPercentual:
        canal.setDeclividadeMaxima(std::max(0.0, value.toDouble()) / 100.0);
        break;
    default:
        return false;
    }

    emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });

    if (index.column() == ColunaDeclividadeFinal) {
        const QModelIndex indicePercentual = this->index(index.row(), ColunaDeclividadeFinalPercentual);
        emit dataChanged(indicePercentual, indicePercentual, { Qt::DisplayRole, Qt::EditRole });
    }
    if (index.column() == ColunaDeclividadeFinalPercentual) {
        const QModelIndex indiceMm = this->index(index.row(), ColunaDeclividadeFinal);
        emit dataChanged(indiceMm, indiceMm, { Qt::DisplayRole, Qt::EditRole });
    }

    if (index.column() == ColunaDeclividadeMinima) {
        const QModelIndex indicePercentual = this->index(index.row(), ColunaDeclividadeMinimaPercentual);
        emit dataChanged(indicePercentual, indicePercentual, { Qt::DisplayRole, Qt::EditRole });
    }
    if (index.column() == ColunaDeclividadeMinimaPercentual) {
        const QModelIndex indiceMm = this->index(index.row(), ColunaDeclividadeMinima);
        emit dataChanged(indiceMm, indiceMm, { Qt::DisplayRole, Qt::EditRole });
    }

    if (index.column() == ColunaDeclividadeMaxima) {
        const QModelIndex indicePercentual = this->index(index.row(), ColunaDeclividadeMaximaPercentual);
        emit dataChanged(indicePercentual, indicePercentual, { Qt::DisplayRole, Qt::EditRole });
    }
    if (index.column() == ColunaDeclividadeMaximaPercentual) {
        const QModelIndex indiceMm = this->index(index.row(), ColunaDeclividadeMaxima);
        emit dataChanged(indiceMm, indiceMm, { Qt::DisplayRole, Qt::EditRole });
    }

    emit dadosAlterados();
    return true;
}

void ModeloTabelaCanais::definirCanais(const QVector<Canal>& canais)
{
    beginResetModel();
    m_canais = canais;
    m_idsSecaoPorLinha.resize(m_canais.size());
    for (int i = 0; i < m_idsSecaoPorLinha.size(); ++i) {
        if (m_idsSecaoPorLinha[i].trimmed().isEmpty()) {
            m_idsSecaoPorLinha[i] = "VTD-1";
        }
    }
    endResetModel();
    emit dadosAlterados();
}

const QVector<Canal>& ModeloTabelaCanais::canais() const
{
    return m_canais;
}

bool ModeloTabelaCanais::adicionarCanal(const QString& idCanal, QString* mensagemErro)
{
    const QString id = idCanal.trimmed();
    if (id.isEmpty()) {
        if (mensagemErro) *mensagemErro = "O ID do canal não pode ser vazio.";
        return false;
    }

    if (existeId(id)) {
        if (mensagemErro) *mensagemErro = "ID de canal já existe.";
        return false;
    }

    const int linhaNova = m_canais.size();
    beginInsertRows(QModelIndex(), linhaNova, linhaNova);
    m_canais.append(Canal(id));
    m_idsSecaoPorLinha.append("VTD-1");
    endInsertRows();

    emit dadosAlterados();
    return true;
}

bool ModeloTabelaCanais::removerCanal(int linha, QString* mensagemErro)
{
    if (linha < 0 || linha >= m_canais.size()) {
        if (mensagemErro) *mensagemErro = "Linha inválida para remoção de canal.";
        return false;
    }

    beginRemoveRows(QModelIndex(), linha, linha);
    m_canais.removeAt(linha);
    if (linha >= 0 && linha < m_idsSecaoPorLinha.size()) {
        m_idsSecaoPorLinha.removeAt(linha);
    }
    endRemoveRows();

    emit dadosAlterados();
    return true;
}

QString ModeloTabelaCanais::idSecaoPorLinha(int linha) const
{
    if (linha < 0 || linha >= m_idsSecaoPorLinha.size()) {
        return QString();
    }

    return m_idsSecaoPorLinha.at(linha);
}

bool ModeloTabelaCanais::existeId(const QString& idCanal, int linhaIgnorada) const
{
    const QString id = idCanal.trimmed();
    for (int i = 0; i < m_canais.size(); ++i) {
        if (i == linhaIgnorada) continue;

        const QString idAtual = m_canais.at(i).id().trimmed();
        if (idAtual.compare(id, Qt::CaseInsensitive) == 0) {
            return true;
        }
    }

    return false;
}
