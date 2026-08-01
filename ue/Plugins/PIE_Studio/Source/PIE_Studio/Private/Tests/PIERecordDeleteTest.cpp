#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Handlers/GameplayHandlers.h"
#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPIERecordDeleteTest,
	"PIEStudio.Record.DeleteStaysInsideRoot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPIERecordDeleteTest::RunTest(const FString& /*Parameters*/)
{
	const FString Base = FPaths::ProjectSavedDir() / TEXT("MCPRecordDeleteTest") /
		FGuid::NewGuid().ToString(EGuidFormats::Digits);
	const FString Root = Base / TEXT("recordings");
	const FString Victim = Base / TEXT("victim");
	const FString Sentinel = Victim / TEXT("keep.txt");
	IFileManager::Get().MakeDirectory(*Root, true);
	IFileManager::Get().MakeDirectory(*Victim, true);
	TestTrue(TEXT("write sentinel"), FFileHelper::SaveStringToFile(TEXT("keep"), *Sentinel));

	{
		TSharedPtr<FJsonObject> Params = MakeShared<FJsonObject>();
		Params->SetStringField(TEXT("recording_dir"), Root);
		Params->SetStringField(TEXT("id"), TEXT("../victim"));
		Params->SetBoolField(TEXT("confirm"), true);
		const TSharedPtr<FJsonValue> Response = FGameplayHandlers::PieRecordDelete(Params);
		const TSharedPtr<FJsonObject> Result = Response.IsValid() ? Response->AsObject() : nullptr;
		bool bSuccess = true;
		if (Result.IsValid()) Result->TryGetBoolField(TEXT("success"), bSuccess);
		TestFalse(TEXT("reject traversal"), bSuccess);
		TestTrue(TEXT("keep sibling directory"), FPaths::DirectoryExists(Victim));
		TestTrue(TEXT("keep sibling file"), FPaths::FileExists(Sentinel));
	}

	{
		const FString Recording = Root / TEXT("safe-recording");
		IFileManager::Get().MakeDirectory(*Recording, true);
		TSharedPtr<FJsonObject> Params = MakeShared<FJsonObject>();
		Params->SetStringField(TEXT("recording_dir"), Root);
		Params->SetStringField(TEXT("id"), TEXT("safe-recording"));
		Params->SetBoolField(TEXT("confirm"), true);
		FGameplayHandlers::PieRecordDelete(Params);
		TestFalse(TEXT("delete direct child"), FPaths::DirectoryExists(Recording));
	}

	IFileManager::Get().DeleteDirectory(*Base, false, true);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
