# Troubleshooting: SkillBookRef é null

## Problema

Log mostra:
```
LogBlueprintUserMessages: [WBP_SkillBookEntry_C_30] WBP_SKILLBOOKENTRY - SKILL BOOK REF NOT VALID!
LogTemp: Warning: [UmbraSkillBookEntryWidget] SkillBookRef é null!
```

Isto significa que `SetSkillBookRef()` **não foi chamado** ou **foi chamado com valor null**.

## Solução

**Em WBP_SkillBook → PopulateSkillList:**

### Passo 1: Verificar a sequência correta

A ordem deve ser:

```
For Each Available Skills
  ├─ Create Widget (Class = WBP_SkillBookEntry)
  │  └─ Salvar em variável "Entry Widget"
  ├─ Set Skill Data (Entry Widget, Array Element)
  ├─ Set Skill Book Ref (Target = Entry Widget, In Skill Book = self)  ← ESTE NÓAQ NÃO PODE FALTAR
  └─ Add Child to Vertical Box (Skills_VBox, Entry Widget)
```

### Passo 2: Verificar se o nó "Set Skill Book Ref" existe

No Blueprint, dentro do For Each Loop:

1. **Clique com botão direito no canvas**
2. **Procure por "Set Skill Book Ref"**
3. Se não aparecer, significa que:
   - O **Target** do nó anterior não é um **WBP_SkillBookEntry**
   - Ou a classe não foi reparentada corretamente

### Passo 3: Verificar o Target do nó

```
Set Skill Book Ref
├─ Target: DEVE SER "Entry Widget" (ou outra variável que guarda a referência)
├─ In Skill Book: DEVE SER "self" (o WBP_SkillBook)
```

**Se o Target for algo diferente de "Entry Widget", o nó não fará efeito.**

### Passo 4: Verificar se "self" é realmente WBP_SkillBook

- Clique no pin de entrada "In Skill Book"
- Procure por nós que retornem `self` ou `Get Parent (User Widget)` ou similar
- Se usar `Get Game Instance`, está errado — deve ser `self`

### Passo 5: Verificar a posição dentro do For Each

**CORRETO:**
```
For Each Loop
├─ Create Widget ← dentro
├─ Set Skill Data ← dentro
├─ Set Skill Book Ref ← dentro (IMPORTANTE)
└─ Add Child to Vertical Box ← dentro
```

**ERRADO:**
```
For Each Loop
├─ Create Widget
├─ Set Skill Data
└─ Add Child to Vertical Box

Set Skill Book Ref ← fora! (nunca executado)
```

## Verificação Rápida

Se você implementou corretamente, o log deve mostrar:

```
LogTemp: Warning: [UmbraSkillBookEntryWidget] >>> OnSkillSelectClicked <<< CachedSkillID=1
LogTemp: Log: [UmbraSkillBookEntryWidget] SelectSkill(1) chamado.
```

**Se mostrar "SkillBookRef é null!", você está faltando o nó "Set Skill Book Ref".**

## Exemplo de Node Graph Correto

```
Event PopulateSkillList (Custom Event)
  ↓
Get GameInstanceRef
  ├─ pin de saída azul
  └→ Get Available Skills (retorna Array)
  ↓
Clear Children (Skills_VBox)
  ↓
For Each Loop (Array = Available Skills)
  ├─ Array Element (Umbra Skill Data)
  ├─ Loop Body
  │  ├─ Create Widget
  │  │  ├─ Class = WBP_SkillBookEntry
  │  │  ├─ Owning Player = Get Player Controller (0)
  │  │  └─ Return Value → [Salvar em variável "Entry Widget"]
  │  │
  │  ├─ Set Skill Data
  │  │  ├─ Target = Entry Widget
  │  │  ├─ Skill Data = Array Element
  │  │  └─ Game Instance Ref = GameInstanceRef
  │  │
  │  ├─ Set Skill Book Ref ← NÃO PODE FALTAR!
  │  │  ├─ Target = Entry Widget
  │  │  └─ In Skill Book = self
  │  │
  │  └─ Add Child to Vertical Box
  │     ├─ Target = Skills_VBox
  │     └─ Content = Entry Widget
```

## Se Ainda Não Funcionar

Adicione logs no Blueprint para debugar:

```
Before Set Skill Book Ref:
  └─ Print String: "Antes de SetSkillBookRef. Entry Widget = " + (Entry Widget)
  └─ Print String: "self = " + (self)

After Set Skill Book Ref:
  └─ Print String: "Depois de SetSkillBookRef"
```

Se os prints não aparecerem, o nó não está sendo executado.

## Implementação Manual (se precisar)

Se o Blueprint não encontrar o nó "Set Skill Book Ref", você pode chamar diretamente via **Promoted to Variable**:

1. Crie uma variável no WBP_SkillBook chamada `LastCreatedEntry` (tipo WBP_SkillBookEntry)
2. Em cada iteração, **Set LastCreatedEntry = Entry Widget**
3. No callback (ou próximo frame), **Get LastCreatedEntry** → **Set Skill Book Ref**

Mas isso é **não recomendado** — o correto é ter o nó no For Each Loop.
