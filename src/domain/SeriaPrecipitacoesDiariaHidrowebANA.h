#pragma once

#include <QDate>        // datas
#include <QMap>         // série ordenada por data
#include <QString>      // strings Qt
#include <QStringList>  // lista de avisos

/**
 * @brief Lê CSV bruto do HidroWeb/ANA (chuvas mensais com Chuva01..Chuva31) e constrói série diária Dia x Pdia (mm).
 *
 */
class SeriaPrecipitacoesDiariaHidrowebANA
{
public:
    SeriaPrecipitacoesDiariaHidrowebANA();                                  // ctor padrão

    bool carregar(const QString& caminhoArquivoCsv,                          // caminho do csv bruto
        bool preencherDatasAusentes = false,                      // se true, completa datas faltantes com NaN
        QString* erro = nullptr);                                 // msg de erro (opcional)

    const QMap<QDate, double>& serieDiaPx() const;                           // série (Dia → Px)
    bool contem(const QDate& dia) const;                                     // existe na série?
    double Pdia(const QDate& dia) const;                                       // retorna Px (mm) ou NaN se não existir

    QDate dataMin() const;                                                   // menor data existente
    QDate dataMax() const;                                                   // maior data existente

    QStringList avisos() const;                                              // avisos de parsing/leitura

    void limpar();                                                           // limpa estado interno

private:
    // --------- utilitários internos ----------
    static QStringList dividirCsv(const QString& linha, QChar sep);          // split CSV com aspas simples
    static double parseDoublePt(const QString& txt);                         // converte "1,23" -> 1.23 (NaN se inválido)
    static bool ehNan(double v);                                             // checa NaN
    static double nan();                                                     // retorna NaN

    bool lerAteCabecalho(QTextStream& ts, QString* linhaCabecalho);          // busca linha de cabeçalho
    QMap<QString, int> mapearColunas(const QStringList& colunas) const;      // nome -> índice
    void preencherDatasAusentesComNaN();                                     // completa a série com NaN no intervalo [min,max]

private:
    QMap<QDate, double> m_pxPorDia;                                          // Dia -> Px (mm)
    QStringList m_avisos;                                                    // warnings
};
