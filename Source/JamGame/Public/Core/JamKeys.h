#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"

/**
 * The fight's key bindings in one place (T-096): the player controller reads these, and the HOW TO PLAY screen
 * (JamHowToRules) labels the same ones, so the screen cannot drift from the real controls. Functions, not globals: the
 * engine's keys are statics of another module.
 */
namespace JamKeys
{
inline FKey Up()
{
	return EKeys::W;
}
inline FKey Left()
{
	return EKeys::A;
}
inline FKey Down()
{
	return EKeys::S;
}
inline FKey Right()
{
	return EKeys::D;
}
inline FKey Dash()
{
	return EKeys::SpaceBar;
}
inline FKey Ultimate()
{
	return EKeys::E;
}
/** After a fight ends: a retry after a defeat, a new fight after a win (Enter does the same). */
inline FKey Retry()
{
	return EKeys::R;
}
/** The HOW TO PLAY screen during the fight, which waits under it. */
inline FKey HowTo()
{
	return EKeys::F1;
}
/** The katana; a click just before an enemy shot lands parries it. */
inline FKey Katana()
{
	return EKeys::LeftMouseButton;
}
inline FKey Shurikens()
{
	return EKeys::RightMouseButton;
}
/** The two options of the steal card, under the left hand; a click on an option picks it too. */
inline FKey CardLeft()
{
	return EKeys::A;
}
inline FKey CardRight()
{
	return EKeys::D;
}
/** The aim follows the cursor. */
inline FKey Aim()
{
	return EKeys::Mouse2D;
}
} // namespace JamKeys
