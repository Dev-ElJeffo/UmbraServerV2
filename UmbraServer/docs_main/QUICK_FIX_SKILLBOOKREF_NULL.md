# Quick Fix: SkillBookRef Null

## ✅ Checklist Rápido

No Blueprint **WBP_SkillBook → PopulateSkillList**:

- [ ] Existe um nó **"Create Widget"** (Class = **WBP_SkillBookEntry**)?
- [ ] A saída "Return Value" foi salva em uma variável local chamada **"Entry Widget"**?
- [ ] Existe um nó **"Set Skill Data"** com **Target = Entry Widget**?
- [ ] Existe um nó **"Set Skill Book Ref"** com **Target = Entry Widget**?
- [ ] O pin **"In Skill Book"** do nó "Set Skill Book Ref" está conectado a **"self"**?
- [ ] Existe um nó **"Add Child to Vertical Box"** com **Target = Skills_VBox**?
- [ ] O nó **"Set Skill Book Ref" está DENTRO do For Each Loop** (não fora)?

Se algum checkbox estiver marcado como ❌, veja abaixo.

## Cenários Comuns

### Cenário 1: "Set Skill Book Ref" não aparece no menu

**Causa:** O Target não é um **WBP_SkillBookEntry**

**Solução:**
1. Clique no nó **Create Widget**
2. Verifique se **Class = WBP_SkillBookEntry** (não UUserWidget genérico)
3. Se for UUserWidget genérico, altere para WBP_SkillBookEntry
4. Recompile Blueprint
5. Tente novamente

### Cenário 2: "Set Skill Book Ref" está fora do For Each Loop

**Causa:** Conectou em série, mas fora do loop

**Solução:**
1. Dentro do For Each Loop, procure pelo **pin branco de execução** (exec pin)
2. A sequência correta é:
   ```
   For Each Loop Body (exec branco)
     ├─ Create Widget
     ├─ Set Skill Data
     ├─ Set Skill Book Ref ← AQUI DENTRO
     └─ Add Child to Vertical Box
   ```

### Cenário 3: "In Skill Book" não está conectado a "self"

**Causa:** Pode estar null ou conectado a outro objeto

**Solução:**
1. No nó **Set Skill Book Ref**, procure pelo pin azul **"In Skill Book"**
2. Se tiver um fio branco, clique no outro lado do fio para ver o quê
3. Se for de um nó **"Get Game Instance"**, está **errado**
4. Deve estar conectado a **self** (azul + widget + self) ou deixe desconectado e use o padrão

### Cenário 4: Entry Widget variável está vazia

**Causa:** "Return Value" de Create Widget não foi guardado

**Solução:**
1. Clique no nó **Create Widget**
2. Procure pelo pin azul de saída **"Return Value"**
3. Arraste para baixo no canvas
4. Procure por **"Promote to Variable"**
5. Renomeie para **"Entry Widget"**
6. Agora use **"Get Entry Widget"** nos próximos nós

## Teste Rápido

Após fazer todas as correções:

1. **Recompile o Blueprint**
2. **Recompile o Projeto C++** (se tiver feito mudanças)
3. **Abra o SkillBook no jogo**
4. **Clique em uma skill**
5. **Veja o Output Log:**

**✅ FUNCIONANDO:**
```
LogTemp: Warning: [UmbraSkillBookEntryWidget] >>> OnSkillSelectPressed <<< (mouse DOWN). CachedSkillID=1
LogBlueprintUserMessages: [WBP_SkillBookEntry_C_30] WBP_SkillBookEntry - BTN_SkillSelect CLICKED!
LogTemp: Warning: [UmbraSkillBookEntryWidget] >>> OnSkillSelectClicked <<< CachedSkillID=1
LogTemp: Log: [UmbraSkillBookEntryWidget] SelectSkill(1) chamado.
LogBlueprintUserMessages: [WBP_SkillBook_C_0] Event Select Skill disparado!
```

**❌ NÃO FUNCIONANDO:**
```
LogTemp: Warning: [UmbraSkillBookEntryWidget] >>> OnSkillSelectPressed <<< (mouse DOWN). CachedSkillID=1
LogBlueprintUserMessages: [WBP_SkillBookEntry_C_30] WBP_SkillBookEntry - BTN_SkillSelect CLICKED!
LogTemp: Warning: [UmbraSkillBookEntryWidget] >>> OnSkillSelectClicked <<< CachedSkillID=1
LogBlueprintUserMessages: [WBP_SkillBookEntry_C_30] WBP_SKILLBOOKENTRY - SKILL BOOK REF NOT VALID!
LogTemp: Warning: [UmbraSkillBookEntryWidget] SkillBookRef é null!
```

Se for ❌, volte aos passos acima.

## Visual do Node Graph (Pseudo-código)

```
PopulateSkillList (função chamada do callback)
├─ Clear Children (Skills_VBox)
├─ For Each Loop
│  └─ Body
│     ├─ Create Widget
│     │  ├─ Class: WBP_SkillBookEntry
│     │  ├─ Owning Player: Get Player Controller 0
│     │  └─ Return Value ──[Promote to Variable]──→ "Entry Widget"
│     │
│     ├─ Set Skill Data
│     │  ├─ Target: Get Entry Widget
│     │  ├─ Skill Data: Array Element (For Each)
│     │  └─ Game Instance Ref: Get GameInstanceRef
│     │
│     ├─ Set Skill Book Ref ← NÃO PODE FALTAR
│     │  ├─ Target: Get Entry Widget
│     │  └─ In Skill Book: self (azul)
│     │
│     └─ Add Child to Vertical Box
│        ├─ Target: Skills_VBox
│        └─ Content: Get Entry Widget
```

## Última Tentativa

Se ainda não funcionar após tudo acima:

1. **Delete o nó "Set Skill Book Ref"** do Blueprint
2. **Right-click no "Get Entry Widget"** (ou Return Value)
3. **Search para "Set Skill Book Ref"**
4. **Reconecte manualmente**
5. **Recompile**

Se continuar não aparecendo, o problema é a classe Blueprint não estar reparentada para **UmbraSkillBookEntryWidget**. Veja o arquivo `PROCEDIMENTO_WBP_SKILLBOOK_BLUEPRINT.md` seção "Reparent Blueprint".
