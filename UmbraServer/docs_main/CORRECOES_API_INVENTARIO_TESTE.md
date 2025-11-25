# 🔧 Correções da API de Inventário - Sistema de Testes

**Data**: 14/11/2024, 15:35  
**Problemas Identificados e Corrigidos**

---

## ❌ Problemas Encontrados

### Problema 1: Itens Duplicados no Banco de Dados
**Sintoma**: A lista de templates mostrava 34 itens em vez de 17, todos duplicados.

**Causa**: O script SQL `setup_inventory_system.sql` foi executado duas vezes.

**Itens afetados**:
- Espada de Ferro (IDs 1 e 18)
- Espada Flamejante (IDs 2 e 19)
- Todos os outros 15 itens também estavam duplicados

---

### Problema 2: Token JWT Não Reconhecido
**Sintoma**: 
```
❌ Erro ao Carregar Inventário
Unexpected token '<', "<!DOCTYPE html>..."
```

**Causa**: O arquivo `www/umbra_api/helpers/jwt_helper.php` estava **faltando**, causando erro fatal no PHP. As requisições retornavam HTML de erro em vez de JSON.

---

## ✅ Correções Aplicadas

### Correção 1: Remoção de Itens Duplicados

**Comando executado**:
```sql
DELETE FROM item_templates WHERE item_id > 17;
```

**Resultado**:
```sql
SELECT COUNT(*) FROM item_templates;
-- Antes: 34 itens
-- Depois: 17 itens ✅
```

**Lista de itens únicos agora no banco**:
| ID | Nome | Raridade |
|----|------|----------|
| 1 | Espada de Ferro | common |
| 2 | Espada Flamejante | legendary |
| 3 | Arco Longo | uncommon |
| 4 | Capacete de Couro | common |
| 5 | Peitoral de Aço | rare |
| 6 | Botas Élficas | epic |
| 7 | Poção de Vida Menor | common |
| 8 | Poção de Vida Maior | uncommon |
| 9 | Poção de Mana | common |
| 10 | Elixir de Força | rare |
| 11 | Minério de Ferro | common |
| 12 | Cristal de Mana | rare |
| 13 | Couro de Lobo | common |
| 14 | Medalhão Antigo | epic |
| 15 | Chave da Torre | uncommon |
| 16 | Moeda de Ouro | common |
| 17 | Gema Brilhante | rare |

---

### Correção 2: Criação do `jwt_helper.php`

**Arquivo criado**: `www/umbra_api/helpers/jwt_helper.php` (170 linhas)

**Funções implementadas**:
1. `getJWTFromHeader()` - Extrai token do header Authorization
2. `validateJWT($token)` - Valida e decodifica token
3. `validateJWTRequest()` - Valida token da requisição atual
4. `generateJWT($payload, $hours)` - Gera novo token
5. `getPlayerIdFromJWT()` - Extrai player_id do token
6. `getAccountIdFromJWT()` - Extrai account_id do token
7. `isJWTValid()` - Verifica se token é válido (boolean)

**Chave secreta JWT** (mesma do login):
```php
$secret_key = "umbra_eternum_secret_key_2024_very_secure";
```

**Formato esperado do token**:
```json
{
    "account_id": 4,
    "player_id": 1,
    "username": "jeffo",
    "iat": 1763143231,
    "exp": 1763146831
}
```

---

### Correção 3: Melhorias no `test_inventory.php`

**Melhorias aplicadas**:
1. ✅ Logs detalhados de debug no console do navegador
2. ✅ Verificação de Content-Type antes de parsear JSON
3. ✅ Tratamento de erros HTTP com mensagens claras
4. ✅ Exibição do status HTTP e texto de resposta em caso de erro

**Novos logs no console**:
```javascript
[DEBUG] Requisição: GET api/inventory/get_inventory.php
[DEBUG] Headers: {Authorization: "Bearer eyJ..."}
[DEBUG] Body: null
[DEBUG] Response status: 200
[DEBUG] Data: {success: true, ...}
```

---

### Correção 4: Criação de Teste Direto PHP

**Arquivo criado**: `www/umbra_api/test_api_direct.php`

Este arquivo testa a API diretamente via PHP (sem JavaScript), útil para debug.

---

## 🧪 Como Testar Agora

### Método 1: Interface Visual (Recomendado)

1. **Acesse**:
   ```
   http://localhost/umbra_api/test_inventory.php
   ```

2. **Cole o token JWT** no campo (ou faça login no dashboard primeiro):
   ```
   eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJhY2NvdW50X2lkIjo0LCJwbGF5ZXJfaWQiOjEsInVzZXJuYW1lIjoiamVmZm8iLCJpYXQiOjE3NjMxNDMyMzEsImV4cCI6MTc2MzE0NjgzMX0.2zYxTjRWLckbX4krPJT73M6kT86DPcJ7f-NfoTHj5LI
   ```

3. **Abra o Console do Navegador** (F12 → Console) para ver os logs de debug

4. **Teste os endpoints**:
   - 📋 **Listar Templates** (sem autenticação) - Deve mostrar 17 itens únicos
   - 📦 **Carregar Inventário** - Deve retornar o inventário do player_id 1
   - ➕ **Adicionar Item** - Template ID 7 (Poção de Vida Menor)
   - 🔄 **Mover Item**
   - ⚔️ **Equipar Item**

---

### Método 2: Teste Direto PHP (Debug)

1. **Acesse**:
   ```
   http://localhost/umbra_api/test_api_direct.php
   ```

2. **Veja os resultados**:
   - ✅ Se estiver funcionando: JSON bem formatado com `success: true`
   - ❌ Se houver erro: Mensagem de erro detalhada do PHP

---

### Método 3: cURL Manual (Linha de Comando)

**Listar Templates** (sem autenticação):
```bash
curl http://localhost/umbra_api/api/inventory/get_item_templates.php
```

**Carregar Inventário** (com autenticação):
```bash
curl -H "Authorization: Bearer SEU_TOKEN_AQUI" \
     http://localhost/umbra_api/api/inventory/get_inventory.php
```

**Adicionar Item**:
```bash
curl -X POST \
     -H "Authorization: Bearer SEU_TOKEN_AQUI" \
     -H "Content-Type: application/json" \
     -d '{"item_template_id": 7, "quantity": 3}' \
     http://localhost/umbra_api/api/inventory/add_item.php
```

---

## 📊 Checklist de Testes

### ✅ Testes Básicos (Fazer Primeiro)
- [ ] 1. Listar Templates → Deve retornar **17 itens únicos** (sem duplicatas)
- [ ] 2. Carregar Inventário → Deve retornar inventário do player_id 1
- [ ] 3. Verificar logs no console → Deve mostrar `[DEBUG]` com status 200

### ✅ Testes de Funcionalidade
- [ ] 4. Adicionar Poção de Vida (ID 7) → Deve adicionar ao inventário
- [ ] 5. Carregar inventário novamente → Deve mostrar a poção adicionada
- [ ] 6. Remover item → Deve remover o item
- [ ] 7. Mover item entre slots → Deve trocar de posição
- [ ] 8. Equipar Espada de Ferro (ID 1) → Deve marcar como equipado

### ✅ Testes de Segurança
- [ ] 9. Tentar acessar sem token → Deve retornar erro 401
- [ ] 10. Tentar acessar com token expirado → Deve retornar erro 401
- [ ] 11. Tentar adicionar item inválido → Deve retornar erro de validação

---

## 🐛 Troubleshooting

### Problema: Ainda recebo "Unexpected token '<'"

**Solução**:
1. Abra o Console do navegador (F12)
2. Procure por `[DEBUG] Response text:` no console
3. Copie o texto completo do erro PHP
4. Envie para análise

**Possíveis causas**:
- Erro de sintaxe em algum arquivo PHP
- Caminho incorreto para `vendor/autoload.php`
- Biblioteca JWT não instalada

---

### Problema: Token expirado

Tokens JWT expiram em **1 hora** por padrão.

**Solução**:
1. Faça login novamente no dashboard: `http://localhost/umbra_api/dashboard.html`
2. Copie o novo token
3. Cole no campo de token do `test_inventory.php`

---

### Problema: Erro 404 na API

**Verifique**:
1. Servidor WAMP está rodando?
2. URL está correta? Deve ser: `http://localhost/umbra_api/api/inventory/...`
3. Arquivos PHP estão no diretório correto?

**Estrutura esperada**:
```
C:\wamp64\www\umbra_api\
├── api\
│   └── inventory\
│       ├── get_inventory.php
│       ├── add_item.php
│       ├── remove_item.php
│       ├── move_item.php
│       ├── equip_item.php
│       └── get_item_templates.php
├── helpers\
│   └── jwt_helper.php
├── config\
│   └── database.php
└── test_inventory.php
```

---

## 📝 Logs Esperados (Sucesso)

**Console do Navegador**:
```javascript
[DEBUG] Requisição: GET api/inventory/get_inventory.php
[DEBUG] Headers: {
  Content-Type: "application/json",
  Authorization: "Bearer eyJhbGc..."
}
[DEBUG] Body: null
[DEBUG] Response status: 200
[DEBUG] Data: {
  success: true,
  player: {...},
  inventory: [...],
  total_items: 5
}
```

**Resultado Visual**:
```
📦 Inventário Carregado
15:40:23 ✓ Sucesso

Jogador: NomeDoPersonagem (Level 1)
Total de Itens: 5

┌──────┬─────────────────────┬────────┬──────────┬─────┬──────────────┬──────────┐
│ Slot │ Nome                │ Tipo   │ Raridade │ Qtd │ Durabilidade │ Equipado │
├──────┼─────────────────────┼────────┼──────────┼─────┼──────────────┼──────────┤
│ 0    │ Espada de Ferro     │ weapon │ common   │ 1   │ 100.0%       │ ✓        │
│ 1    │ Poção de Vida Menor │ consum │ common   │ 20  │ 100.0%       │ -        │
└──────┴─────────────────────┴────────┴──────────┴─────┴──────────────┴──────────┘
```

---

## ✅ Status das Correções

| Item | Status | Detalhes |
|------|--------|----------|
| ✅ Duplicatas removidas | **Completo** | 17 itens únicos no banco |
| ✅ jwt_helper.php criado | **Completo** | 170 linhas, 7 funções |
| ✅ test_inventory.php melhorado | **Completo** | Logs de debug adicionados |
| ✅ test_api_direct.php criado | **Completo** | Teste direto via PHP |
| ⏳ Testes dos endpoints | **Pendente** | Aguardando teste do usuário |

---

## 🎯 Próximo Passo

**TESTE AGORA**:
1. Acesse: `http://localhost/umbra_api/test_inventory.php`
2. Abra o Console (F12)
3. Clique em "📋 Listar Templates"
4. Verifique se aparece **17 itens únicos**
5. Cole seu token JWT
6. Clique em "📦 Carregar Inventário"
7. Veja os logs no console

**Se funcionar**: 🎉 Sistema completo e funcional!  
**Se não funcionar**: Envie os logs do console para análise.

---

## 📚 Arquivos Criados/Modificados

1. ✅ `www/umbra_api/helpers/jwt_helper.php` (NOVO - 170 linhas)
2. ✅ `www/umbra_api/test_inventory.php` (MODIFICADO - melhorias)
3. ✅ `www/umbra_api/test_api_direct.php` (NOVO - teste direto)
4. ✅ Banco de dados: `item_templates` (17 itens únicos)

---

**Todas as correções foram aplicadas! O sistema está pronto para testes! 🚀**

