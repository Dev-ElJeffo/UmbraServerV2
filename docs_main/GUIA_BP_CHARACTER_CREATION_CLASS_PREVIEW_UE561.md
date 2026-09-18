# GUIA — Blueprint: criação de personagem com Class Preview (UE 5.6.1)

## Objetivo

Configurar os Blueprints para o fluxo C++ já implementado em `AUmbraCharacterCreationManager`:

1. Lineup de **placeholders** (vitrine das classes).
2. Ao **clicar / selecionar uma classe**: despawna placeholders e spawna um **Character** (`ClassPreviewClass`) com body/arms/legs da classe.
3. **SpinBox Hair / Head** atualizam só as peças Appearance Parts no preview.
4. **Create** grava o personagem; **Return/Reset** destroí o preview e o BP respawna o lineup.

Referência de dados: [GUIA_MANAGER_APPEARANCE_PARTS_UE561.md](GUIA_MANAGER_APPEARANCE_PARTS_UE561.md)  
Botão Create (nome + IDs): [GUIA_BOTAO_CREATE_COMPLETO.md](../UmbraServer/docs_main/GUIA_BOTAO_CREATE_COMPLETO.md)

---

## Pré-requisitos

- Módulo UE recompilado com o `UmbraCharacterCreationManager` atual (funções `SelectClassAndMoveCamera`, `RefreshSelectedClassAppearance`, propriedade `ClassPreviewClass`).
- `BP_CharacterPreview` já reparentado para `UmbraEternumUECharacter`, com **um único** `Mesh` (GetMesh) — o mesmo usado na seleção.
- Manager: Classes com `skeletal_mesh_path` + torso/arms/legs/feet; Appearance Parts com hair/head.
- Level `Lvl_CharacterCreation` (ou equivalente) com câmera nomeada como o C++ espera (`FindCameraActor`).

---

## Parte 1 — BP_CharacterCreationManager (só parent + defaults)

### 1.1 Parent Class

1. Abra (ou crie) `BP_CharacterCreationManager`.
2. **File → Reparent Blueprint...**
3. Selecione **Umbra Character Creation Manager**.
4. Confirme. Compile e Save.

### 1.2 Class Defaults — Class Preview Class

1. No BP, abra **Class Defaults** (ou Details com Self selecionado).
2. Categoria **Character Creation | Preview**.
3. **Class Preview Class** → escolha **`BP_CharacterPreview`** (Character, não Actor placeholder).
4. Compile e Save.

Sem isso, o C++ despawna placeholders mas **não** spawna o mesh da classe.

### 1.3 Colocar no level

1. Arraste `BP_CharacterCreationManager` para `Lvl_CharacterCreation`.
2. Confirme que o Level Blueprint / widget já guarda referência a esse actor (variável `Manager` no `WBP_CreateCharacter`, como no guia antigo de setar Manager).

---

## Parte 2 — Clique na classe → Select Class And Move Camera

O C++ faz: achar placeholder pelo `ClassID` → `SelectClass` no GameInstance → **destruir todos placeholders** → spawn `ClassPreviewClass` na posição → `ApplyPlayerAppearance` → mover câmera.

### 2.1 Onde chamar

No fluxo que já existe ao escolher classe (botão “CRIAR” do widget 3D, `OnClassSelected`, `SelectClassByID`, etc.), o nó final deve ser:

```
[Select Class And Move Camera]
  Target: BP_CharacterCreationManager (referência do level / variável Manager)
  Class ID: (ID da classe clicada)
  Duration: 1.0  (ou o valor que você já usa)
  Offset X / Y / Z: (os mesmos offsets de câmera do projeto)
  Target Location / Rotation: Zero se usar offsets relativos
```

### 2.2 O que NÃO fazer no BP

- Não destrua placeholders manualmente no BP (o C++ já destroi).
- Não spawn de Character manual no clique — o C++ spawna `ClassPreviewClass`.
- Não chame `HighlightSelected` no placeholder após o select (ele já foi destruído).

### 2.3 Placeholders do lineup

Podem continuar `BP_*_Placeholder` (Actor simples). O nome do actor deve conter **`Placeholder`** e ter `ClassID` (propriedade ou função `GetClassID`) — o C++ usa isso para achar a posição.

---

## Parte 3 — WBP_CreateCharacter: Hair e Head

### 3.1 Designer (se ainda não tiver)

1. Abra `WBP_CreateCharacter`.
2. Adicione **SpinBox** `SpinBox_Hair` e `SpinBox_Head` (Is Variable = true).
3. Sugestão:
   - Min: `0` (0 = sem peça extra, só body da classe)
   - Max: conforme catálogo Appearance Parts (ex. `10`)
   - Delta: `1`
   - Value inicial: `0` ou `1`

### 3.2 Variável Manager (se não existir)

- Tipo: `BP_CharacterCreationManager` Object Reference (ou Actor → Cast).
- Preenchida no Construct / quando o level abre o widget (Set Manager), igual [GUIA_SETAR_MANAGER_WBP_CREATECHARACTER.md](../UmbraServer/docs_main/GUIA_SETAR_MANAGER_WBP_CREATECHARACTER.md).

### 3.3 OnValueChanged — Hair

No Event Graph do `SpinBox_Hair`:

```
[On Value Changed] (SpinBox_Hair)
  → [Get] Manager
  → [Is Valid]
       True → [Refresh Selected Class Appearance]
                Target: Manager
                Hair: (Value do evento — Truncate / Floor to int)
                Head: [Get Value] SpinBox_Head → Truncate
```

Conversão: `Value` do SpinBox é float → use **Truncate** ou **Floor** para int32.

### 3.4 OnValueChanged — Head

Igual, trocando:

```
Hair: [Get Value] SpinBox_Hair → Truncate
Head: Value do evento → Truncate
```

### 3.5 Após selecionar classe (opcional)

Depois de `Select Class And Move Camera` com sucesso, force o visual inicial dos spinboxes:

```
[Select Class And Move Camera] (Return Value True)
  → [Refresh Selected Class Appearance]
       Hair: SpinBox_Hair Value
       Head: SpinBox_Head Value
```

Assim o preview já nasce com os IDs atuais dos spinboxes.

---

## Parte 4 — Botão Create

Siga [GUIA_BOTAO_CREATE_COMPLETO.md](../UmbraServer/docs_main/GUIA_BOTAO_CREATE_COMPLETO.md). Resumo:

```
[BTN_Create OnClicked]
  → Get Text TXT_Name
  → Get Game Instance → Cast UmbraGameInstance
  → Get Selected Class ID  (do GI)  OU  Get Selected Class ID do CreationManager
  → SpinBox_Hair / SpinBox_Head → int
  → Create Character (Name, ClassID, Hair, Head)
```

Preferência: `ClassID` = **Selected Class ID** do `BP_CharacterCreationManager` (já setado no select) ou `GetSelectedClassID` do GameInstance após `SelectClass`.

---

## Parte 5 — Return / Reset

### 5.1 Chamar Reset

No botão Return / Cancel da criação:

```
[Reset Character Creation]
  Target: Manager
  Create Character Widget: Self (WBP_CreateCharacter)
  Placeholder Class: (opcional / legado)
  Spacing: 300
```

O C++:

- Destroí o **Class Preview** Character.
- Destroí placeholders restantes.
- Limpa VBox via `ClearCharacterCreationList` (se existir).
- Move a câmera de volta.

### 5.2 Respawn do lineup (obrigatório no BP)

O C++ **não** respawna sozinho as posições do lineup. Após `Reset Character Creation`, chame a mesma função que você já usa no BeginPlay do level/widget, por exemplo:

```
[Reset Character Creation]
  → [Spawn Placeholders With Transforms]  (ou Spawn Placeholders Simple / Spawn All Placeholders)
       Class IDs + Transforms iguais ao lineup inicial
```

Sem esse passo, a tela fica vazia após Return.

---

## Parte 6 — Checklist de QA

| Passo | Esperado |
|-------|----------|
| Abrir criação | Placeholders das classes visíveis |
| Clicar Templar | Placeholders somem; Character com body Templar |
| Mudar Hair/Head | Só cabeça/cabelo mudam |
| Create | Personagem criado; na seleção aparece classe + hair/head |
| Return | Preview Character some; lineup placeholders volta |
| Class Preview Class vazio | Placeholders somem e **não** aparece Character (log de warning) |

### Logs úteis (Output Log)

- `[UmbraCharacterCreationManager] Class preview spawnado para ClassID ...`
- `[UmbraCharacterCreationManager] ClassPreviewClass não setado no BP`
- `[Appearance] class=... hair=... head=...`
- `[Appearance] Owner não é ACharacter` → `ClassPreviewClass` errado (Actor puro)

---

## Parte 7 — Troubleshooting

| Sintoma | Causa | Ação |
|---------|--------|------|
| Clique não despawna | BP não chama `Select Class And Move Camera` | Trocar o nó final do clique |
| Despawna e fica vazio | `Class Preview Class` None | Setar `BP_CharacterPreview` |
| Preview é manequim Quinn | Mesh errado no `BP_CharacterPreview` | Só `Mesh` GetMesh + Apply (igual seleção) |
| Hair/Head não mudam | OnValueChanged não chama Refresh | Ligar SpinBox → `Refresh Selected Class Appearance` |
| Return sem lineup | Falta respawn no BP após Reset | Chamar Spawn Placeholders de novo |
| Placeholder não achado | Nome sem “Placeholder” ou ClassID errado | Conferir nome + propriedade ClassID |

---

## Resumo rápido

1. Reparent Manager → setar **Class Preview Class**.
2. Clique classe → só **Select Class And Move Camera**.
3. SpinBox → **Refresh Selected Class Appearance**.
4. Create → Name + ClassID + Hair + Head.
5. Return → **Reset Character Creation** + **Spawn Placeholders** de novo.

**Fim do guia.**
