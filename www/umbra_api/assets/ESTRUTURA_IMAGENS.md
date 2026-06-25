# Estrutura de Diretórios para Imagens - UmbraEternum

## 📁 Estrutura Completa

```
www/umbra_api/assets/
├── images/                          # Imagens principais
│   ├── backgrounds/                 # Backgrounds para diferentes seções
│   │   ├── hero-bg.jpg             # Background da hero section (1920x1080+)
│   │   ├── login-bg.jpg            # Background da página de login
│   │   ├── register-bg.jpg         # Background da página de registro
│   │   └── dashboard-bg.jpg        # Background do dashboard
│   ├── logo/                        # Logos e branding
│   │   ├── logo.svg                # Logo principal (já existe)
│   │   ├── logo.png                # Logo em PNG (512x512)
│   │   ├── logo-sm.png             # Logo pequeno (128x128)
│   │   └── favicon.ico             # Favicon do site
│   ├── classes/                     # Imagens das classes
│   │   ├── warrior-portrait.jpg    # Portrait do guerreiro
│   │   ├── mage-portrait.jpg       # Portrait do mago
│   │   ├── rogue-portrait.jpg      # Portrait do ladino
│   │   └── cleric-portrait.jpg     # Portrait do clérigo
│   ├── lore/                        # Imagens da seção de lore
│   │   ├── map.jpg                 # Mapa do mundo
│   │   └── timeline-*.jpg          # Imagens da timeline
│   └── screenshots/                 # Screenshots do jogo
│       ├── screenshot-1.jpg
│       ├── screenshot-2.jpg
│       └── screenshot-3.jpg
├── icons/                           # Ícones SVG (já existem alguns)
│   ├── classes/                     # Ícones de classes
│   │   ├── warrior.svg             # (já existe)
│   │   ├── mage.svg                # (já existe)
│   │   ├── rogue.svg               # (já existe)
│   │   └── cleric.svg              # (já existe)
│   ├── items/                       # Ícones de itens
│   │   ├── weapons/
│   │   ├── armor/
│   │   └── consumables/
│   ├── ui/                          # Ícones de interface
│   │   ├── sword.svg               # (já existe)
│   │   ├── shield.svg              # (já existe)
│   │   └── coin.svg
│   └── social/                      # Ícones sociais customizados
│       ├── discord-custom.svg
│       └── twitch-custom.svg
└── textures/                        # Texturas para backgrounds
    ├── parchment.png               # Textura de pergaminho
    ├── metal.png                   # Textura de metal
    ├── stone.png                   # Textura de pedra
    └── blood-stain.png             # Mancha de sangue (tema dark)
```

## 🎨 Especificações de Imagens por Área

### 1. **Hero Section (Landing Page)**
**Diretório**: `assets/images/backgrounds/hero-bg.jpg`
- **Tamanho**: 1920x1080 ou maior
- **Tema**: Castelo sombrio, floresta nebulosa, batalha épica
- **Estilo**: Dark fantasy, tons escuros com vermelho
- **Formato**: JPG otimizado (< 500KB se possível)

### 2. **Login/Register**
**Diretório**: `assets/images/backgrounds/login-bg.jpg` e `register-bg.jpg`
- **Tamanho**: 1920x1080
- **Tema**: Portão de castelo, ruínas antigas, altar sombrio
- **Estilo**: Atmosfera misteriosa, névoa, vermelho sangue
- **Formato**: JPG otimizado

### 3. **Classes (Cards na Landing)**
**Diretório**: `assets/images/classes/`
- **Arquivos**:
  - `warrior-portrait.jpg` - Guerreiro com armadura pesada
  - `mage-portrait.jpg` - Mago com cajado místico
  - `rogue-portrait.jpg` - Ladino com adagas
  - `cleric-portrait.jpg` - Clérigo com símbolos sagrados
- **Tamanho**: 400x400 (quadrado)
- **Estilo**: Arte de personagem, fundo transparente ou escuro
- **Formato**: JPG ou PNG

### 4. **Dashboard**
**Diretório**: `assets/images/backgrounds/dashboard-bg.jpg`
- **Tamanho**: 1920x1080
- **Tema**: Interior de taverna, salão de guerra, biblioteca arcana
- **Estilo**: Mais sutil que as outras, não pode competir com UI
- **Formato**: JPG com baixa opacidade

### 5. **Lore & História**
**Diretório**: `assets/images/lore/`
- **map.jpg**: Mapa artístico do continente (1200x800)
- **timeline-*.jpg**: Imagens para cada era da história (800x600)
- **Estilo**: Arte conceitual, pergaminho antigo, mapas de fantasia

### 6. **Screenshots do Jogo**
**Diretório**: `assets/images/screenshots/`
- **Quantidade**: 3-6 imagens
- **Tamanho**: 1920x1080 (16:9)
- **Conteúdo**: Combate, exploração, dungeons, cidades
- **Formato**: JPG otimizado

## 📝 Como Adicionar Suas Imagens

### Método 1: Adicionar Diretamente (Recomendado)
```bash
# Copie suas imagens para a estrutura acima
# Exemplo:
d:\UmbraServerV2\www\umbra_api\assets\images\backgrounds\hero-bg.jpg
d:\UmbraServerV2\www\umbra_api\assets\images\classes\warrior-portrait.jpg
```

### Método 2: Usando URLs Externas (Temporário)
Se preferir usar URLs externas temporariamente:
- Unsplash: https://unsplash.com (dark fantasy themed)
- Pexels: https://pexels.com
- Artstation: https://artstation.com (arte profissional)

## 🔧 Otimização de Imagens

**Antes de adicionar**:
1. Redimensione para os tamanhos recomendados
2. Comprima com TinyPNG ou similar (manter < 500KB)
3. Use JPG para fotos, PNG para imagens com transparência
4. SVG para ícones e logos

## 🎨 Fontes de Imagens Gratuitas (Dark Fantasy)

1. **Unsplash** - Palavras-chave:
   - "dark castle"
   - "medieval fantasy"
   - "gothic architecture"
   - "dark forest"

2. **Pexels** - Buscar por:
   - "medieval"
   - "gothic"
   - "dark fantasy"

3. **Pixabay** - Livre de direitos autorais

4. **ArtStation** - Arte profissional (verificar licença)

## 📌 Prioridade de Imagens

**Essencial** (adicionar primeiro):
1. ✅ Logo (já existe em SVG)
2. `hero-bg.jpg` - Background principal
3. `login-bg.jpg` - Background de login
4. 4x `classes/*-portrait.jpg` - Portraits das classes

**Importante** (adicionar depois):
5. `dashboard-bg.jpg`
6. Screenshots do jogo (3 mínimo)
7. Mapa do lore

**Opcional**:
8. Texturas decorativas
9. Imagens da timeline
10. Ícones sociais customizados

## 🖼️ Placeholders Atuais

Enquanto você não adiciona suas imagens, o sistema usa:
- URLs do Unsplash para backgrounds (temporário)
- Ícones SVG que já criei (vermelho agora)
- Cores sólidas como fallback

## ⚠️ Importante

- **NÃO** commite imagens muito grandes (> 1MB) no git
- Use `.gitignore` para assets grandes se necessário
- Considere usar um CDN para imagens em produção
- Mantenha backups dos originais em alta resolução

---

**Depois de adicionar as imagens**, me avise e eu atualizarei todos os arquivos HTML para usar suas imagens personalizadas ao invés dos placeholders!

---

## 🖼️ Galeria de Arte Conceitual (Concepts)

### Estrutura da Galeria

A galeria de arte conceitual usa um sistema dinâmico baseado em JSON para fácil gerenciamento:

```
www/umbra_api/assets/images/concepts/
├── gallery.json                 # Arquivo de configuração da galeria
├── characters/                  # Arte conceitual de personagens
│   ├── barbarian_concept.jpg
│   ├── darkmage_concept.jpg
│   └── ...
├── environments/                # Arte conceitual de ambientes
│   ├── velkaris_forges.jpg
│   ├── grimholt_cemetery.jpg
│   ├── eremos_mist.jpg
│   └── ...
├── creatures/                   # Arte conceitual de criaturas
│   ├── corrupted_beast.jpg
│   ├── portal_horror.jpg
│   └── ...
└── weapons/                     # Arte conceitual de armas
    ├── legendary_sword.jpg
    ├── staff_of_shadows.jpg
    └── ...
```

### Como Adicionar Novas Imagens à Galeria

**📚 Consulte os guias detalhados:**

- **⚡ QUICK_START.md** - Guia rápido em 3 passos (recomendado para começar)
  - Localização: `assets/images/concepts/QUICK_START.md`
  - Perfeito para adicionar imagens rapidamente

- **📖 GUIA_GALLERY_JSON.md** - Documentação completa
  - Localização: `assets/images/concepts/GUIA_GALLERY_JSON.md`
  - Inclui exemplos, troubleshooting, especificações técnicas

### Resumo Rápido

#### Passo 1: Adicionar a Imagem ao Diretório
Coloque sua imagem na categoria apropriada:
```bash
# Exemplo: adicionar arte de personagem
d:\UmbraServerV2\www\umbra_api\assets\images\concepts\characters\warrior_art.jpg
```

#### Passo 2: Editar o gallery.json
Abra o arquivo `assets/images/concepts/gallery.json` e adicione uma entrada na categoria correspondente:

```json
{
  "categories": {
    "characters": {
      "name": "Personagens",
      "description": "Arte conceitual de classes, NPCs e personagens jogáveis",
      "images": [
        {
          "filename": "warrior_art.jpg",
          "title": "Guerreiro - Forjado em Batalha",
          "description": "Arte conceitual da classe Guerreiro no campo de batalha",
          "tags": ["classe", "guerreiro", "combate"]
        }
      ]
    }
  }
}
```

#### Campos do JSON:

- **filename** (obrigatório): Nome do arquivo da imagem
- **title** (obrigatório): Título da arte que aparecerá na galeria
- **description** (opcional): Descrição detalhada da arte
- **tags** (opcional): Array de tags para categorização adicional

### Especificações Recomendadas para Arte Conceitual:

#### **Personagens** (`characters/`)
- **Tamanho**: 1200x1600 ou maior (portrait)
- **Conteúdo**: Classes, NPCs, personagens jogáveis
- **Estilo**: Arte conceitual detalhada, dark fantasy
- **Formato**: JPG otimizado (< 800KB)

#### **Ambientes** (`environments/`)
- **Tamanho**: 1920x1080 ou maior (landscape)
- **Conteúdo**: As três cidades, Fortedras, masmorras, paisagens
- **Estilo**: Environment art, atmosférico
- **Formato**: JPG otimizado (< 1MB)

#### **Criaturas** (`creatures/`)
- **Tamanho**: 1200x1200 (quadrado) ou livre
- **Conteúdo**: Monstros, criaturas corrompidas, bosses
- **Estilo**: Design de criaturas, concept art
- **Formato**: JPG ou PNG se houver transparência

#### **Armas** (`weapons/`)
- **Tamanho**: 800x800 ou maior
- **Conteúdo**: Espadas, cajados, arcos, armas lendárias
- **Estilo**: Item art, detalhamento de equipamentos
- **Formato**: PNG com transparência recomendado

### Exemplo Completo de gallery.json:

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
          "filename": "barbarian_art.jpg",
          "title": "Barbarian - Filha da Ruína",
          "description": "Mulher bárbara sobrevivente das terras de Arraxis",
          "tags": ["classe", "feminino", "guerreiro"]
        },
        {
          "filename": "darkmage_art.jpg",
          "title": "Dark Mage - Herdeira do Véu",
          "description": "Elfa que domina a Neblina Vermelha",
          "tags": ["classe", "feminino", "mago", "élfico"]
        }
      ]
    },
    "environments": {
      "name": "Ambientes",
      "description": "As três cidades, Fortedras e paisagens de Umbra Eternum",
      "images": [
        {
          "filename": "velkaris_forges.jpg",
          "title": "Velkaris - As Forjas de Ferro",
          "description": "Centro industrial onde o metal grita e os homens obedecem",
          "tags": ["cidade", "velkaris", "industrial"]
        },
        {
          "filename": "fortedras_throne.jpg",
          "title": "Fortedras - O Trono das Cinzas",
          "description": "Castelo central de Umbra Eternum, símbolo de poder",
          "tags": ["castelo", "fortedras", "trono"]
        }
      ]
    },
    "creatures": {
      "name": "Criaturas",
      "description": "Monstros, criaturas corrompidas e entidades da Neblina Vermelha",
      "images": [
        {
          "filename": "corrupted_horror.jpg",
          "title": "Horror Corrompido",
          "description": "Criatura nascida da Neblina Vermelha",
          "tags": ["monstro", "neblina", "boss"]
        }
      ]
    },
    "weapons": {
      "name": "Armas",
      "description": "Armamentos, artefatos e equipamentos lendários",
      "images": [
        {
          "filename": "crimson_blade.jpg",
          "title": "Lâmina Carmesim",
          "description": "Espada lendária forjada com Rubicina",
          "tags": ["espada", "lendário", "rubicina"]
        }
      ]
    }
  }
}
```

### Funcionalidades da Galeria:

✅ **Filtros por Categoria**: Clique nos botões para filtrar por tipo de arte
✅ **Lightbox Interativo**: Clique em qualquer imagem para visualização em tela cheia
✅ **Navegação por Teclado**: Use setas ← → para navegar, ESC para fechar
✅ **Lazy Loading**: Imagens carregam conforme você rola a página
✅ **Responsivo**: Funciona em desktop, tablet e mobile
✅ **Fácil Manutenção**: Adicione imagens apenas editando o JSON

### Otimização de Imagens para Galeria:

```bash
# Tamanhos recomendados:
Personagens: 1200x1600 (portrait)
Ambientes: 1920x1080 (landscape)
Criaturas: 1200x1200 (quadrado)
Armas: 800x800 (quadrado)

# Compressão:
- Use TinyPNG ou similar
- Mantenha qualidade de 80-85%
- JPG para arte geral
- PNG para imagens com transparência
```

### Prioridade de Adição:

**1. Essencial** (adicionar primeiro):
- 2-3 portraits de classes principais
- 1-2 ambientes de cidades
- 1 imagem de Fortedras

**2. Importante**:
- Portraits de todas as 6 classes
- Ambientes das 3 cidades
- 2-3 criaturas principais
- 2-3 armas lendárias

**3. Complementar**:
- NPCs importantes
- Masmorras e raids
- Criaturas variadas
- Arsenal completo

---

**A galeria está pronta para receber suas artes!** Basta adicionar as imagens nas pastas e atualizar o `gallery.json`. O sistema cuidará do resto automaticamente!
