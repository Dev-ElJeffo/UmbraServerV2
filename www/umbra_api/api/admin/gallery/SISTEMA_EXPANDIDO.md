# Sistema Expandido de Gerenciamento de Imagens

## 🎯 Visão Geral

O sistema foi **expandido** para gerenciar **TODAS as imagens do site**, não apenas a galeria de concepts!

## 📂 Duas Coleções de Imagens

### 1. **Concepts Gallery** (Sistema Original)
- **Arquivo JSON**: `assets/images/concepts/gallery.json`
- **Editor**: `admin/gallery_editor.html`
- **Categorias**: 
  - Characters (Personagens)
  - Environments (Ambientes)
  - Creatures (Criaturas)
  - Weapons (Armas)
- **Uso**: Galeria de arte conceitual na seção "Concepts" do site

### 2. **Site Images** (Sistema Novo) ✨
- **Arquivo JSON**: `assets/images/site_images.json`
- **APIs**: Novas APIs em `api/admin/gallery/`
- **Categorias**:
  - **Backgrounds**: Fundos de páginas (hero, login, register, dashboard)
  - **Classes**: Portraits das 6 classes (barbarian, darkmage, marcial, templar, alchemist, assassin)
  - **Lore**: Mapas, timeline, imagens de história
  - **Screenshots**: Capturas de tela do jogo
  - **Logos**: Branding, favicons
  - **Icons**: Ícones de UI (com subcategorias: classes, ui, social)

## 🚀 Como Usar

### Para Concepts (Arte Conceitual)
1. Acesse: `http://localhost/umbra_api/admin/gallery_editor.html`
2. Faça login
3. Use normalmente (já funciona)

### Para Site Images (Outras Imagens) ✨

**Opção 1**: Usar APIs diretamente via Postman/Insomnia

**Opção 2**: Criar entrada manual no JSON

**Opção 3**: Esperar editor visual (próximo passo)

## 📁 Estrutura de Diretórios Criada

```
www/umbra_api/assets/images/
├── concepts/               ✅ Já gerenciado pelo editor
│   ├── characters/
│   ├── environments/
│   ├── creatures/
│   └── weapons/
│
├── backgrounds/            ✅ NOVO - Criado
│   ├── hero-bg.jpg
│   ├── login-bg.jpg
│   ├── register-bg.jpg
│   └── dashboard-bg.jpg
│
├── classes/                ✅ NOVO - Criado
│   ├── barbarian.jpg
│   ├── darkmage.jpg
│   ├── marcial.jpg
│   ├── templar.jpg
│   ├── alchemist.jpg
│   └── assassin.jpg
│
├── lore/                   ✅ NOVO - Criado
│   ├── map.jpg
│   └── timeline-*.jpg
│
├── screenshots/            ✅ NOVO - Criado
│   ├── screenshot-1.jpg
│   └── screenshot-*.jpg
│
├── logo/                   ✅ NOVO - Criado
│   ├── logo.svg
│   ├── logo.png
│   └── favicon.ico
│
└── icons/                  ✅ NOVO - Criado
    ├── classes/
    ├── ui/
    └── social/
```

## 🔧 APIs Disponíveis

### Para Site Images:

1. **`scan_site_images.php`**
   - Escaneia todas as pastas de site images
   - Retorna imagens disponíveis, órfãs e faltando
   - Estatísticas completas

2. **`get_site_images.php`**
   - Retorna conteúdo do `site_images.json`
   - Adiciona info de preview e file exists

3. **`manage_site_image.php`**
   - **Ação: add** - Adiciona nova entrada
   - **Ação: update** - Atualiza entrada existente
   - **Ação: delete** - Remove entrada (com opção de deletar arquivo)

4. **`upload_image.php`** (já existe)
   - Pode ser usado para upload em qualquer categoria
   - Adaptável para site images

## 📝 Exemplo de Uso das APIs

### Escanear Site Images

```javascript
fetch('http://localhost/umbra_api/api/admin/gallery/scan_site_images.php', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ username: 'jeffo' })
})
.then(r => r.json())
.then(data => console.log(data));
```

### Adicionar Imagem de Background

```javascript
fetch('http://localhost/umbra_api/api/admin/gallery/manage_site_image.php', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({
        username: 'jeffo',
        action: 'add',
        category: 'backgrounds',
        filename: 'hero-bg.jpg',
        title: 'Hero Section Background',
        description: 'Fundo principal da landing page',
        usage: 'index.html - Hero Section',
        tags: ['background', 'hero', 'landing']
    })
})
.then(r => r.json())
.then(data => console.log(data));
```

### Adicionar Portrait de Classe

```javascript
fetch('http://localhost/umbra_api/api/admin/gallery/manage_site_image.php', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({
        username: 'jeffo',
        action: 'add',
        category: 'classes',
        filename: 'barbarian.jpg',
        title: 'Barbarian - Filha da Ruína',
        description: 'Portrait da classe Barbarian para seção de classes',
        usage: 'index.html - Classes Section',
        tags: ['classe', 'barbarian', 'feminino']
    })
})
.then(r => r.json())
.then(data => console.log(data));
```

## 🎨 Especificações Recomendadas

### Backgrounds
- **Tamanho**: 1920x1080 ou maior
- **Formato**: JPG otimizado
- **Tamanho Máximo**: 500KB

### Classes
- **Tamanho**: 800x800 (quadrado)
- **Formato**: JPG ou PNG
- **Tamanho Máximo**: 300KB

### Screenshots
- **Tamanho**: 1920x1080 (16:9)
- **Formato**: JPG
- **Tamanho Máximo**: 400KB

### Logos
- **Logo Principal**: 512x512 (SVG ou PNG)
- **Logo Pequeno**: 128x128
- **Favicon**: 32x32 (ICO)

### Icons
- **Formatos**: SVG (preferencial), PNG
- **Tamanhos**: 24x24, 32x32, 64x64

## 🔄 Próximos Passos

### Opção A: Editor Visual Unificado
Criar um editor visual que alterna entre "Concepts" e "Site Images" com um toggle no topo.

### Opção B: Editor Separado
Criar `site_images_editor.html` específico para gerenciar imagens do site.

### Opção C: Integração Direta
Usar as APIs via JavaScript no próprio site.

## 📋 Checklist de Implementação

- ✅ Criado `site_images.json` com estrutura completa
- ✅ Criados todos os diretórios necessários
- ✅ Criado `site_images_helper.php` com funções
- ✅ Criado `scan_site_images.php`
- ✅ Criado `get_site_images.php`
- ✅ Criado `manage_site_image.php` (add/update/delete)
- ✅ Sistema de backup integrado
- ⏳ Editor visual (aguardando decisão)

## 🎯 Como Usar Agora

### Método 1: Manualmente no JSON

Edite `assets/images/site_images.json` diretamente:

```json
{
  "categories": {
    "backgrounds": {
      "images": [
        {
          "filename": "hero-bg.jpg",
          "title": "Hero Background",
          "description": "Fundo da hero section",
          "usage": "index.html",
          "tags": ["background", "hero"]
        }
      ]
    }
  }
}
```

### Método 2: Via APIs (Postman/Insomnia)

Use os exemplos de fetch acima.

### Método 3: Console do Navegador

Abra o console (F12) em qualquer página do admin e execute os comandos fetch.

---

**Status**: Sistema backend completo ✅  
**Próximo**: Editor visual (aguardando confirmação do usuário)

**Criado em**: 28/04/2026
