#include "RedeHidrologica.h"

#include <QFile>
#include <QQueue>
#include <QSet>
#include <QStringConverter>
#include <QTextStream>

#include <algorithm>

namespace
{
QStringList dividirCsv(const QString& linha, QChar sep)
{
    QStringList campos;
    QString atual;
    bool emAspas = false;

    for (int i = 0; i < linha.size(); ++i) {
        const QChar ch = linha.at(i);

        if (ch == '"') {
            emAspas = !emAspas;
            continue;
        }

        if (!emAspas && ch == sep) {
            campos.append(atual);
            atual.clear();
        }
        else {
            atual.append(ch);
        }
    }

    campos.append(atual);
    return campos;
}

double parseDoubleFlex(const QString& txt)
{
    QString s = txt.trimmed();
    if (s.isEmpty()) return 0.0;

    s.replace(',', '.');
    bool ok = false;
    const double v = s.toDouble(&ok);
    return ok ? v : 0.0;
}

int indiceColuna(const QStringList& cabecalho, const QString& chave)
{
    const QString k = chave.trimmed().toUpper();
    for (int i = 0; i < cabecalho.size(); ++i) {
        if (cabecalho.at(i).trimmed().toUpper().contains(k)) {
            return i;
        }
    }
    return -1;
}
}

bool RedeHidrologica::existeId(const QString& id) const
{
    const QString alvo = id.trimmed();
    if (alvo.isEmpty()) return false;

    for (const ElementoRedeHidrologica& e : m_elementos) {
        if (e.id == alvo) return true;
    }

    return false;
}

QString RedeHidrologica::gerarProximoId(const QString& prefixo) const
{
    QString base = prefixo.trimmed();
    if (base.isEmpty()) base = "E";

    int numero = 1;
    while (true) {
        const QString candidato = base + QString::number(numero);
        if (!existeId(candidato)) {
            return candidato;
        }
        ++numero;
    }
}

bool RedeHidrologica::adicionarElemento(const QString& id,
                                        const QString& idJusante,
                                        TipoElementoRede tipo,
                                        QString* erro)
{
    const QString elemId = id.trimmed();
    const QString jus = idJusante.trimmed();

    if (elemId.isEmpty()) {
        if (erro) *erro = "ID do elemento deve ser informado.";
        return false;
    }

    if (!jus.isEmpty() && elemId == jus) {
        if (erro) *erro = "ID do elemento e ID jusante não podem ser iguais.";
        return false;
    }

    for (const ElementoRedeHidrologica& e : m_elementos) {
        if (e.id == elemId) {
            if (erro) *erro = "Já existe elemento com o mesmo ID: " + elemId;
            return false;
        }
    }

    m_elementos.append({ elemId, jus, tipo });
    return true;
}

bool RedeHidrologica::definirJusanteElemento(const QString& idElemento,
                                             const QString& idJusante,
                                             QString* erro)
{
    if (erro) erro->clear();

    const QString id = idElemento.trimmed();
    const QString jus = idJusante.trimmed();

    if (id.isEmpty()) {
        if (erro) *erro = "ID do elemento deve ser informado para definição de jusante.";
        return false;
    }

    if (!jus.isEmpty() && id == jus) {
        if (erro) *erro = "ID do elemento e ID jusante não podem ser iguais.";
        return false;
    }

    int indiceElemento = -1;
    for (int i = 0; i < m_elementos.size(); ++i) {
        if (m_elementos.at(i).id == id) {
            indiceElemento = i;
            break;
        }
    }

    if (indiceElemento < 0) {
        if (erro) *erro = "Elemento inexistente na rede: " + id;
        return false;
    }

    if (!jus.isEmpty() && !existeId(jus)) {
        if (erro) *erro = "ID jusante inexistente na rede: " + jus;
        return false;
    }

    m_elementos[indiceElemento].idJusante = jus;
    return true;
}

QString RedeHidrologica::adicionarElementoComIdAutomatico(const QString& idJusante,
                                                          TipoElementoRede tipo,
                                                          const QString& prefixo,
                                                          QString* erro)
{
    const QString idGerado = gerarProximoId(prefixo);

    if (!adicionarElemento(idGerado, idJusante, tipo, erro)) {
        return QString();
    }

    return idGerado;
}

bool RedeHidrologica::importarBaciasCsvCivil3D(const QString& caminhoArquivoCsv,
                                               bool areaEmMetroQuadrado,
                                               QString* erro,
                                               QStringList* avisos)
{
    if (erro) erro->clear();
    if (avisos) avisos->clear();

    QFile arq(caminhoArquivoCsv);
    if (!arq.exists()) {
        if (erro) *erro = "Arquivo não encontrado: " + caminhoArquivoCsv;
        return false;
    }

    if (!arq.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (erro) *erro = "Não foi possível abrir o arquivo: " + caminhoArquivoCsv;
        return false;
    }

    QTextStream ts(&arq);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    ts.setEncoding(QStringConverter::Latin1);
#endif

    if (ts.atEnd()) {
        if (erro) *erro = "Arquivo CSV vazio.";
        return false;
    }

    const QString linhaCabecalho = ts.readLine();
    const QStringList cabecalho = dividirCsv(linhaCabecalho, ',');

    int idxBacia = indiceColuna(cabecalho, "BACIA");
    int idxSubBacia = indiceColuna(cabecalho, "SUBBACIA");
    int idxArea = indiceColuna(cabecalho, "AREA");
    int idxLTalvegue = indiceColuna(cabecalho, "L_TALVEGUE");
    int idxSTalvegue = indiceColuna(cabecalho, "S_TALVEGUE");
    int idxJusante = indiceColuna(cabecalho, "ID_JUSANTE");

    if (idxArea < 0 && cabecalho.size() > 2) idxArea = 2;

    if (idxBacia < 0 || idxArea < 0 || idxLTalvegue < 0 || idxSTalvegue < 0 || idxJusante < 0) {
        if (erro) *erro = "Cabeçalho CSV incompatível com o formato BACIAS_CIVIL3D-B2-R0A.";
        return false;
    }

    int importadas = 0;
    int linhaNumero = 1;

    while (!ts.atEnd()) {
        const QString linha = ts.readLine();
        ++linhaNumero;

        if (linha.trimmed().isEmpty()) continue;

        const QStringList campos = dividirCsv(linha, ',');
        const int idxMax = std::max({ idxBacia, idxSubBacia, idxArea, idxLTalvegue, idxSTalvegue, idxJusante });
        if (campos.size() <= idxMax) {
            if (avisos) avisos->append(QString("Linha %1 ignorada: colunas insuficientes.").arg(linhaNumero));
            continue;
        }

        const QString nomeBacia = campos.at(idxBacia).trimmed();
        const QString subBacia = (idxSubBacia >= 0 && idxSubBacia < campos.size()) ? campos.at(idxSubBacia).trimmed() : QString();
        const QString nome = subBacia.isEmpty() ? nomeBacia : (nomeBacia + " - " + subBacia);

        const double areaBruta = parseDoubleFlex(campos.at(idxArea));
        const double areaKm2 = areaEmMetroQuadrado ? (areaBruta / 1000000.0) : areaBruta;

        const double lTalvegueM = parseDoubleFlex(campos.at(idxLTalvegue));
        const double lTalvegueKm = lTalvegueM / 1000.0;

        const double sTalvegue = parseDoubleFlex(campos.at(idxSTalvegue));
        const QString idJusante = campos.at(idxJusante).trimmed();

        BaciaContribuicao bacia(nome, areaKm2, sTalvegue, lTalvegueKm);

        const QString idElemento = gerarProximoId("B");

        QString erroAdd;
        if (!adicionarElemento(idElemento, idJusante, TipoElementoRede::BaciaContribuicao, &erroAdd)) {
            if (erro) *erro = QString("Falha ao incluir bacia da linha %1: %2").arg(linhaNumero).arg(erroAdd);
            return false;
        }

        if (!m_registroElementos.associarBaciaAoElemento(*this, idElemento, bacia, &erroAdd)) {
            if (erro) *erro = QString("Falha ao associar bacia da linha %1: %2").arg(linhaNumero).arg(erroAdd);
            return false;
        }
        ++importadas;
    }

    if (importadas == 0) {
        if (erro) *erro = "Nenhuma bacia válida foi importada do CSV.";
        return false;
    }

    return true;
}

bool RedeHidrologica::associarBaciaAoElemento(const QString& idElemento,
                                              const BaciaContribuicao& bacia,
                                              QString* erro)
{
    return m_registroElementos.associarBaciaAoElemento(*this, idElemento, bacia, erro);
}

const QVector<ElementoRedeHidrologica>& RedeHidrologica::elementos() const
{
    return m_elementos;
}

const QMap<QString, BaciaContribuicao>& RedeHidrologica::baciasPorId() const
{
    return m_registroElementos.baciasPorId();
}

const BaciaContribuicao* RedeHidrologica::baciaPorId(const QString& idElemento) const
{
    return m_registroElementos.baciaPorId(idElemento);
}

double RedeHidrologica::contribuicaoBaciasParaElemento(const QString& idElemento,
                                                       double intensidadeChuvaBrutaMmH,
                                                       QString* erro) const
{
    return m_registroElementos.contribuicaoBaciasParaElemento(*this, idElemento, intensidadeChuvaBrutaMmH, erro);
}

QVector<QString> RedeHidrologica::idsMontanteDoElemento(const QString& idElemento,
                                                        QString* erro) const
{
    if (erro) erro->clear();

    const QString idDestino = idElemento.trimmed();
    if (idDestino.isEmpty()) {
        if (erro) *erro = "ID do elemento não informado para busca de montantes.";
        return {};
    }

    if (!existeId(idDestino)) {
        if (erro) *erro = "ID do elemento inexistente na rede: " + idDestino;
        return {};
    }

    QSet<QString> visitados;
    QQueue<QString> fila;
    QVector<QString> idsMontante;

    visitados.insert(idDestino);
    fila.enqueue(idDestino);

    while (!fila.isEmpty()) {
        const QString idAlvo = fila.dequeue();

        for (const ElementoRedeHidrologica& e : m_elementos) {
            if (e.idJusante == idAlvo && !visitados.contains(e.id)) {
                visitados.insert(e.id);
                fila.enqueue(e.id);
                idsMontante.append(e.id);
            }
        }
    }

    return idsMontante;
}

void RedeHidrologica::limpar()
{
    m_elementos.clear();
    m_registroElementos.limpar();
}
