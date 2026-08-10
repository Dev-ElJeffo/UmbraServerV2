# Guia UE 5.6.1 — Should Move no ABP (remotes)

O `ABP_Unarmed` costuma ter:

```
Should Move = (Ground Speed > 3) AND (GetCurrentAcceleration != 0)
```

Remotes **não têm input**, então `Acceleration` ficava `(0,0,0)` e a locomotion nunca entrava (idle eterno), mesmo com `Velocity` setada.

## Mitigação C++

Não é possível setar `CMC->Acceleration` de fora (membro **protected** no UE 5.6). A correção é no AnimBP abaixo.

## Ajuste obrigatório no AnimBP

Asset: `/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed`

1. Event Blueprint Update Animation → onde calcula `Should Move`
2. Remover o AND com `Get Current Acceleration`
3. Deixar: `Should Move = Ground Speed > 3.0` (ou threshold existente)
4. Compile + Save

Comentário embutido no asset ainda menciona acceleration/input — ignore após o ajuste.

## QA

| Caso | Esperado |
|------|----------|
| Remote andando (velocity setada pelo NetMovement) | BlendSpace Idle↔Walk ativo |
| Remote parado | Idle |
| Owner local | Inalterado (ainda tem Acceleration real do input) |
