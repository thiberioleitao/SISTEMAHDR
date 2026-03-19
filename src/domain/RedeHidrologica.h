#pragma once

#include "BaciaContribuicao.h"
#include "Bueiro.h"
#include "Canal.h"

#include <QMap>
#include <QString>
#include <QVector>

enum class TipoElementoRede
{
    Canal,
    BaciaContribuicao,
    Bueiro,
    Exutorio,
    Outro
};

class RedeHidrologica
{
public:
    RedeHidrologica() = default;

    bool existeId(const QString& id) const;

    bool adicionarCanal(const Canal& canal);

    bool adicionarBacia(const QString& idElemento,
                        const BaciaContribuicao& bacia,
                        const QString& idJusante = QString());

    bool adicionarBacia(const BaciaContribuicao& bacia);

    bool adicionarBueiro(const Bueiro& bueiro);

    bool adicionarExutorio(const QString& id,
                           const QString& idJusante = QString());

    bool definirJusanteElemento(const QString& idElemento,
                                const QString& idJusante);

    const QMap<QString, BaciaContribuicao>& baciasPorId() const;
    const QMap<QString, Bueiro>& bueirosPorId() const;
    const QMap<QString, Canal>& canaisPorId() const;

    const BaciaContribuicao* baciaPorId(const QString& idElemento) const;
    const Bueiro* bueiroPorId(const QString& idElemento) const;
    const Canal* canalPorId(const QString& idElemento) const;

    BaciaContribuicao* baciaPorId(const QString& idElemento);
    Bueiro* bueiroPorId(const QString& idElemento);
    Canal* canalPorId(const QString& idElemento);

    double intensidadeChuvaProjetoMmH() const;
    void setIntensidadeChuvaProjetoMmH(double intensidadeChuvaProjetoMmH);

    double contribuicaoBaciasParaElemento(const QString& idElemento,
                                          double intensidadeChuvaBrutaMmH) const;

    double calcularComprimentoTotalTalvegueAteElemento(const QString& idElemento) const;

    QMap<QString, double> calcularVazaoAcumuladaPorElemento(
        const QMap<QString, double>& contribuicaoPorElemento = QMap<QString, double>()) const;

    double areaAcumuladaTotalContribuinte(const QString& idElemento) const;

    double coeficienteEscoamentoMedioPonderado(const QString& idElemento) const;

    double tempoConcentracaoKirpichModificadoAreaTotal(const QString& idElemento) const;

    double calcularTempoKirpichModificado(double comprimentoTalvegueKm,
                                          double declividadeTalvegue,
                                          double maiorTempoConcentracaoMontanteMin) const;

    QVector<QString> idsMontanteDoElemento(const QString& idElemento) const;

    QVector<QString> idsElementos() const;

    QString idJusanteDoElemento(const QString& idElemento) const;

    TipoElementoRede tipoDoElemento(const QString& idElemento) const;

    void limpar();

private:
    bool registrarNo(const QString& id,
                     const QString& idJusante,
                     TipoElementoRede tipo);

    QVector<QString> ordemTopologica() const;

    QVector<QString> m_idsElementos;
    QMap<QString, QString> m_idJusantePorId;
    QMap<QString, QVector<QString>> m_idsMontanteDiretoPorId;
    QMap<QString, TipoElementoRede> m_tipoPorId;
    QMap<QString, BaciaContribuicao> m_baciasPorId;
    QMap<QString, Bueiro> m_bueirosPorId;
    QMap<QString, Canal> m_canaisPorId;
    double m_intensidadeChuvaProjetoMmH = 0.0;
};
