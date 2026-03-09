#pragma once

#include <QMap>
#include <QString>
#include <QVector>

struct ElementoRedeHidrologica
{
    QString id;
    QString idJusante;
};

class RedeHidrologica
{
public:
    RedeHidrologica() = default;

    bool existeId(const QString& id) const;

    QString gerarProximoId(const QString& prefixo = "E") const;

    bool adicionarElemento(const QString& id,
                          const QString& idJusante,
                          QString* erro = nullptr);

    QString adicionarElementoComIdAutomatico(const QString& idJusante,
                                             const QString& prefixo = "E",
                                             QString* erro = nullptr);

    const QVector<ElementoRedeHidrologica>& elementos() const;

    QMap<QString, double> calcularVazaoAcumuladaPorElemento(
        const QMap<QString, double>& contribuicaoPorElemento = QMap<QString, double>(),
        QString* erro = nullptr) const;

    void limpar();

private:
    QVector<ElementoRedeHidrologica> m_elementos;
};
