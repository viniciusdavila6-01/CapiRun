#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Components/DynamicMeshComponent.h"
#include "CapiRunComplete.generated.h"

namespace CapiCoord
{
    FORCEINLINE FVector Pos(float Bx, float By, float Bz) { return FVector(Bz * 100.f, Bx * 100.f, By * 100.f); }
    FORCEINLINE FVector Size(float Wx, float Hy, float Dz) { return FVector(Dz * 100.f, Wx * 100.f, Hy * 100.f); }
}

namespace CapiPalette
{
    static const FLinearColor DeepGreen   = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("173E32")));
    static const FLinearColor LightGreen  = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("78A94B")));
    static const FLinearColor Water       = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("287D87")));
    static const FLinearColor Caramel     = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("B77A47")));
    static const FLinearColor Cloth       = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("D8C398")));
    static const FLinearColor Coral       = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("EF7748")));
    static const FLinearColor AraraBlue   = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("2589C8")));
    static const FLinearColor AraraYellow = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("F1C94B")));
    static const FLinearColor Cream       = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("F1E2C0")));
    static const FLinearColor DarkBrown   = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("5A3A22")));
    static const FLinearColor Earth       = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("7A5433")));
    static const FLinearColor Path        = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("CDB57E")));
    static const FLinearColor Rock        = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("8A8A7C")));
    static const FLinearColor RockDark    = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("5F6658")));
    static const FLinearColor Wood        = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("A8743F")));
    static const FLinearColor WoodDark    = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("7B4F2C")));
    static const FLinearColor Olive       = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("6B7A3A")));
    static const FLinearColor Hat         = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("B9A066")));
    static const FLinearColor Black       = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("1E1B18")));
    static const FLinearColor White       = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("FFFFFF")));
    static const FLinearColor Moss        = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("3F6B3C")));
    static const FLinearColor Leaf        = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("4E8F3E")));
    static const FLinearColor LeafDark    = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("2B5C36")));
    static const FLinearColor Straw       = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("E4C56E")));
    static const FLinearColor Red         = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("D9482B")));
    static const FLinearColor Gold        = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("E0B040")));
    static const FLinearColor Onca        = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("DDA640")));
    static const FLinearColor OncaSpot    = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("3B2A1A")));
    static const FLinearColor Mosquito    = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("2C3440")));
    static const FLinearColor MosquitoEye = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("E8552E")));
    static const FLinearColor Jacare      = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("3E6B3A")));
    static const FLinearColor JacareBelly = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("B9C27A")));
    static const FLinearColor Mud         = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("5B4026")));
    static const FLinearColor Foam        = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("EAF6F2")));
    static const FLinearColor Fire        = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("FF9A2E")));
    static const FLinearColor Paper       = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("F4E9CF")));
}

UENUM(BlueprintType)
enum class ECapiBoxKind : uint8
{
    Ground, Cliff, Rock, Wood, Root, Log, Stone, Branch, PoolFloor, Rail
};

UENUM(BlueprintType)
enum class ECapiPlayerState : uint8
{
    Grounded, Airborne, Swimming, Attacking, Dodging, Hurt, CheckpointHighFive, Dead, Respawning, Celebrate
};

USTRUCT()
struct FCapiBoxDef
{
    GENERATED_BODY()
    FString Id;
    float X0, X1, Y0, Y1, Z0, Z1;
    ECapiBoxKind Kind;
    bool bBroken = false;
};

USTRUCT()
struct FCapiWaterDef
{
    GENERATED_BODY()
    FString Id;
    float X0, X1, Z0, Z1, SurfaceY, CurrentX, CurrentZ;
    bool bDeadly;
};

USTRUCT()
struct FCapiMovingPlatform
{
    GENERATED_BODY()
    FString Id;
    float W, H, D, AX, AY, AZ, BX, BY, BZ, Period, Phase;
    UPROPERTY() UDynamicMeshComponent* Mesh = nullptr;
    FVector CurrentPosB = FVector::ZeroVector;
    FVector CurrentVelB = FVector::ZeroVector;
};

UCLASS()
class ACapiCompleteWorld : public APawn
{
    GENERATED_BODY()

public:
    ACapiCompleteWorld();

    UPROPERTY(VisibleAnywhere) USceneComponent* WorldRoot;

    UPROPERTY(VisibleAnywhere) USceneComponent* CapiRoot;
    UPROPERTY(VisibleAnywhere) UDynamicMeshComponent* CapiTorso;
    UPROPERTY(VisibleAnywhere) UDynamicMeshComponent* CapiHead;
    UPROPERTY(VisibleAnywhere) UDynamicMeshComponent* CapiHat;
    UPROPERTY(VisibleAnywhere) UDynamicMeshComponent* CapiBackpack;
    UPROPERTY(VisibleAnywhere) UDynamicMeshComponent* CapiArmL;
    UPROPERTY(VisibleAnywhere) UDynamicMeshComponent* CapiArmR;
    UPROPERTY(VisibleAnywhere) UDynamicMeshComponent* CapiLegL;
    UPROPERTY(VisibleAnywhere) UDynamicMeshComponent* CapiLegR;

    UPROPERTY(VisibleAnywhere) UDynamicMeshComponent* AraraoCP1Mesh;
    UPROPERTY(VisibleAnywhere) UDynamicMeshComponent* AraraoCP2Mesh;
    UPROPERTY(VisibleAnywhere) UDynamicMeshComponent* Mosquito1Mesh;
    UPROPERTY(VisibleAnywhere) UDynamicMeshComponent* Mosquito2Mesh;
    UPROPERTY(VisibleAnywhere) UDynamicMeshComponent* JacareMesh;
    UPROPERTY(VisibleAnywhere) UDynamicMeshComponent* GrandeOncaMesh;

    UPROPERTY(VisibleAnywhere) UDynamicMeshComponent* StaticWorldMesh;
    UPROPERTY(VisibleAnywhere) UDynamicMeshComponent* WaterAndMudMesh;
    UPROPERTY(VisibleAnywhere) UDynamicMeshComponent* PropsAndDecorMesh;
    UPROPERTY(VisibleAnywhere) UDynamicMeshComponent* PickupsMesh;

    UPROPERTY(BlueprintReadOnly) ECapiPlayerState State = ECapiPlayerState::Grounded;
    UPROPERTY(BlueprintReadOnly) int32 Hp = 3;
    UPROPERTY(BlueprintReadOnly) int32 FruitsCollected = 0;
    UPROPERTY(BlueprintReadOnly) int32 RegistrosCollected = 0;
    UPROPERTY(BlueprintReadOnly) int32 ActiveCheckpointIdx = -1;
    UPROPERTY(BlueprintReadOnly) bool bLevelFinished = false;

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
    FVector CapiPosB = FVector(0.f, 0.f, 1.5f);
    FVector CapiVelB = FVector::ZeroVector;
    FVector RespawnPosB = FVector(0.f, 0.f, 1.5f);
    float InputX = 0.f;
    float InputZ = 0.f;
    bool bJumpHeld = false;
    float CoyoteTimer = 0.f;
    float JumpBufferTimer = 0.f;
    float StateTimer = 0.f;
    float InvulnTimer = 0.f;
    float DodgeCooldown = 0.f;
    float AnimPhase = 0.f;

    bool bChaseActive = false;
    float OncaZ = 174.f;
    float OncaY = 3.f;
    TArray<float> OncaHopZ;

    TArray<FCapiBoxDef> LevelBoxes;
    TArray<FCapiBoxDef> FragileBoxes;
    TArray<FCapiBoxDef> BreakableBoxes;
    TArray<FCapiWaterDef> Waters;
    TArray<FBox> MudZones;
    TArray<FCapiMovingPlatform> MovingPlatforms;
    TArray<FVector> FruitPositions;
    TArray<bool> FruitTaken;
    TArray<FVector> RegistroPositions;
    TArray<bool> RegistroTaken;

    void PopulateTrilhaLevelData();
    void BuildEntireLevelGeometry();
    void RebuildPickupsMesh();
    void BuildCapiArticulatedModel();
    void BuildCharactersAndEnemies();
    void AppendColoredBox(UDynamicMeshComponent* Comp, FVector CenterB, FVector SizeB, FLinearColor Color);

    void StepSimulation(float Dt);
    void ResolveCylinderCollisionSwept(float Dt, float Radius, float Height);
    void UpdateEnemiesAndChase(float Dt);
    void UpdateCameraZones(float Dt);
    void AnimateAllModels(float Dt);

    void InputMoveX(float Val) { InputX = Val; }
    void InputMoveZ(float Val) { InputZ = Val; }
    void PressJump();
    void ReleaseJump();
    void PressMochilada();
    void PressDodge();
};
