#include "LHSaveStore.h"
#include "Async/Async.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"

namespace
{
class FLocalStorage final : public ILHSaveStorage
{
    FString Directory;
    FString Path(const FGuid& Character, int32 Slot) const
    { return Directory / (Character.ToString(EGuidFormats::Digits) + (Slot==0 ? TEXT(".A.lhs") : TEXT(".B.lhs"))); }
public:
    explicit FLocalStorage(const FString& InDirectory) : Directory(InDirectory) {}
    virtual TArray<FGuid> Enumerate() override
    {
        TArray<FGuid> Result;
        IPlatformFile& Files=FPlatformFileManager::Get().GetPlatformFile();
        Files.IterateDirectory(*Directory,[&Result](const TCHAR* Filename, bool IsDirectory)
        {
            if (IsDirectory) return true;
            const FString Name=FPaths::GetCleanFilename(Filename);
            if (Name.Len()!=38 || !(Name.EndsWith(TEXT(".A.lhs"),ESearchCase::CaseSensitive) || Name.EndsWith(TEXT(".B.lhs"),ESearchCase::CaseSensitive))) return true;
            FGuid Id;
            if (FGuid::ParseExact(Name.Left(32),EGuidFormats::Digits,Id) && Id.IsValid()) Result.AddUnique(Id);
            return true;
        });
        Result.Sort([](const FGuid& A, const FGuid& B) { return A.ToString()<B.ToString(); });
        return Result;
    }
    virtual bool Read(const FGuid& Character, int32 Slot, TArray<uint8>& Bytes, FLHSaveError& Error) override
    {
        Bytes.Reset(); Error={};
        IPlatformFile& Files=FPlatformFileManager::Get().GetPlatformFile(); const FString Filename=Path(Character,Slot);
        TUniquePtr<IFileHandle> Handle(Files.OpenRead(*Filename));
        if (!Handle) { Error={Files.FileExists(*Filename) ? ELHSaveReason::IoFailure : ELHSaveReason::NotFound,TEXT("Generation could not be opened")}; return false; }
        const int64 Size=Handle->Size();
        if (Size<0 || Size>LHSave::MaxFileBytes) { Error={ELHSaveReason::Oversize,TEXT("Generation file size exceeds limit before allocation")}; return false; }
        Bytes.SetNumUninitialized(static_cast<int32>(Size));
        if ((Size && !Handle->Read(Bytes.GetData(),Size)) || Handle->Size()!=Size)
        { Bytes.Reset(); Error={ELHSaveReason::IoFailure,TEXT("Generation read failed or file changed size")}; return false; }
        return true;
    }
    virtual void Write(const FGuid& Character, int32 Slot, TArray<uint8> Bytes, TFunction<void(bool)> Complete) override
    {
        const FString Filename=Path(Character,Slot), Parent=Directory;
        Async(EAsyncExecution::ThreadPool,[Filename,Parent,Data=MoveTemp(Bytes),Callback=MoveTemp(Complete)]() mutable
        {
            IPlatformFile& Files=FPlatformFileManager::Get().GetPlatformFile();
            bool Success=Files.CreateDirectoryTree(*Parent);
            if (Success)
            {
                TUniquePtr<IFileHandle> Handle(Files.OpenWrite(*Filename,false,false));
                Success=Handle && Handle->Write(Data.GetData(),Data.Num()) && Handle->Flush(true);
                Handle.Reset();
            }
            AsyncTask(ENamedThreads::GameThread,[Success,Callback=MoveTemp(Callback)]() mutable { Callback(Success); });
        });
    }
};
}
TSharedRef<ILHSaveStorage> LHCreateLocalSaveStorage(const FString& Directory)
{ return MakeShared<FLocalStorage>(Directory); }
