#include "UI/JamHowToRules.h"

#include "Core/JamKeys.h"

namespace
{
// The schematic's sizes, in key units: a unit is this share of the screen's height.
constexpr double UnitShare = 0.045;
constexpr double BoardUnits = 15.0;
constexpr double BoardTopShare = 0.36;
constexpr double KeyInset = 0.08;
constexpr double MouseGap = 2.0;
constexpr double MouseWidth = 3.6;
constexpr double MouseHeight = 5.4;
// From the board's top to the mouse's: it stands centred on the board's six rows.
constexpr double MouseDrop = 0.3;
// The parts of the mouse's picture (Art/HowTo/howto_mouse_a.png, 2:3 like the box), as shares of its box: the
// buttons' top and bottom, the left button's sides, the right one's, and the body's sides and bottom.
constexpr double MouseButtonsTop = 0.075;
constexpr double MouseButtonsBottom = 0.405;
constexpr double MouseLeftButton[] = { 0.17, 0.44 };
constexpr double MouseRightButton[] = { 0.56, 0.83 };
constexpr double MouseBody[] = { 0.126, 0.872 };
constexpr double MouseBodyBottom = 0.9375;
// From the board to a row of labels, between labels, from the mouse to its labels; the screen's side margin.
constexpr double RowGap = 1.2;
constexpr double LabelGap = 0.6;
constexpr double ColumnGap = 0.8;
// The first label of the mouse's column is centred this far over the mouse's top.
constexpr double ColumnLift = 1.0;
constexpr double MarginShare = 0.02;

/** One key of the drawn board: the engine's key where it matters to us, its cap and its width in units. */
struct FBoardKey
{
	FKey Key;
	const TCHAR* Cap;
	double Width;
};

/** A gap in a row, as wide as Width units. */
FBoardKey Gap(double Width)
{
	return { FKey(), nullptr, Width };
}

FBoardKey Cap(const TCHAR* Text, double Width = 1.0)
{
	return { FKey(), Text, Width };
}

FBoardKey Bound(const FKey& Key, double Width = 1.0)
{
	return { Key, nullptr, Width };
}

/** A 15-unit keyboard, the function row first. */
TArray<TArray<FBoardKey>> Board()
{
	return {
		{ Cap(TEXT("ESC")), Gap(1.0), Bound(JamKeys::HowTo()), Cap(TEXT("F2")), Cap(TEXT("F3")), Cap(TEXT("F4")), Gap(0.5), Cap(TEXT("F5")),
			Cap(TEXT("F6")), Cap(TEXT("F7")), Cap(TEXT("F8")), Gap(0.5), Cap(TEXT("F9")), Cap(TEXT("F10")), Cap(TEXT("F11")),
			Cap(TEXT("F12")) },
		{ Cap(TEXT("`")), Cap(TEXT("1")), Cap(TEXT("2")), Cap(TEXT("3")), Cap(TEXT("4")), Cap(TEXT("5")), Cap(TEXT("6")), Cap(TEXT("7")),
			Cap(TEXT("8")), Cap(TEXT("9")), Cap(TEXT("0")), Cap(TEXT("-")), Cap(TEXT("=")), Cap(TEXT("BACK"), 2.0) },
		{ Cap(TEXT("TAB"), 1.5), Cap(TEXT("Q")), Bound(JamKeys::Up()), Bound(JamKeys::Ultimate()), Bound(JamKeys::Retry()), Cap(TEXT("T")),
			Cap(TEXT("Y")), Cap(TEXT("U")), Cap(TEXT("I")), Cap(TEXT("O")), Cap(TEXT("P")), Cap(TEXT("[")), Cap(TEXT("]")),
			Cap(TEXT("\\"), 1.5) },
		{ Cap(TEXT("CAPS"), 1.75), Bound(JamKeys::Left()), Bound(JamKeys::Down()), Bound(JamKeys::Right()), Cap(TEXT("F")), Cap(TEXT("G")),
			Cap(TEXT("H")), Cap(TEXT("J")), Cap(TEXT("K")), Cap(TEXT("L")), Cap(TEXT(";")), Cap(TEXT("'")), Cap(TEXT("ENTER"), 2.25) },
		{ Cap(TEXT("SHIFT"), 2.25), Cap(TEXT("Z")), Cap(TEXT("X")), Cap(TEXT("C")), Cap(TEXT("V")), Cap(TEXT("B")), Cap(TEXT("N")),
			Cap(TEXT("M")), Cap(TEXT(",")), Cap(TEXT(".")), Cap(TEXT("/")), Cap(TEXT("SHIFT"), 2.75) },
		{ Cap(TEXT("CTRL"), 1.5), Cap(TEXT("WIN"), 1.25), Cap(TEXT("ALT"), 1.25), Bound(JamKeys::Dash(), 5.75), Cap(TEXT("ALT"), 1.25),
			Cap(TEXT("FN"), 1.25), Cap(TEXT("MENU"), 1.25), Cap(TEXT("CTRL"), 1.5) },
	};
}

/** Where on a key a line to a label in that direction starts: its top or bottom edge, or its middle. */
FVector2D LineStart(const FBox2D& Key, EJamHowToSide Side)
{
	const FVector2D Centre = Key.GetCenter();
	switch (Side)
	{
		case EJamHowToSide::Above:
			return FVector2D(Centre.X, Key.Min.Y);
		case EJamHowToSide::Below:
			return FVector2D(Centre.X, Key.Max.Y);
		default:
			return FVector2D(Key.Max.X, Centre.Y);
	}
}

/** Lays the labels of a row side by side in the order of their keys, centred over them, inside the margins. */
void PlaceRow(FJamHowToLayout& Out, const TArray<int32>& Row, const TArray<double>& Anchors, const TArray<FVector2D>& Sizes, double Y,
	bool bAbove, double Gap, double Left, double Right)
{
	double Total = Gap * FMath::Max(0, Row.Num() - 1);
	double Mean = 0.0;
	for (const int32 Index : Row)
	{
		Total += Sizes[Index].X;
		Mean += Anchors[Index] / Row.Num();
	}
	double X = FMath::Clamp(Mean - 0.5 * Total, Left, FMath::Max(Left, Right - Total));
	for (const int32 Index : Row)
	{
		const double Top = bAbove ? Y - Sizes[Index].Y : Y;
		Out.Labels[Index] = FBox2D(FVector2D(X, Top), FVector2D(X + Sizes[Index].X, Top + Sizes[Index].Y));
		X += Sizes[Index].X + Gap;
	}
}

/** The board's keys and the mouse's parts, laid out from Origin in units of Unit. */
void PlaceKeys(FJamHowToLayout& Out, const FVector2D& Origin, double Unit)
{
	double Y = Origin.Y;
	for (const TArray<FBoardKey>& Row : Board())
	{
		double X = Origin.X;
		for (const FBoardKey& Key : Row)
		{
			if (Key.Key.IsValid() || Key.Cap)
			{
				FJamHowToKey Drawn;
				Drawn.Key = Key.Key;
				Drawn.Cap = Key.Cap ? FString(Key.Cap) : JamHowToRules::CapOf(Key.Key);
				Drawn.Box = FBox2D(FVector2D(X, Y) + KeyInset * Unit, FVector2D(X + Key.Width * Unit, Y + Unit) - KeyInset * Unit);
				Out.Keys.Add(Drawn);
			}
			X += Key.Width * Unit;
		}
		Y += Unit;
	}
	Out.Board = FBox2D(Origin, FVector2D(Origin.X + BoardUnits * Unit, Y));
	const FVector2D MouseAt(Out.Board.Max.X + MouseGap * Unit, Origin.Y + MouseDrop * Unit);
	Out.Mouse = FBox2D(MouseAt, MouseAt + FVector2D(MouseWidth, MouseHeight) * Unit);
	const FVector2D Size = Out.Mouse.GetSize();
	const auto Part = [&MouseAt, &Size](double Left, double Top, double Right, double Bottom)
	{ return FBox2D(MouseAt + FVector2D(Left, Top) * Size, MouseAt + FVector2D(Right, Bottom) * Size); };
	Out.Keys.Add({ JamKeys::Katana(), TEXT("L"), Part(MouseLeftButton[0], MouseButtonsTop, MouseLeftButton[1], MouseButtonsBottom) });
	Out.Keys.Add({ JamKeys::Shurikens(), TEXT("R"), Part(MouseRightButton[0], MouseButtonsTop, MouseRightButton[1], MouseButtonsBottom) });
	Out.Keys.Add({ JamKeys::Aim(), FString(), Part(MouseBody[0], MouseButtonsBottom, MouseBody[1], MouseBodyBottom) });
}

/** The mouse's labels stacked right of it, centred on it. */
void PlaceColumn(FJamHowToLayout& Out, const TArray<int32>& Right, const TArray<FVector2D>& LabelSizes, double Unit)
{
	double Y = Out.Mouse.Min.Y - ColumnLift * Unit - (Right.IsEmpty() ? 0.0 : 0.5 * LabelSizes[Right[0]].Y);
	const double X = Out.Mouse.Max.X + ColumnGap * Unit;
	for (const int32 Index : Right)
	{
		Out.Labels[Index] = FBox2D(FVector2D(X, Y), FVector2D(X, Y) + LabelSizes[Index]);
		Y += LabelSizes[Index].Y + LabelGap * Unit;
	}
}

/**
 * The drawn keys of Callout a line reaches: above or below the board only those of the row nearest the label (W of
 * W A S D), so no line crosses another row of the callout's own keys; beside the mouse every one.
 */
TArray<const FJamHowToKey*> LinedKeys(const FJamHowToLayout& Out, const FJamHowToCallout& Callout)
{
	TArray<const FJamHowToKey*> Keys;
	for (const FKey& Key : Callout.Keys)
	{
		if (const FJamHowToKey* Drawn = Out.Keys.FindByPredicate([&Key](const FJamHowToKey& K) { return K.Key == Key; }))
		{
			Keys.Add(Drawn);
		}
	}
	if (Callout.Side == EJamHowToSide::Right || Keys.IsEmpty())
	{
		return Keys;
	}
	const bool bAbove = Callout.Side == EJamHowToSide::Above;
	double Nearest = bAbove ? TNumericLimits<double>::Max() : TNumericLimits<double>::Lowest();
	for (const FJamHowToKey* Drawn : Keys)
	{
		Nearest = bAbove ? FMath::Min(Nearest, Drawn->Box.Min.Y) : FMath::Max(Nearest, Drawn->Box.Max.Y);
	}
	return Keys.FilterByPredicate([bAbove, Nearest](const FJamHowToKey* Drawn)
		{ return FMath::IsNearlyEqual(bAbove ? Drawn->Box.Min.Y : Drawn->Box.Max.Y, Nearest, 1.0); });
}

/** A line to each callout's label: up or down from its keys to the row's label, across to the mouse's column. */
void AddLines(FJamHowToLayout& Out, const TArray<FJamHowToCallout>& Callouts, double Unit)
{
	for (int32 Index = 0; Index < Callouts.Num(); ++Index)
	{
		const FBox2D& Label = Out.Labels[Index];
		const EJamHowToSide Side = Callouts[Index].Side;
		for (const FJamHowToKey* Drawn : LinedKeys(Out, Callouts[Index]))
		{
			// The mouse's buttons are left from their top edge, its body from its middle.
			const bool bBody = Drawn->Box.IsInside(Out.Mouse.GetCenter());
			const FVector2D From = LineStart(Drawn->Box, Side == EJamHowToSide::Right && !bBody ? EJamHowToSide::Above : Side);
			const double Pad = FMath::Min(0.5 * Label.GetSize().X, 0.4 * Unit);
			const FVector2D To = Side == EJamHowToSide::Right ? FVector2D(Label.Min.X, Label.GetCenter().Y)
															  : FVector2D(FMath::Clamp(From.X, Label.Min.X + Pad, Label.Max.X - Pad),
																	Side == EJamHowToSide::Above ? Label.Max.Y : Label.Min.Y);
			Out.Lines[Index].Add(TPair<FVector2D, FVector2D>(From, To));
		}
	}
}
} // namespace

FString JamHowToRules::CapOf(const FKey& Key)
{
	if (Key == EKeys::SpaceBar)
	{
		return TEXT("SPACE");
	}
	if (Key == EKeys::LeftMouseButton)
	{
		return TEXT("LEFT CLICK");
	}
	if (Key == EKeys::RightMouseButton)
	{
		return TEXT("RIGHT CLICK");
	}
	if (Key == EKeys::Mouse2D)
	{
		return TEXT("MOUSE");
	}
	return Key.GetFName().ToString().ToUpper();
}

FString JamHowToRules::Ordinal(int32 Number)
{
	const int32 Tens = Number % 100;
	const TCHAR* Suffix = TEXT("th");
	if (Tens < 11 || Tens > 13)
	{
		const int32 Last = Number % 10;
		Suffix = Last == 1 ? TEXT("st") : Last == 2 ? TEXT("nd") : Last == 3 ? TEXT("rd") : TEXT("th");
	}
	return FString::Printf(TEXT("%d%s"), Number, Suffix);
}

TArray<FJamHowToCallout> JamHowToRules::Callouts(int32 ExplosiveEvery)
{
	const auto Make = [](TArray<FKey> Keys, FString Caps, FString Label, EJamHowToSide Side)
	{
		FJamHowToCallout Callout;
		Callout.Keys = MoveTemp(Keys);
		Callout.Caps = MoveTemp(Caps);
		Callout.Label = MoveTemp(Label);
		Callout.Side = Side;
		return Callout;
	};
	const FString Move = FString::Printf(
		TEXT("%s %s %s %s"), *CapOf(JamKeys::Up()), *CapOf(JamKeys::Left()), *CapOf(JamKeys::Down()), *CapOf(JamKeys::Right()));
	const FString Card = FString::Printf(TEXT("%s / %s or a click"), *CapOf(JamKeys::CardLeft()), *CapOf(JamKeys::CardRight()));
	return {
		Make({ JamKeys::HowTo() }, CapOf(JamKeys::HowTo()), TEXT("this screen (pauses)"), EJamHowToSide::Above),
		Make({ JamKeys::Up(), JamKeys::Left(), JamKeys::Down(), JamKeys::Right() }, Move, TEXT("move"), EJamHowToSide::Above),
		Make({ JamKeys::Ultimate() }, CapOf(JamKeys::Ultimate()), TEXT("Ultimate (when full)"), EJamHowToSide::Above),
		Make({ JamKeys::Retry() }, CapOf(JamKeys::Retry()), TEXT("retry after a fight"), EJamHowToSide::Above),
		Make({ JamKeys::CardLeft(), JamKeys::CardRight() }, Card, TEXT("pick on the steal card"), EJamHowToSide::Below),
		Make({ JamKeys::Dash() }, CapOf(JamKeys::Dash()), TEXT("dash"), EJamHowToSide::Below),
		Make({ JamKeys::Katana() }, CapOf(JamKeys::Katana()), TEXT("katana; click just before a shot lands to parry it"),
			EJamHowToSide::Right),
		Make({ JamKeys::Shurikens() }, CapOf(JamKeys::Shurikens()),
			FString::Printf(TEXT("shurikens; every %s one explodes"), *Ordinal(ExplosiveEvery)), EJamHowToSide::Right),
		Make({ JamKeys::Aim() }, CapOf(JamKeys::Aim()), TEXT("aim: the ronin faces the cursor"), EJamHowToSide::Right),
	};
}

int32 JamHowToRules::SecondsLeft(float Age)
{
	return FMath::Max(0, FMath::CeilToInt32(AutoStartSeconds - Age));
}

bool JamHowToRules::IsOver(float Age, bool bAnyKey, bool bCountdown)
{
	return (bAnyKey && Age >= KeyGraceSeconds) || (bCountdown && Age >= AutoStartSeconds);
}

FJamHowToLayout JamHowToRules::Layout(
	const FVector2D& Screen, const TArray<FJamHowToCallout>& Callouts, const TArray<FVector2D>& LabelSizes)
{
	FJamHowToLayout Out;
	const double Unit = UnitShare * Screen.Y;
	const double Margin = MarginShare * Screen.X;
	double Column = 0.0;
	for (int32 Index = 0; Index < Callouts.Num(); ++Index)
	{
		if (Callouts[Index].Side == EJamHowToSide::Right && LabelSizes.IsValidIndex(Index))
		{
			Column = FMath::Max(Column, LabelSizes[Index].X);
		}
	}
	// The board, the mouse and the column of its labels, centred together.
	const double Group = (BoardUnits + MouseGap + MouseWidth + ColumnGap) * Unit + Column;
	PlaceKeys(Out, FVector2D(FMath::Max(Margin, 0.5 * (Screen.X - Group)), BoardTopShare * Screen.Y), Unit);

	Out.Labels.SetNum(Callouts.Num());
	Out.Lines.SetNum(Callouts.Num());
	TArray<double> Anchors;
	TArray<int32> Above;
	TArray<int32> Below;
	TArray<int32> Right;
	for (int32 Index = 0; Index < Callouts.Num(); ++Index)
	{
		double Sum = 0.0;
		for (const FKey& Key : Callouts[Index].Keys)
		{
			const FJamHowToKey* Drawn = Out.Keys.FindByPredicate([&Key](const FJamHowToKey& K) { return K.Key == Key; });
			if (Drawn)
			{
				Sum += Drawn->Box.GetCenter().X;
			}
		}
		Anchors.Add(Sum / FMath::Max(1, Callouts[Index].Keys.Num()));
		const EJamHowToSide Side = Callouts[Index].Side;
		(Side == EJamHowToSide::Above ? Above : Side == EJamHowToSide::Below ? Below : Right).Add(Index);
	}
	const auto ByAnchor = [&Anchors](int32 A, int32 B) { return Anchors[A] < Anchors[B]; };
	Above.StableSort(ByAnchor);
	Below.StableSort(ByAnchor);
	PlaceRow(Out, Above, Anchors, LabelSizes, Out.Board.Min.Y - RowGap * Unit, true, LabelGap * Unit, Margin, Screen.X - Margin);
	PlaceRow(Out, Below, Anchors, LabelSizes, Out.Board.Max.Y + RowGap * Unit, false, LabelGap * Unit, Margin, Screen.X - Margin);

	PlaceColumn(Out, Right, LabelSizes, Unit);
	AddLines(Out, Callouts, Unit);
	for (FJamHowToKey& Drawn : Out.Keys)
	{
		Drawn.bLit =
			Drawn.Key.IsValid() && Callouts.ContainsByPredicate([&Drawn](const FJamHowToCallout& C) { return C.Keys.Contains(Drawn.Key); });
	}
	return Out;
}
