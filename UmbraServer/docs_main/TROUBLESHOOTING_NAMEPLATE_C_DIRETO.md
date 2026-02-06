# 🔧 TROUBLESHOOTING: Nameplate Atualização Direta no C++

## 🎯 PROBLEMA

A função `UpdateNameplate` existe no widget, mas não está sendo chamada ou não está funcionando.

## ✅ VERIFICAÇÕES NECESSÁRIAS

### **1. Verificar se o WidgetComponent está configurado no Remote Player**

**No Blueprint do Remote Player (`BP_RemotePlayer` ou similar):**

1. Abra o Blueprint do remote player
2. Verifique se existe um **Widget Component** adicionado ao actor
3. Se não existir:
   - Adicione um `Widget Component` ao actor
   - Configure o `Widget Class` para `WBP_PlayerNameplate`
   - Certifique-se de que o widget está sendo criado (não apenas configurado)

**Como verificar:**
- No Blueprint, vá para `Components`
- Procure por `Widget Component` ou `WidgetComponent`
- Se não existir, adicione um novo componente do tipo `Widget Component`

---

### **2. Verificar se o Widget está sendo criado**

**No Blueprint do Remote Player, no `Event BeginPlay`:**

O widget deve ser criado explicitamente:

```
[Event BeginPlay]
    ↓
[Get Component By Class]
    Actor: self
    Class: Widget Component
    Return Value: WidgetComp
    ↓
[Is Valid?] WidgetComp
    ↓
    then → [Create Widget]
            Widget Class: WBP_PlayerNameplate
            Owner: self
            Return Value: NameplateWidget
            ↓
            [Set Widget]
                Target: WidgetComp
                Widget: NameplateWidget
```

**⚠️ IMPORTANTE:** O widget deve ser criado no `BeginPlay`, não apenas configurado no editor!

---

### **3. Verificar se a função UpdateNameplate está marcada como UFUNCTION**

**No Blueprint `WBP_PlayerNameplate`:**

1. Abra o widget `WBP_PlayerNameplate`
2. Encontre a função `UpdateNameplate`
3. **Verifique as configurações da função:**
   - ✅ Deve estar marcada como **"Call In Editor"** ou **"Blueprint Callable"**
   - ✅ Deve ser **"Public"** (não Private)
   - ✅ Deve ter exatamente 2 inputs:
     - `CharacterName` (String)
     - `TitleName` (String)

**Como verificar:**
- Clique na função `UpdateNameplate`
- No painel de detalhes, verifique:
  - `Access Specifier`: **Public**
  - `Blueprint Callable`: **Marcado** ✅
  - `Call In Editor`: **Opcional, mas recomendado**

---

### **4. Verificar a assinatura da função**

A função `UpdateNameplate` **DEVE** ter exatamente esta assinatura:

```
UpdateNameplate(
    FString CharacterName,
    FString TitleName
)
```

**Ordem dos parâmetros é importante!** O C++ espera:
1. Primeiro: `CharacterName` (String)
2. Segundo: `TitleName` (String)

---

### **5. Verificar os logs**

Após recompilar e testar, verifique os logs:

**Se você ver:**
- `❌ WidgetComponent NÃO encontrado` → O remote player não tem WidgetComponent configurado
- `❌ UserWidget é nullptr` → O widget não foi criado no BeginPlay
- `❌ Função UpdateNameplate NÃO encontrada` → A função não existe ou não está marcada como UFUNCTION
- `✅ ProcessEvent chamado` → A função foi chamada, mas pode não estar funcionando

**Se a função foi chamada mas não atualiza:**
- Verifique se a função `UpdateNameplate` no widget está realmente atualizando os campos de texto
- Verifique se os campos de texto estão conectados corretamente

---

## 🔧 SOLUÇÃO PASSO A PASSO

### **PASSO 1: Configurar WidgetComponent no Remote Player**

1. Abra o Blueprint do remote player
2. No `Event BeginPlay`, adicione:

```
[Event BeginPlay]
    ↓
[Get Component By Class]
    Actor: self
    Class: Widget Component
    Return Value: WidgetComp
    ↓
[Branch] Is Valid?
    Condition: WidgetComp
    ↓
    True → [Create Widget]
            Widget Class: WBP_PlayerNameplate
            Owner: self
            Return Value: NameplateWidget
            ↓
            [Set Widget]
                Target: WidgetComp
                Widget: NameplateWidget
```

---

### **PASSO 2: Verificar função UpdateNameplate no Widget**

1. Abra `WBP_PlayerNameplate`
2. Encontre a função `UpdateNameplate`
3. Verifique:
   - ✅ É **Public**
   - ✅ Está marcada como **Blueprint Callable**
   - ✅ Tem 2 inputs: `CharacterName` (String) e `TitleName` (String)
4. Se não estiver, ajuste:

**Na função UpdateNameplate:**
```
[Function: UpdateNameplate]
    Inputs: CharacterName (String), TitleName (String)
    ↓
[Set Text]
    Target: TextBlock_CharacterName (ou nome do seu campo)
    Text: CharacterName
    ↓
[Set Text]
    Target: TextBlock_TitleName (ou nome do seu campo)
    Text: TitleName
```

---

### **PASSO 3: Testar**

1. Recompile o projeto C++
2. Abra o Unreal Editor
3. Execute o jogo
4. Verifique os logs no Output Log
5. Procure por mensagens começando com `[UmbraGameInstance] 🔍`

---

## 🐛 PROBLEMAS COMUNS

### **Problema 1: WidgetComponent não encontrado**

**Sintoma:** Log mostra `❌ WidgetComponent NÃO encontrado`

**Solução:**
- Adicione um WidgetComponent ao remote player
- Certifique-se de que está sendo criado no BeginPlay

---

### **Problema 2: Função não encontrada**

**Sintoma:** Log mostra `❌ Função UpdateNameplate NÃO encontrada`

**Solução:**
- Verifique se a função existe no widget
- Certifique-se de que está marcada como **Blueprint Callable**
- Verifique se o nome está exatamente `UpdateNameplate` (case-sensitive)

---

### **Problema 3: Widget não criado**

**Sintoma:** Log mostra `❌ UserWidget é nullptr`

**Solução:**
- Certifique-se de que o widget está sendo criado no `BeginPlay` do remote player
- Use `Create Widget` e depois `Set Widget` no WidgetComponent

---

### **Problema 4: Função chamada mas não atualiza**

**Sintoma:** Log mostra `✅ ProcessEvent chamado` mas o nameplate não atualiza

**Solução:**
- Verifique se a função `UpdateNameplate` está realmente atualizando os campos de texto
- Verifique se os campos de texto estão conectados corretamente na função
- Adicione um `Print String` dentro da função para verificar se ela está sendo executada

---

## 📋 CHECKLIST FINAL

- [ ] WidgetComponent está adicionado ao remote player
- [ ] Widget está sendo criado no `BeginPlay` do remote player
- [ ] Função `UpdateNameplate` existe no `WBP_PlayerNameplate`
- [ ] Função está marcada como **Public** e **Blueprint Callable**
- [ ] Função tem 2 inputs: `CharacterName` (String) e `TitleName` (String)
- [ ] Função está atualizando os campos de texto corretamente
- [ ] Projeto C++ foi recompilado
- [ ] Logs mostram que a função foi encontrada e chamada

---

**Se ainda não funcionar após seguir todos os passos, envie os logs completos para análise!**
