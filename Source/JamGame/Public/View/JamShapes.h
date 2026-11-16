#pragma once

#include "CoreMinimal.h"

class UMaterialInterface;
class UStaticMesh;
class UStaticMeshComponent;

/**
 * The engine's basic shapes, which the whole prototype is drawn with until the art arrives ("build from the bean up").
 * Every shape is 100 cm across, so a scale of Size / 100 gives a size in cm.
 */
namespace JamShapes
{
inline constexpr float UnitSize = 100.f;

/** To be called from an actor constructor only: they use ConstructorHelpers. */
JAMGAME_API UStaticMesh* Sphere();
JAMGAME_API UStaticMesh* Cylinder();
JAMGAME_API UStaticMesh* Cube();
JAMGAME_API UStaticMesh* Cone();
JAMGAME_API UMaterialInterface* Material();
/** The engine's grey world grid: the look of a game that has no materials yet. */
JAMGAME_API UMaterialInterface* Grid();
/** Unlit and all emission (Tools/Editor/create_neon_material.py): the neon of the stand-ins. */
JAMGAME_API UMaterialInterface* Neon();

/** The generated props (T-062, Tools/Editor/import_models.py). Constructor only, like the shapes. */
JAMGAME_API UStaticMesh* Shuriken();
JAMGAME_API UStaticMesh* CashStack();
/** Paper with a picture (M_JamCard instances): the Publisher's memo and a dollar bill. */
JAMGAME_API UMaterialInterface* MemoCard();
JAMGAME_API UMaterialInterface* DollarCard();

/** Sets up a component as a piece of scenery: the mesh, the plain material, no collision. Constructor only. */
JAMGAME_API void Setup(UStaticMeshComponent& Component, UStaticMesh* Mesh);

/** Gives the component its own colour. Not for constructors: it creates a dynamic material instance. */
JAMGAME_API void Paint(UStaticMeshComponent& Component, const FLinearColor& Color);

/** Makes the component shine in this colour, Strength times; above 1 it blooms. Not for constructors. */
JAMGAME_API void Glow(UStaticMeshComponent& Component, const FLinearColor& Color, float Strength);

/** Dresses the component in a card material (MemoCard, DollarCard), Glow times bright. Not for constructors. */
JAMGAME_API void Card(UStaticMeshComponent& Component, UMaterialInterface* Card, float Glow);

/** Paints the component, or puts the grey grid on it when bGrid. Not for constructors. */
JAMGAME_API void Look(UStaticMeshComponent& Component, const FLinearColor& Color, bool bGrid);
} // namespace JamShapes
