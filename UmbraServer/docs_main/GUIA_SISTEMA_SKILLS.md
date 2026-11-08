# Sistema de Skills - Umbra Eternum

## Visão Geral

Sistema completo de skills server-side authoritative para MMORPG.

### Princípios Fundamentais

```
❌ Cliente NUNCA calcula dano
❌ Cliente NUNCA decide crítico  
❌ Cliente NUNCA aplica buff
✅ Servidor é a única fonte de verdade
```

### Fluxo de Uso de Skill

```
Client → RequestUseSkill
    ↓
Server → Validate (cooldown, recursos, range, estado)
    ↓
Server → Calculate (dano, cura, efeitos)
    ↓
Server → Apply (HP, buffs, DOTs)
    ↓
Server → Persist (banco de dados)
    ↓
Server → BroadcastResult (todos os clientes relevantes)
```

---

## Estrutura do Banco de Dados

### Tabelas Principais

| Tabela | Descrição |
|--------|-----------|
| `skills` | Definição de todas as 90 skills |
| `skill_types` | ACTIVE, PASSIVE, BUFF, DEBUFF, AURA, ULTIMATE, REACTION, DOT, HOT |
| `skill_targets` | SELF, ENEMY, ALLY, AREA, PARTY, AREA_ALLY |
| `skill_elements` | PHYSICAL, SHADOW, FIRE, HOLY, POISON, ICE, LIGHTNING, ARCANE |
| `skill_effects` | Efeitos normalizados por skill |
| `player_skills` | Skills aprendidas pelo jogador |
| `player_skill_points` | Pontos de skill (3/nível) |
| `player_skillbar` | Configuração da barra (20 slots) |
| `player_cooldowns` | Cooldowns ativos |
| `active_buffs` | Buffs em tempo real |
| `active_dots` | DOTs ativos |

### Executar Scripts SQL

```bash
# 1. Criar schema e tabelas
mysql -u root -p umbra_eternum < www/umbra_api/scripts/create_skill_system.sql

# 2. Inserir todas as 90 skills
mysql -u root -p umbra_eternum < www/umbra_api/scripts/insert_all_skills.sql
```

**Importante:** A API `get_available_skills` usa a tabela **`classes`** (JOIN com `players`). Se o seu banco não tiver essa tabela, execute também:

```bash
mysql -u root -p umbra_eternum < www/umbra_api/scripts/create_character_info_tables.sql
```

### Troubleshooting – "Erro interno do servidor" ao carregar skills

Se o cliente exibir **"Erro interno do servidor"** ao abrir o Livro de Skills:

1. **Verifique o log do PHP** (ex.: `error_log` do Apache/Nginx ou o caminho configurado em `php.ini`). O script `get_available_skills.php` registra a exceção real (ex.: tabela inexistente, coluna ausente).
2. **Confirme que as tabelas existem:** `players`, `classes`, `player_skill_points`, `skills`, `skill_types`, `skill_targets`, `skill_elements`, `skill_scaling_stats`, `player_skills`. Execute `create_skill_system.sql` e `create_character_info_tables.sql` se faltar alguma.
3. **Jogador com classe:** o personagem logado deve ter `class_id` válido em `players` (correspondente a um registro em `classes`).

### Troubleshooting – "skill_id é obrigatório" ao clicar em Aprender

O cliente **só** deve chamar **Learn Skill** quando houver uma skill **selecionada** (SelectedSkillID > 0). Se aparecer "skill_id é obrigatório":

- **No Blueprint WBP_SkillBook:** no **OnActionButtonClicked**, chame **Learn Skill** ou **Upgrade Skill** passando **SelectedSkillID**. Só chame se **SelectedSkillID** for maior que 0 (ou seja, o jogador clicou numa linha da lista antes de clicar no botão Aprender/Melhorar).
- O C++ agora valida: se **SkillID ≤ 0**, não envia a requisição e dispara a mensagem *"Selecione uma skill na lista antes de clicar em Aprender."*

---

## Classes e Skills

### Distribuição de Skills por Nível

| Skill # | Level | Skill # | Level |
|---------|-------|---------|-------|
| 1 | 1 | 9 | 30 |
| 2 | 4 | 10 | 34 |
| 3 | 7 | 11 | 38 |
| 4 | 10 | 12 | 42 |
| 5 | 14 | 13 | 45 |
| 6 | 18 | 14 | 48 |
| 7 | 22 | 15 | 50 (Ultimate) |
| 8 | 26 | | |

### Classes Disponíveis

| Classe | Arquétipo | Scaling Principal |
|--------|-----------|-------------------|
| **Barbarian** | Bruiser ofensiva, dano físico alto | STR + VIT |
| **Dark Mage** | Caster ofensiva, controle, drenagem | INT + LCK |
| **Monk (Marcial)** | Combatente técnico, precisão | DEX + STR |
| **Templar** | Defensora sagrada, proteção, cura | VIT + STR + INT |
| **Alchemist (Cleric)** | Suporte pleno, buffs, cura | INT + LCK + VIT |
| **Assassin** | Burst, crítico, mobilidade | DEX + LCK |

---

## APIs PHP

### Endpoints Disponíveis

| Método | Endpoint | Descrição |
|--------|----------|-----------|
| POST | `/api/skills/get_available_skills.php` | Lista skills da classe com status |
| POST | `/api/skills/get_player_skills.php` | Skills aprendidas |
| POST | `/api/skills/learn_skill.php` | Aprender nova skill |
| POST | `/api/skills/upgrade_skill.php` | Melhorar rank da skill |
| POST | `/api/skills/get_skillbar.php` | Obter barra de skills |
| POST | `/api/skills/set_skillbar.php` | Configurar slot da barra |
| POST | `/api/skills/use_skill.php` | Validar uso de skill |
| POST | `/api/skills/get_cooldowns.php` | Cooldowns ativos |
| POST | `/api/skills/get_active_buffs.php` | Buffs/DOTs ativos |
| GET | `/api/skills/export_skills_json.php` | Exportar JSON |
| GET | `/api/skills/export_skills_csv.php` | Exportar CSV (DataTable) |

### Admin (UmbraManager)

| Método | Endpoint | Descrição |
|--------|----------|-----------|
| POST | `/api/admin/list_skills.php` | Lista skills (+ filtros) |
| POST | `/api/admin/get_skill.php` | Skill + `skill_rank_scaling` |
| POST | `/api/admin/create_skill.php` / `update_skill.php` / `delete_skill.php` | CRUD |
| POST | `/api/admin/upsert_skill_rank_scaling.php` | Upsert rank (bonus + extra_effects) |
| POST | `/api/admin/delete_skill_rank_scaling.php` | Remove linha de rank |
| POST | `/api/admin/list_skill_lookups.php` | types/targets/elements/classes |
| POST | `/api/admin/reload_skills.php` | Sinaliza reload (Manager envia TCP `reload_skills`) |

Tooltips de jogador (`get_player_skills` / `get_available_skills`) calculam power/CD/mana/duração efetivos via `helpers/skill_rank_helper.php` (mesma regra do C++).

### Rank scaling

Tabela `skill_rank_scaling`: bônus por rank + `extra_effects_json` cumulativo (unlock de STUN/SILENCE etc.). Seed: `scripts/seed_skill_rank_scaling_defaults.sql`.

No UmbraManager: aba **Skills** → editar formulário + grade de ranks → **Recarregar no Zone**.

### Exemplo de Request

```json
// POST /api/skills/learn_skill.php
{
    "token": "jwt_token_here",
    "skill_id": 1
}

// Response
{
    "success": true,
    "message": "Skill 'Golpe da Ruína' aprendida com sucesso!",
    "data": {
        "skill_id": 1,
        "skill_name": "Golpe da Ruína",
        "current_rank": 1,
        "max_rank": 5,
        "skill_points": {
            "total_earned": 30,
            "spent": 1,
            "available": 29
        }
    }
}
```

---

## Fórmulas de Cálculo

### Power coef efetivo (rank)

```
Se existe skill_rank_scaling[rank]:
  power = power_coef + power_coef_bonus
Senão (fallback):
  power = power_coef × (1 + (rank-1)×0.1)
```

CD / mana / duração seguem as colunas de bonus da mesma linha. Efeitos = `effects_json` + extras de todos os ranks ≤ atual.

### Dano Físico

```
Damage = (BaseStat × power_coef_efetivo) 
       × (1 + AttributeScaling) 
       × BuffMultipliers 
       × CritMultiplier 
       × PvPModifier 
       - DefenseReduction
       - ResistanceReduction
```

### Redução de Defesa

```
Reduction = Defense / (Defense + 100)
Capped at 90%
```

### Chance de Acerto

```
HitChance = 80% + Accuracy - Dodge
Clamped: 5% - 95%
```

### Chance de Crítico

```
CritChance = AttackerCrit - TargetCritResist
Clamped: 0% - 80%
```

### Scaling por Atributo

```
Cada ponto acima de 10 = +1% do scaling definido
Ex: STR 50, skill com str_scaling=60
Bonus = (50-10) * 0.60 = 24% extra
```

### PvP Modifier

```
PvP Damage = Damage × skill.pvp_modifier × 0.7 (30% redução global)
```

---

## Sistema de Buffs

### Tipos de Buff

| Tipo | Descrição |
|------|-----------|
| BUFF | Melhora stats (positivo) |
| DEBUFF | Reduz stats (negativo) |
| AURA | Afeta área continuamente |
| DOT | Dano ao longo do tempo |
| HOT | Cura ao longo do tempo |
| SHIELD | Absorve dano |

### Aplicação de Buffs

```
Final Stat = (Base + FlatBonus) × (1 + PercentBonus/100)
```

### Stacking

- **Stackable = false**: Apenas 1 instância, refresha duração
- **Stackable = true**: Acumula até max_stacks

### Snapshot vs Dynamic

- **DOT/HOT**: Usa snapshot dos stats no momento do cast
- **Buffs de stat**: Dinâmicos, recalculados em tempo real

---

## Sistema de Threat (Aggro)

```
DamageThreat = Damage × 1.0
HealThreat = Heal × 0.5
TauntThreat = Value × 10.0
FinalThreat = BaseThreat × ThreatModifier
```

---

## Integração com Unreal Engine

### O que precisa estar rodando (stack)

O sistema de skills **não exige nenhum servidor C++ novo**. Tudo o que o cliente UE usa (lista de skills, aprender, melhorar, barra de skills, cooldowns) é atendido pela **API PHP** via HTTP.

| Componente | Necessário para Skills? | Observação |
|------------|--------------------------|------------|
| **MySQL** | Sim | Com schema e dados de skills (tabelas `skills`, `player_skills`, etc.). |
| **Servidor web + PHP** | Sim | Para `www/umbra_api` (endpoints em `api/skills/`). |
| **Auth (C++)** | Não | Só para login TCP; o Skill Book usa HTTP + JWT. |
| **Gateway (C++)** | Não | Para conexão de mundo; skills são via API HTTP. |
| **Zone (C++)** | Não* | *Só se no futuro o combate/uso de skill for validado no Zone. Hoje `use_skill.php` já valida. |
| **Chat (C++)** | Não | Independente do sistema de skills. |

**Resumo:** Para o Skill Book e a Skill Bar funcionarem (carregar lista, aprender, melhorar, configurar barra), basta **MySQL** com as tabelas de skills preenchidas e o **servidor web com PHP** servindo a `umbra_api`. Nenhum binário C++ (Auth/Gateway/Zone/Chat) precisa estar rodando.

---

### Precisa compilar ou executar algo novo?

| Ação | Necessário? |
|------|-------------|
| **Compilar novo servidor C++** | Não. Nenhum servidor C++ é usado pelo fluxo de skills da UI. |
| **Subir um “servidor de skills”** | Não. Não existe servidor dedicado de skills; a API PHP é o backend. |
| **Executar scripts SQL** | Sim, uma vez. Rode `create_skill_system.sql` e `insert_all_skills.sql` no MySQL. |
| **Ter o projeto UE compilado** | Sim. O cliente UE precisa ter a struct **FUmbraSkillDataClient** (recomendada para DataTable) ou `FUmbraSkillData` compilada para importar o DataTable. |
| **Recompilar o cliente UE** | Só se você alterar `UmbraSkillDataStructures.h`; depois reabra o Editor e reimporte o CSV se necessário. |

---

### DataTable no cliente: abordagem mínima (recomendada)

Para evitar expor dados de jogo no cliente (cooldown, cast time, custo, scaling, alcance, etc.), use no Unreal apenas a **struct mínima** e o **export client-only**:

- **Struct:** `FUmbraSkillDataClient` (apenas identificação, nome, classe, tipo, elemento, ícone e descrição flavor).
- **Export PHP:** `export_skills_csv_client.php` — não exporta valores de jogo.
- **Uso:** DataTable para ícones, nomes e tooltips; dados de jogo em runtime vêm **só da API** (`FUmbraSkillData` preenchida pelo servidor).

---

### Guia completo – Importar DataTable de Skills (client-only) na Engine

Do export no backend até o DataTable utilizável no Unreal Editor, usando a struct **FUmbraSkillDataClient** e o CSV gerado por **export_skills_csv_client.php**.

---

#### Pré-requisitos

1. **Banco de dados:** scripts de skills já executados no MySQL:
   - `www/umbra_api/scripts/create_skill_system.sql`
   - `www/umbra_api/scripts/insert_all_skills.sql`
2. **Cliente UE:** projeto compilado com a struct **FUmbraSkillDataClient** (em `UmbraSkillDataStructures.h`). Se você acabou de alterar a struct, recompile e abra o Editor.

---

#### Passo 1 – Exportar o CSV (backend, client-only)

| Ordem | Ação | Detalhes |
|-------|------|----------|
| 1 | Servidor web e PHP rodando | Ex.: XAMPP, WAMP, ou `php -S` com o document root em `www/umbra_api` (ou na pasta que expõe a API). |
| 2 | Abrir no navegador | **`http://localhost/umbra_api/api/skills/export_skills_csv_client.php`** (ajuste host/porta se não for localhost). |
| 3 | Salvar o arquivo | O navegador baixa um CSV. Salve como **DT_Skills_Client.csv** (ex.: em `Content/UI/Skills/DataTables/` ou pasta temporária). |

**Alternativa via linha de comando:**

```bash
cd d:\UmbraServerV2
# Com servidor web apontando para www/umbra_api, ou:
php -r "\$_GET=[]; \$_POST=[]; require 'www/umbra_api/config/database.php'; require 'www/umbra_api/api/skills/export_skills_csv_client.php';" 2>nul > DT_Skills_Client.csv
```

(Adapte o caminho do `require` conforme seu ambiente.)

---

#### Passo 2 – Verificar o CSV

- Primeira coluna = **Row Name** (cabeçalho **---**); o script usa **SkillKey** como valor.
- Colunas devem ter **nomes idênticos** à struct **FUmbraSkillDataClient**: `SkillID`, `SkillKey`, `SkillName`, `ClassID`, `ClassName`, `SkillType`, `Element`, `ElementColor`, `IconPath`, `Description`.
- Encoding: UTF-8 com BOM (o PHP já envia).

---

#### Passo 3 – No Unreal Editor: pasta do DataTable

| Ordem | Ação | Detalhes |
|-------|------|----------|
| 1 | Abrir o projeto | UmbraEternumUE (ou o projeto que contém as structs de skills). |
| 2 | Content Browser | Navegar até a pasta (ex.: `Content/UI/Skills/DataTables/`). Se não existir, criar: botão direito → **New Folder** → **DataTables**. |

---

#### Passo 4 – Importar o CSV como DataTable

| Ordem | Ação | Detalhes |
|-------|------|----------|
| 1 | Importar | Na pasta: botão direito → **Import**. |
| 2 | Selecionar arquivo | **DT_Skills_Client.csv**. |
| 3 | Opções de import | Em **Escolha o tipo de linha DataTable** (Data Table Row Type), selecionar **UmbraSkillDataClient** (no Editor aparece sem o "F"). Se não listar, recompile o projeto com `UmbraSkillDataStructures.h`. |
| 4 | Confirmar | **Import**. O Editor cria/atualiza o asset (ex.: **DT_Skills** ou **DT_Skills_Client**). |
| 5 | Salvar | Ctrl+S no asset e na pasta. |

---

#### Passo 5 – Validar a importação

| Ordem | Ação | Detalhes |
|-------|------|----------|
| 1 | Abrir o DataTable | Duplo clique no asset. |
| 2 | Verificar linhas | Dezenas de linhas (ex.: 90 skills), **Row Name** = SkillKey (ex.: BARB_RUIN_STRIKE). |
| 3 | Verificar colunas | Apenas: SkillID, SkillKey, SkillName, ClassID, ClassName, SkillType, Element, ElementColor, IconPath, Description. Sem cooldown, cast time, custo, scaling, etc. |
| 4 | Erros comuns | **"Expected column X not found"**: use o CSV do **export_skills_csv_client.php**. **Struct não listada**: recompile com a struct no módulo. |

---

#### Passo 6 – Usar o DataTable no projeto

- **GameInstance:** Referência ao DataTable (ex.: **DT_Skills**) para lookup por **SkillKey** (ícone, nome, descrição). A lista jogável (**Available Skills**) vem da API **get_available_skills** (runtime); o DataTable é só para exibição estática/ícones.
- **WBP_SkillBook / WBP_SkillBar:** Available Skills vêm da API; use o DataTable para ícones/nomes quando precisar de fallback ou catálogo por SkillKey. Para ícones e tooltips, consulte **GUIA_DATATABLE_ICONS_TOOLTIP.md**.

---

#### Colunas do CSV client-only (FUmbraSkillDataClient)

| Coluna CSV | Uso |
|------------|-----|
| --- (Row Name) | SkillKey como nome da linha |
| SkillID | Identificação |
| SkillKey | Chave única (lookup) |
| SkillName | Nome para UI |
| ClassID | Filtro por classe |
| ClassName | Nome da classe para UI |
| SkillType | Tipo para UI (Active, Passive, etc.) |
| Element | Elemento para cor/ícone |
| ElementColor | Cor em hex (tint UI) |
| IconPath | Caminho/nome do ícone |
| Description | Descrição flavor (sem fórmulas) |

Nenhum valor de jogo (cooldown, cast time, custo, scaling, alcance) é exportado.

---

### Struct UE5 – FUmbraSkillDataClient (DataTable no cliente)

```cpp
// UmbraSkillDataStructures.h – uso apenas para DataTable importada no cliente
USTRUCT(BlueprintType)
struct FUmbraSkillDataClient : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    int32 SkillID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString SkillKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString SkillName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    int32 ClassID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString ClassName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
    FString SkillType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
    FString Element;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
    FString ElementColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
    FString IconPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
    FString Description;
};
```

---

### Export completo (opcional – não recomendado no cliente)

O script **export_skills_csv.php** exporta **todos** os campos (cooldown, cast time, custo, scaling, alcance, etc.). Use apenas para ferramentas internas ou referência no servidor; **não** use esse CSV como DataTable no cliente para evitar expor dados de jogo.

- URL: `http://localhost/umbra_api/api/skills/export_skills_csv.php`
- Struct no UE: **FUmbraSkillData** (completa). No cliente, **FUmbraSkillData** é usada em **runtime** para o array **Available Skills** preenchido pela API — o servidor envia apenas o que deseja; a DataTable no disco deve ser a mínima (**FUmbraSkillDataClient**).

---

### Struct UE5 – FUmbraSkillData (runtime / API)

```cpp
USTRUCT(BlueprintType)
struct FUmbraSkillData : public FTableRowBase
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SkillID;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SkillKey;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SkillName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ClassID;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ClassName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredLevel;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SkillType;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TargetType;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Element;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ElementColor;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PowerCoef;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ResourceType;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ResourceCost;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CooldownMs;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CastTimeMs;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DurationMs;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString IconPath;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;
};
```

---

## Arquivos do Sistema

### SQL
- `www/umbra_api/scripts/create_skill_system.sql` - Schema
- `www/umbra_api/scripts/insert_all_skills.sql` - 90 skills

### PHP APIs
- `www/umbra_api/api/skills/` - Todos os endpoints
- `www/umbra_api/api/skills/export_skills_csv_client.php` - Export CSV **client-only** (DataTable mínima no UE)
- `www/umbra_api/api/skills/export_skills_csv.php` - Export CSV completo (uso em ferramentas; não no cliente)

### C++ Server
- `src/services/SkillTypes.hpp` - Structs e enums
- `src/services/CombatCalculator.hpp` - Engine de cálculo
- `src/services/SkillService.hpp` - Gerenciamento de skills

---

## Checklist de Implementação

### Backend
- [x] Schema SQL completo
- [x] 90 skills inseridas
- [x] APIs PHP funcionais
- [x] Estruturas C++ definidas

### Cliente UE5
- [ ] Struct FUmbraSkillData
- [ ] DataTable importado
- [ ] WBP_SkillBook implementado
- [ ] WBP_SkillBar implementado
- [ ] Integração com GameInstance
- [ ] Comunicação HTTP com APIs

---

## Próximos Passos

1. **Executar scripts SQL** no banco de dados
2. **Testar APIs** com Postman/Insomnia
3. **Importar DataTable** no Unreal
4. **Implementar WBP_SkillBook** seguindo guia
5. **Implementar WBP_SkillBar** seguindo guia
6. **Integrar com GameInstance** para requests HTTP
7. **Testar fluxo completo**: aprender → equipar → usar
