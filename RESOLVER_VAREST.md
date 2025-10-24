# 🔧 RESOLVER PROBLEMA DO VAREST

**Erro**: `Unable to find plugin 'VaRest'`  
**Solução**: Instalar VaRest corretamente

---

## ✅ MUDANÇAS REVERTIDAS!

O código está de volta ao original com VaRest habilitado.

---

## 🎯 PROBLEMA REAL

Você disse que:
> "quando instalei só o varest pediu o varestx e quando removi o varest e mantive o varestx ele pediu o varest"

Isso indica **dependência circular** ou **nome incorreto**.

---

## 📦 SOLUÇÃO: INSTALAR VAREST CORRETAMENTE

### **PASSO 1: Verificar qual VaRest você tem**

No Epic Games Launcher:

```
1. Library → My Projects & Plugins
2. Procure por "VaRest"
3. Veja qual está instalado
```

**Possibilidades**:
- VaRest Plugin (oficial - Vladimir Alyamkin)
- VaRestX (fork diferente)
- Ambos instalados (PROBLEMA!)

---

### **PASSO 2: Desinstalar TUDO relacionado**

```
1. Epic Launcher → Library
2. Procure "VaRest" e "VaRestX"
3. Desinstale AMBOS se estiverem instalados
4. Feche o Epic Launcher
```

**Limpar pastas manualmente**:

```powershell
# Verificar se existem:
D:\UE_5.6\Engine\Plugins\Marketplace\VaRest\
D:\UE_5.6\Engine\Plugins\Marketplace\VaRestX\
D:\UE_5.6\Engine\Plugins\Marketplace\VaRestReb*\

# Deletar TODAS as pastas que contenham "VaRest" no nome
```

---

### **PASSO 3: Instalar APENAS VaRest oficial**

```
1. Abra Epic Games Launcher
2. Marketplace
3. Busque: "VaRest Plugin"
4. Autor: Vladimir Alyamkin (ufna)
5. URL: https://www.unrealengine.com/marketplace/varest-plugin
6. Install to Engine → Selecione UE 5.6
7. Aguarde instalação
```

**IMPORTANTE**: Instale **SOMENTE** o VaRest oficial!
- ❌ NÃO instale VaRestX
- ❌ NÃO instale outros forks
- ✅ APENAS VaRest Plugin oficial

---

### **PASSO 4: Verificar instalação**

```powershell
# Verificar pasta:
cd D:\UE_5.6\Engine\Plugins\Marketplace\
dir | findstr VaRest

# Deve mostrar APENAS:
# VaRest

# NÃO deve ter:
# VaRestX
# VaRestReb...
# Ou qualquer outra variação
```

---

### **PASSO 5: Limpar projeto e regenerar**

Execute o script de limpeza:

```
1. D:\UmbraServerV2\UmbraEternumUE\CleanProject.bat
2. Duplo clique
3. Digite "S"
4. Aguarde
```

Ou manualmente:
```powershell
cd D:\UmbraServerV2\UmbraEternumUE
Remove-Item -Recurse -Force Binaries,Intermediate,Saved,.vs,DerivedDataCache
Remove-Item *.sln
```

---

### **PASSO 6: Regenerar project files**

```
1. Botão direito em: UmbraEternumUE.uproject
2. "Generate Visual Studio project files"
3. Aguarde (~1 minuto)
```

---

### **PASSO 7: Recompilar**

```
1. Abra: UmbraEternumUE.sln
2. Build → Rebuild Solution
3. Deve compilar SEM erros!
```

---

## 🔍 DIAGN

ÓSTICO

### Verificar se VaRest está disponível

Abra PowerShell:

```powershell
# Verificar plugins instalados:
Get-ChildItem "D:\UE_5.6\Engine\Plugins\Marketplace\" | Where-Object { $_.Name -like "*VaRest*" }

# Resultado esperado:
# Mode    LastWriteTime    Name
# d----   <data>           VaRest

# NÃO deve aparecer:
# VaRestX, VaRestReb..., etc.
```

---

## 🐛 SE AINDA NÃO FUNCIONAR

### Opção A: Instalar no projeto ao invés da engine

```
1. Crie pasta: D:\UmbraServerV2\UmbraEternumUE\Plugins\
2. Baixe VaRest do GitHub:
   https://github.com/ufna/VaRest
3. Clone ou baixe ZIP
4. Extraia para: D:\UmbraServerV2\UmbraEternumUE\Plugins\VaRest\
5. Regenerar project files
6. Recompilar
```

### Opção B: Usar versão específica

Se UE 5.6 não tem VaRest no Marketplace:

```
1. GitHub: https://github.com/ufna/VaRest
2. Releases → Encontre versão para UE 5.6
3. Baixe e instale manualmente
```

---

## 📋 CHECKLIST DE RESOLUÇÃO

### Limpeza Total
- [ ] Desinstalei VaRest do Epic Launcher
- [ ] Desinstalei VaRestX do Epic Launcher
- [ ] Deletei pastas manualmente:
  - [ ] D:\UE_5.6\Engine\Plugins\Marketplace\VaRest*
  - [ ] Verificado que não existe mais nenhuma

### Instalação Limpa
- [ ] Abri Epic Launcher
- [ ] Marketplace → VaRest Plugin (oficial)
- [ ] Autor: Vladimir Alyamkin
- [ ] Install to Engine → UE 5.6
- [ ] Aguardei instalação completar
- [ ] Verificado em: D:\UE_5.6\Engine\Plugins\Marketplace\VaRest\
- [ ] Existe APENAS uma pasta "VaRest"

### Limpeza do Projeto
- [ ] Executei CleanProject.bat
- [ ] Ou deletei manualmente:
  - [ ] Binaries\
  - [ ] Intermediate\
  - [ ] Saved\
  - [ ] .vs\
  - [ ] *.sln

### Regeneração
- [ ] Right-click UmbraEternumUE.uproject
- [ ] Generate Visual Studio project files
- [ ] Aguardei completar

### Compilação
- [ ] Abri UmbraEternumUE.sln
- [ ] Build → Rebuild Solution
- [ ] ✅ Compilação bem-sucedida!

---

## 💡 ENTENDENDO O PROBLEMA

### Por que VaRest pediu VaRestX?

**Causa provável**:
- Você instalou VaRestX primeiro
- VaRestX tem VaRest como dependência
- Quando você removeu VaRest, VaRestX reclamou

**Solução**:
- Remova AMBOS
- Instale APENAS VaRest oficial
- NÃO instale VaRestX

### Por que ter ambos causa problemas?

```
VaRest e VaRestX definem as MESMAS classes:
- UVaRestRequestJSON
- UVaRestJsonObject
- etc.

Isso causa conflito de namespace:
"The namespace already contains a definition for 'VaRest'"
```

---

## 🎯 COMANDOS ÚTEIS

### Verificar instalação:

```powershell
# Ver plugins instalados na engine:
Get-ChildItem "D:\UE_5.6\Engine\Plugins\Marketplace\"

# Ver plugins instalados no projeto:
Get-ChildItem "D:\UmbraServerV2\UmbraEternumUE\Plugins\"

# Procurar VaRest especificamente:
Get-ChildItem "D:\UE_5.6\Engine\Plugins\Marketplace\" -Recurse -Include "VaRest.uplugin"
```

### Limpar tudo de uma vez:

```powershell
# Ir para projeto:
cd D:\UmbraServerV2\UmbraEternumUE

# Limpar:
Remove-Item -Recurse -Force @(
    "Binaries",
    "Intermediate", 
    "Saved",
    ".vs",
    "DerivedDataCache"
) -ErrorAction SilentlyContinue

Remove-Item *.sln -ErrorAction SilentlyContinue

Write-Host "Limpeza completa!"
```

---

## ✅ RESUMO DO PROCEDIMENTO

```
╔══════════════════════════════════════════════════════════╗
║                                                          ║
║         🔧 RESOLVER VAREST - PASSO A PASSO 🔧           ║
║                                                          ║
║  1. LIMPAR TUDO                                         ║
║     - Desinstalar VaRest e VaRestX                      ║
║     - Deletar pastas Marketplace\VaRest*                ║
║                                                          ║
║  2. INSTALAR LIMPO                                      ║
║     - Epic Launcher → Marketplace                       ║
║     - VaRest Plugin (oficial)                           ║
║     - Install to UE 5.6                                 ║
║                                                          ║
║  3. LIMPAR PROJETO                                      ║
║     - CleanProject.bat                                  ║
║     - Ou deletar pastas manualmente                     ║
║                                                          ║
║  4. REGENERAR                                           ║
║     - Generate VS project files                         ║
║                                                          ║
║  5. COMPILAR                                            ║
║     - Rebuild Solution                                  ║
║     - ✅ Sucesso!                                       ║
║                                                          ║
╚══════════════════════════════════════════════════════════╝
```

---

**Criado**: 14/10/2025  
**Status**: ✅ **Código revertido - VaRest habilitado**  
**Ação**: **Siga os passos acima para instalar VaRest**

---

# 🚀 SIGA OS 5 PASSOS ACIMA!

**A solução está em instalar VaRest corretamente, não em removê-lo!** ✨

