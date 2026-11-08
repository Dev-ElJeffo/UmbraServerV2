# GUIA — <Tema da feature> (UE 5.6.1)

> **Template:** copie este arquivo para `docs_main/GUIA_<TEMA>_UE561.md` e remova as seções que não se aplicam.
> Referências de estilo: `GUIA_VFX_BUFF_REFINE_UE561.md`, `GUIA_BP_IMG_CONTROLLOCK_SKILLSLOT_UE561.md`.

Uma frase sobre o que a feature faz e o que **o C++ já resolve**. O Blueprint só define a parent class e o layout.

## Objetivo

- O que muda para o jogador.
- Opcodes envolvidos (ex.: **104**, **117**) e o que **não** muda.

## Dados / Manager

| Campo | Onde | Uso |
|-------|------|-----|
| `<tabela>.<coluna>` | Manager → <aba> → “<label>” | <efeito no jogo> |

SQL (rodar na ordem):

```bat
mysql -u root -p umbra_eternum < www\umbra_api\scripts\add_<tabela>_<campo>.sql
```

---

## 0) Pré-requisitos

1. Rebuild do módulo `UmbraEternumUE` (os binds/funções novas aparecem na reflexão).
2. **Parar o PIE** antes de editar o Designer.
3. SQL acima aplicado.
4. PHP sincronizado com o WAMP: `scripts_main\sync_umbra_api_to_wamp.bat`.
5. `zone_server` recompilado e reiniciado (se houve mudança no servidor).

**Assets:**

| Asset | Path | Parent Class |
|-------|------|--------------|
| <WBP/BP> | `/Game/...` | `Umbra<Classe>` |

---

## 1) Designer — <widget>

### 1.1 Hierarquia

```
[Root — Canvas / Overlay]
  Existing_Widget        ← já existe
  NOVO_Widget            ← NOVO (BindWidgetOptional)
```

### 1.2 Criar o widget

1. Hierarchy → botão direito em `<container>` → **<Tipo>**.
2. Renomear **exatamente** para `<NomeDoBind>` (maiúsculas/minúsculas importam).
3. Details:
   - **Is Variable** = ligado
   - **Visibility** = **Collapsed** (o C++ controla)
4. **Compile** + **Save**.

| Nome do bind | Tipo | Obrigatório | Função |
|--------------|------|-------------|--------|
| `<NomeDoBind>` | `UImage` | Não (`BindWidgetOptional`) | <o que o C++ faz com ele> |

---

## 2) O que o C++ já faz (não precisa de Graph)

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| `<Classe>` | `Source/UmbraEternumUE/<Pasta>/<Arquivo>.cpp` | <comportamento> |

> **Não** adicione lógica no Event Graph. Se algo parecer exigir nó, avise; o caminho é expor no C++.

## 3) Fluxo de nós (só se inevitável)

```
[Event <X>]
  → [Get] <Objeto>
  → [Is Valid]
       True → [<Função C++>]
                Target: <Objeto>
                <Pin>: <valor>
```

---

## 4) Protocolo

### Opcode <N> (`<Nome>`)

| Campo | Tipo | Observação |
|-------|------|------------|
| `<campo>` | `u32` / `f32` / `u16+utf8` | **trailing**; frames antigos → 0 / string vazia |

Servidor: `src/zone/MovementProtocol.hpp` (encode/decode), `<serviço>`.
Cliente: `<handler>` em `UmbraGameInstance.cpp`.

## 5) Manager — passo a passo

1. Abrir **UmbraManager** → **<aba>**.
2. Selecionar/criar <registro>, preencher **<campo>**.
3. **Salvar** (o Manager envia `reload_<x>` para a zone).

---

## 6) QA (PIE)

PIE com **Number of Players ≥ 2**.

| Cenário | Esperado |
|---------|----------|
| <ação local> | <resultado local> |
| <ação vista pelo remoto> | <resultado remoto> |
| Frame antigo / campo vazio | Sem crash, comportamento padrão |

Logs esperados: `"<texto literal do log>"`.

## 7) Troubleshooting

| Sintoma | Causa provável | Ação |
|---------|----------------|------|
| <sintoma> | <causa> | <ação> |

## 8) Referência de código

- Servidor: `src/zone/<arquivo>` (`<função>`)
- Cliente: `UmbraEternumUE/Source/UmbraEternumUE/<arquivo>` (`<função>`)
- PHP/SQL: `www/umbra_api/...`
- Manager: `tools/UmbraManagerWpf/UmbraManager/...`

**Fim do guia.**
