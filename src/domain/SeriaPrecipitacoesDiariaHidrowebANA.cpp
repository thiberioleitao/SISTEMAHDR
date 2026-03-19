#include "SeriaPrecipitacoesDiariaHidrowebANA.h"

#include <QFile>            // arquivo
#include <QTextStream>      // leitura texto
#include <QtMath>           // qIsNaN, etc.
#include <limits>           // NaN

SeriaPrecipitacoesDiariaHidrowebANA::SeriaPrecipitacoesDiariaHidrowebANA()
{
    // nada a fazer
}

void SeriaPrecipitacoesDiariaHidrowebANA::limpar()
{
    m_pxPorDia.clear();     // limpa série
    m_avisos.clear();       // limpa avisos
}

const QMap<QDate, double>& SeriaPrecipitacoesDiariaHidrowebANA::serieDiaPx() const
{
    return m_pxPorDia;      // retorna referência const
}

bool SeriaPrecipitacoesDiariaHidrowebANA::contem(const QDate& dia) const
{
    return m_pxPorDia.contains(dia); // existe chave?
}

double SeriaPrecipitacoesDiariaHidrowebANA::Pdia(const QDate& dia) const
{
    return m_pxPorDia.value(dia, nan());  // se não existe, NaN
}

QDate SeriaPrecipitacoesDiariaHidrowebANA::dataMin() const
{
    if (m_pxPorDia.isEmpty()) return QDate(); // inválida se vazio
    return m_pxPorDia.firstKey();             // primeira chave (ordenado)
}

QDate SeriaPrecipitacoesDiariaHidrowebANA::dataMax() const
{
    if (m_pxPorDia.isEmpty()) return QDate(); // inválida se vazio
    return m_pxPorDia.lastKey();              // última chave (ordenado)
}

QStringList SeriaPrecipitacoesDiariaHidrowebANA::avisos() const
{
    return m_avisos; // cópia dos avisos
}

double SeriaPrecipitacoesDiariaHidrowebANA::nan()
{
    return std::numeric_limits<double>::quiet_NaN(); // NaN padrão
}

bool SeriaPrecipitacoesDiariaHidrowebANA::ehNan(double v)
{
    return qIsNaN(v); // Qt helper
}

/**
 * @brief Split simples de CSV com suporte a aspas duplas.
 *        (Suficiente para CSVs ANA típicos.)
 */
QStringList SeriaPrecipitacoesDiariaHidrowebANA::dividirCsv(const QString& linha, QChar sep)
{
    QStringList campos;                 // saída
    QString atual;                      // campo atual
    bool emAspas = false;               // estado

    for (int i = 0; i < linha.size(); ++i) {              // percorre caracteres
        const QChar ch = linha.at(i);                     // char atual

        if (ch == '\"') {                                 // alterna aspas
            emAspas = !emAspas;                            // toggle
            continue;                                      // não inclui aspas
        }

        if (!emAspas && ch == sep) {                      // separador fora de aspas
            campos.append(atual);                          // fecha campo
            atual.clear();                                 // reseta
        }
        else {
            atual.append(ch);                              // acumula
        }
    }

    campos.append(atual);                                  // último campo
    return campos;                                         // retorna lista
}

/**
 * @brief Converte string pt-BR para double (aceita vírgula decimal).
 *        Retorna NaN se inválido/vazio.
 */
double SeriaPrecipitacoesDiariaHidrowebANA::parseDoublePt(const QString& txt)
{
    QString s = txt.trimmed();                             // remove espaços
    if (s.isEmpty()) return nan();                         // vazio -> NaN

    s.replace(',', '.');                                   // vírgula -> ponto

    bool ok = false;                                       // flag conversão
    double v = s.toDouble(&ok);                            // tenta converter
    if (!ok) return nan();                                 // falhou -> NaN

    // alguns arquivos podem ter sentinelas negativas; mantenha como NaN se desejar:
    // if (v < -9000) return nan();

    return v;                                              // valor válido
}

/**
 * @brief Procura uma linha de cabeçalho que contenha "Data" e "Chuva01".
 *        Isso evita depender estritamente de skiprows=14.
 */
bool SeriaPrecipitacoesDiariaHidrowebANA::lerAteCabecalho(QTextStream& ts, QString* linhaCabecalho)
{
    int limiteLinhas = 250;                                // limite de busca
    int cont = 0;                                          // contador

    while (!ts.atEnd() && cont < limiteLinhas) {           // lê até achar
        QString linha = ts.readLine();                     // linha
        ++cont;                                            // incrementa

        const QString l = linha.trimmed();                 // normaliza

        if (l.contains("Data", Qt::CaseInsensitive) &&
            l.contains("Chuva01", Qt::CaseInsensitive)) {  // achou padrão esperado
            if (linhaCabecalho) *linhaCabecalho = linha;   // retorna cabeçalho
            return true;                                   // sucesso
        }
    }

    return false;                                          // não achou
}

QMap<QString, int> SeriaPrecipitacoesDiariaHidrowebANA::mapearColunas(const QStringList& colunas) const
{
    QMap<QString, int> idx;                                // mapa nome->índice

    for (int i = 0; i < colunas.size(); ++i) {             // percorre header
        const QString nome = colunas.at(i).trimmed();      // nome limpo
        idx.insert(nome, i);                                // salva índice
    }

    return idx;                                            // retorna
}

void SeriaPrecipitacoesDiariaHidrowebANA::preencherDatasAusentesComNaN()
{
    if (m_pxPorDia.isEmpty()) return;                      // nada a fazer

    QDate d0 = dataMin();                                  // início
    QDate d1 = dataMax();                                  // fim

    for (QDate d = d0; d <= d1; d = d.addDays(1)) {        // varre intervalo
        if (!m_pxPorDia.contains(d)) {                     // se ausente
            m_pxPorDia.insert(d, nan());                   // insere NaN
        }
    }
}

/**
 * @brief Carrega o CSV bruto do HidroWeb/ANA e monta Dia x Px (mm).
 */
bool SeriaPrecipitacoesDiariaHidrowebANA::carregar(const QString& caminhoArquivoCsv,
    bool preencherDatasAusentes)
{
    limpar();                                              // reseta estado

    QFile arq(caminhoArquivoCsv);                          // handle arquivo
    if (!arq.exists()) return false;                       // existe?

    if (!arq.open(QIODevice::ReadOnly | QIODevice::Text)) return false; // abre texto

    QTextStream ts(&arq);                                  // stream
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    ts.setEncoding(QStringConverter::Latin1);              // ISO-8859-1 ~ Latin1
#endif

    QString linhaCab;                                      // linha do cabeçalho
    if (!lerAteCabecalho(ts, &linhaCab)) return false;     // acha cabeçalho

    const QChar sep = ';';                                 // separador padrão ANA
    const QStringList colCab = dividirCsv(linhaCab, sep);  // split cabeçalho
    const QMap<QString, int> idx = mapearColunas(colCab);  // índices

    // valida colunas mínimas
    if (!idx.contains("Data") || !idx.contains("NivelConsistencia")) return false;

    // índices das colunas diárias Chuva01..Chuva31 (se existirem)
    QVector<int> idxChuva;                                 // índices das chuvas
    idxChuva.reserve(31);                                  // reserva

    for (int d = 1; d <= 31; ++d) {                        // 1..31
        const QString nome = QString("Chuva%1").arg(d, 2, 10, QChar('0')); // Chuva01
        idxChuva.append(idx.value(nome, -1));              // -1 se não achou
    }

    // Para escolher o maior NivelConsistencia por mês:
    // chave = 1º dia do mês (Ano/Mes), valor = {nc, vetor 31}
    struct RegistroMensal {
        int nc;                                            // nível consistência
        QDate dataRef;                                     // data referência do registro (dd/mm/yyyy)
        QVector<double> chuva31;                            // Chuva01..Chuva31 (mm)
    };

    QMap<QDate, RegistroMensal> meses;                     // mês -> registro

    // lê dados
    while (!ts.atEnd()) {                                  // até EOF
        const QString linha = ts.readLine();               // lê linha
        const QString l = linha.trimmed();                 // normaliza
        if (l.isEmpty()) continue;                         // pula vazio

        const QStringList campos = dividirCsv(linha, sep); // split linha

        // proteção: linha curta demais
        if (campos.size() <= qMax(idx.value("Data"), idx.value("NivelConsistencia"))) {
            m_avisos.append("Linha ignorada (colunas insuficientes).");
            continue;
        }

        // Data (dd/MM/yyyy)
        const QString sData = campos.at(idx.value("Data")).trimmed(); // texto Data
        const QDate data = QDate::fromString(sData, "dd/MM/yyyy");    // parse
        if (!data.isValid()) {                                        // valida
            m_avisos.append("Data inválida ignorada: " + sData);
            continue;
        }

        // NivelConsistencia
        bool okNc = false;                                // flag
        int nc = campos.at(idx.value("NivelConsistencia")).trimmed().toInt(&okNc); // parse
        if (!okNc) nc = -1;                               // se falhar, assume -1

        // chave do mês (Ano/Mes)
        const QDate chaveMes(data.year(), data.month(), 1); // mês como chave

        // monta vetor 31 (NaN se não existir ou inválido)
        QVector<double> chuva31;                          // vetor
        chuva31.resize(31);                               // size 31

        for (int i = 0; i < 31; ++i) {                    // 0..30
            const int col = idxChuva.at(i);               // índice col
            if (col < 0 || col >= campos.size()) {        // coluna não existe
                chuva31[i] = nan();                        // NaN
                continue;
            }
            chuva31[i] = parseDoublePt(campos.at(col));   // parse pt
        }

        // mantém apenas o registro com maior nc para o mesmo mês
        if (!meses.contains(chaveMes)) {                  // se não existe
            meses.insert(chaveMes, { nc, data, chuva31 });   // insere
        }
        else {
            const RegistroMensal& atual = meses.value(chaveMes); // atual
            if (nc >= atual.nc) {                         // substitui se nc maior/igual
                meses[chaveMes] = { nc, data, chuva31 };     // overwrite
            }
        }
    }

    // expande mês → dias e cria Dia x Px
    for (auto it = meses.cbegin(); it != meses.cend(); ++it) { // percorre meses
        const QDate chaveMes = it.key();                      // ano/mês
        const RegistroMensal& reg = it.value();               // registro

        const int diasNoMes = chaveMes.daysInMonth();         // quantos dias

        for (int d = 1; d <= diasNoMes; ++d) {                // 1..diasNoMes
            const QDate dia(chaveMes.year(), chaveMes.month(), d); // data diária
            const double px_mm = reg.chuva31[d - 1];          // ChuvaXX

            // aqui você decide: inserir inclusive NaN/zero, ou somente valores válidos
            // vou inserir tudo (inclusive NaN) para manter coerência temporal
            m_pxPorDia.insert(dia, px_mm);                    // salva
        }
    }

    // se pediu, preenche intervalo com NaN
    if (preencherDatasAusentes) {                             // flag
        preencherDatasAusentesComNaN();                       // completa série
    }

    // validação final
    if (m_pxPorDia.isEmpty()) return false;                   // nada carregado

    return true;                                              // ok
}
