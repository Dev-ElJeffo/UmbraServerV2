# 🎮 **COMO FECHAR APENAS UM CLIENT NO PIE**

## 🎯 **MÉTODO 1: Fechar Janela Específica (RECOMENDADO)**

### **Passo a Passo:**

1. **Inicie o PIE com múltiplos clientes** (ex: 2 ou 3)
2. **Cada cliente abre em uma janela separada**
3. **Clique no X (fechar) da janela de UM cliente específico**
4. **Apenas aquele cliente fecha, os outros continuam rodando**

**⚠️ IMPORTANTE:** Não use ESC, use o botão X da janela!

---

## 🎯 **MÉTODO 2: Usar o Botão "Stop" de Uma Instância**

### **No Editor do Unreal:**

1. **Inicie o PIE com múltiplos clientes**
2. **No canto superior direito de cada janela PIE, há um botão "Stop"**
3. **Clique no "Stop" de UMA instância específica**
4. **Apenas aquela instância para, as outras continuam**

---

## 🎯 **MÉTODO 3: Usar Servidor Dedicado + Clientes Separados**

### **Para Teste Mais Realista:**

1. **Inicie o servidor C++ (`zone_server`)**
2. **Inicie o Unreal Engine normalmente (não PIE)**
3. **Conecte múltiplas instâncias do jogo**
4. **Feche uma instância normalmente (X ou Alt+F4)**
5. **As outras instâncias continuam rodando**

---

## 🎯 **MÉTODO 4: Usar Console Command para Desconectar**

### **No Cliente que Quer Fechar:**

1. **Pressione ` (til) para abrir o console**
2. **Digite:** `disconnect` ou `quit`
3. **Apenas aquele cliente desconecta**

---

## 📋 **TESTE RECOMENDADO:**

### **Para Testar CleanupRemoteActors:**

1. **Inicie PIE com 2 clientes:**
   - Editor → Play → Number of Players: 2
   - Ou: Editor → Play → Advanced Settings → Number of Players: 2

2. **Mova ambos os clientes** (para garantir que há remote actors)

3. **Feche APENAS uma janela PIE** (clique no X, não use ESC)

4. **Verifique nos logs:**
   - Deve aparecer "Event EndPlay EVENTO DISPARADO!" apenas uma vez
   - Deve aparecer "CleanupRemoteActors EXECUTADO!"
   - No outro cliente, os remote actors devem desaparecer

---

## ⚠️ **IMPORTANTE:**

- **ESC fecha TODOS os clientes PIE** - não use para este teste
- **X (fechar janela) fecha apenas aquele cliente** - use este método
- **Se todos fecharem juntos, o `Event EndPlay` pode não disparar corretamente** para cada instância

---

## 🧪 **VERIFICAÇÃO:**

**Após fechar uma janela PIE:**

1. **Verifique os logs:**
   - Deve aparecer logs do `Event EndPlay` apenas uma vez (do cliente que fechou)
   - Não deve aparecer logs de outros clientes

2. **Verifique visualmente:**
   - No cliente que permaneceu aberto, os remote actors do cliente fechado devem desaparecer

3. **Se não aparecer nenhum log:**
   - O `Event EndPlay` pode não estar disparando
   - Tente usar `OnWSClosed` em vez de `Event EndPlay`

