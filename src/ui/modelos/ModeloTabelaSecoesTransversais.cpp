#include "ModeloTabelaSecoesTransversais.h"

#include "domain/RevestimentoCanal.h"

namespace
{
/**
 * @brief Indica se a geometria informada corresponde a uma secao semicircular.
 * @param geometria Texto armazenado no cadastro.
 * @return true quando a geometria for semicircular.
 */
bool ehGeometriaSemicircular(const QString& geometria)
{
    return geometria.trimmed().compare("Semicircular", Qt::CaseInsensitive) == 0;
}

/**
 * @brief Normaliza o texto da geometria para os valores padrao do sistema.
 * @param geometria Texto informado na edicao.
 * @return Nome de geometria normalizado.
 */
QString normalizarGeometria(const QString& geometria)
{
    const QString texto = geometria.trimmed();
    if (texto.compare("Semicircular", Qt::CaseInsensitive) == 0) {
        return "Semicircular";
    }
    return "Trapezoidal";
}

/**
 * @brief Retorna um valor seguro para a altura maxima quando o dado vier vazio.
 * @param registro Registro avaliado.
 * @return Altura maxima padrao em metros.
 */
QString alturaMaximaPadraoTexto(const RegistroSecaoTransversal& registro)
{
    if (ehGeometriaSemicircular(registro.geometria)) {
        return registro.diametroM.trimmed().isEmpty() ? QString("0.50") : registro.diametroM.trimmed();
    }

    return "1.00";
}

/**
 * @brief Retorna um valor seguro para a folga minima quando o dado vier vazio.
 * @return Folga padrao em metros.
 */
QString folgaPadraoTexto()
{
    return "0.10";
}

/**
 * @brief Define o revestimento padrao conforme a geometria principal da secao.
 * @param geometria Geometria cadastrada para a secao.
 * @return Nome do revestimento sugerido.
 */
QString revestimentoPadraoParaGeometria(const QString& geometria)
{
    if (ehGeometriaSemicircular(geometria)) {
        return "Concreto";
    }
    return "Grama";
}

/**
 * @brief Garante defaults seguros ao carregar registros antigos sem os novos campos.
 * @param registro Registro a ser normalizado.
 */
void normalizarCamposOpcionais(RegistroSecaoTransversal* registro)
{
    if (!registro) return;

    registro->geometria = normalizarGeometria(registro->geometria);

    if (registro->alturaMaximaM.trimmed().isEmpty()) {
        registro->alturaMaximaM = alturaMaximaPadraoTexto(*registro);
    }

    if (registro->folgaMinimaM.trimmed().isEmpty()) {
        registro->folgaMinimaM = folgaPadraoTexto();
    }

    if (registro->revestimento.trimmed().isEmpty()) {
        registro->revestimento = revestimentoPadraoParaGeometria(registro->geometria);
    }
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
        case ColunaAlturaMaxima: return registro.alturaMaximaM;
        case ColunaFolgaMinima: return registro.folgaMinimaM;
        case ColunaRevestimento: return registro.revestimento;
        default: return QVariant();
        }
    }

    if (role == Qt::ToolTipRole) {
        switch (index.column()) {
        case ColunaAlturaMaxima:
            return "Altura maxima util da secao usada para verificar a folga.";
        case ColunaFolgaMinima:
            return "Folga minima livre exigida entre a lamina d'agua e o topo da secao.";
        case ColunaRevestimento:
            return "Revestimento hidraulico associado a esta secao transversal.";
        default:
            return QVariant();
        }
    }

    if (role == Qt::TextAlignmentRole) {
        if (index.column() >= ColunaBaseMenor && index.column() != ColunaRevestimento) {
            return Qt::AlignCenter;
        }
    }

    return QVariant();
}

QVariant ModeloTabelaSecoesTransversais::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::ToolTipRole) {
        switch (section) {
        case ColunaAlturaMaxima:
            return "Altura maxima da secao usada para verificar a folga.";
        case ColunaFolgaMinima:
            return "Criterio de folga minima usado nas verificacoes hidraulicas.";
        case ColunaRevestimento:
            return "Classe de revestimento usada para Manning e limites admissiveis.";
        default:
            return QVariant();
        }
    }

    if (role != Qt::DisplayRole) return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case ColunaNomenclatura: return "Nomenclatura";
        case ColunaGeometria: return "Geometria";
        case ColunaBaseMenor: return "b_menor (m)";
        case ColunaTalude: return "Talude / m";
        case ColunaDiametro: return "D (m)";
        case ColunaAlturaMaxima: return "H max. (m)";
        case ColunaFolgaMinima: return "Folga min. (m)";
        case ColunaRevestimento: return "Revestimento";
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

        // Mantem coerencia entre a geometria escolhida e os campos habilitados na linha.
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

        if (registro.alturaMaximaM.trimmed().isEmpty() || registro.alturaMaximaM.trimmed() == "-") {
            registro.alturaMaximaM = alturaMaximaPadraoTexto(registro);
        }

        if (registro.revestimento.trimmed().isEmpty()) {
            registro.revestimento = revestimentoPadraoParaGeometria(registro.geometria);
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
        if (registro.alturaMaximaM.trimmed().isEmpty() || registro.alturaMaximaM.trimmed() == "-") {
            registro.alturaMaximaM = registro.diametroM;
        }
        break;
    case ColunaAlturaMaxima:
        registro.alturaMaximaM = value.toString().trimmed();
        if (registro.alturaMaximaM.isEmpty()) {
            registro.alturaMaximaM = alturaMaximaPadraoTexto(registro);
        }
        break;
    case ColunaFolgaMinima:
        registro.folgaMinimaM = value.toString().trimmed();
        if (registro.folgaMinimaM.isEmpty()) {
            registro.folgaMinimaM = folgaPadraoTexto();
        }
        break;
    case ColunaRevestimento:
        registro.revestimento = value.toString().trimmed();
        if (registro.revestimento.isEmpty()) {
            registro.revestimento = revestimentoPadraoParaGeometria(registro.geometria);
        }
        break;
    default:
        return false;
    }

    if (index.column() == ColunaGeometria) {
        const QModelIndex inicioLinha = this->index(index.row(), 0);
        const QModelIndex fimLinha = this->index(index.row(), TotalColunas - 1);
        emit dataChanged(inicioLinha, fimLinha, { Qt::DisplayRole, Qt::EditRole, Qt::ToolTipRole });
    }
    else {
        emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole, Qt::ToolTipRole });
    }

    return true;
}

void ModeloTabelaSecoesTransversais::carregarPadrao()
{
    beginResetModel();
    m_registros = {
        {"VTD-1", "Trapezoidal", "0.30", "1.00", "-", "1.00", "0.10", "Grama"},
        {"VTD-2", "Trapezoidal", "0.50", "1.00", "-", "1.00", "0.10", "Grama"},
        {"CTD-1", "Trapezoidal", "0.50", "1.00", "-", "1.00", "0.12", "Concreto"},
        {"CSD-1", "Semicircular", "-", "-", "0.40", "0.40", "0.08", "Concreto"},
        {"CSD-2", "Semicircular", "-", "-", "0.50", "0.50", "0.08", "Concreto"},
        {"CSD-3", "Semicircular", "-", "-", "0.60", "0.60", "0.08", "Concreto"},
        {"CSD-4", "Semicircular", "-", "-", "0.80", "0.80", "0.10", "Concreto"},
        {"CSD-5", "Semicircular", "-", "-", "1.00", "1.00", "0.10", "Concreto"}
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
    for (RegistroSecaoTransversal& registro : m_registros) {
        normalizarCamposOpcionais(&registro);
    }
    endResetModel();
}

bool ModeloTabelaSecoesTransversais::obterRegistroPorNomenclatura(const QString& nomenclatura,
                                                                  RegistroSecaoTransversal* registro) const
{
    const QString idAlvo = nomenclatura.trimmed();
    for (const RegistroSecaoTransversal& item : m_registros) {
        if (item.nomenclatura.trimmed().compare(idAlvo, Qt::CaseInsensitive) != 0) continue;

        if (registro) {
            *registro = item;
            normalizarCamposOpcionais(registro);
        }
        return true;
    }

    return false;
}

QStringList ModeloTabelaSecoesTransversais::nomesRevestimentosDisponiveis() const
{
    if (!m_nomesRevestimentosDisponiveis.isEmpty()) {
        return m_nomesRevestimentosDisponiveis;
    }

    return RevestimentoCanal::nomesPadrao();
}

void ModeloTabelaSecoesTransversais::definirNomesRevestimentosDisponiveis(const QStringList& nomes)
{
    m_nomesRevestimentosDisponiveis = nomes;
}
