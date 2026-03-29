#include "ModeloTabelaRevestimentos.h"

#include <algorithm>

ModeloTabelaRevestimentos::ModeloTabelaRevestimentos(QObject* parent)
    : QAbstractTableModel(parent)
{
    carregarPadrao();
}

int ModeloTabelaRevestimentos::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return m_revestimentos.size();
}

int ModeloTabelaRevestimentos::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return TotalColunas;
}

QVariant ModeloTabelaRevestimentos::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return QVariant();
    if (index.row() < 0 || index.row() >= m_revestimentos.size()) return QVariant();

    const RevestimentoCanal& revestimento = m_revestimentos.at(index.row());

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case ColunaNome: return revestimento.nome();
        case ColunaCoeficienteManning: return QString::number(revestimento.coeficienteManning(), 'f', 4);
        case ColunaVelocidadeMaxima: return QString::number(revestimento.velocidadeMaximaAdmissivelMps(), 'f', 3);
        case ColunaTensaoCisalhamentoMaxima: return QString::number(revestimento.tensaoCisalhamentoMaximaAdmissivelPa(), 'f', 2);
        case ColunaEspessura: return QString::number(revestimento.espessura(), 'f', 3);
        default: return QVariant();
        }
    }

    if (role == Qt::TextAlignmentRole && index.column() != ColunaNome) {
        return Qt::AlignCenter;
    }

    if (role == Qt::ToolTipRole) {
        switch (index.column()) {
        case ColunaNome:
            return "Identificador do revestimento usado pelas seções.";
        case ColunaCoeficienteManning:
            return "Coeficiente de rugosidade hidráulica usado na fórmula de Manning.";
        case ColunaVelocidadeMaxima:
            return "Velocidade máxima admissível para verificação hidráulica.";
        case ColunaTensaoCisalhamentoMaxima:
            return "Tensão de cisalhamento máxima admissível do revestimento.";
        case ColunaEspessura:
            return "Parâmetro complementar do revestimento mantido para expansão futura.";
        default:
            return QVariant();
        }
    }

    return QVariant();
}

QVariant ModeloTabelaRevestimentos::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case ColunaNome: return "Revestimento";
        case ColunaCoeficienteManning: return "n_Manning";
        case ColunaVelocidadeMaxima: return "Vmax adm. (m/s)";
        case ColunaTensaoCisalhamentoMaxima: return "Tau max. (Pa)";
        case ColunaEspessura: return "Espessura (m)";
        default: return QVariant();
        }
    }

    return section + 1;
}

Qt::ItemFlags ModeloTabelaRevestimentos::flags(const QModelIndex& index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;

    Qt::ItemFlags flagsBase = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (index.column() == ColunaNome) {
        return flagsBase;
    }

    return flagsBase | Qt::ItemIsEditable;
}

bool ModeloTabelaRevestimentos::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::EditRole) return false;
    if (!index.isValid()) return false;
    if (index.row() < 0 || index.row() >= m_revestimentos.size()) return false;

    RevestimentoCanal& revestimento = m_revestimentos[index.row()];

    switch (index.column()) {
    case ColunaCoeficienteManning:
        revestimento.setCoeficienteManning(std::max(0.0, value.toDouble()));
        break;
    case ColunaVelocidadeMaxima:
        revestimento.setVelocidadeMaximaAdmissivelMps(std::max(0.0, value.toDouble()));
        break;
    case ColunaTensaoCisalhamentoMaxima:
        revestimento.setTensaoCisalhamentoMaximaAdmissivelPa(std::max(0.0, value.toDouble()));
        break;
    case ColunaEspessura:
        revestimento.setEspessura(std::max(0.0, value.toDouble()));
        break;
    default:
        return false;
    }

    emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole, Qt::ToolTipRole });
    emit dadosAlterados();
    return true;
}

void ModeloTabelaRevestimentos::carregarPadrao()
{
    beginResetModel();
    m_revestimentos = RevestimentoCanal::revestimentosPadrao();
    endResetModel();
    emit dadosAlterados();
}

void ModeloTabelaRevestimentos::definirRevestimentos(const QVector<RevestimentoCanal>& revestimentos)
{
    beginResetModel();
    m_revestimentos = revestimentos;
    endResetModel();
    emit dadosAlterados();
}

const QVector<RevestimentoCanal>& ModeloTabelaRevestimentos::revestimentos() const
{
    return m_revestimentos;
}

const RevestimentoCanal* ModeloTabelaRevestimentos::revestimentoPorNome(const QString& nome) const
{
    const QString nomeNormalizado = nome.trimmed();
    for (const RevestimentoCanal& revestimento : m_revestimentos) {
        if (revestimento.nome().compare(nomeNormalizado, Qt::CaseInsensitive) == 0) {
            return &revestimento;
        }
    }

    return nullptr;
}

QStringList ModeloTabelaRevestimentos::nomesDisponiveis() const
{
    QStringList nomes;
    nomes.reserve(m_revestimentos.size());

    for (const RevestimentoCanal& revestimento : m_revestimentos) {
        if (!revestimento.nome().trimmed().isEmpty()) {
            nomes.append(revestimento.nome().trimmed());
        }
    }

    return nomes;
}
