# 🧹 SCRIPTS DE LIMPEZA DO PROJETO UE5

**Problema resolvido**: Scripts que fecham sozinhos!

---

## 📍 LOCALIZAÇÃO DOS SCRIPTS

Os scripts estão no diretório do projeto UE5:

```
D:\UmbraServerV2\UmbraEternumUE\

✅ CleanProject.bat          - RECOMENDADO (Batch file)
✅ CleanProject.ps1          - Alternativa (PowerShell)
✅ COMO_USAR_SCRIPTS.md      - Guia completo
```

---

## ⚡ SOLUÇÃO RÁPIDA

### **USE O ARQUIVO .BAT!**

```
1. Abra o Windows Explorer
2. Navegue até: D:\UmbraServerV2\UmbraEternumUE\
3. Duplo clique em: CleanProject.bat
4. Pressione "S" quando perguntar
5. Aguarde completar
6. Pressione Enter
7. ✅ PRONTO!
```

**Este método SEMPRE funciona!**

---

## 🔧 O QUE OS SCRIPTS FAZEM

Limpam todos os caches de compilação:

```
✅ Delete: Binaries\
✅ Delete: Intermediate\
✅ Delete: Saved\
✅ Delete: .vs\
✅ Delete: DerivedDataCache\
✅ Delete: *.sln
```

---

## 📋 QUANDO USAR

Use os scripts quando:

- ❌ Compilação falha com erros estranhos
- ❌ "Module not found" após adicionar plugins
- ❌ Conflitos de namespace
- ❌ Mudanças no Build.cs não têm efeito
- ❌ Após instalar/remover plugins
- ❌ Antes de regenerar project files

---

## 🚀 FLUXO COMPLETO DE LIMPEZA

```
1. Fechar Visual Studio e Unreal Editor
   ↓
2. Executar CleanProject.bat
   (Duplo clique em D:\UmbraServerV2\UmbraEternumUE\CleanProject.bat)
   ↓
3. Remover VaRest duplicado
   (Delete: D:\UE_5.6\Engine\Plugins\Marketplace\VaRestReb7468bf70331V16\)
   ↓
4. Regenerar project files
   (Right-click UmbraEternumUE.uproject → Generate VS files)
   ↓
5. Recompilar
   (Abrir .sln → Rebuild Solution)
   ↓
6. ✅ Sucesso!
```

---

## 💡 POR QUE O .BAT É MELHOR

### CleanProject.bat (RECOMENDADO)

```
✅ Duplo clique funciona
✅ Não precisa configurar permissões
✅ Funciona sem PowerShell
✅ Pausa automática
✅ Feedback visual
✅ Compatível com qualquer Windows
```

### CleanProject.ps1 (Alternativa)

```
⚠️  Precisa configurar ExecutionPolicy
⚠️  Pode fechar sozinho se mal configurado
⚠️  Requer PowerShell habilitado
✅ Mais poderoso para automação
```

---

## 📚 DOCUMENTAÇÃO COMPLETA

Para guia detalhado com troubleshooting:

```
D:\UmbraServerV2\UmbraEternumUE\COMO_USAR_SCRIPTS.md
```

Contém:
- 3 métodos diferentes de execução
- Troubleshooting completo
- Comandos alternativos
- Checklist de verificação
- Soluções para erros comuns

---

## 🎯 COMANDOS RÁPIDOS

### Via CMD (se scripts não funcionarem)

```cmd
cd D:\UmbraServerV2\UmbraEternumUE
rmdir /s /q Binaries
rmdir /s /q Intermediate
rmdir /s /q Saved
rmdir /s /q .vs
rmdir /s /q DerivedDataCache
del *.sln
```

### Via PowerShell (linha única)

```powershell
cd D:\UmbraServerV2\UmbraEternumUE; Remove-Item -Recurse -Force Binaries,Intermediate,Saved,.vs,DerivedDataCache -ErrorAction SilentlyContinue; Remove-Item *.sln -ErrorAction SilentlyContinue
```

---

## ✅ CHECKLIST

Após executar o script:

- [ ] Pasta Binaries\ foi deletada
- [ ] Pasta Intermediate\ foi deletada
- [ ] Pasta Saved\ foi deletada
- [ ] Pasta .vs\ foi deletada
- [ ] Pasta DerivedDataCache\ foi deletada
- [ ] Arquivos .sln foram deletados
- [ ] Mensagem "✅ LIMPEZA CONCLUÍDA!" apareceu

---

## 📊 RESULTADO ESPERADO

```
========================================
  ✅ LIMPEZA CONCLUÍDA!
========================================

📋 PRÓXIMOS PASSOS:

1. Botão direito em UmbraEternumUE.uproject
2. "Generate Visual Studio project files"
3. Abrir UmbraEternumUE.sln
4. Build → Rebuild Solution
```

---

## 🐛 TROUBLESHOOTING

### Script fecha sozinho

**Solução**: Use o .bat ao invés do .ps1
```
Duplo clique em: CleanProject.bat
```

### "Acesso negado"

**Solução**:
```
1. Feche Visual Studio
2. Feche Unreal Editor
3. Tente novamente
```

### "Arquivo em uso"

**Solução**:
```
1. Task Manager (Ctrl+Shift+Esc)
2. End Task: UnrealEditor.exe, devenv.exe
3. Tente novamente
```

---

## 📝 RESUMO VISUAL

```
╔══════════════════════════════════════════════════════════╗
║                                                          ║
║              🎯 MÉTODO MAIS FÁCIL! 🎯                   ║
║                                                          ║
║  Navegue até:                                           ║
║  D:\UmbraServerV2\UmbraEternumUE\                       ║
║                                                          ║
║  Duplo clique em:                                       ║
║  CleanProject.bat                                       ║
║                                                          ║
║  Pressione "S"                                          ║
║                                                          ║
║  Aguarde (~10 segundos)                                 ║
║                                                          ║
║  ✅ PRONTO!                                             ║
║                                                          ║
╚══════════════════════════════════════════════════════════╝
```

---

**Criado**: 14/10/2025  
**Localização Scripts**: `D:\UmbraServerV2\UmbraEternumUE\`  
**Documentação**: `COMO_USAR_SCRIPTS.md`  
**Status**: ✅ **Scripts funcionais - Use o .bat!**

---

# 🚀 DUPLO CLIQUE NO .BAT E PRONTO!

**Simples, rápido e sempre funciona!** 🎉

