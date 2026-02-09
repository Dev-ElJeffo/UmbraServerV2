# Corrigir slot aparecendo fora do Vertical Box (TheKillZone/3º membro)

Quando um membro aparece **fora** do painel, a causa é **layout do WBP_PartyMemberSlot**.

## Causa

O WBP_PartyMemberSlot provavelmente tem **root Canvas Panel** com **Position X/Y** que desloca o slot para fora do VerticalBox.

## Solução: root do WBP_PartyMemberSlot

1. Abra **WBP_PartyMemberSlot**.
2. **Root atual:** se for **Canvas Panel**, ele usa posicionamento absoluto – isso causa o slot fora do VB.
3. **Troque o root** por **Border** ou **Horizontal Box**:
   - Clique com direito no root atual → **Replace With** → **Border** (ou **Horizontal Box**).
   - Mantenha os filhos (TextPlayerName, Text_Lvl, barras, etc.) dentro do novo root.
4. O Border/Horizontal Box usa layout automático e permanece dentro do VerticalBox.

## Vertical Box Slot (em cada slot no WBP_PartyPanel)

1. No **VB_PartyBox**, selecione cada filho (WBP_PartyMemberSlot).
2. Em **Details** → **Vertical Box Slot**:
   - **Size** = Auto.
   - **Padding** = 2 ou 4 (evite valores negativos).

## Verificação

Os 3 membros devem aparecer **dentro** do painel, empilhados verticalmente no VB.
