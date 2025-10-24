# Política de Segurança

## Versões Suportadas

Atualmente, fornecemos atualizações de segurança para as seguintes versões:

| Versão | Suportada          |
| ------ | ------------------ |
| 1.3.x  | :white_check_mark: |
| < 1.3  | :x:                |

## Reportando uma Vulnerabilidade

A segurança do UmbraEternum Server é levada a sério. Se você descobrir uma vulnerabilidade de segurança, agradecemos sua ajuda em divulgá-la de forma responsável.

### Por favor, NÃO reporte vulnerabilidades de segurança publicamente

**Em vez disso:**

1. **Email**: Envie um email para [security@umbraeternnum.com](mailto:security@umbraeternnum.com)
2. **GitHub Security Advisory**: Use o recurso de [Security Advisories](https://github.com/Dev-ElJeffo/UmbraServerV2/security/advisories) do GitHub

### O que incluir no seu relatório

Para nos ajudar a entender melhor a natureza e o escopo do problema, inclua o máximo de informações possível:

* Tipo de vulnerabilidade (ex: SQL injection, XSS, buffer overflow, etc.)
* Localização do código-fonte afetado (tag/branch/commit ou URL direto)
* Qualquer configuração especial necessária para reproduzir o problema
* Instruções passo a passo para reproduzir o problema
* Prova de conceito ou código de exploit (se possível)
* Impacto do problema, incluindo como um atacante poderia explorar a vulnerabilidade

### O que esperar

* **Confirmação**: Você receberá uma confirmação do seu relatório em até 48 horas
* **Atualizações**: Manteremos você informado sobre o progresso a cada 5-7 dias
* **Divulgação**: Coordenaremos a divulgação pública após a correção
* **Crédito**: Você será creditado (se desejar) na divulgação da correção

### Processo de Resposta

1. **Triagem** (1-2 dias): Confirmamos e avaliamos a severidade
2. **Desenvolvimento** (1-14 dias): Criamos e testamos a correção
3. **Release** (0-7 dias): Publicamos a correção
4. **Divulgação** (após release): Publicamos detalhes da vulnerabilidade

## Melhores Práticas de Segurança

### Para Desenvolvedores

* **Nunca commite:**
  * Senhas, chaves de API ou tokens
  * Arquivos `config/db.json` ou `config/jwt_secret.key`
  * Dados sensíveis ou informações pessoais

* **Sempre use:**
  * Prepared statements para queries SQL
  * Validação de entrada em todos os endpoints
  * HTTPS/TLS para comunicação em produção
  * Tokens JWT com expiração adequada
  * Rate limiting para prevenir DDoS

* **Code Review:**
  * Toda mudança relacionada a segurança requer revisão por 2+ pessoas
  * Testes de segurança devem passar antes do merge

### Para Deployment

* **Configuração:**
  * Use senhas fortes e únicas
  * Gere novas chaves JWT para cada ambiente
  * Configure firewall adequadamente
  * Mantenha MySQL e todas as dependências atualizadas

* **Monitoramento:**
  * Ative logs de auditoria
  * Monitore tentativas de login falhas
  * Configure alertas para atividades suspeitas

* **Backup:**
  * Faça backup regular do banco de dados
  * Teste restauração de backups periodicamente
  * Mantenha backups em local seguro

## Vulnerabilidades Conhecidas

Nenhuma vulnerabilidade conhecida no momento.

Histórico de vulnerabilidades será mantido em [SECURITY_ADVISORIES.md](./SECURITY_ADVISORIES.md)

## Hall da Fama de Segurança

Agradecemos aos seguintes pesquisadores de segurança por reportarem vulnerabilidades de forma responsável:

<!-- Lista será atualizada conforme recebemos relatórios -->

## Recursos de Segurança

* [OWASP Top 10](https://owasp.org/www-project-top-ten/)
* [CWE Top 25](https://cwe.mitre.org/top25/)
* [CVE Database](https://cve.mitre.org/)

---

**Contato de Segurança**: security@umbraeternnum.com  
**PGP Key**: [Disponível em breve]  
**Última atualização**: 2025-10-14

