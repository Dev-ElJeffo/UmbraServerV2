# 🎨 Sistema de Galeria de Arte Conceitual

Bem-vindo ao sistema de galeria de arte conceitual do **Umbra Eternum**!

Este diretório contém todas as artes conceituais organizadas por categoria e um sistema dinâmico de galeria baseado em JSON.

---

## 📚 Documentação Disponível

### Para Começar Rapidamente

🚀 **[QUICK_START.md](QUICK_START.md)**
- Guia rápido em 3 passos
- Ideal para adicionar sua primeira imagem
- Tempo estimado: 2 minutos

### Documentação Completa

📖 **[GUIA_GALLERY_JSON.md](GUIA_GALLERY_JSON.md)**
- Documentação detalhada e completa
- Exemplos práticos para cada categoria
- Especificações técnicas
- Solução de problemas
- Otimização de imagens

### Referência Rápida

📋 **[CHEAT_SHEET.md](CHEAT_SHEET.md)**
- Tabelas de referência rápida
- Tags recomendadas por categoria
- Checklist de validação
- Troubleshooting rápido

---

## 📂 Estrutura de Diretórios

```
concepts/
├── gallery.json              ← Arquivo de configuração principal
├── QUICK_START.md           ← Início rápido
├── GUIA_GALLERY_JSON.md     ← Documentação completa
├── CHEAT_SHEET.md           ← Referência rápida
├── README.md                ← Este arquivo
│
├── characters/               ← Arte de personagens
│   ├── barbarian_01.jpg
│   ├── darkmage_01.jpg
│   └── ...
│
├── environments/             ← Arte de ambientes
│   ├── velkaris_forges.jpg
│   ├── grimholt_cemetery.jpg
│   └── ...
│
├── creatures/                ← Arte de criaturas
│   ├── corrupted_beast.jpg
│   └── ...
│
└── weapons/                  ← Arte de armas
    ├── legendary_sword.jpg
    └── ...
```

---

## 🎯 Como Adicionar uma Nova Imagem

### Passo 1: Escolha a Categoria

| Pasta | Tipo de Conteúdo |
|-------|------------------|
| `characters/` | Personagens, classes, NPCs |
| `environments/` | Cidades, paisagens, locais |
| `creatures/` | Monstros, criaturas corrompidas |
| `weapons/` | Armas, equipamentos |

### Passo 2: Adicione o Arquivo

Coloque sua imagem na pasta correspondente:

```
concepts/characters/minha_arte.jpg
```

**Regras de nomenclatura:**
- ✅ Usar letras minúsculas
- ✅ Separar palavras com underscore `_` ou hífen `-`
- ✅ Sem espaços ou caracteres especiais
- ✅ Extensões: `.jpg`, `.png`, `.webp`

### Passo 3: Edite o gallery.json

Abra o arquivo `gallery.json` e adicione uma entrada:

```json
{
  "filename": "minha_arte.jpg",
  "title": "Título da Arte",
  "description": "Descrição detalhada",
  "tags": ["tag1", "tag2", "tag3"]
}
```

### Passo 4: Valide e Salve

1. Valide o JSON em: https://jsonlint.com
2. Salve o arquivo
3. Atualize a página (F5)

---

## 🎨 Categorias e Tags

### Characters (Personagens)
**Tags comuns:** `classe`, `npc`, `feminino`, `masculino`, `elfico`, `barbarian`, `darkmage`, `marcial`, `templar`, `alchemist`, `assassin`

### Environments (Ambientes)
**Tags comuns:** `cidade`, `masmorra`, `velkaris`, `grimholt`, `eremos`, `fortedras`, `noite`, `dia`, `nevoa`

### Creatures (Criaturas)
**Tags comuns:** `monstro`, `boss`, `corrupto`, `dimensional`, `neblina`, `portal`, `elite`, `lendario`

### Weapons (Armas)
**Tags comuns:** `espada`, `cajado`, `arco`, `machado`, `lendario`, `epico`, `rubicina`, `obsidita`

---

## 🖼️ Especificações Técnicas

### Tamanhos Recomendados

| Categoria | Dimensões | Orientação |
|-----------|-----------|------------|
| Characters | 1200x1600px | Portrait (vertical) |
| Environments | 1920x1080px | Landscape (horizontal) |
| Creatures | 1200x1200px | Quadrado |
| Weapons | 800x800px | Quadrado |

### Formatos e Tamanhos

- **Formatos aceitos:** JPG, PNG, WebP
- **Tamanho máximo:** 1MB por imagem
- **Resolução mínima:** 800px de largura
- **Qualidade JPG:** 80-85%

---

## 🔧 Ferramentas Úteis

### Validação JSON
👉 https://jsonlint.com - Valide seu JSON antes de salvar

### Otimização de Imagens
👉 https://tinypng.com - Comprima suas imagens sem perder qualidade

### Editores JSON Online
- https://jsoneditoronline.org - Editor visual
- https://codebeautify.org/jsonviewer - Formatador

---

## 🚨 Troubleshooting

### Imagem não aparece?

1. **Verifique o nome do arquivo**
   - O `filename` no JSON deve ser exatamente igual ao nome do arquivo
   - Atenção a maiúsculas/minúsculas

2. **Verifique a pasta**
   - Imagem está na pasta correta da categoria?

3. **Valide o JSON**
   - Use JSONLint para verificar erros de sintaxe

4. **Limpe o cache**
   - Pressione `Ctrl + Shift + R` para forçar reload

### JSON inválido?

**Erros comuns:**
- Vírgula extra no último item da lista
- Falta de vírgula entre itens
- Chaves ou colchetes não fechados
- Aspas duplas faltando

**Solução:** Use JSONLint para identificar o erro exato

---

## 📖 Leia Mais

Para informações detalhadas, consulte:

1. **[QUICK_START.md](QUICK_START.md)** - Comece aqui!
2. **[GUIA_GALLERY_JSON.md](GUIA_GALLERY_JSON.md)** - Documentação completa
3. **[CHEAT_SHEET.md](CHEAT_SHEET.md)** - Referência rápida

---

## 📞 Suporte

Se você encontrar problemas:

1. Consulte o [GUIA_GALLERY_JSON.md](GUIA_GALLERY_JSON.md)
2. Verifique o [CHEAT_SHEET.md](CHEAT_SHEET.md)
3. Use o console do navegador (F12) para ver erros
4. Valide o JSON em https://jsonlint.com

---

## 📊 Status Atual

**Versão do Sistema:** 1.0  
**Última Atualização:** 27/04/2026  
**Categorias Disponíveis:** 4 (Characters, Environments, Creatures, Weapons)

---

**UmbraEternum** - Sistema de Galeria de Arte Conceitual  
*Dark Fantasy MMORPG*
