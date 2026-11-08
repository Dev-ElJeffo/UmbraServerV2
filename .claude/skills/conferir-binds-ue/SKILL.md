---
name: conferir-binds-ue
description: Confere no Unreal Editor (via MCP EngineWire) se os WBP/BP de uma feature batem com o C++ e com o guia, olhando a Parent Class, os nomes e tipos dos widgets BindWidgetOptional, os assets /Game/... referenciados e a ausência de lógica no Event Graph. Só leitura. Usar depois que o usuário montar o WBP seguindo um guia, quando um bind "não funciona", ou quando o usuário pedir /conferir-binds-ue.
---

# Conferir binds UE ↔ C++ ↔ guia

**Só leitura.** Nunca editar asset pelo MCP sem pedido explícito: o Editor é papel do usuário. Requer o editor aberto com o EngineWire (`mcp__unreal-engine__unreal`, `http://127.0.0.1:3000/mcp`); se der `NOT_CONNECTED`, avisar e parar.

## 1) Montar o esperado (lado C++)

A partir do guia (`docs_main/GUIA_<TEMA>_UE561.md`) ou da classe indicada:

```bash
cd /d/UmbraServerV2/UmbraEternumUE/Source/UmbraEternumUE
grep -n -A1 "BindWidgetOptional\|meta *= *(BindWidget)" UI/<Classe>.h
grep -n "GetWidgetFromName" UI/<Classe>.cpp          # nomes alternativos aceitos
grep -n "TSubclassOf\|EditDefaultsOnly" UI/<Classe>.h # classes de linha/item a preencher no Details
```

Tabela esperada: `Nome do bind | Tipo C++ (UImage, UTextBlock...) | Obrigatório? | Alternativos`, mais a Parent Class (`Umbra<Classe>`) e as propriedades `EditDefaultsOnly` que o usuário precisa preencher.

## 2) Ler o asset no Editor (MCP)

Fluxo do EngineWire: **search → describe → execute** (as instruções do servidor MCP estão no contexto).
- `search` com 2-4 palavras, ex.: `"inspect widget blueprint"`, `"widget tree"`, `"blueprint parent class"`, `"blueprint graph nodes"`.
- Usar só capacidades com `effect: read`.
- Dados a obter: Parent Class, a árvore de widgets (nome, classe e se é variável), as propriedades de Class Defaults relevantes e os nós do Event Graph.

Path do asset: o do guia (`/Game/...`). Se o usuário não souber, buscar pelo nome com uma capacidade de listar assets.

## 3) Comparar

| Checagem | Problema |
|---|---|
| Parent Class ≠ classe C++ do guia | O C++ não roda; nada funciona |
| Bind com nome diferente (maiúsculas importam) ou tipo diferente | O ponteiro fica `nullptr` em silêncio (é `Optional`) |
| Widget com o nome certo mas **Is Variable** desligado | Bind não resolve |
| `TSubclassOf` / asset `EditDefaultsOnly` vazio | Linhas dinâmicas não aparecem |
| Nós no Event Graph além do padrão vazio | Violação do Blueprint mínimo: listar os nós e propor mover para C++ |
| Soft path `/Game/...` do Manager ou do guia inexistente | `LoadObject` falha (ver o log `[Classe]`) |

## 4) Relatório

```
| Item | Esperado (C++/guia) | No asset | Status |
```

Para cada divergência: a correção que o **usuário** faz no Editor (cliques em **negrito**, nome exato). Se o erro estiver no guia ou no C++, propor o ajuste do arquivo (com OK).
