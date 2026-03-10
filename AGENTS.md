# AGENTS.md

## Objetivo do projeto
Sistema C++/Qt para modelagem hidrológica e hidráulica.

## Papel do agente Codex neste repositório
- Focar principalmente na camada de interface em `src/ui`.
- Preservar a lógica existente em `src/domain` e `src/infrastructure`.
- Fazer alterações pequenas, claras e fáceis de revisar.

## Regras importantes
- Não alterar arquivos em `src/domain` sem pedido explícito.
- Não mover fórmulas, validações ou regras hidráulicas para widgets.
- Não renomear classes públicas ou métodos públicos sem pedido explícito.
- Não fazer refatorações amplas em múltiplos módulos na mesma tarefa.
- Evitar mudanças desnecessárias em `CMakeLists.txt` e `CMakePresets.json`.

## Padrões de arquitetura
- A UI deve coletar entradas, chamar classes existentes e exibir saídas.
- Widgets não devem conter lógica de cálculo relevante.
- Sempre preferir sinais/slots e classes auxiliares de apresentação em vez de lógica pesada no widget.

## Estilo esperado
- Usar C++/Qt.
- Manter nomes coerentes com o projeto atual.
- Inserir comentários curtos quando a lógica não for óbvia.
- Preservar o padrão visual e estrutural já existente.

## Ao finalizar uma tarefa
- Resumir o que foi alterado.
- Informar quais arquivos foram modificados.
- Destacar qualquer ponto que precise de validação manual no Visual Studio.