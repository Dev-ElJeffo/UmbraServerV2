# Solução: Múltiplas Requisições de LoadAvailableSkills

## Problema Identificado

Ao abrir o SkillBook, centenas de requisições HTTP para `get_available_skills.php` eram disparadas sucessivamente, causando:

1. **UI travada** durante reconstrução
2. **Cliques não registravam** porque widgets eram destruídos durante rebuild
3. **Lag extremo** no cliente e servidor

### Causa Raiz

**Múltiplos binds do callback causavam loop infinito:**

```
OpenSkillBook()
  ↓
LoadAvailableSkills() #1 → AddDynamic(OnLoadAvailableSkillsComplete) #1
  ↓
[Resposta da API]
  ↓
OnLoadAvailableSkillsComplete disparado (1x)
  ↓
OnAvailableSkillsLoaded.Broadcast() → PopulateSkillList() no Blueprint
  ↓
PopulateSkillList chama LoadAvailableSkills() #2 → AddDynamic(OnLoadAvailableSkillsComplete) #2
  ↓
[Resposta da API]
  ↓
OnLoadAvailableSkillsComplete disparado (2x - foi bindado 2 vezes!)
  ↓
OnAvailableSkillsLoaded.Broadcast() → PopulateSkillList() DUAS VEZES
  ↓
... loop exponencial ...
```

## Soluções Implementadas

### 1. Flag de Carregamento (`bIsLoadingAvailableSkills`)

**Evita requisições simultâneas.**

```cpp
if (bIsLoadingAvailableSkills)
{
    // Já está carregando, ignore
    return;
}
bIsLoadingAvailableSkills = true;
// ... fazer requisição ...
bIsLoadingAvailableSkills = false; // no callback (complete/fail)
```

### 2. Verificação de Bind Duplicado

**Não adiciona o mesmo callback múltiplas vezes à mesma requisição.**

```cpp
if (!Request->OnRequestComplete.IsAlreadyBound(this, &UUmbraGameInstance::OnLoadAvailableSkillsComplete))
{
    Request->OnRequestComplete.AddDynamic(this, &UUmbraGameInstance::OnLoadAvailableSkillsComplete);
}
```

### 3. Chamada de LoadAvailableSkills em OpenSkillBook

**Garante que skills são carregadas quando o SkillBook abre.**

```cpp
// Em AUmbraEternumUEPlayerController::OpenSkillBook()
UUmbraGameInstance* GameInstance = Cast<UUmbraGameInstance>(GetGameInstance());
if (GameInstance)
{
    GameInstance->LoadAvailableSkills();
}
```

## Arquivos Modificados

1. **UmbraGameInstance.h**
   - Adicionada `bool bIsLoadingAvailableSkills = false;`
   - Adicionada `bool bAvailableSkillsCallbacksRegistered = false;`

2. **UmbraGameInstance.cpp**
   - `LoadAvailableSkills()`: Adiciona verificação `if (bIsLoadingAvailableSkills)`
   - `LoadAvailableSkills()`: Verifica se callback já está bound antes de adicionar
   - `OnLoadAvailableSkillsComplete()`: Reset `bIsLoadingAvailableSkills = false`
   - `OnLoadAvailableSkillsFail()`: Reset `bIsLoadingAvailableSkills = false`

3. **UmbraEternumUEPlayerController.cpp**
   - `OpenSkillBook()`: Chama `GameInstance->LoadAvailableSkills()`

## Teste

### Antes (PROBLEMA)
- Centenas de requisições HTTP
- Barra lateral de output gigante
- UI extremamente lenta

### Depois (ESPERADO)
- UMA requisição ao abrir SkillBook
- UMA requisição ao aprender/upar skill
- UI responsiva
- Cliques funcionam normalmente

## Fluxo Correto Agora

```
OpenSkillBook()
  ↓
LoadAvailableSkills() [flag bIsLoadingAvailableSkills = true]
  ↓
[Resposta da API]
  ↓
OnLoadAvailableSkillsComplete() [flag reset]
  ↓
OnAvailableSkillsLoaded.Broadcast() → PopulateSkillList() [NO BLUEPRINT]
  ↓
PopulateSkillList lê AvailableSkills do GameInstance (sem chamar LoadAvailableSkills)
  ↓
Reconstrói widgets UMA VEZ
  ↓
[Usuário clica em skill]
  ↓
SelectSkill() funciona normalmente
```

## Verificação em Blueprint

**No WBP_SkillBook, Event Construct:**

```
Event Construct
  ↓
Set GameInstanceRef
  ↓
Bind On Available Skills Loaded → PopulateSkillList → UpdateHeader
  ↓ (NÃO CHAMA LoadAvailableSkills aqui)
Bind On Skill Learned → PopulateSkillList → UpdateHeader
  ↓
Bind On Skill Upgraded → PopulateSkillList → UpdateHeader
  ↓
(Fim - espera o widget ser aberto via OpenSkillBook no PlayerController)
```

**Em PopulateSkillList:**
- ✅ Pega `AvailableSkills` do GameInstance
- ✅ Limpa filhos
- ✅ Cria widgets
- ❌ **NÃO deve chamar `LoadAvailableSkills()`**

## Notas

- A chamada `LoadAvailableSkills()` em `OpenSkillBook()` é segura porque a flag `bIsLoadingAvailableSkills` evita requisições simultâneas.
- Aprender/Upar skill ainda chama `LoadAvailableSkills()` normalmente para refrescar status.
- O blueprint NÃO deve incluir `LoadAvailableSkills()` no callback de `OnAvailableSkillsLoaded`.
