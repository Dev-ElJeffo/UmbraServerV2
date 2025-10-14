# 🔌 COMO INSTALAR O PLUGIN VAREST

**Problema**: `Unable to find plugin 'VaRest'`  
**Solução**: Instalar VaRest corretamente via Epic Games Launcher

---

## ⚡ CORREÇÃO APLICADA

✅ **Projeto já está preparado para compilar SEM VaRest**

As classes C++ foram modificadas para:
- ✅ Compilar sem VaRest temporariamente
- ✅ Mostrar avisos quando funções forem chamadas
- ✅ Fácil habilitar depois de instalar VaRest

---

## 🚀 PASSO 1: COMPILAR PRIMEIRO (SEM VAREST)

### Agora você pode compilar o projeto!

```
1. Abra Visual Studio
2. Build → Rebuild Solution
3. Aguarde compilação (~2-5 minutos)
4. ✅ Deve compilar SEM erros!
```

**Por que fazer isso?**
- Verifica que tudo está OK exceto VaRest
- Permite testar o projeto básico
- Facilita debug de outros problemas

---

## 🔌 PASSO 2: INSTALAR VAREST

### Método 1: Via Epic Games Launcher (RECOMENDADO)

```
1. Abra Epic Games Launcher
2. Aba "Unreal Engine" → "Library"
3. Na barra superior, clique em "Marketplace"
4. Busque: "VaRest"
5. Selecione o plugin correto:
   
   Nome: VaRest Plugin
   Autor: Vladimir Alyamkin
   Descrição: REST API plugin
   
6. Clique em "Free" (ou "Install to Engine" se já possuir)
7. Selecione: UE 5.6
8. Clique "Install"
9. Aguarde download e instalação
```

### Verificar Instalação

```
1. Abra Unreal Editor (pode abrir o projeto)
2. Edit → Plugins
3. Busque "VaRest"
4. Deve aparecer:
   ✅ VaRest Plugin
   ✅ Version: [última versão]
   ✅ Installed
5. Marque checkbox "Enabled"
6. Restart Editor
```

---

## 📝 PASSO 3: HABILITAR VAREST NO CÓDIGO

Após instalar VaRest, habilite-o no código:

### 1. Editar Build.cs

**Arquivo**: `D:\UmbraServerV2\UmbraEternumUE\Source\UmbraEternumUE\UmbraEternumUE.Build.cs`

**Linha 28**, descomente:

```csharp
// ANTES:
// "VaRest"  // ❌ TEMPORARIAMENTE DESABILITADO

// DEPOIS:
"VaRest"     // ✅ HABILITADO
```

**Código completo**:
```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "Core",
    "CoreUObject",
    "Engine",
    "InputCore",
    "EnhancedInput",
    "AIModule",
    "StateTreeModule",
    "GameplayStateTreeModule",
    "UMG",
    "Slate",
    "SlateCore",
    "HTTP",
    "Json",
    "JsonUtilities",
    "Sockets",
    "Networking",
    "VaRest"  // ✅ DESCOMENTE ESTA LINHA
});
```

### 2. Editar UmbraGameInstance.h

**Arquivo**: `D:\UmbraServerV2\UmbraEternumUE\Source\UmbraEternumUE\Core\UmbraGameInstance.h`

**Linha 8**, descomente:

```cpp
// ANTES:
// #include "VaRestRequestJSON.h"  // ❌ VaRest não instalado ainda

// DEPOIS:
#include "VaRestRequestJSON.h"     // ✅ VaRest instalado
```

**E remova a forward declaration** (linha 12):
```cpp
// REMOVER:
// class UVaRestRequestJSON;
```

### 3. Editar UmbraGameInstance.cpp

**Arquivo**: `D:\UmbraServerV2\UmbraEternumUE\Source\UmbraEternumUE\Core\UmbraGameInstance.cpp`

**Linhas 4-5**, descomente:

```cpp
// ANTES:
// #include "VaRestSubsystem.h"    // ❌ VaRest não instalado ainda
// #include "VaRestJsonObject.h"   // ❌ VaRest não instalado ainda

// DEPOIS:
#include "VaRestSubsystem.h"       // ✅ VaRest instalado
#include "VaRestJsonObject.h"      // ✅ VaRest instalado
```

**Linhas 21**, altere de `#if 0` para `#if 1`:

```cpp
// ANTES:
#if 0  // ❌ DESABILITADO TEMPORARIAMENTE

// DEPOIS:
#if 1  // ✅ HABILITADO - VAREST INSTALADO
```

**Linhas 369-427**, DELETE a implementação temporária:

```cpp
// DELETE TUDO ENTRE ESTAS LINHAS:
// ========== IMPLEMENTAÇÃO TEMPORÁRIA (SEM VAREST) ==========
...
[todo o código até o final]
```

---

## 🔄 PASSO 4: RECOMPILAR

```
1. Feche Unreal Editor
2. Visual Studio → Build → Rebuild Solution
3. Aguarde compilação
4. ✅ Deve compilar COM VaRest agora!
5. Abra Unreal Editor
6. Teste as funcionalidades de login/register
```

---

## 📊 CHECKLIST COMPLETO

### Fase 1: Compilar Sem VaRest ✅

- [x] Código modificado (já feito automaticamente)
- [ ] Rebuild Solution no Visual Studio
- [ ] Compilação bem-sucedida
- [ ] Projeto abre no Unreal Editor

### Fase 2: Instalar VaRest 📦

- [ ] Abri Epic Games Launcher
- [ ] Marketplace → VaRest
- [ ] Install to Engine → UE 5.6
- [ ] Aguardei instalação completar
- [ ] Verifiquei em Edit → Plugins
- [ ] VaRest aparece como "Installed"
- [ ] Habilitei VaRest (checkbox)
- [ ] Restart Editor

### Fase 3: Habilitar VaRest no Código 🔧

- [ ] Descomentei "VaRest" no Build.cs (linha 28)
- [ ] Descomentei #include no UmbraGameInstance.h (linha 8)
- [ ] Removi forward declaration (linha 12)
- [ ] Descomentei includes no UmbraGameInstance.cpp (linhas 4-5)
- [ ] Mudei #if 0 para #if 1 (linha 21)
- [ ] Deletei implementação temporária (linhas 369-427)

### Fase 4: Recompilar Final 🚀

- [ ] Fechei Unreal Editor
- [ ] Rebuild Solution
- [ ] Compilação bem-sucedida
- [ ] Abri Unreal Editor
- [ ] VaRest carregado sem erros
- [ ] Funcionalidades de API funcionando

---

## 🎯 FLUXO VISUAL

```
┌─────────────────────────────────────┐
│ 1. Compilar SEM VaRest              │
│    (Código já preparado!)           │
│    Build → Rebuild Solution         │
│    ✅ Sucesso!                      │
└──────────────┬──────────────────────┘
               │
               ↓
┌─────────────────────────────────────┐
│ 2. Instalar VaRest                  │
│    Epic Launcher → Marketplace      │
│    VaRest → Install to UE 5.6       │
│    ✅ Instalado!                    │
└──────────────┬──────────────────────┘
               │
               ↓
┌─────────────────────────────────────┐
│ 3. Habilitar no Código              │
│    - Descomentar Build.cs           │
│    - Descomentar includes           │
│    - Ativar código (#if 1)          │
│    - Deletar implementação temp     │
│    ✅ Código atualizado!            │
└──────────────┬──────────────────────┘
               │
               ↓
┌─────────────────────────────────────┐
│ 4. Recompilar COM VaRest            │
│    Rebuild Solution                 │
│    ✅ Tudo funcionando!             │
└─────────────────────────────────────┘
```

---

## 💡 DICAS IMPORTANTES

### VaRest vs VaRestX

Se aparecer "VaRestX" no Marketplace:
- São versões diferentes/forks
- Instale o **VaRest oficial** (Vladimir Alyamkin)
- **NÃO** instale VaRestX (a menos que seja dependência)

### Verificar Plugin Correto

```
Nome correto: VaRest Plugin
Autor: Vladimir Alyamkin (ufna)
Link: https://www.unrealengine.com/marketplace/en-US/product/varest-plugin
```

### Se VaRest não aparecer no Marketplace

1. Verifique sua conta Epic
2. Pode precisar "adicionar ao carrinho" (Free)
3. Depois aparecerá em "Library"

---

## 🐛 TROUBLESHOOTING

### Erro: "Module VaRest not found" após instalar

**Solução**:
```
1. Edit → Plugins
2. VaRest → ☑️ Enabled
3. Restart Editor
4. Rebuild Solution
```

### Erro: VaRest aparece mas diz "incompatível"

**Solução**:
```
1. Desinstale VaRest
2. Reinstale para UE 5.6 especificamente
3. Verifique que selecionou a engine correta
```

### Warnings ao compilar com VaRest

**Normal!** Alguns warnings do plugin são esperados.
- Ignore warnings de "deprecated functions"
- Se compilar (Build succeeded), está OK

---

## 📚 DOCUMENTAÇÃO

**Após instalar VaRest**, consulte:
- `UE5_API_INTEGRATION.md` - Como usar VaRest
- `UE5_QUICKSTART.md` - Setup rápido
- `UE5_CLASSES_CREATED.md` - Classes criadas

---

## ✅ RESUMO

```
╔══════════════════════════════════════════════════════════╗
║                                                          ║
║         ✅ PROJETO PREPARADO PARA COMPILAR! ✅          ║
║                                                          ║
║  1. AGORA: Rebuild Solution                             ║
║     → Compila SEM VaRest                                ║
║     → Deve ter sucesso!                                 ║
║                                                          ║
║  2. DEPOIS: Instalar VaRest                             ║
║     → Epic Launcher → Marketplace                       ║
║     → VaRest → Install                                  ║
║                                                          ║
║  3. ENTÃO: Habilitar no código                          ║
║     → Descomentar linhas                                ║
║     → Seguir guia acima                                 ║
║                                                          ║
║  4. FINAL: Recompilar                                   ║
║     → Com VaRest ativado                                ║
║     → Tudo funcionando!                                 ║
║                                                          ║
╚══════════════════════════════════════════════════════════╝
```

---

**Criado**: 14/10/2025  
**Status**: ✅ **Código preparado - Pronto para compilar!**  
**Próximo passo**: **Rebuild Solution AGORA!**

---

# 🚀 COMPILE AGORA - DEPOIS INSTALE VAREST!

**Build → Rebuild Solution** 🎉

