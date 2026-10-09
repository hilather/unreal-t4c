#include "Character/LHCharacterAuthority.h"
#include "GameFramework/PlayerState.h"

namespace LHCharacterAuthorityPrivate
{
using namespace LH::Rules;
bool Ready(const FLHInteger &V)
{
    return V.Resolution == ELHValueResolution::Resolved && V.Provenance.Status != ELHProvenanceStatus::Missing &&
           V.Provenance.Status != ELHProvenanceStatus::Disputed;
}
bool Ready(const FLHNumber &V)
{
    return V.Resolution == ELHValueResolution::Resolved && FMath::IsFinite(V.Value) &&
           V.Provenance.Status != ELHProvenanceStatus::Missing && V.Provenance.Status != ELHProvenanceStatus::Disputed;
}
FLHInteger Integer(int64 V)
{
    FLHInteger R;
    R.Resolution = ELHValueResolution::Resolved;
    R.Value = V;
    R.Provenance.Status = ELHProvenanceStatus::Prototype;
    R.Provenance.Notes = TEXT("Authority bookkeeping, not historical tuning");
    return R;
}
FLHNumber Number(double V)
{
    FLHNumber R;
    R.Resolution = ELHValueResolution::Resolved;
    R.Value = V;
    R.Provenance.Status = ELHProvenanceStatus::Prototype;
    R.Provenance.Notes = TEXT("Authority computed result under selected profile");
    return R;
}
bool Attributes(const FLHAttributeBlock &B, FAttributes &A)
{
    const FLHInteger *V[] = {&B.Strength, &B.Endurance, &B.Agility, &B.Intelligence, &B.Wisdom};
    int64 *O[] = {&A.Strength, &A.Endurance, &A.Agility, &A.Intelligence, &A.Wisdom};
    for (int32 N = 0; N < 5; ++N)
    {
        if (!Ready(*V[N]) || V[N]->Value < 0)
            return false;
        *O[N] = V[N]->Value;
    }
    return true;
}
FLHAttributeBlock Block(const FAttributes &A)
{
    FLHAttributeBlock B;
    B.Strength = Integer(A.Strength);
    B.Endurance = Integer(A.Endurance);
    B.Agility = Integer(A.Agility);
    B.Intelligence = Integer(A.Intelligence);
    B.Wisdom = Integer(A.Wisdom);
    return B;
}
bool EntityEqual(const FLHEntityId &A, const FLHEntityId &B)
{
    return A.RunId == B.RunId && A.Area.Content.Value == B.Area.Content.Value && A.InstanceId == B.InstanceId;
}
bool EntityValid(const FLHEntityId &I)
{
    return I.RunId.IsValid() && I.InstanceId.IsValid() && !I.Area.Content.Value.IsNone();
}
ELHCommandReason Reason(const FDiagnostic &D)
{
    return D.IsAccepted()                    ? ELHCommandReason::None
           : D.Reason == EReason::Unresolved ? ELHCommandReason::UnresolvedRules
           : D.Reason == EReason::Ineligible ? ELHCommandReason::Ineligible
                                             : ELHCommandReason::InvalidRequest;
}
FLHRngState Rng(int32 Seed, FName Stream)
{
    FLHRngState R;
    R.StreamId = Stream;
    R.Algorithm = TEXT("UE.FRandomStream");
    R.AlgorithmRevision = 1;
    for (int32 N = 0; N < 4; ++N)
        R.State.Add((uint32(Seed) >> (N * 8)) & 255);
    return R;
}
bool Decode(const FLHRngState &R, int32 &Seed)
{
    if (R.Algorithm != TEXT("UE.FRandomStream") || R.AlgorithmRevision != 1 || R.State.Num() != 4)
        return false;
    uint32 V = 0;
    for (int32 N = 0; N < 4; ++N)
        V |= uint32(R.State[N]) << (N * 8);
    FMemory::Memcpy(&Seed, &V, 4);
    return true;
}
FLHRngState Pair(const FGrowthRoll &Roll)
{
    FLHRngState R;
    R.StreamId = TEXT("RNG.Growth");
    R.Algorithm = TEXT("LH.NormalizedRollPair");
    R.AlgorithmRevision = 1;
    for (double Value : {Roll.Health, Roll.Mana})
    {
        uint64 Bits;
        FMemory::Memcpy(&Bits, &Value, 8);
        for (int32 N = 0; N < 8; ++N)
            R.State.Add((Bits >> (N * 8)) & 255);
    }
    return R;
}
void PutRng(FLHSaveSnapshot &S, const FLHRngState &R)
{
    for (auto &Existing : S.Session.GameplayRng)
        if (Existing.StreamId == R.StreamId)
        {
            Existing = R;
            return;
        }
    S.Session.GameplayRng.Add(R);
}
bool Appearance(TArray<FLHContentId> &IDs)
{
    FName Body, Face;
    int32 Bodies = 0, Faces = 0, Hairs = 0, Skins = 0, Outfits = 0;
    TSet<FName> Seen;
    for (const auto &ID : IDs)
    {
        FName V = ID.Value;
        if (Seen.Contains(V))
            return false;
        Seen.Add(V);
        if (V == TEXT("Presentation.Player.Body.A") || V == TEXT("Presentation.Player.Body.B"))
        {
            Body = V;
            ++Bodies;
        }
        else if (V == TEXT("Presentation.Player.Face.A") || V == TEXT("Presentation.Player.Face.B"))
        {
            Face = V;
            ++Faces;
        }
        else if (V == TEXT("Presentation.Player.Hair.Cropped") || V == TEXT("Presentation.Player.Hair.Tied"))
            ++Hairs;
        else if (V == TEXT("Presentation.Player.Skin.LightWarm") || V == TEXT("Presentation.Player.Skin.MediumWarm") ||
                 V == TEXT("Presentation.Player.Skin.DeepWarm"))
            ++Skins;
        else if (V == TEXT("Presentation.Player.Outfit.StarterLinen"))
            ++Outfits;
        else
            return false;
    }
    if (Bodies != 1 || Faces > 1 || Hairs != 1 || Skins != 1 || Outfits != 1)
        return false;
    const FName Expected = Body == TEXT("Presentation.Player.Body.A") ? FName(TEXT("Presentation.Player.Face.A"))
                                                                      : FName(TEXT("Presentation.Player.Face.B"));
    if (Faces && Face != Expected)
        return false;
    if (!Faces)
    {
        FLHContentId I;
        I.Value = Expected;
        IDs.Add(I);
    }
    IDs.Sort([](const FLHContentId &A, const FLHContentId &B) { return A.Value.ToString() < B.Value.ToString(); });
    return true;
}
bool EqualCreation(const FLHCreationRecord &A, const FLHCreationRecord &B)
{
    return FLHCreationRecord::StaticStruct()->CompareScriptStruct(&A, &B, 0);
}
} // namespace

ULHCharacterAuthorityComponent *ULHCharacterAuthorityComponent::Attach(APlayerState *Owner)
{
    using namespace LH::Rules;
    if (!Owner || !Owner->HasAuthority())
        return nullptr;
    if (auto *Existing = Owner->FindComponentByClass<ULHCharacterAuthorityComponent>())
        return Existing;
    auto *C = NewObject<ULHCharacterAuthorityComponent>(Owner);
    Owner->AddInstanceComponent(C);
    C->RegisterComponent();
    return C;
}

bool FLHCharacterAuthority::Initialize(const FLHCharacterProfile &P, const FGuid &Epoch, int32 InitialSeed)
{
    using namespace LH::Rules;
    check(IsInGameThread());
    if (bInitialized || !Epoch.IsValid() || !P.RequestDigest || !P.GrowthId || P.Reference.Id.Value.IsNone() ||
        P.Reference.Revision <= 0 || P.Reference.ContentHash.Len() != 64 || P.Reference.HashAlgorithm != TEXT("SHA256"))
        return false;
    if (P.Rules.Progression.Thresholds.Num() > 20000 || P.Items.Num() > 4096 ||
        (LHCharacterAuthorityPrivate::Ready(P.Rules.Progression.InitialLevel) &&
         (P.Rules.Progression.InitialLevel.Value < 0 || P.Rules.Progression.InitialLevel.Value > 20000)))
        return false;
    TSet<FName> IDs;
    for (const auto &I : P.Items)
    {
        if (I.Id.Value.IsNone() || IDs.Contains(I.Id.Value))
            return false;
        IDs.Add(I.Id.Value);
    }
    Profile = P;
    Seed = InitialSeed;
    State.Session.RequestEpoch = Epoch;
    State.Header.Ruleset = P.Reference;
    State.World.RunId = FGuid::NewGuid();
    LHCharacterAuthorityPrivate::PutRng(State, LHCharacterAuthorityPrivate::Rng(Seed, TEXT("RNG.Creation")));
    LHCharacterAuthorityPrivate::PutRng(State, LHCharacterAuthorityPrivate::Rng(Seed, TEXT("RNG.Growth")));
    bInitialized = true;
    return true;
}
const FLHCharacterItemDefinition *FLHCharacterAuthority::Definition(FName ID) const
{
    using namespace LH::Rules;
    return Profile.Items.FindByPredicate([ID](const auto &D) { return D.Id.Value == ID; });
}
ELHCommandReason FLHCharacterAuthority::Preview(const TArray<FLHQuestionAnswer> &Answers, FLHCharacterPreview &Out)
{
    using namespace LH::Rules;
    check(IsInGameThread());
    if (!bInitialized || State.Header.CharacterId.Value.IsValid())
        return ELHCommandReason::InvalidRequest;
    if (Answers.Num() != 4)
        return ELHCommandReason::InvalidRequest;
    if (!LHCharacterAuthorityPrivate::Ready(Profile.CreationRevision) || Profile.CreationRevision.Value <= 0 ||
        Profile.CreationPolicy.Value.IsNone())
        return ELHCommandReason::UnresolvedRules;
    TArray<int32> Eligible;
    for (int32 N = 0; N < Profile.Rules.Creation.Outcomes.Num(); ++N)
        if (RollCreation(Profile.Rules.Creation, Answers, N).Diagnostic.IsAccepted())
            Eligible.Add(N);
    if (Eligible.IsEmpty())
        return Profile.Rules.Creation.OutcomesResolution == ELHValueResolution::Unresolved
                   ? ELHCommandReason::UnresolvedRules
                   : ELHCommandReason::InvalidRequest;
    FRandomStream Random(Seed);
    const int32 Index = Eligible[Random.RandRange(0, Eligible.Num() - 1)];
    const auto Roll = RollCreation(Profile.Rules.Creation, Answers, Index);
    FLHCharacterPreview Next;
    Next.Token = FGuid::NewGuid();
    Next.Creation.QuestionAnswers = Answers;
    Next.Creation.GenerationPolicy = Profile.CreationPolicy;
    Next.Creation.GenerationRevision = Profile.CreationRevision;
    const auto &A = Profile.Rules.Creation.Outcomes[Index].Attributes;
    Next.Creation.AcceptedAttributes.Strength = A.Strength;
    Next.Creation.AcceptedAttributes.Endurance = A.Endurance;
    Next.Creation.AcceptedAttributes.Agility = A.Agility;
    Next.Creation.AcceptedAttributes.Intelligence = A.Intelligence;
    Next.Creation.AcceptedAttributes.Wisdom = A.Wisdom;
    Next.Creation.AcceptedRollInputs.Add(LHCharacterAuthorityPrivate::Rng(Seed, TEXT("RNG.Creation")));
    Next.UnspentPoints = Roll.Value.UnspentPoints;
    Pending = Next;
    Seed = Random.GetCurrentSeed();
    PendingRng = LHCharacterAuthorityPrivate::Rng(Seed, TEXT("RNG.Creation"));
    Out = Next;
    return ELHCommandReason::None;
}
bool FLHCharacterAuthority::Begin(const FLHRequestId &ID, FName Kind, const UScriptStruct *Type, const void *Payload,
                                  FLHCommandResult &Out, FString &Digest) const
{
    using namespace LH::Rules;
    check(IsInGameThread());
    Out.Request = ID;
    if (!bInitialized || !ID.Value.IsValid() || ID.Epoch != State.Session.RequestEpoch)
        return false;
    Digest = Profile.RequestDigest(Kind, Type, Payload);
    if (Digest.Len() != 64)
    {
        Out.Reason = ELHCommandReason::UnresolvedRules;
        return false;
    }
    for (const auto &R : State.Session.RecentRequests)
        if (R.Request.Value == ID.Value)
        {
            if (R.PayloadDigest != Digest)
            {
                Out.Reason = ELHCommandReason::ReusedRequestId;
                return false;
            }
            Out.Disposition = ELHCommandDisposition::Accepted;
            Out.Reason = ELHCommandReason::None;
            Out.CommittedSequence = R.TransactionSequence;
            Out.bReplay = true;
            return false;
        }
    if (State.Session.RecentRequests.Num() >= 4096 || State.Header.TransactionSequence == MAX_int64)
    {
        Out.Reason = ELHCommandReason::Busy;
        return false;
    }
    return true;
}
FLHCommandResult FLHCharacterAuthority::Commit(const FLHRequestId &ID, const FString &Digest, FLHSaveSnapshot &&Next)
{
    using namespace LH::Rules;
    Next.Header.TransactionSequence = State.Header.TransactionSequence + 1;
    FLHRequestReceipt Receipt;
    Receipt.Request = ID;
    Receipt.PayloadDigest = Digest;
    Receipt.TransactionSequence = Next.Header.TransactionSequence;
    Next.Session.RecentRequests.Add(Receipt);
    State = MoveTemp(Next);
    FLHCommandResult R;
    R.Request = ID;
    R.Disposition = ELHCommandDisposition::Accepted;
    R.Reason = ELHCommandReason::None;
    R.CommittedSequence = State.Header.TransactionSequence;
    return R;
}
FLHCommandResult FLHCharacterAuthority::Execute(const FLHCreateCharacterRequest &R)
{
    using namespace LH::Rules;
    FLHCommandResult Result;
    FString Digest;
    if (!Begin(R.Request, TEXT("CreateCharacter"), R.StaticStruct(), &R, Result, Digest))
        return Result;
    if (State.Header.CharacterId.Value.IsValid() || !R.PreviewToken.IsValid() || R.PreviewToken != Pending.Token ||
        !LHCharacterAuthorityPrivate::EqualCreation(R.Creation, Pending.Creation))
        return Result;
    TArray<FLHContentId> Looks = R.AppearanceIds;
    if (R.DisplayName.IsEmpty() || R.DisplayName.Len() > 64 || R.DisplayName.TrimStartAndEnd() != R.DisplayName ||
        !LHCharacterAuthorityPrivate::Appearance(Looks))
        return Result;
    for (TCHAR C : R.DisplayName)
        if (C < 32 || C == 127)
            return Result;
    if (!LHCharacterAuthorityPrivate::Ready(Profile.InitialHealth) || !LHCharacterAuthorityPrivate::Ready(Profile.InitialMana) || !LHCharacterAuthorityPrivate::Ready(Profile.InitialGold) ||
        !LHCharacterAuthorityPrivate::Ready(Profile.InitialSkillPoints) || !LHCharacterAuthorityPrivate::Ready(Profile.Rules.Progression.InitialLevel) ||
        Profile.Rules.Progression.Thresholds.IsEmpty())
    {
        Result.Reason = ELHCommandReason::UnresolvedRules;
        return Result;
    }
    FLHSaveSnapshot Next = State;
    auto &C = Next.Character;
    Next.Header.CharacterId.Value = FGuid::NewGuid();
    C.DisplayName = R.DisplayName;
    C.AppearanceIds = Looks;
    C.Creation = Pending.Creation;
    C.BaseAttributes = Pending.Creation.AcceptedAttributes;
    C.EarnedLevel = Profile.Rules.Progression.InitialLevel;
    C.ExperienceBalance = Profile.Rules.Progression.Thresholds[0];
    C.ExperienceDebt = LHCharacterAuthorityPrivate::Integer(0);
    C.UnspentAttributePoints = LHCharacterAuthorityPrivate::Integer(Pending.UnspentPoints);
    C.UnspentSkillPoints = Profile.InitialSkillPoints;
    C.Gold = Profile.InitialGold;
    C.EarnedBaseHealth = Profile.InitialHealth;
    C.EarnedBaseMana = Profile.InitialMana;
    C.CurrentHealth = Profile.InitialHealth;
    C.CurrentMana = Profile.InitialMana;
    LHCharacterAuthorityPrivate::PutRng(Next, PendingRng);
    for (const auto &ID : Profile.StarterItems)
    {
        FLHItemInstance I;
        I.Id.RunId = Next.World.RunId;
        I.Id.Area.Content.Value = TEXT("Area.LighthavenTempleDistrict");
        I.Id.InstanceId = FGuid::NewGuid();
        I.Definition = ID;
        I.Quantity = LHCharacterAuthorityPrivate::Integer(1);
        C.Inventory.Add(I);
    }
    Result.Reason = Validate(Next);
    if (Result.Reason != ELHCommandReason::None)
        return Result;
    Result = Commit(R.Request, Digest, MoveTemp(Next));
    Pending = {};
    return Result;
}
LH::Rules::TResult<LH::Rules::FStatsResult> FLHCharacterAuthority::Stats(const FLHCharacterRecord &C) const
{
    using namespace LH::Rules;
    FStatsInput Input;
    TResult<FStatsResult> Reject;
    if (!LHCharacterAuthorityPrivate::Attributes(C.BaseAttributes, Input.Base) || !LHCharacterAuthorityPrivate::Ready(C.EarnedBaseHealth) || !LHCharacterAuthorityPrivate::Ready(C.EarnedBaseMana))
        return Reject;
    Input.EarnedHealth = C.EarnedBaseHealth.Value;
    Input.EarnedMana = C.EarnedBaseMana.Value;
    for (const auto &Binding : C.Equipment)
    {
        const auto *Item = C.Inventory.FindByPredicate([&](const auto &I) { return LHCharacterAuthorityPrivate::EntityEqual(I.Id, Binding.Item); });
        const auto *D = Item ? Definition(Item->Definition.Value) : nullptr;
        if (!D || D->Slot != Binding.Slot || !Item->PermanentRolledValues.IsEmpty())
            return Reject;
        Input.Equipment.Add(D->Modifier);
    }
    return DeriveStats(Profile.Rules.Stats, Input);
}
LH::Rules::TResult<LH::Rules::FStatsResult> FLHCharacterAuthority::Stats() const
{
    using namespace LH::Rules;
    return Stats(State.Character);
}
ELHCommandReason FLHCharacterAuthority::Validate(const FLHSaveSnapshot &S) const
{
    using namespace LH::Rules;
    const auto &C = S.Character;
    if (!S.Header.CharacterId.Value.IsValid() || !S.World.RunId.IsValid() || !S.Session.RequestEpoch.IsValid() ||
        C.RebirthCount != 0)
        return ELHCommandReason::InvalidRequest;
    if (S.Header.Ruleset.Id.Value != Profile.Reference.Id.Value ||
        S.Header.Ruleset.Revision != Profile.Reference.Revision ||
        S.Header.Ruleset.ContentHash != Profile.Reference.ContentHash ||
        S.Header.Ruleset.HashAlgorithm != Profile.Reference.HashAlgorithm)
        return ELHCommandReason::UnresolvedRules;
    if (C.DisplayName.IsEmpty() || C.DisplayName.Len() > 64 || C.DisplayName.TrimStartAndEnd() != C.DisplayName)
        return ELHCommandReason::InvalidRequest;
    for (TCHAR Ch : C.DisplayName)
        if (Ch < 32 || Ch == 127)
            return ELHCommandReason::InvalidRequest;
    auto Looks = C.AppearanceIds;
    if (!LHCharacterAuthorityPrivate::Appearance(Looks) || Looks.Num() != C.AppearanceIds.Num())
        return ELHCommandReason::InvalidRequest;
    if (C.Creation.GenerationPolicy.Value != Profile.CreationPolicy.Value || !LHCharacterAuthorityPrivate::Ready(C.Creation.GenerationRevision) ||
        C.Creation.GenerationRevision.Value != Profile.CreationRevision.Value ||
        C.Creation.QuestionAnswers.Num() != 4 || C.Creation.AcceptedRollInputs.Num() != 1)
        return ELHCommandReason::InvalidRequest;
    int32 BeforeSeed;
    if (C.Creation.AcceptedRollInputs[0].StreamId != TEXT("RNG.Creation") ||
        !LHCharacterAuthorityPrivate::Decode(C.Creation.AcceptedRollInputs[0], BeforeSeed))
        return ELHCommandReason::InvalidRequest;
    TArray<int32> Eligible;
    for (int32 Index = 0; Index < Profile.Rules.Creation.Outcomes.Num(); ++Index)
        if (RollCreation(Profile.Rules.Creation, C.Creation.QuestionAnswers, Index).Diagnostic.IsAccepted())
            Eligible.Add(Index);
    if (Eligible.IsEmpty())
        return ELHCommandReason::UnresolvedRules;
    FRandomStream CreationRandom(BeforeSeed);
    const int32 Outcome = Eligible[CreationRandom.RandRange(0, Eligible.Num() - 1)];
    const auto CreationRoll = RollCreation(Profile.Rules.Creation, C.Creation.QuestionAnswers, Outcome);
    FAttributes Accepted, Base;
    if (!LHCharacterAuthorityPrivate::Attributes(C.Creation.AcceptedAttributes, Accepted) || !LHCharacterAuthorityPrivate::Attributes(C.BaseAttributes, Base))
        return ELHCommandReason::UnresolvedRules;
    const auto &Expected = CreationRoll.Value.Attributes;
    if (Accepted.Strength != Expected.Strength || Accepted.Endurance != Expected.Endurance ||
        Accepted.Agility != Expected.Agility || Accepted.Intelligence != Expected.Intelligence ||
        Accepted.Wisdom != Expected.Wisdom)
        return ELHCommandReason::InvalidRequest;
    const FLHInteger *Values[] = {&C.EarnedLevel,        &C.ExperienceBalance,
                                  &C.ExperienceDebt,     &C.UnspentAttributePoints,
                                  &C.UnspentSkillPoints, &C.Gold};
    for (auto *V : Values)
        if (!LHCharacterAuthorityPrivate::Ready(*V) || V->Value < 0)
            return ELHCommandReason::UnresolvedRules;
    if (!LHCharacterAuthorityPrivate::Ready(Profile.InventorySlots) || Profile.InventorySlots.Value < 0)
        return ELHCommandReason::UnresolvedRules;
    if (C.Inventory.Num() > Profile.InventorySlots.Value || C.Inventory.Num() > 4096)
        return ELHCommandReason::InventoryFull;
    TSet<FName> SkillIDs, SpellIDs;
    if (C.LearnedSkills.Num() > 4096 || C.LearnedSpells.Num() > 4096)
        return ELHCommandReason::InvalidRequest;
    for (const auto &Skill : C.LearnedSkills)
    {
        if (Skill.Skill.Value.IsNone() || SkillIDs.Contains(Skill.Skill.Value) || !LHCharacterAuthorityPrivate::Ready(Skill.TrainedValue) ||
            Skill.TrainedValue.Value < 0)
            return ELHCommandReason::InvalidRequest;
        SkillIDs.Add(Skill.Skill.Value);
    }
    for (const auto &Spell : C.LearnedSpells)
    {
        if (Spell.Value.IsNone() || SpellIDs.Contains(Spell.Value))
            return ELHCommandReason::InvalidRequest;
        SpellIDs.Add(Spell.Value);
    }
    double Weight = 0;
    TSet<FGuid> ItemIds;
    for (const auto &I : C.Inventory)
    {
        const auto *D = Definition(I.Definition.Value);
        if (!LHCharacterAuthorityPrivate::EntityValid(I.Id) || I.Id.RunId != S.World.RunId || ItemIds.Contains(I.Id.InstanceId) ||
            !LHCharacterAuthorityPrivate::Ready(I.Quantity) || I.Quantity.Value <= 0 || !I.PermanentRolledValues.IsEmpty())
            return ELHCommandReason::InvalidRequest;
        ItemIds.Add(I.Id.InstanceId);
        if (!D || !LHCharacterAuthorityPrivate::Ready(D->StackLimit) || !LHCharacterAuthorityPrivate::Ready(D->Weight) || D->Weight.Value < 0)
            return ELHCommandReason::UnresolvedRules;
        if (I.Quantity.Value > D->StackLimit.Value)
            return ELHCommandReason::InvalidRequest;
        Weight += I.Quantity.Value * D->Weight.Value;
    }
    TSet<ELHEquipmentSlot> Slots;
    TSet<FGuid> Bound;
    for (const auto &B : C.Equipment)
    {
        if (B.Slot == ELHEquipmentSlot::Unspecified || uint8(B.Slot) > uint8(ELHEquipmentSlot::Accessory) ||
            Slots.Contains(B.Slot) || Bound.Contains(B.Item.InstanceId))
            return ELHCommandReason::InvalidEquipment;
        Slots.Add(B.Slot);
        Bound.Add(B.Item.InstanceId);
    }
    const auto Derived = Stats(C);
    if (!Derived.Diagnostic.IsAccepted())
        return LHCharacterAuthorityPrivate::Reason(Derived.Diagnostic);
    if (!FMath::IsFinite(Weight) || Weight > Derived.Value.Capacity)
        return ELHCommandReason::InventoryFull;
    FRequirementInput Req;
    LHCharacterAuthorityPrivate::Attributes(C.BaseAttributes, Req.Base);
    Req.Effective = Derived.Value.Effective;
    Req.Level = C.EarnedLevel.Value;
    Req.Skills = C.LearnedSkills;
    Req.Spells = C.LearnedSpells;
    for (const auto &B : C.Equipment)
    {
        const auto *I = C.Inventory.FindByPredicate([&](const auto &X) { return LHCharacterAuthorityPrivate::EntityEqual(X.Id, B.Item); });
        const auto *D = I ? Definition(I->Definition.Value) : nullptr;
        if (!D)
            return ELHCommandReason::InvalidEquipment;
        Req.bBow = D->bBow;
        Req.bCompatibleQuiverEquipped = false;
        for (const auto &Q : C.Equipment)
            if (Q.Slot == ELHEquipmentSlot::Quiver)
            {
                const auto *Quiver =
                    C.Inventory.FindByPredicate([&](const auto &X) { return LHCharacterAuthorityPrivate::EntityEqual(X.Id, Q.Item); });
                if (Quiver)
                    for (const auto &Allowed : D->CompatibleQuivers)
                        if (Allowed.Value == Quiver->Definition.Value)
                            Req.bCompatibleQuiverEquipped = true;
            }
        const auto Check = CheckRequirements(Profile.Rules.Requirements, D->Eligibility, Req);
        if (!Check.Diagnostic.IsAccepted())
            return LHCharacterAuthorityPrivate::Reason(Check.Diagnostic);
    }
    if (!LHCharacterAuthorityPrivate::Ready(C.CurrentHealth) || !LHCharacterAuthorityPrivate::Ready(C.CurrentMana) || C.CurrentHealth.Value < 0 || C.CurrentMana.Value < 0 ||
        C.CurrentHealth.Value > Derived.Value.MaxHealth || C.CurrentMana.Value > Derived.Value.MaxMana)
        return ELHCommandReason::InvalidRequest;
    FProgressionInput P;
    P.EarnedLevel = C.EarnedLevel.Value;
    P.ExperienceBalance = C.ExperienceBalance.Value;
    P.ExperienceDebt = C.ExperienceDebt.Value;
    P.UnspentAttributes = C.UnspentAttributePoints.Value;
    P.UnspentSkills = C.UnspentSkillPoints.Value;
    P.EarnedHealth = C.EarnedBaseHealth.Value;
    P.EarnedMana = C.EarnedBaseMana.Value;
    P.GrowthAttributes = Req.Base;
    const auto NoGrant = Advance(Profile.Rules.Progression, P);
    if (!NoGrant.Diagnostic.IsAccepted())
        return LHCharacterAuthorityPrivate::Reason(NoGrant.Diagnostic);
    if (C.GrowthAwards.Num() > 19999 || !LHCharacterAuthorityPrivate::Ready(Profile.Rules.Progression.InitialLevel) ||
        C.GrowthAwards.Num() != C.EarnedLevel.Value - Profile.Rules.Progression.InitialLevel.Value)
        return ELHCommandReason::InvalidRequest;
    if (!LHCharacterAuthorityPrivate::Ready(Profile.InitialHealth) || !LHCharacterAuthorityPrivate::Ready(Profile.InitialMana))
        return ELHCommandReason::UnresolvedRules;
    double HistoricalHealth = Profile.InitialHealth.Value, HistoricalMana = Profile.InitialMana.Value;
    TSet<FGuid> Awards;
    int64 Level = Profile.Rules.Progression.InitialLevel.Value;
    for (const auto &A : C.GrowthAwards)
    {
        if (!A.AwardId.Value.IsValid() || Awards.Contains(A.AwardId.Value) || !LHCharacterAuthorityPrivate::Ready(A.FromLevel) ||
            !LHCharacterAuthorityPrivate::Ready(A.ToLevel) || A.FromLevel.Value != Level || A.ToLevel.Value != Level + 1 || A.RollInputs.Num() != 1)
            return ELHCommandReason::InvalidRequest;
        if (A.RollInputs[0].Algorithm != TEXT("LH.NormalizedRollPair") || A.RollInputs[0].AlgorithmRevision != 1 ||
            A.RollInputs[0].State.Num() != 16 || A.RollInputs[0].StreamId != TEXT("RNG.Growth"))
            return ELHCommandReason::InvalidRequest;
        const auto ExpectedID = Profile.GrowthId(S.World.RunId, S.Header.CharacterId, A.ToLevel.Value);
        if (A.AwardId.Value != ExpectedID.Value || A.Ruleset.Id.Value != Profile.Reference.Id.Value ||
            A.Ruleset.Revision != Profile.Reference.Revision || A.Ruleset.ContentHash != Profile.Reference.ContentHash)
            return ELHCommandReason::InvalidRequest;
        FGrowthRoll Roll;
        Roll.AwardId = A.AwardId;
        double *PairValues[] = {&Roll.Health, &Roll.Mana};
        for (int32 Part = 0; Part < 2; ++Part)
        {
            uint64 Bits = 0;
            for (int32 Byte = 0; Byte < 8; ++Byte)
                Bits |= uint64(A.RollInputs[0].State[Part * 8 + Byte]) << (Byte * 8);
            FMemory::Memcpy(PairValues[Part], &Bits, 8);
        }
        const int32 Index = int32(Level - Profile.Rules.Progression.InitialLevel.Value);
        if (!Profile.Rules.Progression.Thresholds.IsValidIndex(Index + 1))
            return ELHCommandReason::InvalidRequest;
        FProgressionInput History;
        History.EarnedLevel = Level;
        History.ExperienceBalance = Profile.Rules.Progression.Thresholds[Index].Value;
        History.ExperienceGain = Profile.Rules.Progression.Thresholds[Index + 1].Value - History.ExperienceBalance;
        History.EarnedHealth = HistoricalHealth;
        History.EarnedMana = HistoricalMana;
        History.Ruleset = Profile.Reference;
        History.Rolls.Add(Roll);
        if (!LHCharacterAuthorityPrivate::Attributes(A.GrowthInputs, History.GrowthAttributes))
            return ELHCommandReason::InvalidRequest;
        const auto Replay = Advance(Profile.Rules.Progression, History);
        if (!Replay.Diagnostic.IsAccepted())
            return LHCharacterAuthorityPrivate::Reason(Replay.Diagnostic);
        const auto &ExpectedAward = Replay.Value.Awards[0];
        if (!LHCharacterAuthorityPrivate::Ready(A.HealthIncrement) || !LHCharacterAuthorityPrivate::Ready(A.ManaIncrement) || !LHCharacterAuthorityPrivate::Ready(A.AttributePoints) ||
            !LHCharacterAuthorityPrivate::Ready(A.SkillPoints) || A.HealthIncrement.Value != ExpectedAward.HealthIncrement.Value ||
            A.ManaIncrement.Value != ExpectedAward.ManaIncrement.Value ||
            A.AttributePoints.Value != ExpectedAward.AttributePoints.Value ||
            A.SkillPoints.Value != ExpectedAward.SkillPoints.Value)
            return ELHCommandReason::InvalidRequest;
        HistoricalHealth = Replay.Value.EarnedHealth;
        HistoricalMana = Replay.Value.EarnedMana;
        Awards.Add(A.AwardId.Value);
        ++Level;
    }
    if (HistoricalHealth != C.EarnedBaseHealth.Value || HistoricalMana != C.EarnedBaseMana.Value)
        return ELHCommandReason::InvalidRequest;
    // Permanent allocations conserve the initial budget plus each recorded level grant.
    int64 Granted = CreationRoll.Value.UnspentPoints, Spent = 0;
    const int64 Original[] = {Accepted.Strength, Accepted.Endurance, Accepted.Agility, Accepted.Intelligence,
                              Accepted.Wisdom};
    const int64 Current[] = {Base.Strength, Base.Endurance, Base.Agility, Base.Intelligence, Base.Wisdom};
    for (int32 Index = 0; Index < 5; ++Index)
    {
        if (Current[Index] < Original[Index] || Current[Index] - Original[Index] > MAX_int64 - Spent)
            return ELHCommandReason::InvalidRequest;
        Spent += Current[Index] - Original[Index];
    }
    for (const auto &Award : C.GrowthAwards)
    {
        if (Award.AttributePoints.Value > MAX_int64 - Granted)
            return ELHCommandReason::InvalidRequest;
        Granted += Award.AttributePoints.Value;
    }
    if (Spent > Granted || C.UnspentAttributePoints.Value != Granted - Spent)
        return ELHCommandReason::InvalidRequest;
    return ELHCommandReason::None;
}
FLHCommandResult FLHCharacterAuthority::Execute(const FLHAllocateAttributePointsRequest &R)
{
    using namespace LH::Rules;
    FLHCommandResult Result;
    FString Digest;
    if (!Begin(R.Request, TEXT("AllocateAttributePoints"), R.StaticStruct(), &R, Result, Digest))
        return Result;
    if (!State.Header.CharacterId.Value.IsValid())
        return Result;
    if (State.Character.CurrentHealth.Value <= 0)
    {
        Result.Reason = ELHCommandReason::InvalidLifeState;
        return Result;
    }
    FAttributes D;
    if (!LHCharacterAuthorityPrivate::Attributes(R.Points, D))
        return Result;
    int64 Total = 0;
    for (int64 V : {D.Strength, D.Endurance, D.Agility, D.Intelligence, D.Wisdom})
    {
        if (V > MAX_int64 - Total)
            return Result;
        Total += V;
    }
    if (Total <= 0)
        return Result;
    if (Total > State.Character.UnspentAttributePoints.Value)
    {
        Result.Reason = ELHCommandReason::InsufficientPoints;
        return Result;
    }
    FLHSaveSnapshot Next = State;
    auto &C = Next.Character;
    FLHInteger *B[] = {&C.BaseAttributes.Strength, &C.BaseAttributes.Endurance, &C.BaseAttributes.Agility,
                       &C.BaseAttributes.Intelligence, &C.BaseAttributes.Wisdom};
    const int64 V[] = {D.Strength, D.Endurance, D.Agility, D.Intelligence, D.Wisdom};
    for (int32 N = 0; N < 5; ++N)
    {
        if (V[N] > MAX_int64 - B[N]->Value)
            return Result;
        B[N]->Value += V[N];
    }
    C.UnspentAttributePoints.Value -= Total;
    Result.Reason = Validate(Next);
    if (Result.Reason != ELHCommandReason::None)
        return Result;
    return Commit(R.Request, Digest, MoveTemp(Next));
}
FLHCommandResult FLHCharacterAuthority::Execute(const FLHEquipItemRequest &R)
{
    using namespace LH::Rules;
    FLHCommandResult Result;
    FString Digest;
    if (!Begin(R.Request, TEXT("EquipItem"), R.StaticStruct(), &R, Result, Digest))
        return Result;
    if (!State.Header.CharacterId.Value.IsValid() || State.Character.CurrentHealth.Value <= 0)
    {
        Result.Reason = ELHCommandReason::InvalidLifeState;
        return Result;
    }
    FLHSaveSnapshot Next = State;
    auto &C = Next.Character;
    const auto *Item = C.Inventory.FindByPredicate([&](const auto &I) { return LHCharacterAuthorityPrivate::EntityEqual(I.Id, R.Item); });
    if (!Item)
    {
        Result.Reason = ELHCommandReason::NotFound;
        return Result;
    }
    const auto *D = Definition(Item->Definition.Value);
    if (!D || D->Slot == ELHEquipmentSlot::Unspecified || R.Slot != D->Slot)
    {
        Result.Reason = ELHCommandReason::InvalidEquipment;
        return Result;
    }
    const int32 Existing = C.Equipment.IndexOfByPredicate([&](const auto &B) { return B.Slot == R.Slot; });
    if (R.bUnequip && (Existing == INDEX_NONE || !LHCharacterAuthorityPrivate::EntityEqual(C.Equipment[Existing].Item, R.Item)))
    {
        Result.Reason = ELHCommandReason::InvalidEquipment;
        return Result;
    }
    if (Existing != INDEX_NONE)
        C.Equipment.RemoveAt(Existing);
    if (!R.bUnequip)
    {
        FLHEquipmentBinding B;
        B.Item = R.Item;
        B.Slot = R.Slot;
        C.Equipment.Add(B);
    }
    const auto Derived = Stats(C);
    if (!Derived.Diagnostic.IsAccepted())
    {
        Result.Reason = LHCharacterAuthorityPrivate::Reason(Derived.Diagnostic);
        return Result;
    }
    // Removing maximum-resource gear cannot retain resources above the new maximum.
    C.CurrentHealth.Value = FMath::Min(C.CurrentHealth.Value, Derived.Value.MaxHealth);
    C.CurrentMana.Value = FMath::Min(C.CurrentMana.Value, Derived.Value.MaxMana);
    Result.Reason = Validate(Next);
    if (Result.Reason != ELHCommandReason::None)
        return Result;
    return Commit(R.Request, Digest, MoveTemp(Next));
}
ELHCommandReason FLHCharacterAuthority::GrantExperience(int64 Gain)
{
    using namespace LH::Rules;
    check(IsInGameThread());
    if (!bInitialized || !State.Header.CharacterId.Value.IsValid() || Gain < 0 ||
        State.Header.TransactionSequence == MAX_int64)
        return ELHCommandReason::InvalidRequest;
    const auto &C = State.Character;
    if (Profile.GrowthBasis == EAttributeBasis::Unresolved)
        return ELHCommandReason::UnresolvedRules;
    FProgressionInput I;
    I.EarnedLevel = C.EarnedLevel.Value;
    I.ExperienceBalance = C.ExperienceBalance.Value;
    I.ExperienceDebt = C.ExperienceDebt.Value;
    I.ExperienceGain = Gain;
    I.UnspentAttributes = C.UnspentAttributePoints.Value;
    I.UnspentSkills = C.UnspentSkillPoints.Value;
    I.EarnedHealth = C.EarnedBaseHealth.Value;
    I.EarnedMana = C.EarnedBaseMana.Value;
    I.Ruleset = Profile.Reference;
    const auto Derived = Stats();
    if (!Derived.Diagnostic.IsAccepted())
        return LHCharacterAuthorityPrivate::Reason(Derived.Diagnostic);
    if (Profile.GrowthBasis == EAttributeBasis::Effective)
        I.GrowthAttributes = Derived.Value.Effective;
    else if (!LHCharacterAuthorityPrivate::Attributes(C.BaseAttributes, I.GrowthAttributes))
        return ELHCommandReason::UnresolvedRules;
    const int64 Remaining = Gain - FMath::Min(Gain, C.ExperienceDebt.Value);
    if (Remaining > MAX_int64 - C.ExperienceBalance.Value)
        return ELHCommandReason::InvalidRequest;
    const int64 Balance = C.ExperienceBalance.Value + Remaining;
    int32 Count = 0;
    if (!LHCharacterAuthorityPrivate::Ready(Profile.Rules.Progression.InitialLevel))
        return ELHCommandReason::UnresolvedRules;
    for (int32 N = 0; N < Profile.Rules.Progression.Thresholds.Num(); ++N)
    {
        const auto &T = Profile.Rules.Progression.Thresholds[N];
        if (!LHCharacterAuthorityPrivate::Ready(T))
            return ELHCommandReason::UnresolvedRules;
        if (Profile.Rules.Progression.InitialLevel.Value + N > C.EarnedLevel.Value && T.Value <= Balance)
            ++Count;
    }
    const auto *Stored =
        State.Session.GameplayRng.FindByPredicate([](const auto &R) { return R.StreamId == TEXT("RNG.Growth"); });
    int32 GrowthSeed;
    if (!Stored || !LHCharacterAuthorityPrivate::Decode(*Stored, GrowthSeed))
        return ELHCommandReason::InvalidRequest;
    FRandomStream Random(GrowthSeed);
    TSet<FGuid> IDs;
    for (const auto &A : C.GrowthAwards)
        IDs.Add(A.AwardId.Value);
    for (int32 N = 0; N < Count; ++N)
    {
        FGrowthRoll Roll;
        Roll.AwardId = Profile.GrowthId(State.World.RunId, State.Header.CharacterId, C.EarnedLevel.Value + N + 1);
        if (!Roll.AwardId.Value.IsValid() || IDs.Contains(Roll.AwardId.Value))
            return ELHCommandReason::InvalidRequest;
        IDs.Add(Roll.AwardId.Value);
        Roll.Health = Random.GetFraction();
        Roll.Mana = Random.GetFraction();
        I.Rolls.Add(Roll);
    }
    const auto Advanced = Advance(Profile.Rules.Progression, I);
    if (!Advanced.Diagnostic.IsAccepted())
        return LHCharacterAuthorityPrivate::Reason(Advanced.Diagnostic);
    FLHSaveSnapshot Next = State;
    auto &NC = Next.Character;
    const auto &A = Advanced.Value;
    NC.EarnedLevel.Value = A.EarnedLevel;
    NC.ExperienceBalance.Value = A.ExperienceBalance;
    NC.ExperienceDebt.Value = A.ExperienceDebt;
    NC.UnspentAttributePoints.Value = A.UnspentAttributes;
    NC.UnspentSkillPoints.Value = A.UnspentSkills;
    NC.EarnedBaseHealth.Value = A.EarnedHealth;
    NC.EarnedBaseMana.Value = A.EarnedMana;
    for (int32 N = 0; N < A.Awards.Num(); ++N)
    {
        auto Award = A.Awards[N];
        Award.RollInputs.Add(LHCharacterAuthorityPrivate::Pair(A.AcceptedRolls[N]));
        NC.GrowthAwards.Add(Award);
    }
    LHCharacterAuthorityPrivate::PutRng(Next, LHCharacterAuthorityPrivate::Rng(Random.GetCurrentSeed(), TEXT("RNG.Growth")));
    const auto Valid = Validate(Next);
    if (Valid != ELHCommandReason::None)
        return Valid;
    ++Next.Header.TransactionSequence;
    State = MoveTemp(Next);
    return ELHCommandReason::None;
}
ELHCommandReason FLHCharacterAuthority::AddExperienceDebt(int64 Amount)
{
    using namespace LH::Rules;
    check(IsInGameThread());
    if (!State.Header.CharacterId.Value.IsValid() || Amount < 0 ||
        Amount > MAX_int64 - State.Character.ExperienceDebt.Value || State.Header.TransactionSequence == MAX_int64)
        return ELHCommandReason::InvalidRequest;
    FLHSaveSnapshot Next = State;
    Next.Character.ExperienceDebt.Value += Amount;
    const auto Valid = Validate(Next);
    if (Valid != ELHCommandReason::None)
        return Valid;
    ++Next.Header.TransactionSequence;
    State = MoveTemp(Next);
    return ELHCommandReason::None;
}
ELHCommandReason FLHCharacterAuthority::AddItem(const FLHItemInstance &Item)
{
    using namespace LH::Rules;
    check(IsInGameThread());
    if (!State.Header.CharacterId.Value.IsValid() || State.Header.TransactionSequence == MAX_int64)
        return ELHCommandReason::InvalidRequest;
    FLHSaveSnapshot Next = State;
    Next.Character.Inventory.Add(Item);
    const auto Valid = Validate(Next);
    if (Valid != ELHCommandReason::None)
        return Valid;
    ++Next.Header.TransactionSequence;
    State = MoveTemp(Next);
    return ELHCommandReason::None;
}
ELHCommandReason FLHCharacterAuthority::RemoveItem(const FLHEntityId &Item, int64 Quantity)
{
    using namespace LH::Rules;
    check(IsInGameThread());
    if (Quantity <= 0 || State.Header.TransactionSequence == MAX_int64)
        return ELHCommandReason::InvalidRequest;
    FLHSaveSnapshot Next = State;
    auto &C = Next.Character;
    const int32 Index = C.Inventory.IndexOfByPredicate([&](const auto &I) { return LHCharacterAuthorityPrivate::EntityEqual(Item, I.Id); });
    if (Index == INDEX_NONE)
        return ELHCommandReason::NotFound;
    if (C.Equipment.ContainsByPredicate([&](const auto &B) { return LHCharacterAuthorityPrivate::EntityEqual(B.Item, Item); }))
        return ELHCommandReason::InvalidEquipment;
    if (Quantity > C.Inventory[Index].Quantity.Value)
        return ELHCommandReason::InvalidRequest;
    C.Inventory[Index].Quantity.Value -= Quantity;
    if (C.Inventory[Index].Quantity.Value == 0)
        C.Inventory.RemoveAt(Index);
    const auto Valid = Validate(Next);
    if (Valid != ELHCommandReason::None)
        return Valid;
    ++Next.Header.TransactionSequence;
    State = MoveTemp(Next);
    return ELHCommandReason::None;
}
void FLHCharacterAuthority::Export(FLHSaveSnapshot &Out) const
{
    using namespace LH::Rules;
    Out.Character = State.Character;
    Out.Header.CharacterId = State.Header.CharacterId;
    Out.Header.TransactionSequence = State.Header.TransactionSequence;
    Out.Header.Ruleset = State.Header.Ruleset;
    Out.World.RunId = State.World.RunId;
    Out.Session.RequestEpoch = State.Session.RequestEpoch;
    Out.Session.RecentRequests = State.Session.RecentRequests;
    for (const auto &R : State.Session.GameplayRng)
        if (R.StreamId == TEXT("RNG.Creation") || R.StreamId == TEXT("RNG.Growth"))
            LHCharacterAuthorityPrivate::PutRng(Out, R);
}
ELHCommandReason FLHCharacterAuthority::Import(const FLHSaveSnapshot &S)
{
    using namespace LH::Rules;
    check(IsInGameThread());
    if (!bInitialized || S.Header.SchemaVersion != LHSave::CurrentSchemaVersion || S.Header.TransactionSequence <= 0 ||
        S.Session.RecentRequests.Num() > 4096)
        return ELHCommandReason::InvalidRequest;
    const auto Valid = Validate(S);
    if (Valid != ELHCommandReason::None)
        return Valid;
    TSet<FGuid> Requests;
    for (const auto &R : S.Session.RecentRequests)
    {
        if (!R.Request.Value.IsValid() || R.Request.Epoch != S.Session.RequestEpoch ||
            Requests.Contains(R.Request.Value) || R.TransactionSequence <= 0 ||
            R.TransactionSequence > S.Header.TransactionSequence || R.PayloadDigest.Len() != 64)
            return ELHCommandReason::InvalidRequest;
        Requests.Add(R.Request.Value);
    }
    int32 CreationSeed = 0;
    TSet<FName> Streams;
    for (const auto &R : S.Session.GameplayRng)
    {
        if (Streams.Contains(R.StreamId))
            return ELHCommandReason::InvalidRequest;
        Streams.Add(R.StreamId);
        if (R.StreamId == TEXT("RNG.Creation") || R.StreamId == TEXT("RNG.Growth"))
        {
            int32 Value;
            if (!LHCharacterAuthorityPrivate::Decode(R, Value))
                return ELHCommandReason::InvalidRequest;
            if (R.StreamId == TEXT("RNG.Creation"))
                CreationSeed = Value;
        }
    }
    if (!Streams.Contains(TEXT("RNG.Creation")) || !Streams.Contains(TEXT("RNG.Growth")))
        return ELHCommandReason::InvalidRequest;
    State = S;
    Seed = CreationSeed;
    Pending = {};
    return ELHCommandReason::None;
}
