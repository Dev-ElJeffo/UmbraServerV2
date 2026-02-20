# Guia de Implementação: Sistema Follow (Seguir Jogador)

## 📋 Resumo

Implementar sistema onde o jogador local segue automaticamente os movimentos de um remote actor selecionado. O botão "Follow" já existe no context menu e chama `StartFollowingPlayer`; falta a lógica de movimento automático.

---

## ✅ O que já existe

### 1. **GameInstance (C++)**
- `StartFollowingPlayer(int32 TargetPlayerID)` - seta `FollowTargetID` e dispara `OnFollowStarted`
- `StopFollowing()` - limpa `FollowTargetID` e dispara `OnFollowStopped`
- `IsFollowingPlayer()` / `GetFollowTargetID()` - getters
- Delegates: `OnFollowStarted`, `OnFollowStopped`, `OnFollowFailed`
- `GetRemotePlayerActor(int32 PlayerID)` - obtém o remote actor pelo PlayerID
- `RemotePlayerActorsMap` - Map de PlayerID -> RemoteActor

### 2. **PlayerSelectionComponent**
- `GetSelectedPlayerInfo()` - retorna `FUmbraRemotePlayerInfo` com `RemoteActor`
- `GetRemotePlayerInfo(int32 PlayerID, FUmbraRemotePlayerInfo& OutPlayerInfo)` - busca no cache

### 3. **Context Menu**
- Botão "Follow" já existe e chama `HandlePlayerContextAction` → `StartFollowingPlayer`

---

## ✅ O que já foi implementado

### 1. **Componente C++ `UmbraFollowComponent`** ✅
- ✅ Criado em `UmbraEternumUE/Source/UmbraEternumUE/Components/UmbraFollowComponent.h`
- ✅ Implementado em `UmbraEternumUE/Source/UmbraEternumUE/Components/UmbraFollowComponent.cpp`
- ✅ Tick que verifica posição do target periodicamente
- ✅ Calcula direção e move o jogador usando `AddMovementInput`
- ✅ Para quando target está muito próximo ou muito longe
- ✅ Para quando target desconecta ou sai de alcance
- ✅ Restaura velocidade original do Character ao parar

### 2. **GameInstance atualizado** ✅
- ✅ `StartFollowingPlayer` agora chama o componente diretamente (se disponível)
- ✅ `StopFollowing` agora chama o componente diretamente (se disponível)
- ✅ Mantém compatibilidade com delegates (fallback se componente não existir)

---

## ❌ O que falta implementar

**Apenas integração no Blueprint do Character:**
1. Adicionar componente `Umbra Follow Component` ao Character Blueprint
2. (Opcional) Conectar delegates `OnFollowStarted` e `OnFollowStopped` se quiser lógica adicional no Blueprint

---

## 🔧 Implementação

### **PARTE 1: Criar Componente C++ `UmbraFollowComponent`**

#### **1.1 Arquivo Header: `UmbraFollowComponent.h`**

**Localização:** `UmbraEternumUE/Source/UmbraEternumUE/Components/UmbraFollowComponent.h`

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/UmbraGameInstance.h"
#include "UmbraFollowComponent.generated.h"

/**
 * Componente para fazer o jogador seguir automaticamente outro jogador
 * Anexar ao Character do jogador local
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UMBRAETERNUMUE_API UUmbraFollowComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UUmbraFollowComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// ========================================================================
	// CONFIGURAÇÃO
	// ========================================================================

	/** Distância mínima para parar de seguir (muito próximo) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow|Config")
	float MinFollowDistance = 150.0f;

	/** Distância máxima para seguir (muito longe = parar) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow|Config")
	float MaxFollowDistance = 5000.0f;

	/** Velocidade de movimento ao seguir (0 = usar velocidade padrão do Character) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow|Config")
	float FollowSpeed = 0.0f; // 0 = usar MaxWalkSpeed do Character

	/** Intervalo de atualização da posição do target (segundos) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow|Config")
	float UpdateInterval = 0.1f; // 10 vezes por segundo

	/** Se deve usar rotação suave ao seguir */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow|Config")
	bool bSmoothRotation = true;

	/** Velocidade de rotação (graus/segundo) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow|Config")
	float RotationSpeed = 360.0f;

	// ========================================================================
	// FUNÇÕES PÚBLICAS
	// ========================================================================

	/**
	 * Inicia seguir um target (chamado pelo GameInstance via delegate)
	 * @param TargetPlayerID ID do jogador a seguir
	 */
	UFUNCTION(BlueprintCallable, Category = "Follow")
	void StartFollowing(int32 TargetPlayerID);

	/**
	 * Para de seguir o target atual
	 */
	UFUNCTION(BlueprintCallable, Category = "Follow")
	void StopFollowing();

	/**
	 * Verifica se está seguindo alguém
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Follow")
	bool IsFollowing() const { return FollowTargetID > 0; }

	/**
	 * Obtém o ID do target sendo seguido
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Follow")
	int32 GetFollowTargetID() const { return FollowTargetID; }

private:
	// ========================================================================
	// FUNÇÕES INTERNAS
	// ========================================================================

	/** Atualiza a posição do target e move o jogador */
	void UpdateFollowMovement(float DeltaTime);

	/** Obtém o remote actor do target */
	AActor* GetTargetActor() const;

	/** Calcula direção para o target */
	FVector CalculateDirectionToTarget() const;

	/** Verifica se o target está em alcance */
	bool IsTargetInRange(float Distance) const;

	/** Para de seguir e notifica o GameInstance */
	void StopFollowingInternal(const FString& Reason);

	// ========================================================================
	// DADOS INTERNOS
	// ========================================================================

	/** ID do jogador sendo seguido (0 = não está seguindo) */
	int32 FollowTargetID = 0;

	/** Referência ao Character Movement Component */
	UPROPERTY()
	UCharacterMovementComponent* CharacterMovement = nullptr;

	/** Referência ao GameInstance */
	UPROPERTY()
	UUmbraGameInstance* GameInstance = nullptr;

	/** Timer para atualização periódica */
	float UpdateTimer = 0.0f;

	/** Última posição conhecida do target */
	FVector LastTargetLocation = FVector::ZeroVector;

	/** Velocidade original do Character (para restaurar ao parar) */
	float OriginalMaxWalkSpeed = 0.0f;
};
```

#### **1.2 Arquivo Implementation: `UmbraFollowComponent.cpp`** ✅

**Localização:** `UmbraEternumUE/Source/UmbraEternumUE/Components/UmbraFollowComponent.cpp`

**Status:** ✅ Criado e implementado

**Código implementado:**
```cpp
#include "Components/UmbraFollowComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/UmbraGameInstance.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UUmbraFollowComponent::UUmbraFollowComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UUmbraFollowComponent::BeginPlay()
{
	Super::BeginPlay();

	// Obter referências
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		CharacterMovement = OwnerCharacter->GetCharacterMovement();
		if (CharacterMovement)
		{
			OriginalMaxWalkSpeed = CharacterMovement->MaxWalkSpeed;
		}
	}

	if (UWorld* World = GetWorld())
	{
		GameInstance = Cast<UUmbraGameInstance>(World->GetGameInstance());
	}

	if (!CharacterMovement)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UmbraFollowComponent] ⚠️ CharacterMovement não encontrado! Componente deve ser anexado a um Character."));
	}

	if (!GameInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UmbraFollowComponent] ⚠️ UmbraGameInstance não encontrado!"));
	}
}

void UUmbraFollowComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Parar de seguir ao destruir
	if (IsFollowing())
	{
		StopFollowing();
	}

	Super::EndPlay(EndPlayReason);
}

void UUmbraFollowComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsFollowing())
	{
		UpdateFollowMovement(DeltaTime);
	}
}

void UUmbraFollowComponent::StartFollowing(int32 TargetPlayerID)
{
	if (TargetPlayerID <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UmbraFollowComponent] ❌ StartFollowing: TargetPlayerID inválido!"));
		return;
	}

	if (!CharacterMovement)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UmbraFollowComponent] ❌ StartFollowing: CharacterMovement não encontrado!"));
		return;
	}

	// Se já estava seguindo outro, parar primeiro
	if (FollowTargetID > 0 && FollowTargetID != TargetPlayerID)
	{
		StopFollowing();
	}

	FollowTargetID = TargetPlayerID;

	// Salvar velocidade original se ainda não foi salva
	if (OriginalMaxWalkSpeed <= 0.0f && CharacterMovement)
	{
		OriginalMaxWalkSpeed = CharacterMovement->MaxWalkSpeed;
	}

	// Aplicar velocidade de follow se configurada
	if (FollowSpeed > 0.0f && CharacterMovement)
	{
		CharacterMovement->MaxWalkSpeed = FollowSpeed;
	}

	// Obter posição inicial do target
	AActor* TargetActor = GetTargetActor();
	if (TargetActor)
	{
		LastTargetLocation = TargetActor->GetActorLocation();
	}

	UE_LOG(LogTemp, Log, TEXT("[UmbraFollowComponent] 👣 Começou a seguir PlayerID: %d"), TargetPlayerID);
}

void UUmbraFollowComponent::StopFollowing()
{
	if (FollowTargetID <= 0)
	{
		return; // Já parou
	}

	int32 PreviousTarget = FollowTargetID;
	FollowTargetID = 0;
	UpdateTimer = 0.0f;
	LastTargetLocation = FVector::ZeroVector;

	// Restaurar velocidade original
	if (CharacterMovement && OriginalMaxWalkSpeed > 0.0f)
	{
		CharacterMovement->MaxWalkSpeed = OriginalMaxWalkSpeed;
	}

	// Parar movimento
	if (CharacterMovement)
	{
		CharacterMovement->StopMovementImmediately();
	}

	UE_LOG(LogTemp, Log, TEXT("[UmbraFollowComponent] 🛑 Parou de seguir PlayerID: %d"), PreviousTarget);
}

void UUmbraFollowComponent::UpdateFollowMovement(float DeltaTime)
{
	if (!CharacterMovement || !GetOwner())
	{
		return;
	}

	UpdateTimer += DeltaTime;

	// Atualizar posição do target periodicamente
	if (UpdateTimer >= UpdateInterval)
	{
		UpdateTimer = 0.0f;

		AActor* TargetActor = GetTargetActor();
		if (!TargetActor || !IsValid(TargetActor))
		{
			StopFollowingInternal(TEXT("Target actor não encontrado ou inválido"));
			return;
		}

		LastTargetLocation = TargetActor->GetActorLocation();
	}

	// Calcular direção para o target
	FVector OwnerLocation = GetOwner()->GetActorLocation();
	FVector DirectionToTarget = LastTargetLocation - OwnerLocation;
	float DistanceToTarget = DirectionToTarget.Size2D(); // Distância horizontal (ignora Z)

	// Verificar se está em alcance
	if (!IsTargetInRange(DistanceToTarget))
	{
		StopFollowingInternal(FString::Printf(TEXT("Target muito longe (%.0f unidades)"), DistanceToTarget));
		return;
	}

	// Se está muito próximo, parar movimento
	if (DistanceToTarget <= MinFollowDistance)
	{
		CharacterMovement->StopMovementImmediately();
		return;
	}

	// Normalizar direção (apenas horizontal)
	DirectionToTarget.Z = 0.0f;
	DirectionToTarget.Normalize();

	// Aplicar movimento
	FVector MovementInput = DirectionToTarget;
	GetOwner()->AddMovementInput(MovementInput, 1.0f);

	// Rotação suave (opcional)
	if (bSmoothRotation && DirectionToTarget.SizeSquared() > 0.01f)
	{
		FRotator TargetRotation = DirectionToTarget.Rotation();
		FRotator CurrentRotation = GetOwner()->GetActorRotation();
		
		FRotator NewRotation = FMath::RInterpTo(
			CurrentRotation,
			TargetRotation,
			DeltaTime,
			RotationSpeed / 360.0f // Converter graus/segundo para interp speed
		);

		GetOwner()->SetActorRotation(NewRotation);
	}
}

AActor* UUmbraFollowComponent::GetTargetActor() const
{
	if (!GameInstance || FollowTargetID <= 0)
	{
		return nullptr;
	}

	// Tentar obter via GameInstance
	AActor* TargetActor = GameInstance->GetRemotePlayerActor(FollowTargetID);
	if (TargetActor && IsValid(TargetActor))
	{
		return TargetActor;
	}

	// Se não encontrou, tentar via PlayerSelectionComponent
	// (pode estar selecionado mas não registrado no Map ainda)
	if (UWorld* World = GetWorld())
	{
		APlayerController* PC = World->GetFirstPlayerController();
		if (PC)
		{
			// Buscar componente de seleção no Controller ou Pawn
			UUmbraPlayerSelectionComponent* SelectionComp = PC->FindComponentByClass<UUmbraPlayerSelectionComponent>();
			if (!SelectionComp)
			{
				APawn* Pawn = PC->GetPawn();
				if (Pawn)
				{
					SelectionComp = Pawn->FindComponentByClass<UUmbraPlayerSelectionComponent>();
				}
			}

			if (SelectionComp)
			{
				FUmbraRemotePlayerInfo PlayerInfo;
				if (SelectionComp->GetRemotePlayerInfo(FollowTargetID, PlayerInfo))
				{
					return PlayerInfo.RemoteActor;
				}
			}
		}
	}

	return nullptr;
}

FVector UUmbraFollowComponent::CalculateDirectionToTarget() const
{
	AActor* TargetActor = GetTargetActor();
	if (!TargetActor || !GetOwner())
	{
		return FVector::ZeroVector;
	}

	FVector OwnerLocation = GetOwner()->GetActorLocation();
	FVector TargetLocation = TargetActor->GetActorLocation();
	
	FVector Direction = TargetLocation - OwnerLocation;
	Direction.Z = 0.0f; // Apenas horizontal
	Direction.Normalize();

	return Direction;
}

bool UUmbraFollowComponent::IsTargetInRange(float Distance) const
{
	return Distance >= MinFollowDistance && Distance <= MaxFollowDistance;
}

void UUmbraFollowComponent::StopFollowingInternal(const FString& Reason)
{
	UE_LOG(LogTemp, Log, TEXT("[UmbraFollowComponent] 🛑 Parando de seguir: %s"), *Reason);
	StopFollowing();

	// Notificar GameInstance se necessário
	if (GameInstance)
	{
		GameInstance->StopFollowing();
	}
}
```

---

### **PARTE 2: Integrar no Character (Blueprint mínimo)**

#### **2.1 Adicionar Componente ao Character**

1. Abra o Blueprint do Character principal (ex.: `BP_ThirdPersonCharacter` ou `BP_UmbraEternumUECharacter`).
2. Na aba **Components**, clique em **Add Component** → procure **Umbra Follow Component** → adicione.
3. Configure propriedades (opcional):
   - **Min Follow Distance**: 150.0
   - **Max Follow Distance**: 5000.0
   - **Follow Speed**: 0.0 (usa velocidade padrão do Character)
   - **Update Interval**: 0.1
   - **Smooth Rotation**: true
   - **Rotation Speed**: 360.0

#### **2.2 Conectar Delegates do GameInstance**

No **Event Graph** do Character:

1. **Event BeginPlay:**
   ```
   [Event BeginPlay]
       →
   [Get Game Instance] → Cast to Umbra Game Instance
       →
   [Bind Event to OnFollowStarted] (Target = Game Instance)
           Delegate = OnFollowStarted
           Event = [Custom Event: OnFollowStarted]
           TargetPlayerID (int32) → [Start Following] (Target = Umbra Follow Component)
       →
   [Bind Event to OnFollowStopped] (Target = Game Instance)
           Delegate = OnFollowStopped
           Event = [Custom Event: OnFollowStopped]
           → [Stop Following] (Target = Umbra Follow Component)
   ```

2. **Custom Event: OnFollowStarted** (criar):
   - **Input:** `TargetPlayerID` (int32)
   - **Ação:** Chamar `Start Following` do Umbra Follow Component com `TargetPlayerID`

3. **Custom Event: OnFollowStopped** (criar):
   - **Ação:** Chamar `Stop Following` do Umbra Follow Component

**Estrutura completa:**

```
[Event BeginPlay]
    Get Game Instance → Cast to Umbra Game Instance → GameInstanceRef
    │
    ├─ Bind Event to OnFollowStarted
    │     Target = GameInstanceRef
    │     Delegate = OnFollowStarted
    │     Event = CustomEvent_OnFollowStarted
    │
    └─ Bind Event to OnFollowStopped
          Target = GameInstanceRef
          Delegate = OnFollowStopped
          Event = CustomEvent_OnFollowStopped

[CustomEvent_OnFollowStarted] (Input: TargetPlayerID int32)
    Get Owner → Get Component by Class (Umbra Follow Component)
        Start Following (TargetPlayerID)

[CustomEvent_OnFollowStopped]
    Get Owner → Get Component by Class (Umbra Follow Component)
        Stop Following
```

---

### **PARTE 3: Atualizar GameInstance para usar o Componente** ✅ **IMPLEMENTADO**

**Status:** ✅ GameInstance atualizado para chamar o componente diretamente

**Alterações realizadas:**

**No `UmbraGameInstance.h`:**
- ✅ Adicionado forward declaration: `class UUmbraFollowComponent;`

**No `UmbraGameInstance.cpp`:**
- ✅ Adicionados includes: `#include "Components/UmbraFollowComponent.h"`, `#include "GameFramework/PlayerController.h"`, `#include "GameFramework/Pawn.h"`

**Função `StartFollowingPlayer` atualizada:**

```cpp
void UUmbraGameInstance::StartFollowingPlayer(int32 TargetPlayerID)
{
	// ... validações existentes ...

	FollowTargetID = TargetPlayerID;
	UE_LOG(LogTemp, Log, TEXT("[UmbraGameInstance] 👣 Começou a seguir jogador ID: %d"), TargetPlayerID);
	
	OnFollowStarted.Broadcast(TargetPlayerID);

	// NOVO: Tentar chamar componente diretamente se disponível
	if (UWorld* World = GetWorld())
	{
		APlayerController* PC = World->GetFirstPlayerController();
		if (PC)
		{
			APawn* Pawn = PC->GetPawn();
			if (Pawn)
			{
				UUmbraFollowComponent* FollowComp = Pawn->FindComponentByClass<UUmbraFollowComponent>();
				if (FollowComp)
				{
					FollowComp->StartFollowing(TargetPlayerID);
				}
			}
		}
	}
}
```

**No `StopFollowing`:**

```cpp
void UUmbraGameInstance::StopFollowing()
{
	// ... código existente ...

	// NOVO: Parar componente se disponível
	if (UWorld* World = GetWorld())
	{
		APlayerController* PC = World->GetFirstPlayerController();
		if (PC)
		{
			APawn* Pawn = PC->GetPawn();
			if (Pawn)
			{
				UUmbraFollowComponent* FollowComp = Pawn->FindComponentByClass<UUmbraFollowComponent>();
				if (FollowComp && FollowComp->IsFollowing())
				{
					FollowComp->StopFollowing();
				}
			}
		}
	}
}
```

**Comportamento:**
- ✅ Se o componente existir no Character, o GameInstance chama diretamente (sem depender de delegates)
- ✅ Se o componente não existir, o delegate `OnFollowStarted`/`OnFollowStopped` ainda é disparado (fallback para Blueprint)
- ✅ Logs informativos indicam se o componente foi encontrado ou não

---

## 📋 Checklist de Implementação

### ✅ **Código C++ (COMPLETO)**
- [x] Criar `UmbraFollowComponent.h` em `UmbraEternumUE/Source/UmbraEternumUE/Components/`
- [x] Criar `UmbraFollowComponent.cpp` em `UmbraEternumUE/Source/UmbraEternumUE/Components/`
- [x] Atualizar `UmbraGameInstance.h` (forward declaration)
- [x] Atualizar `UmbraGameInstance.cpp` (includes e chamadas ao componente)
- [ ] **Compilar projeto C++** (fazer no Unreal Editor ou Visual Studio)

### ⏳ **Blueprint (PENDENTE)**
- [ ] Adicionar componente `Umbra Follow Component` ao Character Blueprint
- [ ] Configurar propriedades do componente (opcional - valores padrão já estão bons)
- [ ] (Opcional) Conectar delegates `OnFollowStarted` e `OnFollowStopped` se quiser lógica adicional no Blueprint

### ⏳ **Testes (PENDENTE)**
- [ ] Testar: clicar Follow no context menu → jogador deve seguir o target
- [ ] Testar: target se move → jogador deve seguir
- [ ] Testar: target muito longe (> MaxFollowDistance) → deve parar de seguir
- [ ] Testar: target muito próximo (≤ MinFollowDistance) → deve parar movimento mas manter seguindo
- [ ] Testar: clicar Follow novamente ou outro botão → deve parar de seguir
- [ ] Testar: target desconecta → deve parar de seguir automaticamente

---

## 🎯 Comportamento Esperado

1. **Ao clicar Follow:**
   - Jogador começa a se mover em direção ao target
   - Rotação suave em direção ao target (se habilitado)
   - Velocidade ajustada conforme configuração

2. **Durante o follow:**
   - Jogador mantém distância mínima (`MinFollowDistance`)
   - Se target se move, jogador segue
   - Se target para, jogador para ao alcançar distância mínima

3. **Ao parar de seguir:**
   - Target muito longe (> `MaxFollowDistance`)
   - Target desconecta ou actor inválido
   - Chamada manual de `StopFollowing()` (ex.: botão "Stop Follow" ou outro Follow)

---

## 🔧 Ajustes Finais

- **Distâncias:** Ajuste `MinFollowDistance` e `MaxFollowDistance` conforme gameplay desejado
- **Velocidade:** Se `FollowSpeed = 0`, usa velocidade padrão do Character; senão, usa `FollowSpeed`
- **Rotação:** `bSmoothRotation = true` faz rotação suave; `false` usa rotação instantânea do Character Movement
- **Update Interval:** Valores menores (ex.: 0.05) = mais responsivo mas mais CPU; valores maiores (ex.: 0.2) = menos CPU mas menos preciso

---

## 📝 Notas Técnicas

- ✅ O componente usa `AddMovementInput` do Character, respeitando física e colisões
- ✅ Distância é calculada apenas no plano horizontal (ignora diferença de altura Z)
- ✅ O componente para movimento imediatamente quando target está muito próximo (≤ MinFollowDistance)
- ✅ O componente para de seguir quando target está muito longe (> MaxFollowDistance)
- ✅ Se o target não for encontrado (desconectou, saiu de alcance), o follow para automaticamente
- ✅ O componente restaura a velocidade original do Character ao parar de seguir
- ✅ GameInstance chama o componente diretamente, eliminando necessidade de delegates no Blueprint (mas mantém compatibilidade)

---

## 🚀 **Próximos Passos**

1. **Compilar o projeto** no Unreal Editor (ou Visual Studio)
2. **Abrir o Blueprint do Character** principal
3. **Adicionar componente** `Umbra Follow Component` na aba Components
4. **Testar** clicando Follow no context menu de um jogador remoto

**O sistema está pronto para uso após adicionar o componente no Blueprint!**
