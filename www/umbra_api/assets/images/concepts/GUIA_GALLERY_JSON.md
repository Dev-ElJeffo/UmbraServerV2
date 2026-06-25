# Guia de Uso - gallery.json
## Sistema de Galeria de Arte Conceitual do Umbra Eternum

---

## 📋 Índice

1. [Visão Geral](#visao-geral)
2. [Estrutura do Arquivo](#estrutura-do-arquivo)
3. [Como Adicionar Imagens](#como-adicionar-imagens)
4. [Categorias Disponíveis](#categorias-disponiveis)
5. [Exemplos Práticos](#exemplos-praticos)
6. [Especificações de Imagens](#especificacoes-de-imagens)
7. [Solução de Problemas](#solucao-de-problemas)

---

## 🎯 Visão Geral

O arquivo `gallery.json` controla toda a galeria de arte conceitual do site. Ele funciona como um índice que lista todas as imagens disponíveis, suas descrições e categorias.

**Localização:** `d:\UmbraServerV2\www\umbra_api\assets\images\concepts\gallery.json`

---

## 📁 Estrutura do Arquivo

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
          "filename": "nome-da-imagem.jpg",
          "title": "Título da Arte",
          "description": "Descrição detalhada",
          "tags": ["tag1", "tag2", "tag3"]
        }
      ]
    }
  }
}
```

### Campos Principais:

| Campo | Tipo | Obrigatório | Descrição |
|-------|------|-------------|-----------|
| `filename` | string | ✅ Sim | Nome exato do arquivo da imagem |
| `title` | string | ✅ Sim | Título que aparece na galeria |
| `description` | string | ❌ Não | Texto descritivo (recomendado) |
| `tags` | array | ❌ Não | Lista de palavras-chave |

---

## 🎨 Como Adicionar Imagens

### Passo 1: Escolher a Categoria

Escolha em qual categoria sua imagem se encaixa:

- **characters** - Personagens, classes, NPCs
- **environments** - Cidades, paisagens, locais
- **creatures** - Monstros, criaturas corrompidas
- **weapons** - Armas, equipamentos

### Passo 2: Adicionar o Arquivo da Imagem

Coloque sua imagem na pasta correspondente:

```
assets/images/concepts/
├── characters/       ← Para personagens
├── environments/     ← Para ambientes
├── creatures/        ← Para criaturas
└── weapons/          ← Para armas
```

**Exemplo:**
```
assets/images/concepts/characters/barbarian_arte.jpg
```

### Passo 3: Editar o gallery.json

Abra o arquivo e adicione uma entrada na categoria apropriada:

```json
{
  "categories": {
    "characters": {
      "name": "Personagens",
      "description": "Arte conceitual de classes, NPCs e personagens jogáveis",
      "images": [
        {
          "filename": "barbarian_arte.jpg",
          "title": "Barbarian - Filha da Ruína",
          "description": "Arte conceitual da classe Barbarian em combate",
          "tags": ["classe", "feminino", "guerreiro", "barbarian"]
        }
      ]
    }
  }
}
```

### Passo 4: Salvar e Testar

1. Salve o arquivo `gallery.json`
2. Atualize a página do site (`F5`)
3. Navegue até a seção "Concepts"
4. Sua imagem deve aparecer!

---

## 📂 Categorias Disponíveis

### 1. **Characters** (Personagens)

**Para:** Classes, NPCs, personagens jogáveis

**Exemplos de nomes:**
- `barbarian_concept.jpg`
- `darkmage_portrait.jpg`
- `selindra_npc.jpg`

**Tags sugeridas:**
- Classe: `classe`, `barbarian`, `darkmage`, `marcial`, etc.
- Gênero: `feminino`, `masculino`, `elfico`
- Tipo: `guerreiro`, `mago`, `suporte`, `npc`

---

### 2. **Environments** (Ambientes)

**Para:** Cidades, paisagens, masmorras, locais

**Exemplos de nomes:**
- `velkaris_forges.jpg`
- `grimholt_cemetery.jpg`
- `fortedras_throne.jpg`

**Tags sugeridas:**
- Cidade: `velkaris`, `grimholt`, `eremos`, `fortedras`
- Tipo: `cidade`, `masmorra`, `paisagem`, `castelo`
- Atmosfera: `industrial`, `sombrio`, `nevoa`

---

### 3. **Creatures** (Criaturas)

**Para:** Monstros, criaturas corrompidas, bosses

**Exemplos de nomes:**
- `corrupted_beast.jpg`
- `portal_horror.jpg`
- `boss_behemoth.jpg`

**Tags sugeridas:**
- Tipo: `monstro`, `boss`, `corrupto`
- Origem: `neblina`, `portal`, `dimensional`
- Perigo: `elite`, `comum`, `lendario`

---

### 4. **Weapons** (Armas)

**Para:** Espadas, cajados, arcos, equipamentos

**Exemplos de nomes:**
- `crimson_blade.jpg`
- `staff_shadows.jpg`
- `legendary_axe.jpg`

**Tags sugeridas:**
- Tipo: `espada`, `cajado`, `arco`, `machado`
- Raridade: `comum`, `raro`, `lendario`
- Material: `rubicina`, `aço`, `obsidita`

---

## 📝 Exemplos Práticos

### Exemplo 1: Adicionar Arte de Personagem

**Arquivo:** `assets/images/concepts/characters/selindra_portrait.jpg`

```json
{
  "categories": {
    "characters": {
      "images": [
        {
          "filename": "selindra_portrait.jpg",
          "title": "Lady Selindra Von'Mahr",
          "description": "Arquinecromante de Grimholt, bela e terrível. Dizem que perdeu o próprio coração décadas atrás.",
          "tags": ["npc", "grimholt", "necromancia", "feminino"]
        }
      ]
    }
  }
}
```

---

### Exemplo 2: Adicionar Arte de Ambiente

**Arquivo:** `assets/images/concepts/environments/velkaris_night.jpg`

```json
{
  "categories": {
    "environments": {
      "images": [
        {
          "filename": "velkaris_night.jpg",
          "title": "Velkaris à Noite",
          "description": "As forjas nunca descansam. A cidade brilha em vermelho sob o céu carmesim.",
          "tags": ["velkaris", "cidade", "noite", "industrial", "forjas"]
        }
      ]
    }
  }
}
```

---

### Exemplo 3: Adicionar Múltiplas Imagens

```json
{
  "categories": {
    "characters": {
      "images": [
        {
          "filename": "barbarian_01.jpg",
          "title": "Barbarian em Combate",
          "description": "Filha da Ruína enfrentando hordas",
          "tags": ["classe", "barbarian", "combate"]
        },
        {
          "filename": "barbarian_02.jpg",
          "title": "Barbarian - Portrait",
          "description": "Retrato de guerreira bárbara",
          "tags": ["classe", "barbarian", "portrait"]
        },
        {
          "filename": "darkmage_01.jpg",
          "title": "Dark Mage - Ritual",
          "description": "Herdeira do Véu manipulando a Neblina",
          "tags": ["classe", "darkmage", "magia", "elfico"]
        }
      ]
    },
    "weapons": {
      "images": [
        {
          "filename": "crimson_sword.jpg",
          "title": "Lâmina Carmesim",
          "description": "Espada forjada com Rubicina pura",
          "tags": ["espada", "lendario", "rubicina"]
        }
      ]
    }
  }
}
```

---

## 🖼️ Especificações de Imagens

### Formatos Aceitos
- ✅ JPG / JPEG (recomendado)
- ✅ PNG (para transparências)
- ✅ WebP (mais leve)
- ❌ GIF (não recomendado)

### Tamanhos Recomendados

| Categoria | Tamanho Ideal | Orientação |
|-----------|---------------|------------|
| **Personagens** | 1200x1600px | Portrait (vertical) |
| **Ambientes** | 1920x1080px | Landscape (horizontal) |
| **Criaturas** | 1200x1200px | Quadrado |
| **Armas** | 800x800px | Quadrado |

### Otimização

**Tamanho máximo por imagem:** 1MB (recomendado)

**Como otimizar:**
1. Use [TinyPNG](https://tinypng.com) para comprimir
2. Redimensione para os tamanhos recomendados
3. Qualidade JPG: 80-85%
4. Remova metadados desnecessários

---

## ⚠️ Regras Importantes

### ✅ Fazer:

1. **Use nomes descritivos** para os arquivos
   - ✅ `barbarian_combat_scene.jpg`
   - ❌ `IMG_12345.jpg`

2. **Sem espaços nos nomes**
   - ✅ `dark_mage_portrait.jpg`
   - ❌ `dark mage portrait.jpg`

3. **Use caracteres simples**
   - ✅ Letras, números, underscore `_`, hífen `-`
   - ❌ Acentos, símbolos especiais

4. **Mantenha consistência**
   - Todas as imagens de uma categoria devem ter estilo similar

5. **Adicione descrições**
   - Ajuda os usuários a entenderem o contexto

### ❌ Evitar:

1. ❌ Arquivos muito grandes (> 2MB)
2. ❌ Imagens com baixa resolução (< 500px)
3. ❌ Nomes duplicados
4. ❌ Esquecer de salvar o `gallery.json`
5. ❌ Adicionar vírgula após o último item da lista

---

## 🔧 Solução de Problemas

### Problema 1: Imagem não aparece

**Possíveis causas:**

1. **Nome do arquivo errado no JSON**
   - ✅ Verifique se o `filename` no JSON corresponde exatamente ao nome do arquivo
   - Exemplo: `barbarian.jpg` ≠ `Barbarian.JPG` (maiúsculas/minúsculas)

2. **Imagem na pasta errada**
   - ✅ Confirme que a imagem está na pasta correta da categoria

3. **Erro de sintaxe no JSON**
   - ✅ Use um validador JSON online: [JSONLint](https://jsonlint.com)
   - ✅ Verifique vírgulas, aspas e chaves

### Problema 2: Galeria vazia

**Solução:**

1. Abra o console do navegador (F12)
2. Vá para a aba "Console"
3. Procure por erros em vermelho
4. Corrija o erro indicado no `gallery.json`

### Problema 3: Imagem quebrada (ícone de X)

**Causas:**

1. Caminho do arquivo incorreto
2. Imagem corrompida
3. Formato não suportado

**Solução:**

1. Verifique se a imagem abre normalmente no explorador de arquivos
2. Re-exporte a imagem em JPG
3. Verifique o nome exato do arquivo

---

## 📋 Checklist Rápido

Antes de adicionar uma nova imagem:

- [ ] A imagem está na pasta correta? (`characters/`, `environments/`, etc.)
- [ ] O nome do arquivo não tem espaços ou caracteres especiais?
- [ ] O arquivo tem menos de 1MB?
- [ ] A resolução é adequada (mínimo 800px)?
- [ ] Adicionei a entrada no `gallery.json` na categoria correta?
- [ ] O `filename` no JSON está exatamente igual ao nome do arquivo?
- [ ] Adicionei um `title` descritivo?
- [ ] Adicionei uma `description` (recomendado)?
- [ ] Adicionei `tags` relevantes?
- [ ] Não tem vírgula extra após o último item da lista?
- [ ] O JSON está válido? (testar em [JSONLint](https://jsonlint.com))
- [ ] Salvei o arquivo?

---

## 🚀 Exemplo Completo

**Estrutura de arquivos:**
```
assets/images/concepts/
├── gallery.json
├── characters/
│   ├── barbarian_01.jpg
│   └── darkmage_01.jpg
└── environments/
    └── velkaris_night.jpg
```

**Conteúdo do gallery.json:**
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
          "description": "Guerreira bárbara moldada pela perda absoluta, endurecida pela Neblina Vermelha",
          "tags": ["classe", "feminino", "barbarian", "guerreiro"]
        },
        {
          "filename": "darkmage_01.jpg",
          "title": "Dark Mage - Herdeira do Véu",
          "description": "Elfa que aceitou a corrupção como linguagem, negociando com o que restou entre os mundos",
          "tags": ["classe", "feminino", "darkmage", "elfico", "magia"]
        }
      ]
    },
    "environments": {
      "name": "Ambientes",
      "description": "As três cidades, Fortedras e paisagens de Umbra Eternum",
      "images": [
        {
          "filename": "velkaris_night.jpg",
          "title": "Velkaris - Forjas Noturnas",
          "description": "As forjas que nunca descansam, iluminando a cidade com fogo carmesim",
          "tags": ["velkaris", "cidade", "noite", "forjas", "industrial"]
        }
      ]
    },
    "creatures": {
      "name": "Criaturas",
      "description": "Monstros, criaturas corrompidas e entidades da Neblina Vermelha",
      "images": []
    },
    "weapons": {
      "name": "Armas",
      "description": "Armamentos, artefatos e equipamentos lendários",
      "images": []
    }
  },
  "instructions": {
    "howToAdd": "Para adicionar novas imagens à galeria:",
    "steps": [
      "1. Adicione a imagem na pasta correspondente (characters/, environments/, creatures/ ou weapons/)",
      "2. Adicione uma entrada no array 'images' da categoria com os campos: filename, title, description e tags (opcional)",
      "3. Exemplo: { \"filename\": \"barbarian_concept.jpg\", \"title\": \"Barbarian - Filha da Ruína\", \"description\": \"Arte conceitual da classe Barbarian\", \"tags\": [\"classe\", \"feminino\", \"guerreiro\"] }"
    ]
  }
}
```

---

## 💡 Dicas Extras

### Organização de Arquivos

Use um padrão de nomenclatura consistente:

```
[categoria]_[nome]_[variacao].jpg

Exemplos:
- character_barbarian_01.jpg
- character_barbarian_02.jpg
- environment_velkaris_day.jpg
- environment_velkaris_night.jpg
- creature_behemoth_boss.jpg
- weapon_sword_legendary.jpg
```

### Tags Úteis

**Gerais:**
- `concept`, `final`, `sketch`, `wip`

**Classes:**
- `barbarian`, `darkmage`, `marcial`, `templar`, `alchemist`, `assassin`

**Cidades:**
- `velkaris`, `grimholt`, `eremos`, `fortedras`

**Facções:**
- `novarra`, `eldros`

**Estilo:**
- `portrait`, `action`, `landscape`, `closeup`

---

## 📞 Suporte

Se encontrar problemas:

1. Valide o JSON em: https://jsonlint.com
2. Verifique o console do navegador (F12)
3. Confirme que os arquivos de imagem existem nas pastas corretas
4. Teste com uma imagem simples primeiro

---

**Última atualização:** 27/04/2026  
**Versão do guia:** 1.0

---

## 🎯 Resumo Rápido

1. Coloque a imagem em `assets/images/concepts/[categoria]/`
2. Edite `gallery.json` e adicione:
```json
{
  "filename": "nome.jpg",
  "title": "Título",
  "description": "Descrição",
  "tags": ["tag1", "tag2"]
}
```
3. Salve e atualize a página!

**Pronto!** 🎨
