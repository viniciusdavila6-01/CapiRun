#include "CapiRunComplete.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "GeometryScript/MeshVertexColorFunctions.h"
#include "GeometryScript/MeshNormalsFunctions.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"

ACapiCompleteWorld::ACapiCompleteWorld()
{
    PrimaryActorTick.bCanEverTick = true;
    WorldRoot = CreateDefaultSubobject<USceneComponent>(TEXT("WorldRoot"));
    RootComponent = WorldRoot;

    CapiRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CapiRoot"));
    CapiRoot->SetupAttachment(WorldRoot);

    auto MakeMesh = [&](FName Name, USceneComponent* Parent) -> UDynamicMeshComponent* {
        UDynamicMeshComponent* M = CreateDefaultSubobject<UDynamicMeshComponent>(Name);
        M->SetupAttachment(Parent);
        M->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        return M;
    };

    CapiTorso    = MakeMesh(TEXT("CapiTorso"), CapiRoot);
    CapiHead     = MakeMesh(TEXT("CapiHead"), CapiRoot);
    CapiHat      = MakeMesh(TEXT("CapiHat"), CapiHead);
    CapiBackpack = MakeMesh(TEXT("CapiBackpack"), CapiTorso);
    CapiArmL     = MakeMesh(TEXT("CapiArmL"), CapiTorso);
    CapiArmR     = MakeMesh(TEXT("CapiArmR"), CapiTorso);
    CapiLegL     = MakeMesh(TEXT("CapiLegL"), CapiRoot);
    CapiLegR     = MakeMesh(TEXT("CapiLegR"), CapiRoot);

    AraraoCP1Mesh  = MakeMesh(TEXT("AraraoCP1Mesh"), WorldRoot);
    AraraoCP2Mesh  = MakeMesh(TEXT("AraraoCP2Mesh"), WorldRoot);
    Mosquito1Mesh  = MakeMesh(TEXT("Mosquito1Mesh"), WorldRoot);
    Mosquito2Mesh  = MakeMesh(TEXT("Mosquito2Mesh"), WorldRoot);
    JacareMesh     = MakeMesh(TEXT("JacareMesh"), WorldRoot);
    GrandeOncaMesh = MakeMesh(TEXT("GrandeOncaMesh"), WorldRoot);

    StaticWorldMesh   = MakeMesh(TEXT("StaticWorldMesh"), WorldRoot);
    WaterAndMudMesh   = MakeMesh(TEXT("WaterAndMudMesh"), WorldRoot);
    PropsAndDecorMesh = MakeMesh(TEXT("PropsAndDecorMesh"), WorldRoot);
    PickupsMesh       = MakeMesh(TEXT("PickupsMesh"), WorldRoot);
}

void ACapiCompleteWorld::BeginPlay()
{
    Super::BeginPlay();
    PopulateTrilhaLevelData();
    BuildEntireLevelGeometry();
    RebuildPickupsMesh();
    BuildCapiArticulatedModel();
    BuildCharactersAndEnemies();
}

void ACapiCompleteWorld::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    PlayerInputComponent->BindAxis("MoveRight", this, &ACapiCompleteWorld::InputMoveX);
    PlayerInputComponent->BindAxis("MoveForward", this, &ACapiCompleteWorld::InputMoveZ);
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACapiCompleteWorld::PressJump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACapiCompleteWorld::ReleaseJump);
    PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &ACapiCompleteWorld::PressMochilada);
    PlayerInputComponent->BindAction("Dodge", IE_Pressed, this, &ACapiCompleteWorld::PressDodge);
}

void ACapiCompleteWorld::AppendColoredBox(UDynamicMeshComponent* Comp, FVector CenterB, FVector SizeB, FLinearColor Color)
{
    UDynamicMesh* TempMesh = NewObject<UDynamicMesh>(this);
    FGeometryScriptPrimitiveOptions Opts;
    FVector UESize = CapiCoord::Size(SizeB.X, SizeB.Y, SizeB.Z);
    FVector UECenter = CapiCoord::Pos(CenterB.X, CenterB.Y - SizeB.Y * 0.5f, CenterB.Z);
    UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendBox(
        TempMesh, Opts, FTransform(UECenter), UESize.X, UESize.Y, UESize.Z);
    FGeometryScriptColorFlags ColorFlags;
    UGeometryScriptLibrary_MeshVertexColorFunctions::SetMeshConstantVertexColor(TempMesh, Color, ColorFlags, true);
    UGeometryScriptLibrary_MeshBasicEditFunctions::AppendMesh(Comp->GetDynamicMesh(), TempMesh, FTransform::Identity);
}

void ACapiCompleteWorld::PopulateTrilhaLevelData()
{
    auto AddBox = [&](FString Id, float x0, float x1, float y0, float y1, float z0, float z1, ECapiBoxKind Kind) {
        LevelBoxes.Add({Id, x0, x1, y0, y1, z0, z1, Kind, false});
    };
    auto AddWalls = [&](FString Id, float z0, float z1, float top, float x = 7.f) {
        AddBox(Id + TEXT("_L"), -x - 1.f, -x, -3.f, top, z0, z1, ECapiBoxKind::Cliff);
        AddBox(Id + TEXT("_R"), x, x + 1.f, -3.f, top, z0, z1, ECapiBoxKind::Cliff);
    };

    AddBox(TEXT("gA"), -7, 7, -3, 0, -4, 48, ECapiBoxKind::Ground);
    AddBox(TEXT("wBack"), -8, 8, -3, 2.2f, -5, -4, ECapiBoxKind::Cliff);
    AddWalls(TEXT("wAB"), -4, 48, 2.2f);
    AddBox(TEXT("root1"), -7, 7, 0, 0.6f, 15, 15.7f, ECapiBoxKind::Root);
    AddBox(TEXT("gateB_L"), -7, -1.3f, 0, 2.6f, 27.4f, 28.6f, ECapiBoxKind::Rock);
    AddBox(TEXT("gateB_R"), 1.3f, 7, 0, 2.6f, 27.4f, 28.6f, ECapiBoxKind::Rock);

    AddBox(TEXT("p1"), -1.6f, 1.6f, -0.6f, 0, 50, 53, ECapiBoxKind::Wood);
    AddBox(TEXT("p2"), -1.6f, 1.6f, -0.6f, 0, 55.2f, 58, ECapiBoxKind::Wood);
    AddBox(TEXT("p3"), -1.6f, 1.6f, -0.6f, 0, 67.5f, 70.5f, ECapiBoxKind::Wood);
    AddBox(TEXT("s1"), -5.4f, -3.6f, 0, 0.6f, 68, 70, ECapiBoxKind::Wood);
    AddBox(TEXT("s2"), -9.4f, -7.4f, 0.6f, 1.2f, 68, 70, ECapiBoxKind::Wood);
    AddBox(TEXT("gC2"), -7, 7, -3, 0, 77, 94, ECapiBoxKind::Ground);
    AddBox(TEXT("wC2_L"), -8, -7, -3, 2.2f, 77, 106.5f, ECapiBoxKind::Cliff);
    AddBox(TEXT("wC2_R"), 7, 8, -3, 2.2f, 77, 112, ECapiBoxKind::Cliff);
    AddBox(TEXT("cp1rock"), -7, -2, 0, 1.8f, 84, 88, ECapiBoxKind::Rock);

    AddBox(TEXT("pool1floor"), -7, 7, -3, -2.2f, 94, 106, ECapiBoxKind::PoolFloor);
    AddBox(TEXT("gIsland"), -7, 7, -3, 0, 106, 112, ECapiBoxKind::Ground);
    AddBox(TEXT("wIslandL2"), -8, -7, -3, 2.2f, 111.5f, 112, ECapiBoxKind::Cliff);
    AddBox(TEXT("gNook"), -15, -7, -3, 0, 106.5f, 111.5f, ECapiBoxKind::Ground);
    AddBox(TEXT("wNookS"), -16, -8, -3, 1.2f, 105.5f, 106.5f, ECapiBoxKind::Cliff);
    AddBox(TEXT("wNookN"), -16, -8, -3, 2.2f, 111.5f, 112.5f, ECapiBoxKind::Cliff);
    AddBox(TEXT("wNookW"), -16, -15, -3, 2.2f, 106.5f, 111.5f, ECapiBoxKind::Cliff);
    AddBox(TEXT("pool2floor"), -7, 12.5f, -3, -2.2f, 112, 124, ECapiBoxKind::PoolFloor);
    AddBox(TEXT("wD_L"), -8, -7, -3, 2.2f, 112, 124, ECapiBoxKind::Cliff);
    AddBox(TEXT("wCanalS"), 8, 13.5f, -3, 2.2f, 111, 112, ECapiBoxKind::Cliff);
    AddBox(TEXT("wCanalN"), 7, 13.5f, -3, 2.2f, 124, 125, ECapiBoxKind::Cliff);
    AddBox(TEXT("wCanalE"), 12.5f, 13.5f, -3, 2.2f, 112, 124, ECapiBoxKind::Cliff);
    AddBox(TEXT("divider"), 4.5f, 5.2f, -3, 0.5f, 112, 118, ECapiBoxKind::Rock);
    AddBox(TEXT("stone1"), -2.6f, -0.8f, -1.2f, -0.1f, 114.4f, 116.2f, ECapiBoxKind::Stone);
    AddBox(TEXT("stone2"), -0.9f, 0.9f, -1.2f, -0.1f, 118.2f, 120, ECapiBoxKind::Stone);
    AddBox(TEXT("stone3"), -2.6f, -0.8f, -1.2f, -0.1f, 121.5f, 123, ECapiBoxKind::Stone);

    AddBox(TEXT("gE"), -7, 7, -3, 0, 124, 150, ECapiBoxKind::Ground);
    AddBox(TEXT("wE_L"), -8, -7, -3, 5.2f, 124, 150, ECapiBoxKind::Cliff);
    AddBox(TEXT("wE_R"), 7, 8, -3, 5.2f, 125, 150, ECapiBoxKind::Cliff);
    AddBox(TEXT("ledge1"), -3, 3, 0, 1, 138, 142, ECapiBoxKind::Rock);
    AddBox(TEXT("ledge2"), -3, 3, 0, 2, 144, 148, ECapiBoxKind::Rock);
    AddBox(TEXT("plateau"), -7, 7, -3, 3, 150, 182, ECapiBoxKind::Ground);
    AddBox(TEXT("wP_L"), -8, -7, -3, 5.2f, 150, 182, ECapiBoxKind::Cliff);
    AddBox(TEXT("wP_R1"), 7, 8, -3, 5.2f, 150, 170, ECapiBoxKind::Cliff);
    AddBox(TEXT("wP_R2"), 7, 8, -3, 5.2f, 174, 182, ECapiBoxKind::Cliff);
    AddBox(TEXT("branchE"), -7, 7, 3.75f, 5.0f, 158, 159.6f, ECapiBoxKind::Branch);
    AddBox(TEXT("cp2rock"), -7, -2, 3, 4.8f, 163, 167, ECapiBoxKind::Rock);
    AddBox(TEXT("pillar"), 16.8f, 19.4f, 0, 3.6f, 170.7f, 173.3f, ECapiBoxKind::Rock);
    AddBox(TEXT("gate_L"), -8, -4.5f, 3, 5.2f, 181, 182, ECapiBoxKind::Rock);
    AddBox(TEXT("gate_R"), 4.5f, 8, 3, 5.2f, 181, 182, ECapiBoxKind::Rock);

    AddBox(TEXT("gF1"), -4.5f, 4.5f, 0, 3, 182, 275, ECapiBoxKind::Ground);
    AddBox(TEXT("gF2"), -4.5f, 4.5f, 0, 3, 277.4f, 371, ECapiBoxKind::Ground);
    AddBox(TEXT("gF3"), -4.5f, 4.5f, 0, 3, 373.4f, 446, ECapiBoxKind::Ground);
    AddBox(TEXT("gF4"), -4.5f, 4.5f, 0, 3, 448.4f, 470, ECapiBoxKind::Ground);
    AddBox(TEXT("wF_L"), -5.5f, -4.5f, 0, 6.5f, 182, 472.6f, ECapiBoxKind::Cliff);
    AddBox(TEXT("wF_R"), 4.5f, 5.5f, 0, 6.5f, 182, 472.6f, ECapiBoxKind::Cliff);
    AddBox(TEXT("o1log"), -4.5f, 4.5f, 3, 3.7f, 215, 215.8f, ECapiBoxKind::Log);
    AddBox(TEXT("o2branch"), -4.5f, 4.5f, 3.75f, 5.0f, 245, 246, ECapiBoxKind::Branch);
    AddBox(TEXT("o6log"), -4.5f, 4.5f, 3, 3.7f, 365, 365.8f, ECapiBoxKind::Log);
    AddBox(TEXT("o7branch"), -4.5f, 4.5f, 3.75f, 5.0f, 400, 401, ECapiBoxKind::Branch);
    AddBox(TEXT("o7rock"), 1, 4.5f, 3, 4.7f, 407, 408.5f, ECapiBoxKind::Rock);
    AddBox(TEXT("o8log"), -4.5f, 4.5f, 3, 3.7f, 440, 440.8f, ECapiBoxKind::Log);

    AddBox(TEXT("gG"), -7, 7, 0, 3, 472.6f, 500, ECapiBoxKind::Ground);
    AddBox(TEXT("wG_L"), -8, -7, 0, 5.2f, 472.6f, 500, ECapiBoxKind::Cliff);
    AddBox(TEXT("wG_R"), 7, 8, 0, 5.2f, 472.6f, 500, ECapiBoxKind::Cliff);
    AddBox(TEXT("wG_SL"), -7, -5.5f, 3, 5.2f, 472.6f, 473.2f, ECapiBoxKind::Cliff);
    AddBox(TEXT("wG_SR"), 5.5f, 7, 3, 5.2f, 472.6f, 473.2f, ECapiBoxKind::Cliff);
    AddBox(TEXT("railG"), -7, 7, 3, 3.9f, 499.4f, 500, ECapiBoxKind::Rail);

    FragileBoxes.Add({TEXT("fr1"), -1.5f, 1.5f, -0.5f, 0, 72.5f, 75, ECapiBoxKind::Wood, false});
    BreakableBoxes.Add({TEXT("galhada1"), -1.3f, 1.3f, 0, 2.4f, 27.5f, 28.5f, ECapiBoxKind::Wood, false});
    BreakableBoxes.Add({TEXT("galhada2"), -4.5f, 4.5f, 3, 4.0f, 335.f, 335.8f, ECapiBoxKind::Wood, false});

    Waters.Add({TEXT("agua1"), -7, 7, 94, 106, -0.35f, 0, 0, false});
    Waters.Add({TEXT("agua2"), -7, 5, 112, 124, -0.35f, 1.4f, 0, false});
    Waters.Add({TEXT("canal"), 5, 10.8f, 112, 124, -0.35f, 0.7f, 0, false});
    Waters.Add({TEXT("cachoeira"), 10.8f, 12.5f, 112, 124, -0.35f, 2.0f, 0, true});

    MudZones.Add(FBox(FVector(-4.5f, 2.8f, 303.f), FVector(0.5f, 3.2f, 309.f)));
    MudZones.Add(FBox(FVector(-4.5f, 2.8f, 428.f), FVector(0.5f, 3.2f, 434.f)));
    MovingPlatforms.Add({TEXT("m1"), 3.f, 0.5f, 3.f, 0.f, 0.f, 60.5f, 0.f, 0.f, 64.5f, 6.f, 0.f});
    MovingPlatforms.Add({TEXT("m2"), 2.4f, 0.5f, 2.4f, 9.6f, 3.f, 172.f, 13.4f, 3.f, 172.f, 5.f, 0.f});

    OncaHopZ = {215.4f, 245.5f, 276.2f, 335.4f, 365.4f, 372.2f, 400.5f, 440.4f, 447.2f};

    auto F = [&](float x, float y, float z) {
        FruitPositions.Add(FVector(x, y, z));
        FruitTaken.Add(false);
    };
    F(-1.5f, 0.8f, 6); F(0, 0.8f, 8); F(1.5f, 0.8f, 10);
    F(0, 1.3f, 14.2f); F(0, 1.7f, 15.35f); F(0, 1.3f, 16.5f);
    F(-2, 0.8f, 19.5f); F(2, 0.8f, 20.5f);
    F(0, 0.8f, 29.8f); F(0, 0.8f, 31); F(0, 0.8f, 32.2f);
    F(-3, 0.8f, 36); F(3, 0.8f, 36); F(-3, 0.8f, 41); F(3, 0.8f, 41);
    F(0, 0.8f, 51.5f); F(0, 0.8f, 56.6f); F(0, 0.9f, 62.5f); F(0, 0.8f, 69);
    F(-4.5f, 1.5f, 69); F(-6.4f, 2.2f, 69); F(0, 0.8f, 73.7f); F(0, 0.8f, 78.5f);
    F(0, 0.25f, 96); F(1, 0.25f, 98.5f); F(1.5f, 0.25f, 101); F(1, 0.25f, 103.5f); F(0, 0.25f, 105.2f);
    F(-2, 0.8f, 109); F(2.5f, 0.8f, 109);
    F(-8.5f, 0.8f, 109); F(-10, 0.8f, 109); F(-11.5f, 0.8f, 109);
    F(-1.7f, 0.7f, 115.3f); F(0, 0.7f, 119.1f); F(-1.7f, 0.7f, 122.2f); F(0, 0.8f, 126);
    F(0, 1.8f, 140); F(0, 2.8f, 146); F(0, 3.8f, 151.5f);
    F(-3, 3.8f, 154); F(3, 3.8f, 156); F(0, 3.35f, 158.8f); F(0, 3.8f, 169);
    F(11.5f, 4.3f, 172); F(15.7f, 4.7f, 172);
    F(0, 3.8f, 196); F(-2, 3.8f, 204); F(0, 4.6f, 215.4f); F(2, 3.8f, 232);
    F(0, 3.35f, 245.5f); F(0, 4.7f, 276.2f); F(3, 3.8f, 300); F(3, 3.8f, 306);
    F(0, 3.8f, 350); F(0, 4.6f, 365.4f); F(-2.5f, 3.8f, 407.7f); F(0, 4.7f, 447.2f);
    F(-1.5f, 3.8f, 480); F(1.5f, 3.8f, 484);

    RegistroPositions = { FVector(-8.4f, 2.1f, 69.f), FVector(-13.f, 0.9f, 109.f), FVector(18.1f, 4.5f, 172.f) };
    RegistroTaken = { false, false, false };
}

void ACapiCompleteWorld::BuildEntireLevelGeometry()
{
    StaticWorldMesh->GetDynamicMesh()->Reset();
    WaterAndMudMesh->GetDynamicMesh()->Reset();
    PropsAndDecorMesh->GetDynamicMesh()->Reset();

    for (const FCapiBoxDef& B : LevelBoxes)
    {
        FVector Center((B.X0 + B.X1) * 0.5f, B.Y1, (B.Z0 + B.Z1) * 0.5f);
        FVector Size(B.X1 - B.X0, B.Y1 - B.Y0, B.Z1 - B.Z0);
        FLinearColor Col = CapiPalette::LightGreen;
        if (B.Kind == ECapiBoxKind::Cliff) Col = CapiPalette::Earth;
        else if (B.Kind == ECapiBoxKind::Rock || B.Kind == ECapiBoxKind::Stone) Col = CapiPalette::Rock;
        else if (B.Kind == ECapiBoxKind::Wood || B.Kind == ECapiBoxKind::Rail) Col = CapiPalette::Wood;
        else if (B.Kind == ECapiBoxKind::Root || B.Kind == ECapiBoxKind::Branch || B.Kind == ECapiBoxKind::Log) Col = CapiPalette::WoodDark;
        else if (B.Kind == ECapiBoxKind::PoolFloor) Col = CapiPalette::DarkBrown;

        AppendColoredBox(StaticWorldMesh, Center, Size, Col);
    }

    for (const FCapiWaterDef& W : Waters)
    {
        FVector Center((W.X0 + W.X1) * 0.5f, W.SurfaceY, (W.Z0 + W.Z1) * 0.5f);
        FVector Size(W.X1 - W.X0, 0.2f, W.Z1 - W.Z0);
        AppendColoredBox(WaterAndMudMesh, Center, Size, W.bDeadly ? CapiPalette::Foam : CapiPalette::Water);
    }

    AppendColoredBox(WaterAndMudMesh, FVector(-2.f, 3.05f, 306.f), FVector(5.f, 0.1f, 6.f), CapiPalette::Mud);
    AppendColoredBox(WaterAndMudMesh, FVector(-2.f, 3.05f, 431.f), FVector(5.f, 0.1f, 6.f), CapiPalette::Mud);

    AppendColoredBox(PropsAndDecorMesh, FVector(-4.f, 1.4f, 24.5f), FVector(2.2f, 1.4f, 2.2f), CapiPalette::Cloth);
    AppendColoredBox(PropsAndDecorMesh, FVector(-1.6f, 0.4f, 23.4f), FVector(0.8f, 0.4f, 0.8f), CapiPalette::Fire);
    AppendColoredBox(PropsAndDecorMesh, FVector(5.f, 1.2f, 31.5f), FVector(1.4f, 1.2f, 1.4f), CapiPalette::Wood);
    AppendColoredBox(PropsAndDecorMesh, FVector(-5.2f, 0.2f, 95.6f), FVector(1.4f, 0.5f, 2.8f), CapiPalette::WoodDark);
    AppendColoredBox(PropsAndDecorMesh, FVector(6.2f, 1.1f, 111.2f), FVector(0.8f, 1.1f, 0.2f), CapiPalette::AraraYellow);
    AppendColoredBox(PropsAndDecorMesh, FVector(0.f, 5.5f, 496.f), FVector(0.2f, 2.5f, 0.8f), CapiPalette::Gold);
    AppendColoredBox(PropsAndDecorMesh, FVector(-3.5f, 4.1f, 497.5f), FVector(0.4f, 1.1f, 0.6f), CapiPalette::DarkBrown);

    for (int32 i = 0; i < 100; ++i)
    {
        float Z = -2.f + i * 5.0f;
        float SideX = (i % 2 == 0) ? -9.2f : 9.2f;
        float BaseY = (Z > 124.f) ? ((Z > 182.f && Z < 472.f) ? 6.5f : 5.2f) : 2.2f;
        AppendColoredBox(PropsAndDecorMesh, FVector(SideX, BaseY + 2.2f, Z), FVector(0.7f, 2.2f, 0.7f), CapiPalette::WoodDark);
        AppendColoredBox(PropsAndDecorMesh, FVector(SideX, BaseY + 4.2f, Z), FVector(2.8f, 2.2f, 2.8f), (i % 3 == 0) ? CapiPalette::Leaf : CapiPalette::DeepGreen);
    }

    UGeometryScriptLibrary_MeshNormalsFunctions::RecomputeNormals(StaticWorldMesh->GetDynamicMesh(), FGeometryScriptCalculateNormalsOptions());
    UGeometryScriptLibrary_MeshNormalsFunctions::RecomputeNormals(WaterAndMudMesh->GetDynamicMesh(), FGeometryScriptCalculateNormalsOptions());
    UGeometryScriptLibrary_MeshNormalsFunctions::RecomputeNormals(PropsAndDecorMesh->GetDynamicMesh(), FGeometryScriptCalculateNormalsOptions());
}

void ACapiCompleteWorld::RebuildPickupsMesh()
{
    PickupsMesh->GetDynamicMesh()->Reset();
    for (int32 i = 0; i < FruitPositions.Num(); ++i)
    {
        if (!FruitTaken[i])
        {
            AppendColoredBox(PickupsMesh, FruitPositions[i], FVector(0.42f, 0.42f, 0.42f), CapiPalette::Coral);
        }
    }
    for (int32 i = 0; i < RegistroPositions.Num(); ++i)
    {
        if (!RegistroTaken[i])
        {
            AppendColoredBox(PickupsMesh, RegistroPositions[i], FVector(0.55f, 0.65f, 0.18f), CapiPalette::Paper);
        }
    }
    for (const FCapiBoxDef& Brk : BreakableBoxes)
    {
        if (!Brk.bBroken)
        {
            FVector Center((Brk.X0 + Brk.X1) * 0.5f, Brk.Y1, (Brk.Z0 + Brk.Z1) * 0.5f);
            FVector Size(Brk.X1 - Brk.X0, Brk.Y1 - Brk.Y0, Brk.Z1 - Brk.Z0);
            AppendColoredBox(PickupsMesh, Center, Size, CapiPalette::WoodDark);
        }
    }
    UGeometryScriptLibrary_MeshNormalsFunctions::RecomputeNormals(PickupsMesh->GetDynamicMesh(), FGeometryScriptCalculateNormalsOptions());
}

void ACapiCompleteWorld::BuildCapiArticulatedModel()
{
    FGeometryScriptPrimitiveOptions Opts;
    FGeometryScriptColorFlags CFlags;

    UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendCapsule(CapiTorso->GetDynamicMesh(), Opts, FTransform(FVector(0, 0, 42)), 26.f, 38.f);
    UGeometryScriptLibrary_MeshVertexColorFunctions::SetMeshConstantVertexColor(CapiTorso->GetDynamicMesh(), CapiPalette::Caramel, CFlags, true);

    UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendBox(CapiHead->GetDynamicMesh(), Opts, FTransform(FVector(12, 0, 82)), 38.f, 30.f, 28.f);
    UGeometryScriptLibrary_MeshVertexColorFunctions::SetMeshConstantVertexColor(CapiHead->GetDynamicMesh(), CapiHead->GetDynamicMesh() ? CapiPalette::Caramel : CapiPalette::Caramel, CFlags, true);

    UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendCylinder(CapiHat->GetDynamicMesh(), Opts, FTransform(FVector(8, 0, 102)), 26.f, 6.f);
    UGeometryScriptLibrary_MeshVertexColorFunctions::SetMeshConstantVertexColor(CapiHat->GetDynamicMesh(), CapiPalette::Hat, CFlags, true);

    UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendBox(CapiBackpack->GetDynamicMesh(), Opts, FTransform(FVector(-24, 0, 48)), 22.f, 30.f, 32.f);
    UGeometryScriptLibrary_MeshVertexColorFunctions::SetMeshConstantVertexColor(CapiBackpack->GetDynamicMesh(), CapiPalette::Olive, CFlags, true);

    UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendCapsule(CapiArmL->GetDynamicMesh(), Opts, FTransform(FVector(0, -24, 45)), 8.f, 24.f);
    UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendCapsule(CapiArmR->GetDynamicMesh(), Opts, FTransform(FVector(0, 24, 45)), 8.f, 24.f);
    UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendCapsule(CapiLegL->GetDynamicMesh(), Opts, FTransform(FVector(0, -14, 15)), 10.f, 20.f);
    UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendCapsule(CapiLegR->GetDynamicMesh(), Opts, FTransform(FVector(0, 14, 15)), 10.f, 20.f);
    UGeometryScriptLibrary_MeshVertexColorFunctions::SetMeshConstantVertexColor(CapiArmL->GetDynamicMesh(), CapiPalette::Caramel, CFlags, true);
    UGeometryScriptLibrary_MeshVertexColorFunctions::SetMeshConstantVertexColor(CapiArmR->GetDynamicMesh(), CapiPalette::Caramel, CFlags, true);
    UGeometryScriptLibrary_MeshVertexColorFunctions::SetMeshConstantVertexColor(CapiLegL->GetDynamicMesh(), CapiPalette::DarkBrown, CFlags, true);
    UGeometryScriptLibrary_MeshVertexColorFunctions::SetMeshConstantVertexColor(CapiLegR->GetDynamicMesh(), CapiPalette::DarkBrown, CFlags, true);
}

void ACapiCompleteWorld::BuildCharactersAndEnemies()
{
    AppendColoredBox(AraraoCP1Mesh, FVector(3.7f, 1.4f, 86.7f), FVector(0.6f, 1.4f, 0.6f), CapiPalette::AraraBlue);
    AppendColoredBox(AraraoCP1Mesh, FVector(3.7f, 2.3f, 86.7f), FVector(0.4f, 0.4f, 0.4f), CapiPalette::AraraYellow);

    AppendColoredBox(AraraoCP2Mesh, FVector(3.7f, 4.4f, 165.7f), FVector(0.6f, 1.4f, 0.6f), CapiPalette::AraraBlue);
    AppendColoredBox(AraraoCP2Mesh, FVector(3.7f, 5.3f, 165.7f), FVector(0.4f, 0.4f, 0.4f), CapiPalette::AraraYellow);

    AppendColoredBox(Mosquito1Mesh, FVector(0, 0.4f, 0), FVector(0.6f, 0.4f, 0.7f), CapiPalette::Mosquito);
    AppendColoredBox(Mosquito2Mesh, FVector(0, 0.4f, 0), FVector(0.6f, 0.4f, 0.7f), CapiPalette::Mosquito);

    AppendColoredBox(JacareMesh, FVector(0, 0.4f, 0), FVector(0.9f, 0.45f, 2.2f), CapiPalette::Jacare);

    AppendColoredBox(GrandeOncaMesh, FVector(0, 1.1f, 0), FVector(1.1f, 1.1f, 2.4f), CapiPalette::Onca);
}

void ACapiCompleteWorld::PressJump() { bJumpHeld = true; JumpBufferTimer = 0.12f; }
void ACapiCompleteWorld::ReleaseJump() { bJumpHeld = false; if (CapiVelB.Y > 3.f) CapiVelB.Y = 3.f; }
void ACapiCompleteWorld::PressMochilada() { if (State != ECapiPlayerState::Attacking) { State = ECapiPlayerState::Attacking; StateTimer = 0.45f; } }
void ACapiCompleteWorld::PressDodge() { if (DodgeCooldown <= 0.f) { State = ECapiPlayerState::Dodging; StateTimer = 0.32f; DodgeCooldown = 0.7f; } }

void ACapiCompleteWorld::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (bLevelFinished) return;
    float ClampedDt = FMath::Min(DeltaTime, 0.05f);
    StepSimulation(ClampedDt);
    UpdateEnemiesAndChase(ClampedDt);
    UpdateCameraZones(ClampedDt);
    AnimateAllModels(ClampedDt);
}

void ACapiCompleteWorld::StepSimulation(float Dt)
{
    if (CoyoteTimer > 0.f) CoyoteTimer -= Dt;
    if (JumpBufferTimer > 0.f) JumpBufferTimer -= Dt;
    if (DodgeCooldown > 0.f) DodgeCooldown -= Dt;
    if (InvulnTimer > 0.f) InvulnTimer -= Dt;
    if (StateTimer > 0.f)
    {
        StateTimer -= Dt;
        if (StateTimer <= 0.f) State = ECapiPlayerState::Grounded;
    }

    if (CapiPosB.Z >= 184.f && CapiPosB.Z < 473.f)
    {
        if (!bChaseActive) { bChaseActive = true; OncaZ = CapiPosB.Z - 10.f; }
    }
    else bChaseActive = false;

    float MudMult = 1.0f;
    for (const FBox& Mud : MudZones)
    {
        if (Mud.IsInside(CapiPosB)) { MudMult = 0.6f; break; }
    }

    float MaxSpd = ((State == ECapiPlayerState::Dodging) ? 8.5f : (bChaseActive ? 6.5f : 5.5f)) * MudMult;
    float ForwardInput = bChaseActive ? 1.0f : InputZ;
    CapiVelB.X = InputX * MaxSpd;
    CapiVelB.Z = ForwardInput * MaxSpd;

    if (JumpBufferTimer > 0.f && CoyoteTimer > 0.f)
    {
        CapiVelB.Y = 8.0f;
        JumpBufferTimer = 0.f;
        CoyoteTimer = 0.f;
    }

    float Grav = (CapiVelB.Y > 0.f && bJumpHeld) ? 22.f : 28.f;
    CapiVelB.Y = FMath::Max(-20.f, CapiVelB.Y - Grav * Dt);

    float Height = (State == ECapiPlayerState::Dodging) ? 0.6f : 1.1f;
    ResolveCylinderCollisionSwept(Dt, 0.35f, Height);

    bool bNeedsPickupRebuild = false;
    if (State == ECapiPlayerState::Attacking)
    {
        for (FCapiBoxDef& Brk : BreakableBoxes)
        {
            if (!Brk.bBroken && FMath::Abs(CapiPosB.Z - (Brk.Z0 + Brk.Z1) * 0.5f) < 1.8f)
            {
                Brk.bBroken = true;
                bNeedsPickupRebuild = true;
            }
        }
    }

    if (FVector::Dist2D(FVector(CapiPosB.X, CapiPosB.Z, 0), FVector(2.f, 86.f, 0)) < 3.6f && ActiveCheckpointIdx < 0)
    {
        ActiveCheckpointIdx = 0; Hp = 3; RespawnPosB = FVector(1.5f, 0.f, 83.f);
        State = ECapiPlayerState::CheckpointHighFive; StateTimer = 1.2f;
    }
    if (FVector::Dist2D(FVector(CapiPosB.X, CapiPosB.Z, 0), FVector(2.f, 165.f, 0)) < 3.6f && ActiveCheckpointIdx < 1)
    {
        ActiveCheckpointIdx = 1; Hp = 3; RespawnPosB = FVector(1.5f, 3.f, 162.3f);
        State = ECapiPlayerState::CheckpointHighFive; StateTimer = 1.2f;
    }

    for (int32 i = 0; i < FruitPositions.Num(); ++i)
    {
        if (!FruitTaken[i] && FVector::Dist(CapiPosB, FruitPositions[i]) <= 0.95f)
        {
            FruitTaken[i] = true;
            FruitsCollected++;
            bNeedsPickupRebuild = true;
        }
    }
    for (int32 i = 0; i < RegistroPositions.Num(); ++i)
    {
        if (!RegistroTaken[i] && FVector::Dist(CapiPosB, RegistroPositions[i]) <= 1.1f)
        {
            RegistroTaken[i] = true;
            RegistrosCollected++;
            bNeedsPickupRebuild = true;
        }
    }
    if (bNeedsPickupRebuild) RebuildPickupsMesh();

    if (FVector::Dist2D(FVector(CapiPosB.X, CapiPosB.Z, 0), FVector(0.f, 491.f, 0)) <= 3.2f)
    {
        bLevelFinished = true;
        State = ECapiPlayerState::Celebrate;
    }

    if (CapiPosB.Y < -7.f)
    {
        CapiPosB = RespawnPosB;
        CapiVelB = FVector::ZeroVector;
        Hp = 3;
    }

    CapiRoot->SetWorldLocation(CapiCoord::Pos(CapiPosB.X, CapiPosB.Y, CapiPosB.Z));
}

void ACapiCompleteWorld::ResolveCylinderCollisionSwept(float Dt, float Radius, float Height)
{
    FVector NextPos = CapiPosB + CapiVelB * Dt;

    for (const FCapiWaterDef& W : Waters)
    {
        if (NextPos.X >= W.X0 && NextPos.X <= W.X1 && NextPos.Z >= W.Z0 && NextPos.Z <= W.Z1)
        {
            if (W.bDeadly && NextPos.Y <= W.SurfaceY + 0.2f)
            {
                CapiPosB = RespawnPosB; CapiVelB = FVector::ZeroVector; return;
            }
            if (NextPos.Y < W.SurfaceY - 0.55f)
            {
                NextPos.Y = W.SurfaceY - 0.55f;
                CapiVelB.Y = 0.f;
                CoyoteTimer = 0.12f;
                NextPos.X += W.CurrentX * Dt;
            }
        }
    }

    for (const FCapiBoxDef& Brk : BreakableBoxes)
    {
        if (!Brk.bBroken && NextPos.X + Radius > Brk.X0 && NextPos.X - Radius < Brk.X1 &&
            NextPos.Z + Radius > Brk.Z0 && NextPos.Z - Radius < Brk.Z1 &&
            NextPos.Y < Brk.Y1 && NextPos.Y + Height > Brk.Y0)
        {
            if (CapiPosB.Z + Radius <= Brk.Z0) NextPos.Z = Brk.Z0 - Radius;
        }
    }

    for (const FCapiBoxDef& B : LevelBoxes)
    {
        if (NextPos.X + Radius > B.X0 && NextPos.X - Radius < B.X1 &&
            NextPos.Z + Radius > B.Z0 && NextPos.Z - Radius < B.Z1)
        {
            if (CapiPosB.Y >= B.Y1 - 0.35f && NextPos.Y <= B.Y1 && CapiVelB.Y <= 0.f)
            {
                NextPos.Y = B.Y1;
                CapiVelB.Y = 0.f;
                CoyoteTimer = 0.12f;
            }
            else if (NextPos.Y < B.Y1 && NextPos.Y + Height > B.Y0)
            {
                if (CapiPosB.Z + Radius <= B.Z0) NextPos.Z = B.Z0 - Radius;
                else if (CapiPosB.Z - Radius >= B.Z1) NextPos.Z = B.Z1 + Radius;
                if (CapiPosB.X + Radius <= B.X0) NextPos.X = B.X0 - Radius;
                else if (CapiPosB.X - Radius >= B.X1) NextPos.X = B.X1 + Radius;
            }
        }
    }

    CapiPosB = NextPos;
}

void ACapiCompleteWorld::UpdateEnemiesAndChase(float Dt)
{
    AnimPhase += Dt;
    Mosquito1Mesh->SetWorldLocation(CapiCoord::Pos(FMath::Sin(AnimPhase * 2.f) * 2.5f, 1.1f, 38.5f));
    Mosquito2Mesh->SetWorldLocation(CapiCoord::Pos(FMath::Cos(AnimPhase * 2.f) * 2.5f, 4.1f, 154.f));

    float JacZ = FMath::Lerp(113.6f, 122.4f, (FMath::Sin(AnimPhase * 1.5f) + 1.f) * 0.5f);
    JacareMesh->SetWorldLocation(CapiCoord::Pos(8.f, -0.35f, JacZ));

    if (bChaseActive && OncaZ < 467.5f)
    {
        float Gap = CapiPosB.Z - OncaZ;
        float OncaSpd = (Gap > 10.f) ? 9.0f : 6.4f;
        OncaZ += OncaSpd * Dt;

        float HopOffset = 0.f;
        for (float HZ : OncaHopZ)
        {
            float DistToHop = FMath::Abs(OncaZ - HZ);
            if (DistToHop < 2.2f) HopOffset = FMath::Cos((DistToHop / 2.2f) * PI * 0.5f) * 1.6f;
        }
        GrandeOncaMesh->SetWorldLocation(CapiCoord::Pos(CapiPosB.X * 0.7f, 3.0f + HopOffset, OncaZ));

        if (Gap <= 1.3f)
        {
            CapiPosB = RespawnPosB; CapiVelB = FVector::ZeroVector; bChaseActive = false;
        }
    }
}

void ACapiCompleteWorld::UpdateCameraZones(float Dt)
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return;

    FVector TargetWorld = CapiRoot->GetComponentLocation();
    FVector CamOffset(-650.f, 0.f, 320.f);
    if (CapiPosB.Z >= 47.f && CapiPosB.Z <= 77.5f)
    {
        CamOffset = FVector(0.f, -950.f, 260.f);
    }
    else if (bChaseActive)
    {
        CamOffset = FVector(720.f, 0.f, 300.f);
    }

    FRotator LookRot = (TargetWorld - (TargetWorld + CamOffset)).Rotation();
    PC->SetControlRotation(LookRot);
}

void ACapiCompleteWorld::AnimateAllModels(float Dt)
{
    float WalkSwing = FMath::Sin(AnimPhase * 12.f) * 35.f * FMath::Clamp(CapiVelB.Size2D() / 5.5f, 0.f, 1.f);
    CapiLegL->SetRelativeRotation(FRotator(WalkSwing, 0, 0));
    CapiLegR->SetRelativeRotation(FRotator(-WalkSwing, 0, 0));
    CapiArmL->SetRelativeRotation(FRotator(-WalkSwing, 0, 0));
    CapiArmR->SetRelativeRotation(FRotator(WalkSwing, 0, 0));

    if (State == ECapiPlayerState::Attacking)
    {
        CapiTorso->SetRelativeRotation(FRotator(0, AnimPhase * 1800.f, 0));
        CapiBackpack->SetRelativeScale3D(FVector(1.45f));
    }
    else if (State == ECapiPlayerState::CheckpointHighFive)
    {
        CapiArmR->SetRelativeRotation(FRotator(110.f, 0, 25.f));
    }
    else
    {
        CapiTorso->SetRelativeRotation(FRotator::ZeroRotator);
        CapiBackpack->SetRelativeScale3D(FVector(1.0f));
    }
}
