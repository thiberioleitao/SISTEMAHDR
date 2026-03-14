#pragma once

#include "BaciaContribuicao.h"

#include <QMap>
#include <QString>
#include <QStringList>
#include <QVector>

enum class TipoElementoRede
{
    Canal,
    BaciaContribuicao,
    Bueiro,
    Outro
};

struct ElementoRedeHidrologica
{
    QString id;
    QString idJusante;
    TipoElementoRede tipo = TipoElementoRede::Canal;
};

class RedeHidrologica
{
public:
    RedeHidrologica() = default;

    bool existeId(const QString& id) const;

    QString gerarProximoId(const QString& prefixo = "E") const;

    bool adicionarElemento(const QString& id,
                          const QString& idJusante,
                          TipoElementoRede tipo = TipoElementoRede::Canal,
                          QString* erro = nullptr);

    QString adicionarElementoComIdAutomatico(const QString& idJusante,
                                             TipoElementoRede tipo = TipoElementoRede::Canal,
                                             const QString& prefixo = "E",
                                             QString* erro = nullptr);

    bool importarBaciasCsvCivil3D(const QString& caminhoArquivoCsv,
                                  bool areaEmMetroQuadrado = true,
                                  QString* erro = nullptr,
                                  QStringList* avisos = nullptr);

    const QMap<QString, BaciaContribuicao>& baciasPorId() const;

    const BaciaContribuicao* baciaPorId(const QString& idElemento) const;

    double contribuicaoBaciasParaElemento(const QString& idElemento,
                                          double intensidadeChuvaBrutaMmH,
                                          QString* erro = nullptr) const;

    const QVector<ElementoRedeHidrologica>& elementos() const;

    void limpar();

private:
    QVector<ElementoRedeHidrologica> m_elementos;
    QMap<QString, BaciaContribuicao> m_baciasPorId;
};
