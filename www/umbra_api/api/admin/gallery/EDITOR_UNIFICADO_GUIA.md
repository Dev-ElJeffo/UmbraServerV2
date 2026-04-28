# 🎯 Editor Unificado - Guia de Uso

## ✨ Novidade: Editor com Toggle!

O editor agora é **UNIFICADO** e permite alternar entre dois modos de gerenciamento:

### 🎨 Modo 1: **Concepts Gallery**
Gerencia a galeria de arte conceitual (sistema original):
- **Characters** (Personagens)
- **Environments** (Ambientes)
- **Creatures** (Criaturas)
- **Weapons** (Armas)

### 🖼️ Modo 2: **Site Images** (NOVO!)
Gerencia todas as outras imagens do site:
- **Backgrounds** (hero, login, register, dashboard)
- **Classes** (Portraits das 6 classes)
- **Lore** (mapas, timeline)
- **Screenshots** (capturas do jogo)
- **Logos** (branding, favicons)
- **Icons** (UI, classes, social)

---

## 🚀 Como Usar

### 1. Acesse o Editor
```
http://localhost/umbra_api/admin/gallery_editor.html
```

### 2. Faça Login
- Digite seu **username** (ex: `jeffo`)
- O usuário deve ter `isadmin = 1` no banco

### 3. Selecione o Modo
No topo da interface, você verá dois botões grandes:

```
[ 🎨 Concepts Gallery ]  [ 🖼️ Site Images ]
```

**Clique no modo** que deseja gerenciar:
- **Concepts Gallery**: Para arte conceitual da galeria
- **Site Images**: Para imagens do site (backgrounds, classes, etc.)

### 4. Escolha a Categoria
Abaixo do seletor de modo, você verá **tabs** com as categorias do modo selecionado.

**No modo Concepts:**
- Personagens | Ambientes | Criaturas | Armas

**No modo Site Images:**
- Backgrounds | Classes | Lore | Screenshots | Logos | Ícones

**Clique na categoria** que deseja gerenciar.

### 5. Gerencie as Imagens

#### 📤 **Upload de Nova Imagem**
1. Clique em **"Upload Imagem"**
2. Selecione a categoria
3. Escolha o arquivo (JPG, PNG, WebP)
4. Clique em **"Enviar"**

#### ➕ **Adicionar Entrada ao JSON**
1. Clique em **"Adicionar Entrada"**
2. Preencha:
   - **Categoria**
   - **Filename** (nome do arquivo com extensão)
   - **Título**
   - **Descrição** (opcional)
   - **Uso no Site** (só para Site Images) - onde a imagem é usada
   - **Tags** (separadas por vírgula)
3. Clique em **"Salvar"**

#### ✏️ **Editar Entrada**
1. Clique em **"Editar"** no card da imagem
2. Modifique os campos
3. Clique em **"Salvar"**

#### 🗑️ **Deletar Entrada**
1. Clique em **"Deletar"** no card
2. Marque **"Deletar também o arquivo físico"** se quiser remover o arquivo
3. Confirme

#### 🔄 **Escanear Pastas**
1. Clique em **"Escanear Pastas"**
2. O sistema mostra:
   - Imagens disponíveis nas pastas
   - Imagens órfãs (na pasta mas não no JSON)
   - Arquivos faltando (no JSON mas não na pasta)

---

## 🎨 Diferenças Entre os Modos

### Concepts Gallery (Original)
- **JSON**: `assets/images/concepts/gallery.json`
- **Estrutura**: Categorias simples (4 categorias)
- **Campos**: filename, title, description, tags
- **APIs**: `get_gallery.php`, `add_entry.php`, `update_entry.php`, `delete_entry.php`

### Site Images (Novo)
- **JSON**: `assets/images/site_images.json`
- **Estrutura**: Categorias + subcategorias (7 categorias, icons tem 3 subcategorias)
- **Campos**: filename, title, description, **usage**, tags
- **APIs**: `get_site_images.php`, `manage_site_image.php` (unified CRUD)
- **Campo Extra**: **"Uso no Site"** - indica onde a imagem é utilizada

---

## 📋 Exemplos Práticos

### Exemplo 1: Adicionar Background do Hero
1. Selecione modo: **Site Images**
2. Selecione categoria: **Backgrounds**
3. Clique em **"Upload Imagem"** e envie `hero-bg.jpg`
4. Clique em **"Adicionar Entrada"**
5. Preencha:
   - Filename: `hero-bg.jpg`
   - Título: `Hero Section Background`
   - Descrição: `Fundo principal da landing page`
   - Uso: `index.html - Hero Section`
   - Tags: `background, hero, landing`

### Exemplo 2: Adicionar Portrait de Classe
1. Selecione modo: **Site Images**
2. Selecione categoria: **Classes**
3. Upload da imagem `barbarian.jpg`
4. Adicionar entrada:
   - Filename: `barbarian.jpg`
   - Título: `Barbarian - Filha da Ruína`
   - Descrição: `Portrait da classe Barbarian`
   - Uso: `index.html - Classes Section - Barbarian Card`
   - Tags: `classe, barbarian, feminino`

### Exemplo 3: Adicionar Arte Conceitual
1. Selecione modo: **Concepts Gallery**
2. Selecione categoria: **Characters**
3. Upload da imagem `selindra_portrait.jpg`
4. Adicionar entrada:
   - Filename: `selindra_portrait.jpg`
   - Título: `Lady Selindra Von'Mahr`
   - Descrição: `Arquinecromante de Grimholt`
   - Tags: `npc, grimholt, necromancia, feminino`

---

## 🔍 Indicadores Visuais

### Status do Arquivo
Cada card mostra um ícone no canto:
- ✅ **Check verde**: Arquivo físico existe
- ❌ **X vermelho**: Arquivo não encontrado

### Campo "Uso no Site" (Site Images)
Aparece abaixo da descrição nos cards de Site Images, mostrando onde a imagem é utilizada.

---

## ⚠️ Dicas Importantes

1. **Toggle de Modo**: Ao trocar de modo, as categorias mudam automaticamente
2. **Campo Usage**: Só aparece no modo "Site Images"
3. **Categorias Dinâmicas**: Cada modo tem suas próprias categorias
4. **Backups Automáticos**: Ambos os sistemas fazem backup antes de modificações
5. **Sincronização**: Alterações aparecem imediatamente após salvar

---

## 📊 Estatísticas

O painel de estatísticas mostra:
- **Total no JSON**: Imagens cadastradas
- **Arquivos Disponíveis**: Imagens físicas nas pastas
- **Imagens Órfãs**: Na pasta mas não no JSON
- **Arquivos Faltando**: No JSON mas não na pasta

As estatísticas são **específicas do modo atual**.

---

## 🎯 Atalhos

- **F5**: Recarregar página
- **Ctrl + Shift + R**: Recarregar forçado (limpa cache)
- **Esc**: Fechar modal (navegadores modernos)

---

## 📝 Estrutura de Pastas por Modo

### Concepts Gallery
```
assets/images/concepts/
├── characters/
├── environments/
├── creatures/
└── weapons/
```

### Site Images
```
assets/images/
├── backgrounds/
├── classes/
├── lore/
├── screenshots/
├── logo/
└── icons/
    ├── classes/
    ├── ui/
    └── social/
```

---

**Versão**: 2.0 (Editor Unificado)  
**Data**: 28/04/2026  
**Projeto**: UmbraEternum
