// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/AzSpeechSettings.h"

#if WITH_EDITOR
#include "Misc/MessageDialog.h"
#endif // WITH_EDITOR

UAzSpeechSettings::UAzSpeechSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), TimeOutInSeconds(15.f)
{
	CategoryName = TEXT("Plugins");

#if ENGINE_MAJOR_VERSION >= 5
	if (AutoLanguageCandidates.IsEmpty())
#else
	if (AutoLanguageCandidates.Num() == 0)
#endif
	{
		AutoLanguageCandidates.Add(LanguageID);
	}
}

#if WITH_EDITOR
void UAzSpeechSettings::PreEditChange(FProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);

	if (PropertyAboutToChange->GetFName() == GET_MEMBER_NAME_CHECKED(UAzSpeechSettings, LanguageID))
	{
		AutoLanguageCandidates.Remove(LanguageID);
	}
}

void UAzSpeechSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UAzSpeechSettings, AutoLanguageCandidates)
		|| PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UAzSpeechSettings, LanguageID))
	{
		if (!AutoLanguageCandidates.Contains(LanguageID))
		{
			AutoLanguageCandidates.Insert(LanguageID, 0);
		}
	}

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UAzSpeechSettings, AutoLanguageCandidates))
	{
		if (AutoLanguageCandidates.Num() > 4)
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("You can only include up to 4 languages for at-start LID and up to 10 languages for continuous LID, but continuous recognition has not yet been implemented."));

			AutoLanguageCandidates.RemoveAtSwap(4, AutoLanguageCandidates.Num() - 4, true);
		}
	}
}
#endif // WITH_EDITOR
