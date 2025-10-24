# Contribuindo para UmbraEternum Server

Obrigado por considerar contribuir para o projeto UmbraEternum! Este documento fornece diretrizes para contribuições.

## Código de Conduta

- Seja respeitoso e profissional
- Aceite feedback construtivo
- Foque no que é melhor para o projeto
- Mantenha discussões técnicas e objetivas

## Como Contribuir

### 1. Reportando Bugs

Ao reportar um bug, inclua:
- **Descrição clara**: O que aconteceu vs. o que era esperado
- **Passos para reproduzir**: Lista numerada de passos
- **Ambiente**: SO, versão do compilador, configurações
- **Logs**: Logs relevantes ou stack traces
- **Screenshots**: Se aplicável

### 2. Sugerindo Features

Para sugerir uma nova feature:
- Verifique se já não existe uma issue similar
- Descreva o problema que a feature resolve
- Explique a solução proposta
- Considere alternativas
- Adicione exemplos de uso

### 3. Pull Requests

#### Antes de Submeter

1. Fork o repositório
2. Crie um branch a partir de `develop`:
   ```bash
   git checkout -b feature/minha-feature develop
   ```
3. Faça suas mudanças seguindo as diretrizes de código
4. Execute os testes:
   ```bash
   cd build
   ctest --output-on-failure
   ```
5. Commit suas mudanças seguindo Conventional Commits

#### Conventional Commits

Formato: `<tipo>(<escopo>): <descrição>`

**Tipos:**
- `feat`: Nova feature
- `fix`: Correção de bug
- `docs`: Apenas documentação
- `style`: Formatação, ponto e vírgula, etc.
- `refactor`: Refatoração de código
- `perf`: Melhoria de performance
- `test`: Adição de testes
- `chore`: Tarefas de build, CI, etc.

**Exemplos:**
```
feat(auth): adiciona suporte a OAuth2
fix(zone): corrige crash ao desconectar jogador
docs(readme): atualiza instruções de build
refactor(network): extrai lógica de parsing para classe separada
```

#### Durante o PR

- Descreva claramente o que foi mudado e por quê
- Referencie issues relacionadas (#123)
- Adicione labels apropriadas: `enhancement`, `bug`, `ue5-integration`
- Solicite pelo menos 2 revisores
- Responda a comentários de forma construtiva

### 4. Revisão de Código

Ao revisar PRs:
- Verifique conformidade com padrões de código
- Teste localmente se possível
- Seja construtivo e específico nos comentários
- Aprove quando satisfeito com as mudanças

## Padrões de Código

### C++ Style Guide

Seguimos o [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) com algumas adaptações:

#### Nomenclatura

```cpp
// Classes: PascalCase
class PlayerManager {};

// Funções/Métodos: camelCase
void updatePosition();

// Variáveis: camelCase
int playerCount = 0;

// Constantes: UPPER_SNAKE_CASE
const int MAX_PLAYERS = 1000;

// Membros privados: trailing underscore
class MyClass {
 private:
  int privateVar_;
};

// Namespaces: PascalCase
namespace Umbra {
namespace Core {
```

#### Indentação e Formatação

```cpp
// 2 espaços de indentação
void myFunction() {
  if (condition) {
    doSomething();
  }
}

// 80 caracteres por linha
// Quebrar linhas longas de forma legível
void functionWithManyParameters(
    int firstParameter,
    int secondParameter,
    int thirdParameter) {
  // ...
}

// Chaves em nova linha para funções/classes
class MyClass 
{
 public:
  void method();
};

// Chaves na mesma linha para if/for/while
if (condition) {
  doSomething();
}
```

#### Headers

```cpp
#pragma once  // Preferir ao invés de include guards

// Ordem de includes:
// 1. Header correspondente (.cpp)
// 2. C system headers
// 3. C++ standard library
// 4. Third-party libraries
// 5. Project headers

#include "MyClass.hpp"

#include <cstring>
#include <iostream>
#include <vector>

#include <nlohmann/json.hpp>

#include "core/Logger.hpp"
```

#### Smart Pointers

```cpp
// Preferir smart pointers a raw pointers
std::unique_ptr<Object> obj = std::make_unique<Object>();
std::shared_ptr<Resource> res = std::make_shared<Resource>();

// RAII para gerenciamento de recursos
class ResourceManager {
 public:
  ResourceManager() { acquire(); }
  ~ResourceManager() { release(); }
  
  // Delete copy, allow move
  ResourceManager(const ResourceManager&) = delete;
  ResourceManager& operator=(const ResourceManager&) = delete;
  ResourceManager(ResourceManager&&) = default;
  ResourceManager& operator=(ResourceManager&&) = default;
};
```

#### Tratamento de Erros

```cpp
// Usar std::optional para retornos opcionais
std::optional<Player> findPlayer(uint64_t id) {
  if (playerExists(id)) {
    return player;
  }
  return std::nullopt;
}

// Usar exceções para erros excepcionais
void criticalOperation() {
  if (!canProceed()) {
    throw std::runtime_error("Cannot proceed with operation");
  }
}

// Retornos precoces para evitar aninhamento
bool validateInput(const std::string& input) {
  if (input.empty()) {
    return false;
  }
  
  if (input.length() > MAX_LENGTH) {
    return false;
  }
  
  return true;
}
```

### Documentação

Use comentários Doxygen para APIs públicas:

```cpp
/**
 * @brief Adiciona jogador à zona
 * 
 * @param player Dados do jogador
 * @return true se adicionado com sucesso, false se zona cheia
 * 
 * @note Esta função é thread-safe
 */
bool addPlayer(const Player& player);
```

### Testes

Todo código novo deve incluir testes:

```cpp
TEST(PlayerManagerTest, AddPlayer) {
  PlayerManager manager(10);
  Player player;
  player.id = 1;
  
  EXPECT_TRUE(manager.addPlayer(player));
  EXPECT_EQ(manager.getPlayerCount(), 1);
}
```

Cobertura mínima: **80%**

## Checklist de PR

Antes de submeter, verifique:

- [ ] Código compila sem warnings
- [ ] Testes passam (`ctest`)
- [ ] Cobertura de testes adequada
- [ ] Documentação atualizada
- [ ] Changelog atualizado (se aplicável)
- [ ] Commits seguem Conventional Commits
- [ ] Código formatado corretamente
- [ ] Sem código comentado ou debug prints
- [ ] Logs apropriados (níveis corretos)

## Processo de Desenvolvimento

### Branches

- `main`: Produção, sempre estável
- `develop`: Desenvolvimento, integração de features
- `feature/*`: Novas features
- `bugfix/*`: Correções de bugs
- `hotfix/*`: Correções urgentes para produção
- `release/*`: Preparação de releases

### Workflow

```bash
# 1. Atualizar develop
git checkout develop
git pull origin develop

# 2. Criar feature branch
git checkout -b feature/minha-feature

# 3. Desenvolver e commitar
git add .
git commit -m "feat(module): descrição da mudança"

# 4. Push para origin
git push origin feature/minha-feature

# 5. Abrir Pull Request no GitHub
# - Base: develop
# - Compare: feature/minha-feature

# 6. Após aprovação e merge, deletar branch
git branch -d feature/minha-feature
```

### Sprints

- Duração: 1 semana
- Planning: Segunda-feira
- Review: Sexta-feira
- Retrospectiva: Sexta-feira

## Comunicação

- **Issues**: Para bugs e features
- **Pull Requests**: Para revisão de código
- **Discord**: Para discussões rápidas
- **Email**: Para questões confidenciais

## Recursos

- [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- [Conventional Commits](https://www.conventionalcommits.org/)
- [CMake Documentation](https://cmake.org/documentation/)
- [Doxygen Manual](https://www.doxygen.nl/manual/)

## Licença

Ao contribuir, você concorda que suas contribuições serão licenciadas sob a mesma licença do projeto.

---

Obrigado por contribuir para o UmbraEternum! 🎮

