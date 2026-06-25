# ⚡ Gallery.json - Quick Start

## 🎯 3 Passos para Adicionar uma Imagem

### 1️⃣ Adicione a Imagem na Pasta
```
assets/images/concepts/characters/     ← Personagens
assets/images/concepts/environments/   ← Ambientes
assets/images/concepts/creatures/      ← Criaturas
assets/images/concepts/weapons/        ← Armas
```

### 2️⃣ Edite o gallery.json
Abra: `assets/images/concepts/gallery.json`

Adicione dentro da categoria apropriada:
```json
{
  "filename": "nome-do-arquivo.jpg",
  "title": "Título da Arte",
  "description": "Descrição detalhada",
  "tags": ["tag1", "tag2", "tag3"]
}
```

### 3️⃣ Salve e Atualize
- Salve o arquivo
- Pressione F5 no navegador
- ✅ Pronto!

---

## 📋 Modelo para Copiar

### Personagem
```json
{
  "filename": "barbarian_portrait.jpg",
  "title": "Barbarian - Filha da Ruína",
  "description": "Guerreira bárbara em combate",
  "tags": ["classe", "feminino", "barbarian", "guerreiro"]
}
```

### Ambiente
```json
{
  "filename": "velkaris_forges.jpg",
  "title": "Forjas de Velkaris",
  "description": "As forjas ardentes da cidade industrial",
  "tags": ["velkaris", "cidade", "forjas", "noite"]
}
```

### Criatura
```json
{
  "filename": "corrupted_beast.jpg",
  "title": "Besta Corrompida",
  "description": "Criatura deformada pela Neblina Vermelha",
  "tags": ["monstro", "corrupto", "elite"]
}
```

### Arma
```json
{
  "filename": "crimson_blade.jpg",
  "title": "Lâmina Carmesim",
  "description": "Espada forjada com Rubicina pura",
  "tags": ["espada", "lendario", "rubicina"]
}
```

---

## ⚠️ Regras de Ouro

### ✅ FAZER
- Nomes sem espaços: `barbarian_01.jpg`
- Usar underscore: `dark_mage.jpg`
- Imagens < 1MB
- Mínimo 800px de largura
- Salvar em JPG ou PNG

### ❌ EVITAR
- Espaços: ~~`minha imagem.jpg`~~
- Acentos: ~~`personágem.jpg`~~
- Imagens muito grandes (> 2MB)
- Vírgula no último item da lista

---

## 🔧 Erro Comum

### Imagem não aparece?

**Checklist:**
- [ ] Nome no JSON = nome do arquivo?
- [ ] Arquivo na pasta correta?
- [ ] Tem vírgula extra no JSON?
- [ ] JSON válido? Teste em: https://jsonlint.com

---

## 📝 Exemplo Completo

**Arquivo:** `assets/images/concepts/characters/selindra.jpg`

**No gallery.json:**
```json
{
  "categories": {
    "characters": {
      "images": [
        {
          "filename": "selindra.jpg",
          "title": "Lady Selindra Von'Mahr",
          "description": "Arquinecromante de Grimholt",
          "tags": ["npc", "grimholt", "necromancia", "feminino"]
        }
      ]
    }
  }
}
```

---

## 📚 Documentação Completa

Para informações detalhadas, veja:
👉 **GUIA_GALLERY_JSON.md** (mesmo diretório)

---

**Quick Start v1.0** | 27/04/2026
