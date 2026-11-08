# Guia de Implementação: Tooltip da Skill Bar (hover)

**Engine:** Unreal Engine 5.6.x · **Código:** submódulo `UmbraEternumUE/`

**Organização:** mesmo padrão narrativo de `UmbraServer/docs_main/GUIA_IMPLEMENTACAO_FOLLOW.md` (resumo → o que já existe → implementação por partes) e de `UmbraServer/docs_main/PROCEDIMENTO_SISTEMA_AMIZADE.md` (visão geral em tabela + fluxos Blueprint em cadeia `→`).

---

## Índice

1. [Resumo](#1-resumo)
2. [Visão geral](#2-visão-geral)
3. [O que já existe (C++)](#3-o-que-já-existe-c)
4. [Pré-requisitos de dados](#4-pré-requisitos-de-dados)
5. [Implementação](#5-implementação)
6. [Problemas frequentes](#6-problemas-frequentes)
7. [Opcional: hover em C++](#7-opcional-hover-em-c)
8. [Apêndice: nós pure vs exec](#8-apêndice-nós-pure-vs-exec)

---

## 1. Resumo

Mostrar **nome, tipo, descrição, custo, cooldown, rank, alcance** etc. ao passar o mouse sobre um slot **com skill** na skill bar. **`UUmbraSkillTooltipWidget`** (C++) preenche os textos, incluindo descrição a partir de **`tooltip_template`** + placeholders (`FormatSkillTooltipText` no `UmbraGameInstance`). O Blueprint do tooltip só precisa dos widgets com nomes corretos (**§5.1**); o slot chama **`Apply From Game Instance`** e posiciona o widget (**§5.4**).

---

## 2. Visão geral

| Peça | Responsabilidade |
|------|------------------|
| **`WBP_SkillTooltip`** (parent `UUmbraSkillTooltipWidget`) | Só **layout** e nomes `Tooltip_*`; a lógica é **`Apply From Game Instance`** no C++ da classe base. |
| **`WBP_SkillSlot`** (parent `UmbraSkillSlotWidget`) | Hover: criar/reusar tooltip, preencher, `Add to Viewport`, posição; leave: remover/ocultar. |
| **`UmbraGameInstance`** | `Load Available Skills`, `Get Skill Data By ID`, `Get Skillbar Slot`. |

---

## 3. O que já existe (C++)

### 3.1 `UmbraSkillTooltipWidget` (`UI/UmbraSkillTooltipWidget.h`)

- `SetSkillData` / `SetSkillDataFull` / `ClearTooltip`
- `BindWidgetOptional`: nomes exatos em **§5.1** (tabela `Tooltip_*`).

### 3.2 `UmbraSkillSlotWidget` (`UI/UmbraSkillSlotWidget.h`)

- `SkillID`, `SlotIndex`, `Skill_Icon` (Image), etc.

### 3.3 `UmbraGameInstance` — Skills (nós típicos no Blueprint)

| Função | Nó | Tipo |
|--------|-----|------|
| `LoadAvailableSkills` | **Load Available Skills** | Callable (fio branco) |
| `GetSkillDataByID` | **Get Skill Data By ID** | Pure (só dados) |
| `GetSkillbarSlot` | **Get Skillbar Slot** | Pure (só dados) |

Pure = sem execução; ligam **Condition** / pins de dados em nós brancos (`Branch`, `Set Skill Data Full`, …).

### 3.4 Arquivos

- `UmbraSkillTooltipWidget.h` / `.cpp`, `UmbraSkillSlotWidget.h`, `UmbraSkillDataStructures.h`, `UmbraGameInstance.h`

---

## 4. Pré-requisitos de dados

`GetSkillDataByID` depende de **`AvailableSkills`**, preenchido após **`LoadAvailableSkills()`**. Se isso nunca rodou, use fallback: **`Get Skillbar Slot`** (`SlotIndex`) + **`Set Skill Data`**.

**Recomendado:** disparar **`Load Available Skills`** cedo (mundo/HUD) **e** manter o fallback no tooltip.

---

## 5. Implementação

### 5.1 Parte 1 — Asset `WBP_SkillTooltip` (Designer)

#### 1.1 Criar o Widget Blueprint

1. No **Content Browser**, pasta sugerida: `Content/Widgets/UI/Skills/` (ou a que você usa para skills).
2. **Clique direito → User Interface → Widget Blueprint**.
3. Nome sugerido: **`WBP_SkillTooltip`**.

#### 1.2 Reparentar para a classe C++

1. Abrir `WBP_SkillTooltip`.
2. **Barra superior → Class Settings** (ícone de engrenagem / “Class Defaults”).
3. **Parent Class** → pesquisar **`UmbraSkillTooltip`** / **`Skill Tooltip Widget`** (nome de exibição da `UUmbraSkillTooltipWidget`).
4. **Compile** e **Save**.

#### 1.3 Criar widgets com nomes EXATOS (BindWidgetOptional)

O C++ associa pelo **nome do widget** no Designer. Se o nome estiver errado, esse campo não é atualizado (não quebra o jogo).

| Nome EXATO no Designer | Tipo UMG |
|------------------------|----------|
| `Tooltip_Icon` | **Image** |
| `Tooltip_SkillName` | **Text Block** |
| `Tooltip_SkillType` | **Text Block** |
| `Tooltip_Element` | **Text Block** |
| `Tooltip_Description` | **Text Block** |
| `Tooltip_Effects` | **Text Block** (lista: dano, CC, buffs — **não** repetir na descrição) |
| `Tooltip_Cost` | **Text Block** |
| `Tooltip_Cooldown` | **Text Block** |
| `Tooltip_CastTime` | **Text Block** |
| `Tooltip_Range` | **Text Block** |
| `Tooltip_Duration` | **Text Block** |
| `Tooltip_Rank` | **Text Block** |
| `Tooltip_Target` | **Text Block** |
| `Tooltip_Power` | **Text Block** |
| `Tooltip_Scaling` | **Text Block** |

#### 1.4 Layout sugerido (Designer)

Estrutura mínima legível:

```
Border (fundo escuro semi-opaco, padding 12)
  └── VerticalBox
        ├── HorizontalBox (ícone + nome)
        │     ├── Tooltip_Icon  (64x64 ou 48x48)
        │     └── Tooltip_SkillName (fonte maior, Bold)
        ├── Tooltip_SkillType
        ├── Tooltip_Element
        ├── Tooltip_Rank
        ├── Tooltip_Target
        ├── Tooltip_Cost
        ├── Tooltip_Cooldown
        ├── Tooltip_CastTime
        ├── Tooltip_Range
        ├── Tooltip_Duration
        ├── Tooltip_Power
        ├── Tooltip_Scaling
        ├── Tooltip_Description   (só o template: “Causa 180 (252) de dano…”)
        └── Tooltip_Effects       (bullets: Dano, Atordoado, Double Attack…)
```

**Descrição vs Effects:** `FormatSkillTooltipText` **não** anexa a lista de efeitos. `{damage}` vira `180 (252)` (base e rank). CC entra só em `Tooltip_Effects`.

**Boas práticas:**

- Em **`Tooltip_Description`**: ativar **Wrap Text At** (ex.: 300-360) para textos longos.
- Tamanho do root: **não** forçar fullscreen; tooltip compacto.
- **Visibility** padrão: **Collapsed** ou **Hidden** até o primeiro hover (se você controlar só por código).

#### 1.5 Compile e Save

Salvar o asset antes de referenciar no WBP do slot.

---

### 5.2 Parte 2 — `ApplyFromGameInstance`

A implementação **completa** está em **`UUmbraSkillTooltipWidget::ApplyFromGameInstance`** (C++): `GetSkillDataByID`, ajuste de rank pelo slot da barra, `SetSkillDataFull` / fallback `SetSkillData`, e descrição via **`FormatSkillTooltipText`** (não uses o campo `Description` cru no Blueprint).

**No `WBP_SkillTooltip`:** não precisas de Graph com `Get Skill Data By ID` — **reparent** o asset para **`UmbraSkillTooltipWidget`**, compila o módulo, e no slot chama só o nó **`Apply From Game Instance`** (Skill ID, Slot Index, Icon Brush).

*(Se mantiveres uma função Blueprint com o mesmo nome, remove-a ou reparenta para não sobrescrever o C++.)*

**Texto legado abaixo** — fluxo equivalente em Blueprint **só** se não usares a classe C++:

#### 2.0 Variáveis no `WBP_SkillSlot` (antes do hover)

| Onde | Nome sugerido | Tipo | Detalhe |
|------|---------------|------|---------|
| `WBP_SkillSlot` | `SkillTooltipInstance` | Object Reference → **`WBP_SkillTooltip`** (self-class) | Instância reutilizada do tooltip. |
| `WBP_SkillSlot` | `TooltipZOrder` | **Integer** | Padrão `1000`. |
| `WBP_SkillTooltip` | *(parâmetros da função)* | Ver **§5.2** |

**Parent do slot:** `UmbraSkillSlotWidget`. Propriedades **`Skill ID`**, **`Slot Index`**, **`Skill_Icon`**.

#### 2.1 Função `ApplyFromGameInstance` — pin a pin

**LOCALIZAÇÃO:** `WBP_SkillTooltip` → **Graph** → **Functions** → **+ Function** → nome: **`ApplyFromGameInstance`**.

**Parâmetros da função (Function Entry):** `SkillID` (Integer), `SlotIndex` (Integer), `IconBrush` (Slate Brush).

**Regra:** `Get Game Instance` é nó **pure** (verde, sem fio branco). O fio branco começa no **`Function Entry`** e vai direto para o **`Cast To UmbraGameInstance`**.

---

##### Dados (fios coloridos) — montar primeiro

| Passo | Do pin | Para pin |
|-------|--------|----------|
| D1 | `Get Game Instance` → **Return Value** | `Cast To UmbraGameInstance` → **Object** |
| D2 | Saída **`Umbra Game Instance`** do `Cast To UmbraGameInstance` | `Get Skill Data By ID` → **Target** |
| D3 | `Function Entry` → **Skill ID** | `Get Skill Data By ID` → **Skill ID** |
| D4 | `Get Skill Data By ID` → **Return Value** (bool) | `Branch A` → **Condition** |
| D5 | `Get Skill Data By ID` → **Out Skill Data** | `Set Skill Data Full` → **Skill Data** |
| D6 | `Function Entry` → **Icon Brush** | `Set Skill Data Full` → **Icon Brush** e `Set Skill Data` → **Icon Brush** |
| D7 | Saída **`Umbra Game Instance`** do `Cast` | `Get Skillbar Slot` → **Target** |
| D8 | `Function Entry` → **Slot Index** | `Get Skillbar Slot` → **Slot Index** |
| D9 | `Get Skillbar Slot` → **Return Value** (bool) | `Branch B` → **Condition** |
| D10 | `Get Skillbar Slot` → **Out Slot** | `Set Skill Data` → **Slot Data** |

**Targets de widget (Self):** arraste **Self** para o gráfico e ligue a **Target** em: `Set Skill Data Full`, `Set Skill Data`, `Clear Tooltip` (todas as ocorrências).

---

##### Execução (fio branco)

**Cadeia equivalente (estilo `PROCEDIMENTO_SISTEMA_AMIZADE`):**  
`Function Entry` → `Cast To UmbraGameInstance` → (*Cast Failed* → `Clear Tooltip` (Self) → `Return`*) | (*OK* → `Branch A` → *True* → `Set Skill Data Full` → `Return`*) | (*False* → `Branch B` → *True* → `Set Skill Data` → `Return`*) | (*False* → `Clear Tooltip` → `Return`*)

Lista numerada:

1. `Function Entry` → **execute** → `Cast To UmbraGameInstance` → **execute** (entrada).
2. `Cast To UmbraGameInstance` → **Cast Failed** → `Clear Tooltip` → **execute** → `Return Node`.
3. `Clear Tooltip` → **Target** = **Self** (via nó **Self**).
4. `Cast To UmbraGameInstance` → **saída de sucesso do cast** (exec branco após cast OK) → `Branch A` → **execute** (entrada).
5. `Branch A` → **True** → `Set Skill Data Full` → **execute** → `Return Node`.
6. `Set Skill Data Full` → **Skill Data** já ligado em **D5**; **Icon Brush** em **D6**; **Target** = **Self**.
7. `Branch A` → **False** → `Branch B` → **execute** (entrada).
8. `Branch B` → **True** → `Set Skill Data` → **execute** → `Return Node`.
9. `Set Skill Data` → **Slot Data** em **D10**; **Icon Brush** em **D6**; **Target** = **Self**.
10. `Branch B` → **False** → `Clear Tooltip` (**Target** = **Self**) → **execute** → `Return Node`.

**Nomes usados:** `Branch A` = primeiro `Branch` (dados completos?). `Branch B` = segundo `Branch` (fallback skillbar?).

**Resumo em uma linha (exec):**  
`Function Entry` → `Cast` → (*Failed* → `Clear` → `Return`*) | (*OK* → `Branch A` → *True* → `Set Skill Data Full` → `Return`*) | (*False* → `Branch B` → *True* → `Set Skill Data` → `Return`*) | (*False* → `Clear` → `Return`*)

---

### 5.3 Parte 3 — `WBP_SkillSlot` (hover)

O slot de jogo herda **`UUmbraSkillSlotWidget`**. O WBP (ex.: `WBP_SkillSlot`) deve:

1. **Detectar hover** em uma área que não quebre o drag.
2. **Criar ou reutilizar** uma instância de `WBP_SkillTooltip`.
3. **Preencher** dados e **mostrar**.
4. No **mouse leave**, **esconder** ou **remover** o tooltip.

#### 3.1 Hit-test (Designer)

- Preferir um **Border** ou **Overlay** que envolva o **ícone + keybind**, com **Visibility** hit-testável.
- Evitar cobrir com um **Image HitTestInvisible** que bloqueie eventos se você precisar de Hover no pai — testar no PIE.

#### 3.2 On Mouse Enter — conexão pin a pin

**LOCALIZAÇÃO:** `WBP_SkillSlot` → **Designer** → **Border** (hit-test) → **Details → Events → On Mouse Enter** (+).

**Pré-requisito:** **§5.2** (`ApplyFromGameInstance` no `WBP_SkillTooltip`).

#### Variável `SkillTooltipInstance` (criar uma vez)

No **`WBP_SkillSlot`**: **My Blueprint** → **Variables** → tipo **Object Reference** → classe **`WBP_SkillTooltip`**. Valor inicial **None**. Arraste para o gráfico para obter **Get Skill Tooltip Instance** quando o guia disser “referência da variável”.

---

##### Dados (fios coloridos)

| Passo | Do pin | Para pin |
|-------|--------|----------|
| CD1 | Arraste **`Skill_Icon`** do *Hierarchy* → **Get Skill_Icon** (referência) | `Get Brush` → **Target** |
| CD2 | `Get Brush` → **Return Value** (Slate Brush) | `Apply From Game Instance` → **Icon Brush** |
| CD3 | **Integer > Integer** (A = **Skill ID**, B = `0`) → **Return Value** | `Branch Skill` → **Condition** |
| CD4 | **Get** variável **`Slot Index`** | `Apply From Game Instance` → **Slot Index** |
| CD5 | **Get** variável **`Skill ID`** | `Apply From Game Instance` → **Skill ID** |
| CD6 | **Get Skill Tooltip Instance** | `Is Valid Tooltip` → **Input Object** |
| CD7 | **Get Skill Tooltip Instance** | `Apply From Game Instance` → **Target**; `Add to Viewport` → **Target**; `Set Position in Viewport` → **Target** (**§5.4**) |

**Nomes usados no texto:** `Branch Skill` = `Branch` que testa se há skill no slot. `Is Valid Tooltip` = nó **Is Valid** que testa a variável do tooltip.

---

##### Execução (fio branco)

**Cadeia (mesmo estilo dos guias Party/Friend):**  
`On Mouse Enter` → `Branch` (Skill ID > 0) → *False* → fim | *True* → `Is Valid` (SkillTooltipInstance) → *False* → `Create Widget` → `SET SkillTooltipInstance` ─┐ → *True* ─────────────────────────────────────────┘→ `Apply From Game Instance` → `Add to Viewport` → **§5.4** (posição)

Lista numerada:

1. `On Mouse Enter` → **Then** → `Branch Skill` → **execute** (entrada).
2. `Branch Skill` → **False** → *(fim do fluxo)* — sem skill no slot.
3. `Branch Skill` → **True** → `Is Valid Tooltip` → **execute** (entrada).
4. `Is Valid Tooltip` → **False** → `Create Widget` → **execute** (entrada).
5. `Create Widget` → **Class** = `WBP_SkillTooltip`.
6. `Get Owning Player` (**Target** = **Self**) → ligue ao pin **Owning Player** do `Create Widget`.
7. `Create Widget` → **Return Value** → entrada do **`SET Skill Tooltip Instance`** (o widget criado vira valor da variável).
8. `SET Skill Tooltip Instance` → **Then** → **entrada execute** de **`Apply From Game Instance`** *(primeiro fio)*.
9. `Is Valid Tooltip` → **True** → **mesma entrada execute** de **`Apply From Game Instance`** *(segundo fio — junção no mesmo pin)*.
10. `Apply From Game Instance` → **Then** → `Add to Viewport` → **execute** (entrada).
11. `Add to Viewport` → **Target** = saída de **Get Skill Tooltip Instance**. **Z Order** = `TooltipZOrder` ou `1000` (**Advanced**).
12. `Add to Viewport` → **Then** → posição: **§5.4** (`Get Owning Player` → `Get Mouse Position on Viewport` → offset → `Set Position in Viewport`).

**Junção (passos 8 e 9):** o nó **`Apply From Game Instance`** tem **uma** entrada de execução; **dois** fios chegam nela — um do **`SET`** (primeira vez) e um do **`Is Valid` = True** (já existe instância). O motor aceita os dois no mesmo pin.

**Resumo visual:**  
`Branch Skill` True → `Is Valid` → *False* → `Create Widget` → `SET` ─┐  
                              → *True* ─────────────────────────────┼→ `Apply From Game Instance` → `Add to Viewport` → §5.4  

**Sem função no tooltip:** substitua **`Apply From Game Instance`** pela lógica da **§5.2** no grafo do slot, com **Target** = **Get Skill Tooltip Instance**.

---

#### 3.3 On Mouse Leave (exec)

**LOCALIZAÇÃO:** mesmo **Border** → **On Mouse Leave** (+).

1. `On Mouse Leave` → **Then** → `Is Valid` (**Input Object** = **Get Skill Tooltip Instance**).
2. `Is Valid` → **True** → `Remove from Parent` (**Target** = **Get Skill Tooltip Instance**).
3. *(Opcional)* `Remove from Parent` → **Then** → `Clear Tooltip` (**Target** = **Get Skill Tooltip Instance**).

**Variante:** troque **Remove from Parent** por **Set Visibility** (Collapsed) + **Clear Tooltip**.

**Variáveis no `WBP_SkillSlot`:** `SkillTooltipInstance` (Object → `WBP_SkillTooltip`); `TooltipZOrder` (Integer, ex.: 1000).

#### 3.4 Conflito com Drag & Drop

O slot usa **DetectDrag** no botão esquerdo (`NativeOnMouseButtonDown` em C++).

- Verificar no PIE: hover **não** deve impedir o início do drag.
- Se o hover disparar com atraso e o tooltip aparecer **em cima** do cursor, ajustar **offset** na posição ou **ZOrder** apenas do tooltip (tooltip **Self Hit Test Invisible** pode ajudar a não “roubar” o rastro do cursor em alguns casos — testar).

---

### 5.4 Posicionar o tooltip no viewport (passo a passo)

**Contexto:** no `WBP_SkillSlot`, no fluxo do **On Mouse Enter**, depois de preencher o tooltip e de **`Add to Viewport`**. A posição é **sempre o último passo** (nunca antes de o tooltip existir no viewport).

**Regra de ouro:** `Set Position in Viewport` **depois** de `Add to Viewport` **e** depois de aplicar dados (`Apply From Game Instance` ou equivalente), para o tamanho do conteúdo estar estável.

Faça **nesta ordem**:

1. **Mouse** — `Get Owning Player` (self = slot) → `Get Mouse Position on Viewport` → guarde **Mx** e **My** (componentes X e Y do `Vector2D`).

2. **Tamanho do retângulo do tooltip** — defina dois números em pixels:
   - **TW** = largura (use a largura fixa do teu painel no Designer, ex. 320, **ou** `Get Desired Size` no widget do tooltip após o `Add to Viewport`; se vier 0, use o valor fixo).
   - **TH** = altura (idem, ex. 200–280, ou `Desired Size`).

3. **Margem** — um número pequeno, ex. **12**, entre o cursor e a borda do tooltip.

4. **Posição bruta (à esquerda do ponteiro)** — não uses offset (+16,+16) “para a direita/baixo” na barra de baixo: isso cobre os ícones e corta no rodapé.
   - **PosX** = **Mx − TW − Margem**
   - **PosY** = **My − (TH × 0,5)**  *(tooltip centrado na vertical com o cursor; alternativa: **My − TH − Margem** para ficar totalmente acima do cursor)*

5. **Tela** — `Get Viewport Size` → **SizeX** e **SizeY**.

6. **Clamp** (para não sair da tela):
   - **PosX** = `Clamp(PosX, 0, SizeX − TW)` — se **SizeX − TW** for negativo, usa **0**.
   - **PosY** = `Clamp(PosY, 0, SizeY − TH)`.

7. **Aplicar** — `Set Position in Viewport` → **Target** = instância do tooltip → **Position** = **Make Vector2D(PosX, PosY)**. Se em resoluções altas a posição falhar, experimenta marcar **Remove DPI Scale** neste nó.

8. **Z order** — já definido no `Add to Viewport` (ex. 1000); não precisa repetir no `Set Position`.

9. **Opcional** — se quiseres que o tooltip **acompanhe** o rato enquanto o hover está ativo: **Set Timer** (0,03 s) a repetir os passos 1 e 4–7, e no **On Mouse Leave** **Clear Timer**.

---

### 5.5 Parte 5 — Testes

| Teste | Resultado esperado |
|-------|-------------------|
| Hover em slot **com** skill | Tooltip com nome + campos preenchidos; descrição se `GetSkillDataByID` OK. |
| Hover em slot **vazio** (`SkillID == 0`) | Nenhum tooltip (ou mensagem “Vazio” se você implementar). |
| Sem `LoadAvailableSkills` prévio | Fallback `SetSkillData` ainda mostra dados vindos da skillbar API (nome, CD, custo). |
| Abrir livro depois do hover | Após `LoadAvailableSkills`, próximo hover deve mostrar descrição completa. |
| Arrastar skill do slot | Drag continua funcionando; tooltip fecha no leave. |
| Múltiplos slots | Cada slot usa o mesmo `Skill Tooltip Instance` ou instância por HUD — evita dois tooltips abertos (fechar ao trocar de slot). |

---

## 6. Problemas frequentes

| Sintoma | Causa provável | Ação |
|---------|----------------|------|
| Campos do tooltip vazios | Nomes dos widgets diferentes da tabela do passo A.3 | Renomear no Designer. |
| Sempre fallback “pobre” | `AvailableSkills` vazio | Chamar `LoadAvailableSkills` mais cedo. |
| `GetSkillDataByID` sempre false | SkillID errado ou lista não carregada | Logar `SkillID` no hover; verificar HTTP no Output Log. |
| Tooltip não aparece | Não executou `Add to Viewport` ou Z-order atrás do HUD | Subir ZOrder (ex.: 1000+). |
| Tooltip bloqueia cliques | Hit test no tooltip | Visibility **Self Hit Test Invisible** no root do tooltip (testar). |
| Tooltip **só na primeira vez** hover | No **`Is Valid`**, o pin **True** não liga na mesma entrada exec de **`Apply From Game Instance`** | Juntar os dois fios no **mesmo** pin de exec do **`Apply`** (**§5.3**, passos 8–9). |
| **Dados diferentes** entre hovers | Recriar widget sem **`Apply`** com **Skill ID** / **Slot Index** atuais, ou instância velha | Sempre **`Apply From Game Instance`** após garantir pins de dados corretos; preferir **uma** instância reutilizada. |
| Tooltip **corta embaixo** ou cobre a barra | Offset no estilo (+X,+Y) sem clamp | Seguir **§5.4** (à esquerda do mouse + clamp). |
| Descrição cortada | Sem Wrap / caixa estreita | **Wrap Text At** no `Tooltip_Description`. |

---

## 7. Opcional: hover em C++

Se quiser **centralizar** hover no C++ (menos Blueprint):

1. Em `UmbraSkillSlotWidget.h`: declarar `NativeOnMouseEnter` / `NativeOnMouseLeave` (ou `OnMouseEnter` da camada UMG, se preferir message router).
2. Propriedades: `TSubclassOf<UUmbraSkillTooltipWidget> TooltipClass`, ponteiro `UUmbraSkillTooltipWidget* ActiveTooltip`.
3. No `.cpp`: criar tooltip, `GI->GetSkillDataByID`, chamar `SetSkillDataFull` / `SetSkillData`, posicionar.

**Vantagem:** Um único local para regras. **Custo:** recompilar + alinhar com todos os WBP de slot.

---

## 8. Apêndice: nós pure vs exec

| Tipo | Cor | Fio branco? | Exemplos |
|------|-----|-------------|----------|
| **Callable** | Branco | Sim | `Cast`, `Branch`, `SET`, `Apply From Game Instance`, `Add to Viewport` |
| **Pure** | Verde | Não | `Get Game Instance`, `Get Skill Data By ID`, `Get Skillbar Slot`, `Get Brush`, `Integer > Integer` |

**Regra:** o fio branco nunca “entra” em nó pure; os pure só ligam **dados** aos pins dos callables.

**Se um nó da API não aparecer na paleta:** recompile o módulo **UmbraEternumUE** e confira **Parent Class** do widget / cast para **`UmbraGameInstance`**.

---

## Referências

- Drag de skills (mesmo ecossistema de UI): `UmbraEternumUE/docs/GUIA_SKILL_DRAG_DROP.md`
- Estrutura “resumo + o que existe + partes”: `UmbraServer/docs_main/GUIA_IMPLEMENTACAO_FOLLOW.md`
- Fluxos Blueprint em cadeia (`→`): `UmbraServer/docs_main/PROCEDIMENTO_SISTEMA_AMIZADE.md`

---

**Fim do guia.** Atualizar se `FUmbraSkillData` ou `BindWidgetOptional` em `UmbraSkillTooltipWidget` mudarem.
