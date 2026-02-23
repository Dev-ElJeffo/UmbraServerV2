# Solução: ScrollBox + Clique Funcionando

**Problema:** ScrollBox bloqueia `OnClicked` dos botões dos entries (bug conhecido do engine).

**Solução:** Cada entry é colocado **dentro de um wrapper** cujo **root é um Button**. O clique chega ao Button do wrapper (filho direto do VBox), não ao botão interno do entry, evitando o bloqueio do ScrollBox.

---

## 1. C++ Adicionado

### 1.1 UUmbraSkillBookEntryWidget
- **Novo método:** `GetCachedSkillID() const` — retorna o SkillID para o wrapper chamar `SelectSkill`.

### 1.2 UUmbraSkillBookEntryWrapperWidget (novo arquivo)
- **Arquivos:** `Source/UmbraEternumUE/UI/UmbraSkillBookEntryWrapperWidget.h` e `.cpp`
- **Root:** `BTN_Row` (Button) — BindWidgetOptional
- **Métodos:**
  - `SetSkillBookRef(UUmbraSkillBookWidget*)` — atribui o parent (Skill Book)
  - `SetEntryContent(UUserWidget*)` — coloca o Entry como conteúdo do botão
  - `OnRowClicked()` — ao clicar, pega o Entry filho, chama `GetCachedSkillID()` e `SelectSkill`

---

## 2. Blueprint: WBP_SkillBookEntryWrapper

| Passo | Ação |
|-------|------|
| 1 | Crie novo **User Widget**: nome = **WBP_SkillBookEntryWrapper**. |
| 2 | **Reparent:** Parent Class = **UmbraSkillBookEntryWrapperWidget**. |
| 3 | No **Designer**, root = **Button**. Nome do botão: **BTN_Row**. |
| 4 | Não adicione nada dentro do botão (o conteúdo será o Entry, adicionado em runtime por `SetEntryContent`). |
| 5 | Configure o botão: tamanho auto ou Fill conforme layout da lista. |
| 6 | Salve. |

---

## 3. Atualizar PopulateSkillList no WBP_SkillBook

**Antes:** Create Entry → SetSkillData → SetSkillBookRef → Add Child to Skills_VBox.

**Agora:** Create Entry → SetSkillData. Create Wrapper → SetEntryContent(Entry) → SetSkillBookRef(Wrapper) → Add Child to Skills_VBox (Wrapper).

| Ordem | Nó | Target | Parâmetros |
|-------|------|--------|------------|
| 1 | **Clear Children** | **Skills_VBox** | — |
| 2 | **For Each Loop** | **Available Skills** (array de skills) | — |
| 3 | **Create Widget** | Class = **WBP_SkillBookEntry** | Owning Player = Get Player Controller (0). Saída: **Entry Widget**. |
| 4 | **Set Skill Data** | Target = **Entry Widget** | Skill Data = **Array Element**, Game Instance Ref = **MyGameInstanceRef**. |
| 5 | **Create Widget** | Class = **WBP_SkillBookEntryWrapper** | Owning Player = Get Player Controller (0). Saída: **Wrapper Widget**. |
| 6 | **Set Entry Content** | Target = **Wrapper Widget** | Entry Widget = **Entry Widget** (do passo 3). |
| 7 | **Set Skill Book Ref** | Target = **Wrapper Widget** | In Skill Book = **self** (referência ao WBP_SkillBook). |
| 8 | **Add Child to Vertical Box** | Target = **Skills_VBox** | Content = **Wrapper Widget**. |

**Resultado:** A hierarquia fica: ScrollBox → Skills_VBox → [Wrapper (Button root)] → Entry (Canvas → BTN_SkillSelect → ...). O clique chega ao Button do wrapper (filho direto do VBox), o wrapper chama `SelectSkill` no Skill Book.

---

## 4. Compilar e Testar

1. Recompile o projeto (C++ adicionado: `UUmbraSkillBookEntryWrapperWidget` e `GetCachedSkillID()`).
2. Crie o **WBP_SkillBookEntryWrapper** (root = Button chamado `BTN_Row`, parent = `UmbraSkillBookEntryWrapperWidget`).
3. Atualize **PopulateSkillList** no **WBP_SkillBook** com os passos acima.
4. Teste: abra o Skill Book, clique numa entrada. Deve aparecer o log `SelectSkill` e o painel de detalhes deve atualizar.

---

## 5. Por Que Funciona

- **Antes:** Entry (Canvas → Button) era filho direto do VBox dentro do ScrollBox. O ScrollBox bloqueava o clique no botão interno.
- **Agora:** Wrapper (Button root) é filho direto do VBox. O Entry é filho do Button do wrapper. O ScrollBox **não** bloqueia o clique no Button do wrapper (porque é o primeiro nível), então `OnClicked` do wrapper dispara. O wrapper pega o Entry filho e chama `SelectSkill` com o `SkillID` do Entry.
- O scroll continua funcionando (mouse wheel + arrastar scrollbar) porque não alteramos `SetConsumeMouseWheel`.

---

## 6. Checklist Final

- [ ] C++ recompilado (Entry com `GetCachedSkillID()`, Wrapper novo).
- [ ] **WBP_SkillBookEntryWrapper** criado (root = Button `BTN_Row`, parent = `UmbraSkillBookEntryWrapperWidget`).
- [ ] **PopulateSkillList** atualizado: Create Entry → SetSkillData. Create Wrapper → SetEntryContent(Entry) → SetSkillBookRef(Wrapper) → Add Child to VBox (Wrapper).
- [ ] Testado: clique na entrada seleciona a skill e atualiza o painel.
