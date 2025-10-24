# Suporte

Obrigado por usar o UmbraEternum Server! Este documento fornece informações sobre como obter ajuda.

## 📚 Documentação

Antes de pedir ajuda, consulte nossa documentação:

* [README.md](../README.md) - Visão geral do projeto
* [QUICKSTART.md](../docs/QUICKSTART.md) - Guia de início rápido
* [BUILD_INSTRUCTIONS.md](../docs/BUILD_INSTRUCTIONS.md) - Instruções de build detalhadas
* [ARCHITECTURE.md](../docs/ARCHITECTURE.md) - Arquitetura do sistema
* [INTEGRATION_UE5.md](../docs/INTEGRATION_UE5.md) - Integração com Unreal Engine 5
* [CONTRIBUTING.md](../CONTRIBUTING.md) - Como contribuir

## ❓ FAQ - Perguntas Frequentes

### Build e Compilação

**Q: Erro "CMake 3.20 or higher is required"**  
A: Atualize o CMake. Veja [BUILD_INSTRUCTIONS.md](../docs/BUILD_INSTRUCTIONS.md#troubleshooting)

**Q: Erro "Could NOT find MySQL"**  
A: Instale MySQL Development Libraries. Veja [BUILD_INSTRUCTIONS.md](../docs/BUILD_INSTRUCTIONS.md#troubleshooting)

**Q: Erro com nlohmann/json.hpp**  
A: Execute `git submodule update --init --recursive`

### Execução

**Q: "Failed to connect to database"**  
A: Verifique credenciais em `config/db.json` e se MySQL está rodando

**Q: "Port already in use"**  
A: Altere portas em `config/server.json`

**Q: Servidor crasha ao iniciar**  
A: Verifique logs em `logs/` e veja se todas as dependências estão instaladas

## 🐛 Reportar Bugs

Encontrou um bug? Por favor:

1. Verifique se já não foi reportado em [Issues](https://github.com/Dev-ElJeffo/UmbraServerV2/issues)
2. Use o template de [Bug Report](.github/ISSUE_TEMPLATE/bug_report.md)
3. Inclua:
   * Passos para reproduzir
   * Comportamento esperado vs atual
   * Logs relevantes
   * Informações do ambiente (OS, compilador, etc.)

## 💡 Sugerir Features

Tem uma ideia? 

1. Verifique se não existe uma issue similar
2. Use o template de [Feature Request](.github/ISSUE_TEMPLATE/feature_request.md)
3. Descreva o problema que resolve e a solução proposta

## 💬 Onde Pedir Ajuda

### GitHub Issues
Para problemas técnicos, bugs ou dúvidas sobre o código:
* [Abrir nova issue](https://github.com/Dev-ElJeffo/UmbraServerV2/issues/new/choose)

### GitHub Discussions
Para discussões gerais, perguntas e compartilhar ideias:
* [Discussions](https://github.com/Dev-ElJeffo/UmbraServerV2/discussions)

### Discord (Em breve)
Para ajuda em tempo real e chat com a comunidade:
* Link será disponibilizado em breve

## 🤝 Como Ajudar Outros

Se você já tem experiência com o projeto:

* Responda perguntas em Issues e Discussions
* Melhore a documentação
* Compartilhe suas experiências e soluções
* Ajude a revisar Pull Requests

## 📞 Contato Direto

Para questões sensíveis (segurança, legal, etc.):
* **Email**: support@umbraeternnum.com
* **Segurança**: security@umbraeternnum.com

## ⏱️ Tempo de Resposta Esperado

* **Issues**: 1-3 dias úteis
* **Pull Requests**: 2-5 dias úteis
* **Discussões**: 1-7 dias
* **Email**: 3-7 dias úteis

## 🌍 Idiomas

Suportamos principalmente:
* 🇧🇷 Português (primário)
* 🇬🇧 English (secondary)

## 📋 Antes de Perguntar

Por favor, certifique-se de:

- [ ] Ler a documentação relevante
- [ ] Pesquisar issues existentes
- [ ] Tentar reproduzir o problema
- [ ] Coletar logs e informações do ambiente
- [ ] Formular uma pergunta clara e específica

## 🙏 Obrigado!

Agradecemos por usar o UmbraEternum Server e por ajudar a melhorá-lo!

---

**Última atualização**: 2025-10-14

