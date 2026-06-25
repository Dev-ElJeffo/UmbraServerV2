# Troubleshooting - Editor de Galeria

## ❌ Erro: "Unexpected token '<', "<br /> <fo"... is not valid JSON"

### Causa
Este erro aparecia quando as APIs PHP tinham caminhos incorretos para o arquivo `database.php` e tentavam instanciar uma classe inexistente.

### ✅ Solução Aplicada
- Corrigido caminho relativo em todas as APIs: `/../../../config/database.php`
- Substituído `new Database()` por `getConnection()` (função correta do database.php)
- Aplicado em todos os 6 arquivos PHP da API

## ⚠️ IMPORTANTE: Como Fazer Login

### ❌ ERRADO
```
Campo: eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJhY2NvdW50X2lkIjo0...
```
**Não use o token JWT no campo de login!**

### ✅ CORRETO
```
Campo: jeffo
```
**Use apenas o nome de usuário (username)!**

## Como Usar o Editor

1. **Abra**: `http://localhost/umbra_api/admin/gallery_editor.html`

2. **Login**: Digite apenas seu **username** (exemplo: `jeffo`)
   - Não cole o token JWT
   - Não use o email
   - Apenas o username simples

3. **Requisitos**:
   - O usuário deve existir na tabela `accounts`
   - O campo `isadmin` deve ser `1`
   - O usuário não pode estar banido (`banned = 0`)

## Verificar se seu Usuário é Admin

Execute no MySQL:

```sql
SELECT username, isadmin, banned FROM accounts WHERE username = 'jeffo';
```

Resultado esperado:
```
username | isadmin | banned
---------|---------|-------
jeffo    | 1       | 0
```

Se `isadmin` não for `1`, execute:

```sql
UPDATE accounts SET isadmin = 1 WHERE username = 'jeffo';
```

## Outros Erros Comuns

### Erro 404 na API
**Causa**: Caminho incorreto ou arquivo PHP não encontrado

**Solução**: Verifique se os arquivos existem em:
- `www/umbra_api/api/admin/gallery/get_gallery.php`
- `www/umbra_api/api/admin/gallery/scan_images.php`
- etc.

### Erro de Conexão com Banco
**Causa**: Credenciais incorretas em `config/database.php`

**Solução**: Verifique:
```php
define('DB_HOST', 'localhost');
define('DB_NAME', 'umbra_eternum');
define('DB_USER', 'root');
define('DB_PASS', 'sua_senha');
```

### Upload Falha
**Causas possíveis**:
- Arquivo maior que 2MB
- Formato não suportado (use JPG, PNG ou WebP)
- Permissões da pasta

**Solução**:
1. Verifique tamanho do arquivo
2. Converta para JPG se necessário
3. Verifique permissões: `chmod 755 assets/images/concepts/`

## Status das Correções

✅ **Corrigido** (28/04/2026 11:30):
- Caminhos de database.php em todas as APIs
- Substituição de `new Database()` por `getConnection()`
- Interface atualizada com instruções claras
- Placeholder no campo de login

## Teste Rápido

Para testar se está funcionando:

1. Abra o Editor de Galeria
2. Digite seu username (exemplo: `jeffo`)
3. Clique em "Entrar"
4. Deve aparecer a interface principal com estatísticas

Se ainda houver erro:
1. Abra o Console do navegador (F12)
2. Vá para a aba "Network"
3. Tente fazer login novamente
4. Clique na requisição `get_gallery.php`
5. Verifique a resposta (Response tab)

---

**Última atualização**: 28/04/2026
