# Guia UE 5.6.1 — Overlay `IMG_ControlLock` no skill slot (STUN / SILENCE)

Após recompilar o módulo `UmbraEternumUE`, o C++ de `UUmbraSkillSlotWidget` já gerencia o lock visual via `SetControlLock` / `ApplyControlLockVisual`. Sem o widget **`IMG_ControlLock`** no Designer, o ícone da skill ainda fica cinza sob CC, mas **não há overlay** (cadeado / ícone de CC).

**Assets (parent class `UmbraSkillSlotWidget`):**

| Asset | Path |
|-------|------|
| Slot principal | `/Game/Widgets/UI/Skills/WBP_SkillSlot` |
| Variante (se usada na barra) | `/Game/Widgets/UI/Skills/WBP_SkillSlot1` |
| Barra HUD | `/Game/Widgets/HUD/WBP_SkillBar1` (só referencia os slots; **não** precisa do `IMG_ControlLock`) |

Aplique o overlay em **todos** os WBPs de slot que a `WBP_SkillBar1` instancia (MainBar + SecondaryBar).

---

## 0) Pré-requisitos

1. Rebuild do editor: `UmbraEternumUE` compilado (aparece `IMG_ControlLock` / `Set Control Lock` na reflexão).
2. **Parar o PIE** antes de editar o Designer.
3. Texturas de CC importadas (opcional para brush default do overlay):

   `UmbraEternumUE/Content/Widgets/UI/Status/Source/` → `/Game/Widgets/UI/Status/Source/`

   | Textura | Uso sugerido no brush |
   |---------|------------------------|
   | `T_CC_Stun` | Brush default do overlay (STUN) |
   | `T_CC_Silence` | Alternativa (SILENCE) |
   | Cadeado genérico / branco | Também serve; o C++ aplica tint |

---

## 1) Designer — adicionar `IMG_ControlLock`

Abra **`WBP_SkillSlot`** (e `WBP_SkillSlot1` se existir na barra).

### 1.1 Hierarquia

Coloque o Image **acima** do `Skill_Icon` (mesma área do ícone), tipicamente dentro do mesmo `Canvas` / `Overlay` / `Border` do slot:

```
[Root do slot — Border_Background / Canvas / Overlay]
  Skill_Icon              ← já existe
  Cooldown_Overlay        ← já existe (fill + texto)
  Resource_Warning        ← já existe (mana insuficiente)
  IMG_ControlLock         ← NOVO — cobrindo o ícone
  Keybind_Text / Quantity_Text / Keybind_Button
```

Ordem no **Overlay** (Z-order): `Skill_Icon` (fundo) → `IMG_ControlLock` → `Cooldown_Overlay` (pode ficar por cima do lock) → textos.

### 1.2 Criar o widget

1. Hierarchy → botão direito no container do ícone → **Image**.
2. Renomear **exatamente** para: **`IMG_ControlLock`**  
   (o bind C++ é `BindWidgetOptional` + nome do member; maiúsculas/minúsculas importam.)
3. Details:
   - **Is Variable** = ligado (padrão ao criar Image com nome).
   - **Visibility** = **Collapsed** (o C++ abre com `HitTestInvisible` quando locked).
   - **Size** = mesmo do `Skill_Icon` (ex.: 48×48 ou o Desired Size do slot).
   - **Anchors** = fill no slot do ícone (Stretch 0–1 se for Overlay Slot), ou offsets iguais ao `Skill_Icon`.
   - **Alignment** = centro.
4. **Brush**:
   - Texture = `T_CC_Stun` **ou** `T_CC_Silence` **ou** textura de cadeado.
   - Draw As = Image.
   - Tint no Designer pode ficar branco; em runtime o C++ sobrescreve:
     - **STUN** → amarelo ~`(0.95, 0.75, 0.15, 0.85)`
     - **SILENCE** → azul ~`(0.45, 0.55, 0.95, 0.85)`
5. **Importante:** **não** use `Set Is Enabled = false` no slot raiz. O C++ **não** desabilita o widget inteiro (preserva drag-and-drop).
6. **Hit Test:** Visibility em runtime é `HitTestInvisible` — o overlay **não** come o clique/drag. Deixe **Visibility Self Hit Test Invisible** só se testar no Designer; em PIE o C++ manda.

Compile + Save o WBP.

Repita em **`WBP_SkillSlot1`** se a barra usar essa classe.

---

## 2) O que o C++ já faz (não precisa Graph)

| Peça | Arquivo | Comportamento |
|------|---------|----------------|
| Bind | `UmbraSkillSlotWidget.h` | `IMG_ControlLock` (`BindWidgetOptional`) |
| Aplicar | `SetControlLock` / `ApplyControlLockVisual` | STUN: todos os slots com conteúdo; SILENCE: só `EntryType == Skill` |
| Ícone cinza | `Skill_Icon` ColorAndOpacity `0.4` | Mesmo com overlay ausente |
| Quem chama | `UUmbraSkillBarWidget::RefreshControlLocks` | Escuta `OnCombatControlStateUpdated` (player local) + pós-`PopulateSkillBar` / buffs |

**Não é necessário** Event Graph com `On Combat Control State Updated` no slot — a SkillBar já propaga.

Gate de cast (independente do overlay):

- `UseSkill` / `UseSkillFromSlot` → bloqueiam STUN e SILENCE
- Consumível → bloqueado só no **STUN**
- Ataque básico → bloqueado só no **STUN**

---

## 3) Checklist rápido no Editor

1. Abrir `WBP_SkillSlot` → Hierarchy tem **`IMG_ControlLock`** (nome exato).
2. Parent Class = **`UmbraSkillSlotWidget`**.
3. Compile WBP sem erro de bind (Compile do Blueprint).
4. `WBP_SkillBar1` → slots filhos ainda são instâncias desse WBP (sem reparent errado).
5. Save All.

---

## 4) QA (PIE)

| Cenário | Esperado |
|---------|----------|
| Sem CC | `IMG_ControlLock` Collapsed; ícone normal |
| **STUN** | Overlay amarelo em **skills e poções**; ícone cinza; UseSkill falha; AA bloqueado; consumível bloqueado |
| **SILENCE** | Overlay azul **só em skills**; poção sem lock; AA ok; UseSkill falha |
| Drag-and-drop na barra | Continua funcionando com lock ativo |
| Overlay ausente no WBP | Só ícone cinza (sem crash) — BindWidgetOptional |

Feedback de cast (toast): *"Atordoado: não é possível usar habilidades"* / *"Silenciado: ..."*.

---

## 5) Problemas comuns

| Sintoma | Causa provável | Correção |
|---------|----------------|----------|
| Sem overlay, mas cast bloqueia | Nome ≠ `IMG_ControlLock` ou WBP errado (`SkillSlot1` sem o Image) | Renomear / copiar o Image |
| Overlay bloqueia drag | Visibility Self Hit Testable | Usar Collapsed / HitTestInvisible (C++ já define) |
| Slot inteiro “morto” | `Is Enabled = false` no root | Remover; só overlay + tint |
| Tint não muda STUN vs SILENCE | Brush com material que ignora ColorAndOpacity | Preferir Texture2D UI simples |
| Compile C++ ok, BP não vê member | Editor aberto com DLL antiga | Fechar PIE → Live Coding / rebuild → reabrir WBP |

---

## 6) Referência de código

```45:46:UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraSkillSlotWidget.h
	/** Overlay visual de STUN/SILENCE (BindWidgetOptional no WBP do slot). */
	TObjectPtr<UImage> IMG_ControlLock;
```

```384:412:UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraSkillSlotWidget.cpp
void UUmbraSkillSlotWidget::ApplyControlLockVisual()
{
	// STUN: todos os slots com conteúdo; SILENCE: só skills
	// IMG_ControlLock → HitTestInvisible + tint amarelo/azul
	// Skill_Icon → cinza se locked
}
```

SkillBar: `RefreshControlLocks()` em `UmbraSkillBarWidget.cpp`.
