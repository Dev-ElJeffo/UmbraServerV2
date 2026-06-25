# Editor de Galeria - Umbra Eternum

Sistema administrativo completo para gerenciar o `gallery.json` da galeria de arte conceitual.

## 📍 Localização

**Interface:** `www/umbra_api/admin/gallery_editor.html`

**APIs Backend:** `www/umbra_api/api/admin/gallery/`

## 🚀 Como Usar

### 1. Acesso

1. Abra `http://localhost/umbra_api/admin/gallery_editor.html` no navegador
2. Faça login com um usuário que tenha `isadmin = 1` no banco de dados
3. O sistema validará suas credenciais e carregará a interface

### 2. Funcionalidades

#### Upload de Imagens
- Clique em **"Upload Imagem"**
- Selecione a categoria (Personagens, Ambientes, Criaturas, Armas)
- Escolha o arquivo (JPG, PNG ou WebP, máximo 2MB)
- Clique em **"Enviar"**
- A imagem será salva na pasta correspondente

#### Adicionar Entrada ao JSON
- Clique em **"Adicionar Entrada"**
- Preencha os campos:
  - **Categoria**: onde a imagem está
  - **Filename**: nome exato do arquivo (com extensão)
  - **Título**: título da arte
  - **Descrição**: descrição detalhada (opcional)
  - **Tags**: palavras-chave separadas por vírgula (opcional)
- Clique em **"Salvar"**

#### Editar Entrada
- Clique em **"Editar"** no card da imagem
- Modifique os campos desejados (filename não pode ser alterado)
- Clique em **"Salvar"**

#### Deletar Entrada
- Clique em **"Deletar"** no card da imagem
- Marque a opção **"Deletar também o arquivo físico"** se quiser remover a imagem da pasta
- Confirme a exclusão

#### Escanear Pastas
- Clique em **"Escanear Pastas"**
- O sistema verificará:
  - Imagens disponíveis nas pastas
  - Imagens órfãs (na pasta mas não no JSON)
  - Arquivos faltando (no JSON mas não na pasta)
- As estatísticas serão atualizadas

#### Recarregar
- Clique em **"Recarregar"** para atualizar os dados do servidor

## 📂 Estrutura de Arquivos

```
www/umbra_api/
├── admin/
│   └── gallery_editor.html          # Interface principal
├── api/admin/gallery/
│   ├── gallery_helper.php           # Funções utilitárias
│   ├── scan_images.php              # Escaneia pastas
│   ├── get_gallery.php              # Retorna JSON atual
│   ├── add_entry.php                # Adiciona entrada
│   ├── update_entry.php             # Edita entrada
│   ├── delete_entry.php             # Remove entrada
│   └── upload_image.php             # Upload de imagens
└── assets/images/concepts/
    ├── gallery.json                 # Arquivo de configuração
    ├── backups/                     # Backups automáticos
    ├── characters/                  # Imagens de personagens
    ├── environments/                # Imagens de ambientes
    ├── creatures/                   # Imagens de criaturas
    └── weapons/                     # Imagens de armas
```

## 🔒 Segurança

- Todas as APIs validam se o usuário é admin (`isadmin = 1`)
- Upload valida tipo MIME real (não só extensão)
- Nomes de arquivo são sanitizados automaticamente
- Backup automático antes de qualquer modificação
- Path traversal protection
- Tamanho máximo de upload: 2MB

## 🔄 Backups

- Backup automático antes de cada modificação
- Backups salvos em `assets/images/concepts/backups/`
- Formato: `gallery_backup_YYYY-MM-DD_HH-MM-SS.json`
- Sistema mantém automaticamente os 10 backups mais recentes

## 📊 Estatísticas

O painel mostra:
- **Total no JSON**: Quantidade de imagens cadastradas
- **Arquivos Disponíveis**: Imagens físicas nas pastas
- **Imagens Órfãs**: Na pasta mas não no JSON
- **Arquivos Faltando**: No JSON mas não na pasta

## ⚠️ Indicadores Visuais

Cada card de imagem mostra:
- ✅ **Check verde**: Arquivo físico existe
- ❌ **X vermelho**: Arquivo físico não encontrado
- **Ícone de alerta**: Imagem não carregou

## 🎨 Interface

- **Dark Fantasy Theme**: Estilo consistente com o resto do site
- **Cores**: Paleta vermelha (ao invés de dourado)
- **Responsivo**: Funciona em desktop e mobile
- **Modais**: Para upload, adicionar/editar e confirmar exclusão
- **Toasts**: Notificações de sucesso/erro

## 📝 Formato do gallery.json

```json
{
  "version": "1.0",
  "lastUpdated": "2026-04-27",
  "categories": {
    "characters": {
      "name": "Personagens",
      "description": "Arte conceitual de classes, NPCs e personagens jogáveis",
      "images": [
        {
          "filename": "barbarian_01.jpg",
          "title": "Barbarian - Filha da Ruína",
          "description": "Guerreira bárbara...",
          "tags": ["classe", "feminino", "barbarian"]
        }
      ]
    }
  }
}
```

## 🛠️ Troubleshooting

### Erro ao fazer upload
- Verifique se o arquivo é JPG, PNG ou WebP
- Tamanho máximo: 2MB
- Verifique permissões da pasta de destino

### Entrada não aparece após adicionar
- Verifique se o filename está correto (incluindo extensão)
- Use "Escanear Pastas" para ver arquivos disponíveis
- Clique em "Recarregar" para atualizar a visualização

### Imagem com X vermelho
- O arquivo físico não existe na pasta
- Verifique o nome exato do arquivo
- Use "Escanear Pastas" para ver o que está disponível

### Erro de permissão
- Verifique se o usuário tem `isadmin = 1` no banco
- Confirme que está usando um usuário válido

## 🔗 Integração

As alterações no `gallery.json` refletem automaticamente na galeria pública do site (`index.html` → seção Concepts).

## 📚 Documentação Relacionada

- `GUIA_GALLERY_JSON.md` - Documentação completa do gallery.json
- `QUICK_START.md` - Guia rápido para adicionar imagens manualmente
- `CHEAT_SHEET.md` - Referência rápida

---

**Versão:** 1.0  
**Data:** 27/04/2026  
**Projeto:** UmbraEternum - Sistema de Galeria
