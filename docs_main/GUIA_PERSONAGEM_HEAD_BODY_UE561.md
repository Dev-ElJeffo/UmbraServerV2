# GUIA — Personagem head + body com equipamento por cima (UE 5.6.1)

O personagem passa a ter só duas peças fixas: **body** (da classe) e **head/hair** (Appearance Parts). Cada equipamento visual (head, chest, hands, legs, feet) vira uma **peça a mais por cima do body** e não remove nada. O C++ monta, empilha e esconde as regiões do body cobertas pela armadura; não há Blueprint novo.

## Objetivo

- Fim do corpo em 4 partes (torso/arms/legs/feet). A classe define só o **body**.
- Equipar um item **adiciona** a malha dele; desequipar remove só ela. O body nunca some.
- Para evitar a pele atravessando a armadura, o item pode **esconder regiões do body** (`hide_body_slots`), por slot de material.
- Opcode **117** ganha um campo trailing (`hide_body_slots`). **Não muda:** armas (equipar troca a arma default da classe), `hide_hair`, socket `WeaponBack`, modo combate (119).

## Dados / Manager

| Campo | Onde | Uso |
|-------|------|-----|
| `classes.skeletal_mesh_path` | Manager → **Classes** → "skeletal_mesh_path (rig master)" | Líder do Leader Pose (anima; pode ser translúcido/invisível) |
| `classes.torso_mesh_path` | Manager → **Classes** → "body_mesh_path" | **Body** da classe (SKM seguidor) |
| `player_appearance_parts` `head`/`hair` | Manager → **Appearance Parts** | Cabeça e cabelo por personagem (`players.head` / `players.hair`), sem mudança |
| `item_templates.visual_meshes_json` → `hide_body_slots` | Manager → **Items** → meshes visuais → "Esconder no corpo (slots)" | Slots de material do body escondidos enquanto o item está equipado |

**SQL:** nenhum. `torso_mesh_path` foi reaproveitado como body. As colunas `arms_mesh_path`, `legs_mesh_path` e `feet_mesh_path` continuam no banco, mas o Manager e o cliente não as usam mais (os valores ficam preservados).

---

## 0) Pré-requisitos

1. Rebuild do módulo `UmbraEternumUE` (arquivos na §8).
2. PHP sincronizado com o WAMP: `scripts_main\sync_umbra_api_to_wamp.bat` (mudou `helpers/item_visual_helper.php`).
3. `zone_server` recompilado e reiniciado (opcode 117 com campo novo).
4. Manager novo (`dotnet build` ou publish para o `dist/`).

---

## 1) Preparar o body no Editor/DCC

O body precisa usar o **mesmo Skeleton** do rig master (`skeletal_mesh_path`). Senão o log mostra `Skeleton asset diferente`.

### 1.1 Dividir o body em slots de material

O `hide_body_slots` esconde **seções por slot de material**. Exporte o body com as regiões em materiais separados, com nomes fixos:

| Slot de material (nome exato) | Região | Escondido por (exemplo) |
|-------------------------------|--------|-------------------------|
| `Torso` | peito, costas, abdômen | armadura de peito |
| `Arms` | braços (ombro ao pulso) | armadura de peito com mangas |
| `Hands` | mãos | luvas |
| `Legs` | pernas e quadril | calças/grevas |
| `Feet` | pés | botas |

Como fazer:

1. No Blender, selecione as faces de cada região → **Material Properties** → crie um material por região com o nome da tabela (`Torso`, `Arms`...). Pode ser o mesmo shader duplicado.
2. Exporte o FBX e importe na UE com o Skeleton do rig master.
3. Abra o SKM do body → painel **Asset Details** → **Material Slots** e confira o campo **Material Slot Name** de cada um. É esse nome que o Manager usa (maiúsculas/minúsculas importam).

> Se o body tiver um slot só, nada é escondido; o equipamento só fica por cima.

### 1.2 Equipamentos

- Cada peça (capacete, peito, luvas, calça, bota) é um **SKM no mesmo Skeleton**, modelado um pouco maior que o body.
- Uma peça pode cobrir várias regiões (ex.: armadura completa no slot `chest`, escondendo `Torso,Arms,Legs`).

---

## 2) O que o C++ já faz (não precisa de Graph)

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| `UUmbraPlayerAppearanceComponent` | `Components/UmbraPlayerAppearanceComponent.cpp` | Monta `BodyPartMesh` (classe, Leader Pose) + `HairPartMesh`/`HeadPartMesh` (Appearance Parts). Equipamento em componentes próprios (`EquipChestMesh`, `EquipHandsMesh`, `EquipLegsMesh`, `EquipFeetMesh`, `HelmetPartMesh`) que são criados/destruídos sem tocar no body. |
| `SetHiddenBodySlots` | idem | Esconde no **body** as seções cujo slot de material está na lista, em todas as LODs (`ShowMaterialSection`). Reaplica quando o body é recriado. |
| `ApplyEquipmentVisualOverrides` | `Core/UmbraGameInstanceEquipmentVisual.cpp` | Junta os `hide_body_slots` de todos os visuais ativos e aplica no body (local e remotos). |
| Encode/parse 117 | `Network/WSBinaryBPFL.cpp` | Envia e lê o trailing3 `hide_body_slots`. |
| VFX de refino | `GetAttachComponentForEquipmentSlot` | Ancora na peça equipada; sem peça, no body. |

> **Por que não `HideBone`?** Com Leader Pose a visibilidade de ossos vem do líder: esconder `hand_r` some com a mão do body **e da luva**. Slot de material afeta só o componente do body.

> **Não** adicione lógica no Event Graph.

---

## 4) Protocolo

### Opcode 117 (`PlayerEquipmentVisualUpdate`)

Layout atual: `[117][playerId u32][count u8]` + por entrada `[equipSlot u8][pathLen u16][path utf8]` + trailing `flags u8` (bit0 = `hide_hair`) + trailing2 `refinementLevel u8` + **trailing3 (novo)**:

| Campo | Tipo | Observação |
|-------|------|------------|
| `hide_body_slots` (por entrada, na ordem das entradas) | `u16` + UTF-8 | **trailing**; até 256 bytes; frame antigo → vazio (nada escondido) |

Servidor: `src/zone/MovementProtocol.hpp` (`encode/decodePlayerEquipmentVisualUpdate`). A zone só guarda e retransmite; não interpreta o campo.
Cliente: `UWSBinaryBPFL::EncodePlayerEquipmentVisualUpdate` / `ParsePlayerEquipmentVisualUpdate`.

JSON do item (`visual_meshes_json`):

```json
{
  "default": [
    { "slot": "chest", "path": "/Game/Armor/SK_Chest_Plate.SK_Chest_Plate", "hide_body_slots": "Torso,Arms" },
    { "slot": "head",  "path": "/Game/Armor/SK_Helm.SK_Helm", "hide_hair": true }
  ]
}
```

O PHP (`helpers/item_visual_helper.php`) sanitiza `hide_body_slots` (só `[A-Za-z0-9_]`, separados por vírgula, até 256 caracteres) e agora também **grava o `hide_hair`**, que antes se perdia ao salvar.

## 5) Manager — passo a passo

### 5.1 Classe

1. **UmbraManager** → **Classes** → selecione a classe.
2. Grupo **Corpo** → **body_mesh_path**: path completo do SKM do body (`/Game/.../SK_Body.SK_Body`).
3. **skeletal_mesh_path (rig master)** e **anim_blueprint_path** continuam como estão.
4. **Salvar**.

### 5.2 Cabeça e cabelo

Sem mudança: aba **Appearance Parts** (`head` / `hair`), ver `GUIA_MANAGER_APPEARANCE_PARTS_UE561.md`.

### 5.3 Item

1. **Items** → selecione o item → meshes visuais (**padrão** ou **override por classe**).
2. **+ Adicionar mesh padrão** → **Slot** (`chest`, `hands`, `legs`, `feet`, `head`) e **Path SKM/SM**.
3. **Esconder no corpo (slots)**: nomes dos slots de material do body separados por vírgula, ex. `Hands` ou `Torso,Arms`. Vazio = não esconde nada.
4. **Esconder cabelo**: só para capacete.
5. **Salvar**.

---

## 6) QA (PIE)

PIE com **Number of Players ≥ 2**.

| Cenário | Esperado |
|---------|----------|
| Personagem sem equipamento | Body + cabeça + cabelo; nada faltando |
| Equipar peito com `hide_body_slots=Torso` | Armadura aparece; braços/pernas do body continuam; peito do body some por baixo |
| Equipar luva sem `hide_body_slots` | Luva por cima da mão (pode atravessar); body inteiro visível |
| Desequipar | Só a peça some; região escondida do body volta |
| Dois itens escondendo slots diferentes | União dos slots escondidos |
| Remoto vê o mesmo | Outro cliente mostra a peça e a região escondida (opcode 117) |
| Cliente antigo / item sem o campo | Sem crash; nada escondido |
| Arma equipada | Troca a arma default; modo combate (Z) segue funcionando |

Logs esperados: `[Appearance] Montou slot=7 path=...` (body), `[Appearance] Montou slot=0 path=...` (peito equipado).

## 7) Troubleshooting

| Sintoma | Causa provável | Ação |
|---------|----------------|------|
| Body não aparece | `body_mesh_path` vazio ou path sem o sufixo `.Asset` | Conferir no Manager; log `Falha modular SKM slot=7` |
| Região não some | Nome em `hide_body_slots` não bate com o **Material Slot Name** | Abrir o SKM do body → Material Slots e copiar o nome exato |
| Some a região errada | Faces atribuídas ao material errado no Blender | Revisar a divisão de materiais e reimportar |
| Peça "solta" do corpo | SKM de equipamento em outro Skeleton | Reimportar no Skeleton do rig master |
| Remoto não esconde | Zone antiga (sem o campo novo no 117) | Recompilar e reiniciar a `zone_server` |
| `hide_hair` sumia ao salvar o item | Bug antigo do PHP (gravava só slot/path) | Corrigido; sincronizar o WAMP e salvar o item de novo |

## 8) Referência de código

- Servidor: `src/zone/MovementProtocol.hpp` (`EquippedVisualWireEntry::hideBodySlots`, trailing3); testes `tests/test_zone.cpp` (`EquipmentVisualProtocolTest`).
- Cliente (compilar):
  - `Source/UmbraEternumUE/Components/UmbraPlayerAppearanceComponent.{h,cpp}`
  - `Source/UmbraEternumUE/Core/UmbraGameInstanceEquipmentVisual.cpp`
  - `Source/UmbraEternumUE/Core/UmbraGameInstance.cpp`
  - `Source/UmbraEternumUE/Data/UmbraDataStructures.h`
  - `Source/UmbraEternumUE/Network/WSBinaryBPFL.cpp`
- PHP: `www/umbra_api/helpers/item_visual_helper.php`.
- Manager: `Models/ClassRow.cs`, `Models/ItemVisualMeshRow.cs`, `ViewModels/MainViewModel.Classes.cs`, `ViewModels/MainViewModel.ItemVisual.cs`, `Views/ClassesEditorView.xaml`, `Views/ItemsEditorView.xaml`.

**Fim do guia.**
