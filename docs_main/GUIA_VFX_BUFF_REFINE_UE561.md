# GUIA — Niagara persistente: buff de corpo + refino +7..+12 (UE 5.6.1)

## Objetivo

- **Buff (opcode 104):** loop Niagara anexado ao `Mesh` enquanto o buff estiver ativo (local, remoto e NPC).
- **Refino (opcode 117):** glow por slot em níveis **≥ 7**, paths por nível/slot no Manager (`refinement_config.slot_vfx_json`) — **armadura e armas**.
- Cast one-shot (**97/99**, `vfx_path`) **não** muda — continua `SpawnSystemAtLocation`.

## Dados / Manager

| Campo | Onde | Uso |
|-------|------|-----|
| `skills.buff_vfx_path` | Skills editor → “Buff Niagara (loop)” | Aura no corpo |
| `refinement_config.slot_vfx_json` | AdminHub → Refinement → slots | Glow +7..+12 (arma + armadura) |

SQL:

- `www/umbra_api/scripts/add_skill_buff_vfx_path.sql`
- `www/umbra_api/scripts/add_refinement_slot_vfx_json.sql`

### Efeito distinto por nível (+7..+12)

Cada **linha** de nível no Manager tem seu próprio `slot_vfx_json`. Edite o nível 7, salve; depois o 8 com Niagara diferente; e assim por diante.

Exemplo nível **+7**:

```json
{
  "main_hand": "/Game/VFX/NS_WeaponGlow_7.NS_WeaponGlow_7",
  "chest": "/Game/VFX/NS_ArmorGlow_7.NS_ArmorGlow_7",
  "head": "/Game/VFX/NS_ArmorGlow_7.NS_ArmorGlow_7"
}
```

Exemplo nível **+8** (efeito diferente):

```json
{
  "main_hand": "/Game/VFX/NS_WeaponGlow_8.NS_WeaponGlow_8",
  "chest": "/Game/VFX/NS_ArmorGlow_8.NS_ArmorGlow_8",
  "head": "/Game/VFX/NS_ArmorGlow_8.NS_ArmorGlow_8"
}
```

Chaves: `main_hand`, `off_hand`, `head`, `chest`, `hands`, `legs`, `feet`.

> **Buff vs refino:** `buff_vfx_path` = aura no corpo inteiro (opcode 104). Refine = glow por slot de equipamento (opcode 117). Não misturar os dois campos.

## Passo a passo — criar e cadastrar Niagara de refino

Fluxo do artista até o PIE (armas e armaduras, níveis **+7..+12**).

### 1) Pré-requisitos

- Unreal Engine **5.6.1** com plugin **Niagara** ativo no projeto `UmbraEternumUE`.
- Coluna MySQL `refinement_config.slot_vfx_json` (script `www/umbra_api/scripts/add_refinement_slot_vfx_json.sql`).
- PHP do WAMP alinhado ao repo: `api/refinement/get_refinement_config.php`, `api/admin/upsert_refinement_config.php` em `C:\wamp64\www\umbra_api`.
- Cliente com `UUmbraPersistentVfxComponent` e load de config no select de personagem (`EnsureRefinementConfigLoadedForVfx`) — não depende de abrir a UI de refino.

### 2) Criar assets no Content Browser

Convenção de pastas e nomes (sugestão):

| Pasta | Assets |
|-------|--------|
| `Content/VFX/Refine/Weapons/` | `NS_WeaponGlow_7` … `NS_WeaponGlow_12` |
| `Content/VFX/Refine/Armor/` | `NS_ArmorGlow_7` … `NS_ArmorGlow_12` |

No Editor (UE 5.6.1):

1. No Content Browser, crie as pastas acima (ou equivalentes).
2. Clique direito → **FX → Niagara System** (ou duplique um template de **loop** que você já use).
3. Nomeie conforme o nível, ex.: `NS_WeaponGlow_7`, `NS_ArmorGlow_7`.
4. No Niagara System: emitters em **loop contínuo** (não Burst one-shot). O cliente faz `SpawnSystemAttached` com `SetAutoDestroy(false)` e `Activate(true)` — o sistema precisa permanecer ativo sozinho.
5. Escala / look:
   - **Arma:** efeito localizado na lâmina/empunhadura (vai no mesh da arma ou no socket da mão).
   - **Armadura:** glow na peça (peito, capacete, etc.), **não** aura de corpo inteiro (isso é papel do buff).
6. Salve o asset. Copie o path no formato que o `StaticLoadObject` espera (caminho + `.NomeDoAsset`):

```
/Game/VFX/Refine/Weapons/NS_WeaponGlow_7.NS_WeaponGlow_7
/Game/VFX/Refine/Armor/NS_ArmorGlow_7.NS_ArmorGlow_7
```

**Reuso:** um asset **por nível** se o visual mudar (+7 vs +12). No **mesmo** nível, você pode repetir o mesmo path em vários slots (ex. peito e capacete +7 iguais).

Repita para +8..+12 (ou duplique o +7 e aumente intensidade/cor).

### 3) Como o cliente anexa (oriente o FX)

Componente: `UUmbraPersistentVfxComponent` (`ApplyRefineSlotVfx`).

| Slot Manager | Chave JSON | Attach no pawn |
|--------------|------------|----------------|
| Main Hand | `main_hand` | Mesh da arma via Appearance; se só body → socket `HandGrip_R` |
| Off Hand | `off_hand` | Mesh da arma; se só body → socket `HandGrip_L` |
| Head / Chest / Hands / Legs / Feet | `head`…`feet` | Peça LeaderPose do slot; senão body mesh; **nunca** HandGrip |

Regras:

- Só spawna se o item equipado tiver `refinement_level ≥ 7` **e** houver path na linha daquele nível no Manager.
- Nível abaixo de 7 (ou path vazio / unequip): remove o Niagara daquele slot.
- Config vem de `UmbraRefinementSubsystem` (`slot_vfx_json` por nível) → `ApplyRefineVfxFromEntries`.

### 4) Cadastrar no UmbraManager

1. Abra o **UmbraManager** → AdminHub → **Refinement**.
2. Selecione a **linha do nível** (comece pelo 7). Cada nível tem seu próprio `slot_vfx_json`.
3. Preencha os TextBoxes de Niagara por slot:
   - Main Hand / Off Hand (armas)
   - Head / Chest / Hands / Legs / Feet (armadura)
4. Cole os paths `/Game/.../NS_....NS_...` (com o sufixo `.NomeDoAsset`).
5. **Salve** a linha do nível.
6. Repita para +8, +9, … +12 com Niagara distinto (ou o mesmo path se quiser o mesmo efeito).
7. Se o save não persistir: sincronize o PHP do repo para `C:\wamp64\www\umbra_api` (mesmos arquivos de refine listados nos pré-requisitos) e rode o SQL da coluna se ainda não existir.

### 5) Validar no jogo

1. Equipar arma e/ou peito **+7** com path cadastrado no nível 7 → glow aparece.
2. Output Log **sem** `Falha ao carregar refine Niagara: ...`.
3. Login / select de personagem **sem** abrir a UI de refino → glow ainda aparece.
4. Trocar item +7 → +8 → Niagara muda para o path do nível 8.
5. Unequip ou baixar para **+6** → glow some naquele slot.
6. PIE com 2 clients: o remoto vê o glow após o opcode **117**.

### 6) Troubleshooting

| Sintoma | Causa provável | Ação |
|---------|----------------|------|
| Log `Falha ao carregar refine Niagara` | Path errado ou sem `.NomeDoAsset` | Copiar path completo do Content Browser / asset |
| Glow some na hora | Niagara one-shot / Burst only | Trocar para loop contínuo |
| Manager salva e path some | PHP WAMP antigo ou coluna SQL ausente | Sync PHP + `add_refinement_slot_vfx_json.sql` |
| Nada aparece no mundo | Config não carregou / nível abaixo de 7 / slot sem path | Reentrar no personagem; conferir linha do nível e TextBox do slot |
| Arma ok, armadura não | Path só em `main_hand` | Preencher `chest` / `head` / etc. na mesma linha de nível |

## Protocolo

### Opcode 104 (`SkillBuffSync`)

Após `effectType`, append `buffVfxPath` (u16+utf8, até 512). Frames antigos sem o campo: string vazia.

Zone: `SkillService` carrega `buff_vfx_path`; `enrichSkillBuffSyncPayload` preenche o payload.

### Opcode 117 (`PlayerEquipmentVisualUpdate`)

Após o bloco de `flags`, trailing `refinementLevel:u8` por entrada (0–12). Frames antigos: 0.

Cliente preenche o nível ao agregar visual do inventário e envia no encode; zone retransmite.

## Cliente UE

Componente: `UUmbraPersistentVfxComponent`

- Buff: `SpawnSystemAttached` no `GetMesh()`; mapa `buffId → Niagara`.
- Refino arma: mesh da arma ou `HandGrip_R/L` no body.
- Refino armadura: peça LeaderPose (`TorsoPartMesh`, etc.); se ainda não existir, fallback no body mesh (sem socket de arma).
- Config: carregada no **select de personagem** via `EnsureRefinementConfigLoadedForVfx` (não depende de abrir a UI de refino). Ao `OnRefinementConfigLoaded`, reaplica local + remotos.

Wiring:

- `HandleSkillBuffSyncMessage` → apply/remove no ator alvo.
- `LoadSkillActiveBuffs` / register remoto / spawn NPC → `SyncBuffVfxFromList`.
- `ApplyEquipmentVisualOverrides` → `ApplyRefineVfxFromEntries` (lookup `nível do item` + chave do slot).
- `resolved_visual` da API: `EnrichVisualEntriesWithEquippedRefinement` copia `refinement_level` do inventário equipado.

## QA checklist

1. Skill com `buff_vfx_path`: cast → aura no Mesh do caster e do alvo remoto; some ao expirar/dispell.
2. PIE 2 clients: ambos veem auras.
3. **Login sem abrir UI de refino** → peito/capacete/arma +7 com path no nível 7: glow aparece.
4. Trocar item +7 → +8: Niagara muda para o path do nível 8.
5. Slots `head`/`chest`/`hands`/`legs`/`feet` +7..+12: cada um brilha; unequip / +6 remove.
6. Arma +7..+12: glow em `HandGrip_R` (ou mesh da arma) local e remoto após 117.
7. Cast 97 (`vfx_path`) continua one-shot; não misturar com loop de buff.
8. Rodar os dois scripts SQL no MySQL antes de testar em ambiente limpo.
