# 🔧 **CORREÇÃO: ClassIDs dos Placeholders no Nível**

## ❌ **PROBLEMA**

Se apenas Assassin (ClassID 3) e Dark Mage (ClassID 5) estão trocados, significa que os **ClassIDs dos placeholders no nível estão errados**.

**Causa:**
- O placeholder de Assassin no nível tem `ClassID = 5` (deveria ser 3)
- O placeholder de Dark Mage no nível tem `ClassID = 3` (deveria ser 5)

---

## ✅ **SOLUÇÃO: Corrigir ClassIDs no Nível**

### **PASSO 1: Verificar ClassIDs Atuais**

**No Unreal Editor:**

1. **Abra o nível `Lvl_Character_Creation`** (ou o nível onde os placeholders estão)

2. **Selecione o placeholder de Assassin** no World Outliner ou na viewport

3. **No Details Panel, encontre a variável `ClassID`**
   - Anote o valor atual

4. **Repita para o placeholder de Dark Mage**

### **PASSO 2: Corrigir os ClassIDs**

**Para o placeholder de Assassin:**
- **ClassID deve ser: `3`**
- Se estiver como `5`, mude para `3`

**Para o placeholder de Dark Mage:**
- **ClassID deve ser: `5`**
- Se estiver como `3`, mude para `5`

---

## 📋 **ClassIDs Corretos**

| Classe | ClassID Correto |
|--------|----------------|
| Barbarian | 1 |
| Templar | 2 |
| Assassin | 3 |
| Cleric | 4 |
| Dark Mage | 5 |
| Monk | 6 |

---

## ⚠️ **IMPORTANTE**

**Se os placeholders são Blueprints (`BP_Assassin_Placeholder`, `BP_DarkMage_Placeholder`), você pode:**

1. **Abrir o Blueprint** do placeholder
2. **No Event Graph ou Construction Script**, verificar onde o `ClassID` é setado
3. **Corrigir o valor padrão** da variável `ClassID` no Blueprint

**OU:**

1. **No nível**, selecione cada placeholder
2. **No Details Panel**, encontre a variável `ClassID`
3. **Altere o valor** diretamente no nível (isso sobrescreve o valor padrão do Blueprint)

---

## 🔍 **Como Verificar Todos os ClassIDs**

**Para garantir que todos estão corretos:**

1. **Selecione cada placeholder no nível**
2. **Verifique o `ClassID` no Details Panel**
3. **Compare com a tabela acima**

**Ou adicione logs temporários no Blueprint:**
- No `Event Construct` do `WBP_CreateCharacter`, após encontrar cada placeholder, adicione `Print String` com: `"Placeholder encontrado: ClassID X"`

---

**FIM DA CORREÇÃO**

