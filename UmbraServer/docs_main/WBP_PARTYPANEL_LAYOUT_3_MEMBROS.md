# WBP_PartyPanel: exibir todos os 3+ membros dentro do Vertical Box

Se um membro aparece **fora** do painel, o problema é o **root do WBP_PartyMemberSlot**.

## Correção principal

**WBP_PartyMemberSlot** – o root **não** deve ser Canvas Panel com Position X/Y. Troque por **Border** ou **Horizontal Box** para o slot ficar dentro do VB.

## Verificações

- **VB_PartyBox:** sem altura fixa que corte os slots.
- **WBP_PartyMemberSlot:** nomes TextPlayerName, Text_Lvl, Text_HP, Text_MP, ProgressBar_HP, ProgressBar_MP.
- **ForLoop:** FirstIndex 0, LastIndex 5.
