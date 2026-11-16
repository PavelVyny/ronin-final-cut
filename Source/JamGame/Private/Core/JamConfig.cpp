#include "Core/JamConfig.h"

#include "JamGame.h"
#include "Misc/ConfigCacheIni.h"
#include "UObject/UnrealType.h"

void JamConfig::LoadStruct(const UScriptStruct& Struct, void* Data, const TCHAR* Section, TArray<FString>* OutMissing)
{
	for (TFieldIterator<FProperty> It(&Struct); It; ++It)
	{
		const FProperty* Property = *It;
		const FString Name = Property->GetName();
		FString Text;
		const bool bFound = GConfig && GConfig->GetString(Section, *Name, Text, GGameIni);
		const bool bRead = bFound && Property->ImportText_Direct(*Text, Property->ContainerPtrToValuePtr<void>(Data), nullptr, PPF_None);
		if (!bRead)
		{
			UE_LOG(LogJamGame, Error, TEXT("[%s] %s is missing or unreadable in DefaultGame.ini"), Section, *Name);
			if (OutMissing)
			{
				OutMissing->Add(Name);
			}
		}
	}
}
