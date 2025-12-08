# 🔧 **CORREÇÃO: Erro 500 ao Criar Personagem**

## ❌ **PROBLEMA**

Ao tentar criar um personagem, a API retorna erro 500 com a mensagem genérica "Erro ao criar personagem", sem detalhes do erro real.

**Logs:**
```
LogTemp: [UmbraGameInstance] Criando personagem: wonk (ClassID: 6, Hair: 3, Head: 6)
LogVaRest: Response (500): {"success":false,"message":"Erro ao criar personagem"}
```

---

## ✅ **SOLUÇÃO 1: Habilitar Logs de Erro no PHP**

O código PHP está com `error_reporting(0)` e `display_errors` desabilitado, o que esconde o erro real.

### **PASSO 1: Modificar create_character.php Temporariamente**

**Arquivo:** `www/umbra_api/api/character/create_character.php`

**Linha 13-14:** Comentar ou modificar para ver erros:

```php
// ANTES:
error_reporting(0);
ini_set('display_errors', '0');

// DEPOIS (temporariamente para debug):
error_reporting(E_ALL);
ini_set('display_errors', '1');
ini_set('log_errors', '1');
ini_set('error_log', __DIR__ . '/../../logs/php_errors.log');
```

### **PASSO 2: Melhorar Tratamento de Erro**

**Linha 368-372:** Modificar o catch para mostrar o erro real:

```php
} catch (Exception $e) {
    error_log("Create Character Error: " . $e->getMessage());
    error_log("Stack trace: " . $e->getTraceAsString());
    
    // Para debug, retornar o erro real (REMOVER EM PRODUÇÃO)
    http_response_code(500);
    echo json_encode([
        'success' => false, 
        'message' => 'Erro ao criar personagem',
        'error' => $e->getMessage(), // ← ADICIONAR ESTA LINHA TEMPORARIAMENTE
        'file' => $e->getFile(),
        'line' => $e->getLine()
    ]);
}
```

---

## 🔍 **POSSÍVEIS CAUSAS DO ERRO 500**

### **1. Problema com a Query SQL**

A query INSERT pode estar falhando por:
- Coluna não existe na tabela `players`
- Tipo de dado incorreto
- Constraint violada (ex: foreign key)

### **2. Problema com Campos Obrigatórios**

Verificar se a tabela `players` tem todos os campos necessários:
- `hair` (INT)
- `head` (INT)
- `class_id` (INT)
- Todos os campos de stats

### **3. Problema com Foreign Key**

Se `class_id` não existir na tabela `classes`, a inserção falhará.

---

## ✅ **SOLUÇÃO 2: Verificar Estrutura da Tabela**

Execute no MySQL:

```sql
DESCRIBE players;
```

Verifique se existem as colunas:
- ✅ `hair` (INT UNSIGNED)
- ✅ `head` (INT UNSIGNED)
- ✅ `class_id` (INT)

---

## ✅ **SOLUÇÃO 3: Testar Query SQL Diretamente**

Execute no MySQL para testar:

```sql
INSERT INTO players (
    account_id,
    character_name,
    class_id,
    hair,
    head,
    level,
    experience,
    pos_x,
    pos_y,
    pos_z,
    current_zone,
    health,
    max_health,
    mana,
    max_mana,
    stamina,
    max_stamina,
    strength,
    dexterity,
    intelligence,
    vitality,
    luck,
    created_at
) VALUES (
    1,  -- account_id (substitua pelo ID real)
    'Teste',  -- character_name
    6,  -- class_id
    3,  -- hair
    6,  -- head
    1,  -- level
    0,  -- experience
    0.0, 0.0, 0.0,  -- pos_x, pos_y, pos_z
    'Tutorial',  -- current_zone
    100,  -- health
    100,  -- max_health
    50,   -- mana
    50,   -- max_mana
    80,   -- stamina
    80,   -- max_stamina
    10,   -- strength
    12,   -- dexterity
    15,   -- intelligence
    14,   -- vitality
    10,   -- luck
    NOW()  -- created_at
);
```

Se esta query funcionar, o problema está no PHP. Se não funcionar, o erro SQL mostrará o problema.

---

## ✅ **SOLUÇÃO 4: Verificar Logs do PHP**

Após habilitar logs (Solução 1), verifique o arquivo:
- `www/umbra_api/logs/php_errors.log`

Ou o log do servidor web (Apache/Nginx).

---

## 🔧 **CORREÇÃO DO SetInputMode_UIOnly**

O erro `SetInputMode_UIOnly espera um controlador de reprodutor válido` indica que o `PlayerController` é `null` quando a função é chamada.

### **Solução:**

No Blueprint, antes de chamar `Set Input Mode UI Only`, verifique se o Player Controller é válido:

```
[Get Player Controller]
    ↓
[Is Valid]
    • Object: (Player Controller)
    ↓ (True)
    [Set Input Mode UI Only]
        • Player Controller: (Player Controller)
    ↓ (False)
    [Print String]
        • In String: "Player Controller inválido!"
```

---

## 📋 **CHECKLIST DE DIAGNÓSTICO**

1. ✅ Habilitar logs de erro no PHP (Solução 1)
2. ✅ Verificar estrutura da tabela `players` (Solução 2)
3. ✅ Testar query SQL diretamente (Solução 3)
4. ✅ Verificar logs do PHP (Solução 4)
5. ✅ Corrigir SetInputMode com validação (Solução 4)

---

## 🎯 **PRÓXIMOS PASSOS**

1. **Habilite os logs de erro** no PHP (Solução 1)
2. **Tente criar o personagem novamente**
3. **Verifique o erro real** nos logs ou na resposta JSON
4. **Corrija o problema específico** baseado no erro real
5. **Desabilite os logs de erro** após corrigir (volte para `error_reporting(0)`)

---

## 📌 **NOTA IMPORTANTE**

Após identificar e corrigir o erro, **REMOVA** as linhas de debug (`error`, `file`, `line` do JSON de resposta) para não expor informações sensíveis em produção.

