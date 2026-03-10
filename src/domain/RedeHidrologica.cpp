#include "RedeHidrologica.h"

#include <QFile>
#include <QQueue>
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

    m_elementos.append({ elemId, jus });
    return true;
}

QString RedeHidrologica::adicionarElementoComIdAutomatico(const QString& idJusante,
                                                          const QString& prefixo,
                                                          QString* erro)
{
    const QString idGerado = gerarProximoId(prefixo);

    if (!adicionarElemento(idGerado, idJusante, erro)) {
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

    if (idxArea < 0 && cabecalho.size() > 2) idxArea = 2; // fallback para coluna ÁREA

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

        BaciaContribuicao bacia(nome, idJusante, areaKm2, sTalvegue, lTalvegueKm);

        QString erroAdd;
        if (!adicionarElemento(bacia.id(), bacia.idJusante(), &erroAdd)) {
            if (erro) *erro = QString("Falha ao incluir bacia da linha %1: %2").arg(linhaNumero).arg(erroAdd);
            return false;
        }

        m_bacias.append(bacia);
        ++importadas;
    }

    if (importadas == 0) {
        if (erro) *erro = "Nenhuma bacia válida foi importada do CSV.";
        return false;
    }

    return true;
}

const QVector<ElementoRedeHidrologica>& RedeHidrologica::elementos() const
{
    return m_elementos;
}

const QVector<BaciaContribuicao>& RedeHidrologica::bacias() const
{
    return m_bacias;
}

QMap<QString, double> RedeHidrologica::contribuicaoBasePorElemento() const
{
    QMap<QString, double> contribuicao;
    for (const BaciaContribuicao& b : m_bacias) {
        contribuicao.insert(b.id(), 0.0);
    }
    return contribuicao;
}

QMap<QString, double> RedeHidrologica::calcularVazaoAcumuladaPorElemento(
    const QMap<QString, double>& contribuicaoPorElemento,
    QString* erro) const
{
    QMap<QString, double> vazaoAcumuladaPorElemento;
    if (erro) erro->clear();

    if (m_elementos.isEmpty()) {
        return vazaoAcumuladaPorElemento;
    }

    QMap<QString, int> indicePorId;
    QMap<QString, int> grauEntrada;
    QMap<QString, double> vazaoEntradaAcumulada;

    for (int i = 0; i < m_elementos.size(); ++i) {
        const ElementoRedeHidrologica& e = m_elementos.at(i);
        indicePorId.insert(e.id, i);
        grauEntrada.insert(e.id, 0);
        vazaoEntradaAcumulada.insert(e.id, 0.0);
        vazaoAcumuladaPorElemento.insert(e.id, 0.0);
    }

    for (const ElementoRedeHidrologica& e : m_elementos) {
        if (e.idJusante.isEmpty()) continue; // exutório da rede

        if (!indicePorId.contains(e.idJusante)) {
            if (erro) *erro = "ID jusante inexistente na rede: " + e.idJusante;
            return QMap<QString, double>();
        }

        grauEntrada[e.idJusante] += 1;
    }

    QQueue<QString> fila;
    for (auto it = grauEntrada.cbegin(); it != grauEntrada.cend(); ++it) {
        if (it.value() == 0) fila.enqueue(it.key());
    }

    int elementosProcessados = 0;

    while (!fila.isEmpty()) {
        const QString idAtual = fila.dequeue();
        const ElementoRedeHidrologica& eAtual = m_elementos.at(indicePorId.value(idAtual));

        const double contribuicaoLocal = std::max(0.0, contribuicaoPorElemento.value(idAtual, 0.0));
        const double vazaoSaida = vazaoEntradaAcumulada.value(idAtual) + contribuicaoLocal;
        vazaoAcumuladaPorElemento[idAtual] = vazaoSaida;

        if (!eAtual.idJusante.isEmpty()) {
            vazaoEntradaAcumulada[eAtual.idJusante] = vazaoEntradaAcumulada.value(eAtual.idJusante) + vazaoSaida;

            grauEntrada[eAtual.idJusante] -= 1;
            if (grauEntrada.value(eAtual.idJusante) == 0) {
                fila.enqueue(eAtual.idJusante);
            }
        }

        elementosProcessados += 1;
    }

    if (elementosProcessados != m_elementos.size()) {
        if (erro) *erro = "A rede possui ciclo ou estrutura inválida para acumulação de vazão.";
        return QMap<QString, double>();
    }

    return vazaoAcumuladaPorElemento;
}

void RedeHidrologica::limpar()
{
    m_elementos.clear();
    m_bacias.clear();
}
