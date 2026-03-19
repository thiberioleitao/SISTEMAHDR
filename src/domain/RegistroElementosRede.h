#pragma once

#include <QMap>
#include <QString>
#include <QVector>

class RedeHidrologica;
enum class TipoElementoRede;

// Representa um elemento basico da topologia da rede.
class ElementoTopologicoInterno
{
public:
    ElementoTopologicoInterno();
    ElementoTopologicoInterno(const QString& id,
                              const QString& idJusante,
                              TipoElementoRede tipo);

    QString id;
    QString idJusante;
    TipoElementoRede tipo;
};

// Estrutura de apoio para ordenacao topologica.
class EstruturaTopologica
{
public:
    void limpar();

    QMap<QString, int> indicePorId;
    QMap<QString, int> grauEntrada;
    QVector<QString> ordemTopologica;
};

// Monta uma lista topologica com ID, jusante e tipo da rede.
QVector<ElementoTopologicoInterno> criarElementosTopologicos(const RedeHidrologica& rede);
