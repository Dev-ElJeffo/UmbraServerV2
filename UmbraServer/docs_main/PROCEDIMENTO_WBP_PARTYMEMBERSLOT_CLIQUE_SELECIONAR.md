# Procedimento: Clique no WBP_PartyMemberSlot para Abrir WBP_SelectedPlayerInfo

## Objetivo
Ao clicar em um slot de membro do grupo no painel, o jogador deve ser selecionado e o menu de contexto (WBP_SelectedPlayerInfo) deve abrir, igual ao comportamento de clicar em um remote actor no mundo.

## Implementação C++ (concluída)
- `UmbraPlayerSelectionComponent::SelectPlayerFromPartyMember()` – seleciona membro por dados do grupo
- `UmbraPartyMemberSlotWidget` – delegate `OnPartyMemberSlotClicked`, botão opcional `Button_Slot`
- `UmbraPartyWidget::OnPartyMemberSlotClickedHandler()` – vincula clique ao SelectionComponent

## Ajuste no Blueprint WBP_PartyMemberSlot

1. Abra o Blueprint **WBP_PartyMemberSlot** no Editor.

2. Adicione um **Button** como elemento clicável:
   - Se o root atual for um **Border** ou **Canvas**: coloque um **Button** como filho que cubra toda a área do slot, ou use o Button como novo root.
   - O Button deve ter o nome **`Button_Slot`** (exatamente) para o C++ fazer o bind automático.
   - Configure o Button para ser transparente (Appearance → Background Color com alpha 0) para não alterar o visual.

3. Alternativa: se o root for um **Border**, você pode trocar para **Button** como root e manter o conteúdo atual como filho do Button. Nesse caso, renomeie o Button para `Button_Slot`.

4. Salve e compile o Blueprint.

## Verificação
- Com o jogo rodando e um grupo formado, clique em um membro no painel.
- O WBP_SelectedPlayerInfo deve abrir com as opções Add Friend, Party, Trade, Whisper, etc.
- O comportamento deve ser igual ao clique em um jogador remoto no mundo.
