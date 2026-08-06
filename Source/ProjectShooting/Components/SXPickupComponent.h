// SXPickupComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "Interaction/SXInteractableInterface.h"
#include "SXPickupComponent.generated.h"

class ASXPlayerCharacter;

UENUM(BlueprintType)
enum class ESXPickupMethod : uint8
{
	Magnet,
	Interaction
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPickUp, ASXPlayerCharacter*, InPickUpCharacter);

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTSHOOTING_API USXPickupComponent : public UStaticMeshComponent, public ISXInteractableInterface
{
	GENERATED_BODY()

public:
	USXPickupComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual bool CanInteract_Implementation(ASXPlayerCharacter* InteractingCharacter) const override;

	virtual void Interact_Implementation(ASXPlayerCharacter* InteractingCharacter) override;

	virtual FText GetInteractionText_Implementation() const override;

	UFUNCTION(BlueprintCallable, Category="SX|Pickup")
	void PickUp(ASXPlayerCharacter* InPickUpCharacter);

	void DisablePickupBehavior();

	UFUNCTION(BlueprintCallable, Category="SX|Pickup")
	void SetPickupMethod(ESXPickupMethod NewPickupMethod) { PickupMethod = NewPickupMethod; }

	UFUNCTION(BlueprintPure, Category="SX|Pickup")
	ESXPickupMethod GetPickupMethod() const { return PickupMethod; }

	UPROPERTY(BlueprintAssignable)
	FOnPickUp OnPickUp;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleOnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleOnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void StartMagnetPickup(ASXPlayerCharacter* InPickUpCharacter);
	void StartDropPhysics();
	void StopDropPhysics();
	void UpdateKinematicDrop(float DeltaTime);
	void StartFloating();
	void UpdateFloating(float DeltaTime);
	void RefreshTickState();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Pickup")
	ESXPickupMethod PickupMethod = ESXPickupMethod::Interaction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Pickup")
	FText InteractionText = FText::FromString(TEXT("[F] Pick Up"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Pickup|Magnet", meta=(ClampMin="0.0", Units=cm))
	float MagnetAttractRadius = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Pickup|Magnet", meta=(ClampMin="0.0", Units=cm))
	float MagnetMoveSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Pickup|Magnet", meta=(ClampMin="0.0", Units=cm))
	float PickupDistance = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Pickup|Drop", meta=(DisplayName="Use Drop Physics"))
	bool bUseDropPhysics = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Pickup|Drop", meta=(ClampMin="0.0", Units=s, EditCondition="bUseDropPhysics"))
	float DropPhysicsTime = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Pickup|Drop", meta=(ClampMin="0.0", Units="cm/s", EditCondition="bUseDropPhysics"))
	float DropImpulseXY = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Pickup|Drop", meta=(ClampMin="0.0", Units="cm/s", EditCondition="bUseDropPhysics"))
	float DropImpulseZ = 260.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Pickup|Drop", meta=(ClampMin="0.0", Units="deg/s", EditCondition="bUseDropPhysics"))
	float DropAngularImpulse = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Pickup|Floating")
	bool bFloatAfterDrop = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Pickup|Floating", meta=(ClampMin="0.0", Units=cm, EditCondition="bFloatAfterDrop"))
	float FloatingHeight = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Pickup|Floating", meta=(ClampMin="0.0", Units=cm, EditCondition="bFloatAfterDrop"))
	float FloatingAmplitude = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Pickup|Floating", meta=(ClampMin="0.0", EditCondition="bFloatAfterDrop"))
	float FloatingSpeed = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Pickup|Floating", meta=(Units="deg/s", EditCondition="bFloatAfterDrop"))
	float FloatingRotationSpeed = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Pickup|Floating", meta=(EditCondition="bFloatAfterDrop"))
	bool bSnapToGroundBeforeFloating = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SX|Pickup|Floating", meta=(ClampMin="0.0", Units=cm, EditCondition="bSnapToGroundBeforeFloating"))
	float GroundTraceDistance = 300.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Pickup")
	TObjectPtr<ASXPlayerCharacter> MagnetTargetCharacter;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Pickup")
	bool bPickedUp = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Pickup|Drop")
	bool bDropPhysicsActive = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Pickup|Drop")
	bool bKinematicDropActive = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Pickup|Floating")
	bool bFloatingActive = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="SX|Pickup|Floating")
	FVector FloatingBaseLocation = FVector::ZeroVector;

	FVector KinematicDropVelocity = FVector::ZeroVector;

	float KinematicDropElapsedTime = 0.0f;

	float FloatingElapsedTime = 0.0f;

	FTimerHandle DropPhysicsTimerHandle;

	void UpdateMagnetTarget();
};
