#pragma once

#include "BaciaContribuicao.h"

#include <QMap>
#include <QString>

class RedeHidrologica;

class RegistroElementosRede
{
public:
    RegistroElementosRede() = default;

    bool associarBaciaAoElemento(const RedeHidrologica& rede,
                                 const QString& idElemento,
                                 const BaciaContribuicao& bacia,
                                 QString* erro = nullptr);

    const QMap<QString, BaciaContribuicao>& baciasPorId() const;

    const BaciaContribuicao* baciaPorId(const QString& idElemento) const;

    double contribuicaoBaciasParaElemento(const RedeHidrologica& rede,
                                          const QString& idElemento,
                                          double intensityChuvaBrutaMmH,
                                          QString* erro = nullptr) const;

    void limpar();

private:
    QMap<QString, BaciaContribuicao> m_baciasPorId;
};
