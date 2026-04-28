# 📋 Gallery.json - Cheat Sheet

## 🎯 Estrutura Básica

```json
{
  "version": "1.0",
  "lastUpdated": "2026-04-27",
  "categories": {
    "NOME_DA_CATEGORIA": {
      "name": "Nome Exibido",
      "description": "Descrição da categoria",
      "images": [
        {
          "filename": "arquivo.jpg",
          "title": "Título",
          "description": "Descrição",
          "tags": ["tag1", "tag2"]
        }
      ]
    }
  }
}
```

---

## 📂 Categorias e Pastas

| Categoria | Pasta | Tipo de Conteúdo |
|-----------|-------|------------------|
| `characters` | `concepts/characters/` | Personagens, classes, NPCs |
| `environments` | `concepts/environments/` | Cidades, paisagens, locais |
| `creatures` | `concepts/creatures/` | Monstros, criaturas corrompidas |
| `weapons` | `concepts/weapons/` | Armas, equipamentos |

---

## 🎨 Tags por Categoria

### Characters (Personagens)

| Tipo | Tags Sugeridas |
|------|----------------|
| **Classes** | `classe`, `barbarian`, `darkmage`, `marcial`, `templar`, `alchemist`, `assassin` |
| **Gênero** | `feminino`, `masculino`, `elfico` |
| **NPCs** | `npc`, `boss`, `comerciante`, `guarda` |
| **Facções** | `novarra`, `eldros` |

### Environments (Ambientes)

| Tipo | Tags Sugeridas |
|------|----------------|
| **Cidades** | `velkaris`, `grimholt`, `eremos`, `fortedras` |
| **Locais** | `cidade`, `masmorra`, `castelo`, `taverna`, `forja` |
| **Atmosfera** | `noite`, `dia`, `nevoa`, `sombrio` |
| **Região** | `industrial`, `necrotico`, `mistico` |

### Creatures (Criaturas)

| Tipo | Tags Sugeridas |
|------|----------------|
| **Tipo** | `monstro`, `boss`, `corrupto`, `dimensional` |
| **Origem** | `neblina`, `portal`, `abissal` |
| **Dificuldade** | `comum`, `elite`, `raro`, `lendario` |

### Weapons (Armas)

| Tipo | Tags Sugeridas |
|------|----------------|
| **Tipo** | `espada`, `cajado`, `arco`, `machado`, `adaga` |
| **Raridade** | `comum`, `raro`, `epico`, `lendario` |
| **Material** | `rubicina`, `aço`, `obsidita`, `madeira-rubra` |

---

## 🖼️ Especificações de Imagem

| Aspecto | Recomendação |
|---------|--------------|
| **Formato** | JPG (preferencial), PNG, WebP |
| **Tamanho Máximo** | 1MB |
| **Resolução Mínima** | 800px (largura) |
| **Nome do Arquivo** | Sem espaços, sem acentos, usar `_` ou `-` |

### Tamanhos Ideais por Categoria

| Categoria | Tamanho | Orientação |
|-----------|---------|------------|
| **Characters** | 1200x1600px | Portrait (vertical) |
| **Environments** | 1920x1080px | Landscape (horizontal) |
| **Creatures** | 1200x1200px | Quadrado |
| **Weapons** | 800x800px | Quadrado |

---

## ✅ Nomenclatura de Arquivos

### ✓ Correto

```
barbarian_01.jpg
dark_mage_portrait.jpg
velkaris-night.jpg
sword_legendary_crimson.jpg
```

### ✗ Incorreto

```
Barbarian 01.jpg          ← Tem espaço
personágem.jpg            ← Tem acento
IMG_12345.jpg             ← Não descritivo
DarkMage(Portrait).jpg    ← Parênteses
```

---

## 🔍 Validação JSON

Sempre valide seu JSON antes de salvar:

👉 https://jsonlint.com

### Erros Comuns

| Erro | Causa | Solução |
|------|-------|---------|
| `Unexpected token }` | Vírgula extra | Remover vírgula do último item |
| `Unexpected token ]` | Falta de vírgula | Adicionar vírgula entre itens |
| `Unexpected end of JSON` | Chave não fechada | Verificar `{` e `}` |

---

## 📝 Template para Copiar

```json
{
  "filename": "NOME_DO_ARQUIVO.jpg",
  "title": "Título da Arte",
  "description": "Descrição detalhada sobre a arte",
  "tags": ["tag1", "tag2", "tag3"]
}
```

---

## 🚨 Troubleshooting Rápido

| Problema | Solução |
|----------|---------|
| Imagem não aparece | Verificar nome exato do arquivo e caminho |
| Ícone de X quebrado | Verificar se arquivo existe e não está corrompido |
| Filtros não funcionam | Verificar tags no JSON |
| Galeria vazia | Abrir F12 → Console e verificar erros |
| JSON inválido | Usar JSONLint para validar |

---

## 💾 Backup

Sempre faça backup antes de editar:

```bash
# Windows PowerShell
Copy-Item gallery.json gallery.json.backup

# Windows CMD
copy gallery.json gallery.json.backup
```

---

## 📱 Atalhos Úteis

| Ação | Atalho |
|------|--------|
| Abrir Console | F12 |
| Recarregar página | F5 |
| Recarregar forçado | Ctrl + Shift + R |
| Salvar arquivo | Ctrl + S |

---

## 🎯 Checklist de Adição

```
□ Imagem na pasta correta
□ Nome sem espaços/acentos
□ Arquivo < 1MB
□ Resolução >= 800px
□ Entrada no gallery.json
□ filename correto
□ title descritivo
□ description adicionada
□ tags relevantes
□ JSON válido (JSONLint)
□ Sem vírgula extra
□ Arquivo salvo
```

---

## 📚 Documentação Completa

Para informações detalhadas:

- **⚡ QUICK_START.md** - Guia rápido
- **📖 GUIA_GALLERY_JSON.md** - Documentação completa

---

**Cheat Sheet v1.0** | 27/04/2026  
**Projeto:** UmbraEternum - Sistema de Galeria de Arte Conceitual
