// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEAzSpeech

#include "AzSpeech/AzSpeechSettings.h"

UAzSpeechSettings::UAzSpeechSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (AutoLanguageCandidates.IsEmpty())
	{
		AutoLanguageCandidates.Add(LanguageID);
	}

#if WITH_EDITOR
	PreviousLanguage = LanguageID;
#endif // WITH_EDITOR
}

#if WITH_EDITOR
void UAzSpeechSettings::PreEditChange(FProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);

	if (PropertyAboutToChange->GetFName() == GET_MEMBER_NAME_CHECKED(UAzSpeechSettings, LanguageID))
	{
		AutoLanguageCandidates.Remove(PreviousLanguage);
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
			AutoLanguageCandidates.Add(LanguageID);
			PreviousLanguage = LanguageID;
		}
	}
}
#endif // WITH_EDITOR
