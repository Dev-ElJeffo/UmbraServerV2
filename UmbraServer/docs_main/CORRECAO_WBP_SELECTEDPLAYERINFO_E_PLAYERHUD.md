# Correção: WBP_SelectedPlayerInfo (progress bars) e WBP_PlayerHUD (erro None)

## 1. WBP_SelectedPlayerInfo – progress bars HP/MP não preenchem

### Causa
As barras de progresso precisam de um valor **0.0 a 1.0**. Fazer no Blueprint `CurrentHealth / MaxHealth` pode falhar se `MaxHealth` for 0 (divisão por zero) ou se o binding não usar a struct corretamente.

### Solução (C++ já feito)
Foi adicionada a **Blueprint Function Library** `UmbraPlayerInfoBPFL` com:

- **Get Health Percent** – recebe `FUmbraRemotePlayerInfo`, retorna `float` 0.0–1.0 (trata MaxHealth = 0).
- **Get Mana Percent** – idem para MP.

### O que fazer no Blueprint WBP_SelectedPlayerInfo

1. Abra **WBP_SelectedPlayerInfo**.
2. Para a **Progress Bar de HP**:
   - No **Percent** (ou binding da barra), use **Get Health Percent** (categoria `Umbra | PlayerInfo`).
   - Entrada: a variável/parâmetro do tipo **Remote Player Info** (a mesma que vem do evento `OnSelectedPlayerInfoUpdated`).
   - Saída: usar direto no **Percent** da Progress Bar (0–1).
3. Para a **Progress Bar de MP**:
   - Use **Get Mana Percent** com a mesma **Remote Player Info** e ligue o retorno ao **Percent** da barra de MP.
4. Salve e compile o Blueprint.

Assim as barras passam a usar sempre um percentual seguro (0–1), mesmo quando MaxHealth/MaxMana forem 0.

---

## 2. WBP_PlayerHUD – erro “Acessado None” em Remove from Parent

### Mensagem
- `Acessado "None" ao tentar ler a propriedade SelectedPlayerInfoWidgetREF em WBP_PlayerHUD_C`
- `Acessado "None" ao tentar ler a propriedade PlayerContextMenuWidgetREF em WBP_PlayerHUD_C`  
  No nó **Remove from Parent** no **EventGraph**.

### Causa
O **Remove from Parent** é chamado com a referência do widget (SelectedPlayerInfoWidgetREF ou PlayerContextMenuWidgetREF) quando ela ainda é **None** (widget ainda não foi criado ou já foi destruído).

### Solução recomendada (C++ + Blueprint): Safe Remove

No C++ foi adicionada a função **Safe Remove Widget From Parent** (categoria `Umbra | PlayerInfo`): ela só chama RemoveFromParent se o widget for válido; se for None, não faz nada e não gera erro.

**No WBP_PlayerHUD, no evento que remove os widgets (ex.: OnPlayerDeselected):**

1. Em vez de usar o nó **Remove from Parent** ligado direto à ref, use o nó **Safe Remove Widget From Parent** (procure em Umbra | PlayerInfo).
2. Passe **SelectedPlayerInfoWidgetREF** como parâmetro → chame **Safe Remove Widget From Parent**.
3. Em seguida **Set SelectedPlayerInfoWidgetREF = None**.
4. Chame **Safe Remove Widget From Parent** com **PlayerContextMenuWidgetREF**.
5. **Set PlayerContextMenuWidgetREF = None**.

Assim você não usa **Remove from Parent** diretamente (que acessa a ref e quebra se for None). O Safe Remove recebe a ref como parâmetro (passar None é seguro) e só remove se for válido. Depois de chamar, setar a ref em None garante que na próxima abertura um novo widget seja criado.

### Solução alternativa (só Blueprint) – ordem que evita quebrar o menu

Use **esta ordem** no ramo **True** do Branch (para cada um dos dois widgets), senão o context menu pode parar de funcionar:

1. Abra **WBP_PlayerHUD** e o **EventGraph** onde está o **Remove from Parent** (geralmente no evento **OnPlayerDeselected** do Player Selection Component).
2. Para **cada** widget (SelectedPlayerInfo e Context Menu), faça **dois Branches independentes** (os dois saem do mesmo evento, não um do outro):
   - **SelectedPlayerInfoWidgetREF:**  
     - **Is Valid (SelectedPlayerInfoWidgetREF)** → Branch.  
     - No **True**:  
       1. **Copie** `SelectedPlayerInfoWidgetREF` para uma **variável local** (ex.: *Widget To Remove*).  
       2. **Set SelectedPlayerInfoWidgetREF = None** (limpar a ref antes de remover).  
       3. **Remove from Parent** usando a **variável local** (Widget To Remove), não a ref.  
     - No **False**: não faça nada.
   - **PlayerContextMenuWidgetREF:**  
     - **Is Valid (PlayerContextMenuWidgetREF)** → Branch.  
     - No **True**:  
       1. Copie para uma variável local.  
       2. **Set PlayerContextMenuWidgetREF = None**.  
       3. **Remove from Parent** na variável local.  
     - No **False**: não faça nada.
3. Confirme que, ao **abrir** o menu (eventos OnPlayerSelected / OnPlayerContextMenuRequested), você **seta** `SelectedPlayerInfoWidgetREF` e `PlayerContextMenuWidgetREF` com o widget criado (Create Widget + Add to Viewport → **Set ... REF = created widget**). Se essas refs nunca forem setadas ao abrir, elas ficam None e o Remove nunca roda; além disso, pode haver vários widgets criados e não referenciados.
4. Salve e compile.

Resumo (alternativa): **Ordem no True:** (1) guardar ref em variável local, (2) setar ref = None, (3) Remove from Parent na local. Garantir que ao abrir o menu as refs são setadas com o widget criado. **Preferir a solução com Safe Remove Widget From Parent** para não depender de Branch e ordem.
