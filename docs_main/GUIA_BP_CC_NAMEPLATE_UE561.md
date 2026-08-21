# Guia UE 5.6.1 — Ícones de CC no nameplate (`WBP_PlayerNameplate`)

Após recompilar o módulo `UmbraEternumUE`, o C++ de `UUmbraRemoteNameplateWidget` lê dois widgets **opcionais**. Sem eles no Designer, a BuffBar do HUD de seleção continua funcionando; **no mundo** o ícone de CC **não aparece**.

**Asset:** `/Game/.../WBP_PlayerNameplate` (o mesmo Widget Component do personagem local e remoto).

Parent class tem que continuar **`UmbraRemoteNameplateWidget`**.

---

## 1) Importar as texturas de CC

Placeholders PNG (64×64) em:

`UmbraEternumUE/Content/Widgets/UI/Status/Source/`

| Arquivo | Destino no Content Browser (qualquer um vale) |
|---------|-----------------------------------------------|
| `T_CC_Stun.png` | `/Game/Widgets/UI/Status/T_CC_Stun` **ou** `/Game/Widgets/UI/Status/Source/T_CC_Stun` |
| `T_CC_Silence.png` | `/Game/Widgets/UI/Status/T_CC_Silence` **ou** `.../Source/T_CC_Silence` |
| `T_CC_Root.png` | `/Game/Widgets/UI/Status/T_CC_Root` **ou** `.../Source/T_CC_Root` |
| `T_CC_Slow.png` | `/Game/Widgets/UI/Status/T_CC_Slow` **ou** `.../Source/T_CC_Slow` |

Passos no Editor **5.6.1**:

1. Content Browser → pasta `Widgets/UI/Status` **ou** `Widgets/UI/Status/Source`.
2. Arraste os 4 PNG para a pasta (importar direto em `Source` já funciona).
3. Em cada textura: **Details** → **Compression** → **Texture Group** = `UI` (não World).
4. **Never Stream** ligado (ícone de HUD).
5. **Save**.

O C++ tenta, nesta ordem: `/Game/Widgets/UI/Status/T_CC_Stun` e depois `/Game/Widgets/UI/Status/Source/T_CC_Stun` (idem para Silence/Root/Slow). Se nenhum asset existir, cai no ícone da skill.

---

## 2) Designer — `WBP_PlayerNameplate`

**Parar o PIE** antes de Compile pesado.

Hierarquia sugerida **acima** do nome (`TB_CharacterName` / `TXT_CharacterName`):

```
[Vertical Box ou Overlay do nameplate]
  IMG_CcBreak          ← Image 36×36, Visibility = Collapsed
  HBX_BuffBar          ← Horizontal Box, Alignment Center
  TB_CharacterName
  TB_Title
  TB_GuildName
  HBX_ShopBalloon      ← já existente (loja)
```

| Nome no Designer (exato) | Tipo | Comportamento |
|--------------------------|------|----------------|
| **`IMG_CcBreak`** | `Image` | Ícone **grande** só do CC quebrável. Prioridade C++: STUN > ROOT > SILENCE > SLOW. Inicial **Collapsed**. Size override ~36×36. |
| **`HBX_BuffBar`** | `Horizontal Box` | Mini-barra de buffs/debuffs daquele personagem (opcode 104). Slot padding 1–2 px. |

**Is Variable** ligado nos dois (o bind C++ usa `BindWidgetOptional` + `GetWidgetFromName`).

### Class Defaults do WBP

| Propriedade | Valor |
|-------------|--------|
| **Buff Icon Widget Class** | `WBP_BuffIcon` (o mesmo da skillbar / seleção) |
| **Buff Tooltip Widget Class** | `WBP_BuffTooltip` (opcional) |

Se a classe do ícone ficar vazia, o C++ tenta carregar `/Game/Widgets/UI/WBP_BuffIcon` e, se falhar, usa `UUmbraBuffIconWidget` puro (sem layout BP).

Compile → Save.

Não é obrigatório Event Graph: `NativeConstruct` liga `OnActiveBuffsLoaded` e `OnRemotePlayerBuffsUpdated` e filtra pelo `PlayerId` do Widget Component.

---

## 3) NPC overhead

`AUmbraNpcCharacter` já usa `UUmbraNpcOverheadWidget` 100% C++ (`WidgetTree->ConstructWidget`). **Não** precisa de WBP: o HorizontalBox de ícones e o `Image` grande de CC são criados no código. Só importe as texturas da §1.

---

## 4) HUD de seleção (já existente)

Não altere o layout de `WBP_SelectedPlayerInfo` / `WBP_SelectedNpcInfo`. Confirme o bind:

- Nome **`BuffBar_HBox`** (propriedade C++). Se no Designer estiver `BuffBar_Hbox`, teste o bind; se não preencher, alinhe o nome.

---

## 5) QA PIE

1. Rodar o seed SQL (`BARB_RUIN_STRIKE` rank 1 = STUN) e **Recarregar no Zone**.
2. Bárbaro rank 1 vs NPC: `WBP_SelectedNpcInfo` → `BuffBar_HBox` mostra `T_CC_Stun` **e** ícone grande no overhead do NPC.
3. Alvo player: idem em `WBP_SelectedPlayerInfo`.
4. Outro cliente na zona: vê o mesmo no nameplate do alvo (`IMG_CcBreak` + `HBX_BuffBar`).
5. Opcode 104 `action=1` (expire): ícones somem.
