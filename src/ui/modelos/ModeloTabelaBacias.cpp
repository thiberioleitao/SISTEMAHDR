#include "ModeloTabelaBacias.h"

#include "ModeloTabelaUsoOcupacaoSolo.h"

#include <algorithm>

namespace
{
QString normalizarTexto(const QString& texto)
{
    return texto.trimmed();
}
}

ModeloTabelaBacias::ModeloTabelaBacias(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int ModeloTabelaBacias::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return m_bacias.size();
}

int ModeloTabelaBacias::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return TotalColunas;
}

QVariant ModeloTabelaBacias::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return QVariant();
    if (index.row() < 0 || index.row() >= m_bacias.size()) return QVariant();

    const BaciaContribuicao& bacia = m_bacias.at(index.row());

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case ColunaBacia: return m_nomesBaciaPorLinha.value(index.row());
        case ColunaSubbacia: return m_nomesSubbaciaPorLinha.value(index.row());
        case ColunaUsoOcupacaoSolo: return m_idsUsoOcupacaoPorLinha.value(index.row());
        case ColunaIdJusante: return bacia.idJusante();
        case ColunaAreaKm2: return QString::number(bacia.areaKm2(), 'f', 3);
        case ColunaAreaM2: return QString::number(bacia.areaM2(), 'f', 2);
        case ColunaDeclividadeMedia: return QString::number(bacia.declividadeMedia(), 'f', 4);
        case ColunaTalveguePrincipal: return QString::number(bacia.comprimentoTalveguePrincipalKm(), 'f', 3);
        default: return QVariant();
        }
    }

    if (role == Qt::TextAlignmentRole) {
        if (index.column() >= ColunaAreaKm2) {
            return Qt::AlignCenter;
        }
    }

    return QVariant();
}

QVariant ModeloTabelaBacias::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case ColunaBacia: return "Bacia";
        case ColunaSubbacia: return "Subbacia";
        case ColunaUsoOcupacaoSolo: return "Uso/Ocupacao do solo";
        case ColunaIdJusante: return "ID Jusante";
        case ColunaAreaKm2: return "Area (km2)";
        case ColunaAreaM2: return "Area (m2)";
        case ColunaDeclividadeMedia: return "Declividade Media";
        case ColunaTalveguePrincipal: return "Talvegue Principal (km)";
        default: return QVariant();
        }
    }

    return section + 1;
}

Qt::ItemFlags ModeloTabelaBacias::flags(const QModelIndex& index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

bool ModeloTabelaBacias::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::EditRole) return false;
    if (!index.isValid()) return false;
    if (index.row() < 0 || index.row() >= m_bacias.size()) return false;

    BaciaContribuicao& bacia = m_bacias[index.row()];

    switch (index.column()) {
    case ColunaBacia: {
        const QString novoNomeBacia = normalizarTexto(value.toString());
        const QString nomeSubbacia = m_nomesSubbaciaPorLinha.value(index.row());
        if (novoNomeBacia.isEmpty()) return false;
        if (nomeSubbacia.isEmpty()) return false;
        if (existeCombinacao(novoNomeBacia, nomeSubbacia, index.row())) return false;

        m_nomesBaciaPorLinha[index.row()] = novoNomeBacia;
        bacia.setID(criarIdComposto(novoNomeBacia, nomeSubbacia));
        emit dataChanged(this->index(index.row(), ColunaSubbacia),
                         this->index(index.row(), ColunaSubbacia),
                         { Qt::DisplayRole, Qt::EditRole });
        break;
    }
    case ColunaSubbacia: {
        const QString novoNomeSubbacia = normalizarTexto(value.toString());
        const QString nomeBacia = m_nomesBaciaPorLinha.value(index.row());
        if (novoNomeSubbacia.isEmpty()) return false;
        if (nomeBacia.isEmpty()) return false;
        if (existeCombinacao(nomeBacia, novoNomeSubbacia, index.row())) return false;

        m_nomesSubbaciaPorLinha[index.row()] = novoNomeSubbacia;
        bacia.setID(criarIdComposto(nomeBacia, novoNomeSubbacia));
        emit dataChanged(this->index(index.row(), ColunaBacia),
                         this->index(index.row(), ColunaBacia),
                         { Qt::DisplayRole, Qt::EditRole });
        break;
    }
    case ColunaUsoOcupacaoSolo: {
        const QString idUsoOcupacao = normalizarTexto(value.toString());
        m_idsUsoOcupacaoPorLinha[index.row()] = idUsoOcupacao;

        if (m_modeloUsoOcupacaoSolo) {
            if (const TipoUsoOcupacaoSolo* tipo = m_modeloUsoOcupacaoSolo->tipoPorId(idUsoOcupacao)) {
                bacia.setC_10(tipo->cRunoff());
            }
        }
        break;
    }
    case ColunaIdJusante:
        bacia.setIdJusante(value.toString());
        break;
    case ColunaAreaKm2:
        bacia.setAreaKm2(std::max(0.0, value.toDouble()));
        break;
    case ColunaAreaM2:
        bacia.setAreaM2(std::max(0.0, value.toDouble()));
        break;
    case ColunaDeclividadeMedia:
        bacia.setDeclividadeMedia(std::max(0.0, value.toDouble()));
        break;
    case ColunaTalveguePrincipal:
        bacia.setComprimentoTalveguePrincipalKm(std::max(0.0, value.toDouble()));
        break;
    default:
        return false;
    }

    emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });

    if (index.column() == ColunaAreaKm2) {
        const QModelIndex indiceAreaM2 = this->index(index.row(), ColunaAreaM2);
        emit dataChanged(indiceAreaM2, indiceAreaM2, { Qt::DisplayRole, Qt::EditRole });
    }
    if (index.column() == ColunaAreaM2) {
        const QModelIndex indiceAreaKm2 = this->index(index.row(), ColunaAreaKm2);
        emit dataChanged(indiceAreaKm2, indiceAreaKm2, { Qt::DisplayRole, Qt::EditRole });
    }

    emit dadosAlterados();
    return true;
}

void ModeloTabelaBacias::definirBacias(const QVector<BaciaContribuicao>& bacias)
{
    beginResetModel();
    m_bacias = bacias;
    m_nomesBaciaPorLinha.clear();
    m_nomesSubbaciaPorLinha.clear();
    m_idsUsoOcupacaoPorLinha.clear();
    m_nomesBaciaPorLinha.reserve(m_bacias.size());
    m_nomesSubbaciaPorLinha.reserve(m_bacias.size());
    m_idsUsoOcupacaoPorLinha.reserve(m_bacias.size());

    for (const BaciaContribuicao& bacia : m_bacias) {
        QString nomeBacia;
        QString nomeSubbacia;
        decomporIdComposto(bacia.id(), &nomeBacia, &nomeSubbacia);
        if (nomeBacia.isEmpty()) nomeBacia = bacia.id();
        if (nomeSubbacia.isEmpty()) nomeSubbacia = QString("SB01");
        m_nomesBaciaPorLinha.append(nomeBacia);
        m_nomesSubbaciaPorLinha.append(nomeSubbacia);

        QString idUsoPadrao;
        if (m_modeloUsoOcupacaoSolo && !m_modeloUsoOcupacaoSolo->idsDisponiveis().isEmpty()) {
            idUsoPadrao = m_modeloUsoOcupacaoSolo->idsDisponiveis().first();
        }
        m_idsUsoOcupacaoPorLinha.append(idUsoPadrao);
    }

    // Atualiza C(runoff) com base no uso selecionado.
    for (int linha = 0; linha < m_bacias.size(); ++linha) {
        if (!m_modeloUsoOcupacaoSolo) break;

        const QString idUso = m_idsUsoOcupacaoPorLinha.value(linha);
        if (const TipoUsoOcupacaoSolo* tipo = m_modeloUsoOcupacaoSolo->tipoPorId(idUso)) {
            m_bacias[linha].setC_10(tipo->cRunoff());
        }
    }

    endResetModel();
    emit dadosAlterados();
}

const QVector<BaciaContribuicao>& ModeloTabelaBacias::bacias() const
{
    return m_bacias;
}

bool ModeloTabelaBacias::adicionarBacia(const QString& nomeBacia,
                                        const QString& nomeSubbacia,
                                        QString* mensagemErro)
{
    const QString nomeBaciaNormalizado = normalizarTexto(nomeBacia);
    const QString nomeSubbaciaNormalizado = normalizarTexto(nomeSubbacia);

    if (nomeBaciaNormalizado.isEmpty()) {
        if (mensagemErro) *mensagemErro = "O nome da bacia não pode ser vazio.";
        return false;
    }

    if (nomeSubbaciaNormalizado.isEmpty()) {
        if (mensagemErro) *mensagemErro = "O nome da subbacia não pode ser vazio.";
        return false;
    }

    if (existeCombinacao(nomeBaciaNormalizado, nomeSubbaciaNormalizado)) {
        if (mensagemErro) *mensagemErro = "Já existe uma bacia com a mesma combinação Bacia-Subbacia.";
        return false;
    }

    const int linhaNova = m_bacias.size();
    beginInsertRows(QModelIndex(), linhaNova, linhaNova);
    m_bacias.append(BaciaContribuicao(criarIdComposto(nomeBaciaNormalizado, nomeSubbaciaNormalizado)));
    m_nomesBaciaPorLinha.append(nomeBaciaNormalizado);
    m_nomesSubbaciaPorLinha.append(nomeSubbaciaNormalizado);

    QString idUsoPadrao;
    if (m_modeloUsoOcupacaoSolo && !m_modeloUsoOcupacaoSolo->idsDisponiveis().isEmpty()) {
        idUsoPadrao = m_modeloUsoOcupacaoSolo->idsDisponiveis().first();
        if (const TipoUsoOcupacaoSolo* tipo = m_modeloUsoOcupacaoSolo->tipoPorId(idUsoPadrao)) {
            m_bacias.last().setC_10(tipo->cRunoff());
        }
    }
    m_idsUsoOcupacaoPorLinha.append(idUsoPadrao);
    endInsertRows();

    emit dadosAlterados();
    return true;
}

bool ModeloTabelaBacias::removerBacia(int linha, QString* mensagemErro)
{
    if (linha < 0 || linha >= m_bacias.size()) {
        if (mensagemErro) *mensagemErro = "Linha inválida para remoção de bacia.";
        return false;
    }

    beginRemoveRows(QModelIndex(), linha, linha);
    m_bacias.removeAt(linha);
    m_nomesBaciaPorLinha.removeAt(linha);
    m_nomesSubbaciaPorLinha.removeAt(linha);
    m_idsUsoOcupacaoPorLinha.removeAt(linha);
    endRemoveRows();

    emit dadosAlterados();
    return true;
}

bool ModeloTabelaBacias::existeCombinacao(const QString& nomeBacia,
                                          const QString& nomeSubbacia,
                                          int linhaIgnorada) const
{
    const QString nomeBaciaNormalizado = normalizarTexto(nomeBacia);
    const QString nomeSubbaciaNormalizado = normalizarTexto(nomeSubbacia);

    for (int i = 0; i < m_bacias.size(); ++i) {
        if (i == linhaIgnorada) continue;

        const QString baciaAtual = m_nomesBaciaPorLinha.value(i).trimmed();
        const QString subbaciaAtual = m_nomesSubbaciaPorLinha.value(i).trimmed();
        if (baciaAtual.compare(nomeBaciaNormalizado, Qt::CaseInsensitive) == 0
            && subbaciaAtual.compare(nomeSubbaciaNormalizado, Qt::CaseInsensitive) == 0) {
            return true;
        }
    }

    return false;
}

QString ModeloTabelaBacias::nomeBaciaPorLinha(int linha) const
{
    if (linha < 0 || linha >= m_nomesBaciaPorLinha.size()) return QString();
    return m_nomesBaciaPorLinha.at(linha);
}

QString ModeloTabelaBacias::nomeSubbaciaPorLinha(int linha) const
{
    if (linha < 0 || linha >= m_nomesSubbaciaPorLinha.size()) return QString();
    return m_nomesSubbaciaPorLinha.at(linha);
}

QString ModeloTabelaBacias::idUsoOcupacaoPorLinha(int linha) const
{
    if (linha < 0 || linha >= m_idsUsoOcupacaoPorLinha.size()) return QString();
    return m_idsUsoOcupacaoPorLinha.at(linha);
}

void ModeloTabelaBacias::definirModeloUsoOcupacaoSolo(ModeloTabelaUsoOcupacaoSolo* modeloUsoOcupacaoSolo)
{
    m_modeloUsoOcupacaoSolo = modeloUsoOcupacaoSolo;
    sincronizarCoeficienteRunoffPeloUsoOcupacao();
}

void ModeloTabelaBacias::sincronizarCoeficienteRunoffPeloUsoOcupacao()
{
    if (!m_modeloUsoOcupacaoSolo) return;
    if (m_bacias.isEmpty()) return;

    bool alterou = false;
    for (int linha = 0; linha < m_bacias.size(); ++linha) {
        const QString idUso = m_idsUsoOcupacaoPorLinha.value(linha);
        if (idUso.trimmed().isEmpty()) continue;

        if (const TipoUsoOcupacaoSolo* tipo = m_modeloUsoOcupacaoSolo->tipoPorId(idUso)) {
            m_bacias[linha].setC_10(tipo->cRunoff());
            alterou = true;
        }
    }

    if (alterou) {
        emit dataChanged(index(0, ColunaUsoOcupacaoSolo),
                         index(rowCount() - 1, ColunaUsoOcupacaoSolo),
                         { Qt::DisplayRole, Qt::EditRole });
        emit dadosAlterados();
    }
}

QString ModeloTabelaBacias::criarIdComposto(const QString& nomeBacia,
                                            const QString& nomeSubbacia) const
{
    return QString("%1-%2").arg(normalizarTexto(nomeBacia), normalizarTexto(nomeSubbacia));
}

void ModeloTabelaBacias::decomporIdComposto(const QString& idComposto,
                                            QString* nomeBacia,
                                            QString* nomeSubbacia) const
{
    if (!nomeBacia || !nomeSubbacia) return;

    const QString texto = idComposto.trimmed();
    const int separador = texto.indexOf('-');
    if (separador <= 0) {
        *nomeBacia = texto;
        *nomeSubbacia = QString();
        return;
    }

    *nomeBacia = texto.left(separador).trimmed();
    *nomeSubbacia = texto.mid(separador + 1).trimmed();
}
