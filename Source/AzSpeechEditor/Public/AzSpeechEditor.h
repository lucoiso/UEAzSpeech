// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEAzSpeech

#pragma once

#include <CoreMinimal.h>

class FAzSpeechEditorModule : public IModuleInterface
{
protected:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    TSharedRef<SDockTab> OnSpawnTab(const FSpawnTabArgs& SpawnTabArgs) const;

private:
    void RegisterMenus();
};
