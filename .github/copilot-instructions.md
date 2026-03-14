# Copilot Instructions

## Project Guidelines
# Copilot Instructions

## Contexto
Projeto SISTEMAHDR em **C++/Qt**, usando **Visual Studio**, **CMake** e **Git**.

## Diretrizes
- Responder e gerar código em **português**.
- Usar **nomes em português** para variáveis, funções, métodos e classes.
- Adicionar **comentários curtos** explicando a lógica.
- Priorizar código **simples, legível e fácil de manter**.
- Evitar abstrações excessivas e refatorações desnecessárias.
- Preservar o padrão e a arquitetura já existentes no projeto.

## Implementação
- Preferir tipos Qt quando fizer sentido: `QString`, `QMap`, `QVector`, `QQueue` etc.
- Usar `const` e referências constantes sempre que possível.
- Separar regra de negócio da interface gráfica.
- Evitar duplicação de código.
- Validar entradas e retornar mensagens de erro claras.

## Domínio
O projeto trata de **hidrologia, hidráulica e drenagem**.
Ao implementar cálculos:
- deixar claras as **unidades**;
- documentar hipóteses importantes;

## Evitar
- lógica de cálculo dentro da UI;
- nomes vagos;
- bibliotecas externas sem necessidade;
- mudanças grandes fora do escopo solicitado.