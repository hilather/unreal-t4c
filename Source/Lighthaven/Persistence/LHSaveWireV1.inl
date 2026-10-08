// Explicit frozen v1 allowlist. No runtime reflection or property-order dependency.
static void Visit(FWire& W, FLHAreaId& V);
static void Visit(FWire& W, FLHAreaRecord& V);
static void Visit(FWire& W, FLHAttributeBlock& V);
static void Visit(FWire& W, FLHBossRecord& V);
static void Visit(FWire& W, FLHCharacterId& V);
static void Visit(FWire& W, FLHCharacterRecord& V);
static void Visit(FWire& W, FLHCheckpoint& V);
static void Visit(FWire& W, FLHContentId& V);
static void Visit(FWire& W, FLHCooldownRecord& V);
static void Visit(FWire& W, FLHCorpseLootRecord& V);
static void Visit(FWire& W, FLHCreationRecord& V);
static void Visit(FWire& W, FLHDurableEffectRecord& V);
static void Visit(FWire& W, FLHEncounterRecord& V);
static void Visit(FWire& W, FLHEntityId& V);
static void Visit(FWire& W, FLHEntranceId& V);
static void Visit(FWire& W, FLHEquipmentBinding& V);
static void Visit(FWire& W, FLHFieldProvenance& V);
static void Visit(FWire& W, FLHGrowthAward& V);
static void Visit(FWire& W, FLHInteger& V);
static void Visit(FWire& W, FLHItemInstance& V);
static void Visit(FWire& W, FLHLearnedSkill& V);
static void Visit(FWire& W, FLHMechanicalField& V);
static void Visit(FWire& W, FLHNumber& V);
static void Visit(FWire& W, FLHObjectRecord& V);
static void Visit(FWire& W, FLHQuestRecord& V);
static void Visit(FWire& W, FLHQuestionAnswer& V);
static void Visit(FWire& W, FLHRequestId& V);
static void Visit(FWire& W, FLHRequestReceipt& V);
static void Visit(FWire& W, FLHRewardId& V);
static void Visit(FWire& W, FLHRngState& V);
static void Visit(FWire& W, FLHRulesetRef& V);
static void Visit(FWire& W, FLHSaveHeader& V);
static void Visit(FWire& W, FLHSessionRecord& V);
static void Visit(FWire& W, FLHSpawnLifeId& V);
static void Visit(FWire& W, FLHWorldRecord& V);
static void Visit(FWire& W, ELHProvenanceStatus& V);
static void Visit(FWire& W, ELHValueResolution& V);
static void Visit(FWire& W, ELHMigrationPolicy& V);
static void Visit(FWire& W, ELHEquipmentSlot& V);
static void Visit(FWire& W, ELHEncounterLifeState& V);
static void Visit(FWire& W, ELHEffectSavePolicy& V);
template<class T> static TArray<uint8> Key(T V);
static TArray<uint8> Key(FLHContentId V);
static TArray<uint8> Key(FLHRewardId V);
static TArray<uint8> Key(FLHAreaRecord V);
static TArray<uint8> Key(FLHEncounterRecord V);
static TArray<uint8> Key(FLHCorpseLootRecord V);
static TArray<uint8> Key(FLHObjectRecord V);
static TArray<uint8> Key(FLHItemInstance V);
static TArray<uint8> Key(FLHEquipmentBinding V);
static TArray<uint8> Key(FLHLearnedSkill V);
static TArray<uint8> Key(FLHMechanicalField V);
static TArray<uint8> Key(FLHQuestRecord V);
static TArray<uint8> Key(FLHBossRecord V);
static TArray<uint8> Key(FLHRngState V);
static TArray<uint8> Key(FLHCooldownRecord V);
static TArray<uint8> Key(FLHDurableEffectRecord V);
static TArray<uint8> Key(FLHRequestReceipt V);
static TArray<uint8> Key(FLHEntityId V);
template<class T> static void Field(FWire& W, const ANSICHAR* Name, T& V)
{
    W.Name(Name); Visit(W, V);
}
template<class T> static void Array(FWire& W, TArray<T>& Values, uint32 Limit, bool Ordered)
{
    uint32 N = W.bWrite ? static_cast<uint32>(Values.Num()) : 0;
    W.U32(N);
    if (!W.Ok()) return;
    if (N > Limit || N > 100000 - W.Elements || (!W.bWrite && N > static_cast<uint32>(W.Remaining())))
    { W.Fail(ELHSaveReason::Oversize, TEXT("Collection count exceeds bounded capacity")); return; }
    W.Elements += N;
    if (W.bWrite)
    {
        struct FEntry { int32 Index; TArray<uint8> Bytes; };
        TArray<FEntry> Entries;
        if (!Ordered)
        {
            for (uint32 I=0; I<N; ++I) Entries.Add({static_cast<int32>(I), Key(Values[I])});
            Entries.Sort([](const FEntry& A, const FEntry& B) { return Less(A.Bytes, B.Bytes); });
            for (int32 I=1; I<Entries.Num(); ++I)
                if (Entries[I-1].Bytes == Entries[I].Bytes) W.Fail(ELHSaveReason::InvalidSnapshot, TEXT("Duplicate set key"));
        }
        for (uint32 I=0; I<N && W.Ok(); ++I) Visit(W, Values[Ordered ? I : Entries[I].Index]);
    }
    else if (W.bScan)
    {
        for (uint32 I=0; I<N && W.Ok(); ++I) { T Local{}; Visit(W, Local); }
    }
    else
    {
        // Entire payload has already passed the allocation-free collection preflight.
        Values.SetNum(N);
        for (T& V : Values) { Visit(W, V); if (!W.Ok()) break; }
    }
}
template<class T> static TArray<uint8> Key(T V)
{
    FLHSaveError E; FWire W(E); Visit(W, V); return MoveTemp(W.Output);
}
static TArray<uint8> Key(FLHContentId V)
{
    FLHSaveError E; FWire W(E);
    Visit(W, V.Value);
    return MoveTemp(W.Output);
}
static TArray<uint8> Key(FLHRewardId V)
{
    FLHSaveError E; FWire W(E);
    Visit(W, V.Value);
    return MoveTemp(W.Output);
}
static TArray<uint8> Key(FLHAreaRecord V)
{
    FLHSaveError E; FWire W(E);
    Visit(W, V.Area);
    return MoveTemp(W.Output);
}
static TArray<uint8> Key(FLHEncounterRecord V)
{
    FLHSaveError E; FWire W(E);
    Visit(W, V.Life.Area); Visit(W, V.Life.SpawnSlot); Visit(W, V.Life.LifeGeneration);
    return MoveTemp(W.Output);
}
static TArray<uint8> Key(FLHCorpseLootRecord V)
{
    FLHSaveError E; FWire W(E);
    Visit(W, V.Container);
    return MoveTemp(W.Output);
}
static TArray<uint8> Key(FLHObjectRecord V)
{
    FLHSaveError E; FWire W(E);
    Visit(W, V.Id);
    return MoveTemp(W.Output);
}
static TArray<uint8> Key(FLHItemInstance V)
{
    FLHSaveError E; FWire W(E);
    Visit(W, V.Id);
    return MoveTemp(W.Output);
}
static TArray<uint8> Key(FLHEquipmentBinding V)
{
    FLHSaveError E; FWire W(E);
    Visit(W, V.Slot);
    return MoveTemp(W.Output);
}
static TArray<uint8> Key(FLHLearnedSkill V)
{
    FLHSaveError E; FWire W(E);
    Visit(W, V.Skill);
    return MoveTemp(W.Output);
}
static TArray<uint8> Key(FLHMechanicalField V)
{
    FLHSaveError E; FWire W(E);
    Visit(W, V.Key);
    return MoveTemp(W.Output);
}
static TArray<uint8> Key(FLHQuestRecord V)
{
    FLHSaveError E; FWire W(E);
    Visit(W, V.Quest);
    return MoveTemp(W.Output);
}
static TArray<uint8> Key(FLHBossRecord V)
{
    FLHSaveError E; FWire W(E);
    Visit(W, V.Boss);
    return MoveTemp(W.Output);
}
static TArray<uint8> Key(FLHRngState V)
{
    FLHSaveError E; FWire W(E);
    Visit(W, V.StreamId);
    return MoveTemp(W.Output);
}
static TArray<uint8> Key(FLHCooldownRecord V)
{
    FLHSaveError E; FWire W(E);
    Visit(W, V.Owner);
    Visit(W, V.Ability);
    return MoveTemp(W.Output);
}
static TArray<uint8> Key(FLHDurableEffectRecord V)
{
    FLHSaveError E; FWire W(E);
    Visit(W, V.Owner);
    Visit(W, V.Effect);
    Visit(W, V.Source);
    return MoveTemp(W.Output);
}
static TArray<uint8> Key(FLHRequestReceipt V)
{
    FLHSaveError E; FWire W(E);
    Visit(W, V.Request.Value);
    return MoveTemp(W.Output);
}
static TArray<uint8> Key(FLHEntityId V)
{
    FLHSaveError E; FWire W(E);
    Visit(W, V.RunId);
    Visit(W, V.Area);
    Visit(W, V.InstanceId);
    return MoveTemp(W.Output);
}
static void Visit(FWire& W, ELHProvenanceStatus& V)
{
    FString Token;
    if (W.bWrite)
    {
        switch (V)
        {
        case ELHProvenanceStatus::Missing: Token = TEXT("Missing"); break;
        case ELHProvenanceStatus::Confirmed: Token = TEXT("Confirmed"); break;
        case ELHProvenanceStatus::VerifiedT4C: Token = TEXT("VerifiedT4C"); break;
        case ELHProvenanceStatus::Disputed: Token = TEXT("Disputed"); break;
        case ELHProvenanceStatus::Modernized: Token = TEXT("Modernized"); break;
        case ELHProvenanceStatus::Prototype: Token = TEXT("Prototype"); break;
        default: W.Fail(ELHSaveReason::Malformed, TEXT("Unknown enum")); return;
        }
    }
    W.String(Token, 128, true, true);
    if (!W.bWrite && W.Ok())
    {
        if (Token == TEXT("Missing")) V = ELHProvenanceStatus::Missing;
        else if (Token == TEXT("Confirmed")) V = ELHProvenanceStatus::Confirmed;
        else if (Token == TEXT("VerifiedT4C")) V = ELHProvenanceStatus::VerifiedT4C;
        else if (Token == TEXT("Disputed")) V = ELHProvenanceStatus::Disputed;
        else if (Token == TEXT("Modernized")) V = ELHProvenanceStatus::Modernized;
        else if (Token == TEXT("Prototype")) V = ELHProvenanceStatus::Prototype;
        else W.Fail(ELHSaveReason::Malformed, TEXT("Unknown enum token"));
    }
}
static void Visit(FWire& W, ELHValueResolution& V)
{
    FString Token;
    if (W.bWrite)
    {
        switch (V)
        {
        case ELHValueResolution::Unresolved: Token = TEXT("Unresolved"); break;
        case ELHValueResolution::Resolved: Token = TEXT("Resolved"); break;
        default: W.Fail(ELHSaveReason::Malformed, TEXT("Unknown enum")); return;
        }
    }
    W.String(Token, 128, true, true);
    if (!W.bWrite && W.Ok())
    {
        if (Token == TEXT("Unresolved")) V = ELHValueResolution::Unresolved;
        else if (Token == TEXT("Resolved")) V = ELHValueResolution::Resolved;
        else W.Fail(ELHSaveReason::Malformed, TEXT("Unknown enum token"));
    }
}
static void Visit(FWire& W, ELHMigrationPolicy& V)
{
    FString Token;
    if (W.bWrite)
    {
        switch (V)
        {
        case ELHMigrationPolicy::Reject: Token = TEXT("Reject"); break;
        case ELHMigrationPolicy::RequireExplicitMigration: Token = TEXT("RequireExplicitMigration"); break;
        case ELHMigrationPolicy::NewCharacterRequired: Token = TEXT("NewCharacterRequired"); break;
        default: W.Fail(ELHSaveReason::Malformed, TEXT("Unknown enum")); return;
        }
    }
    W.String(Token, 128, true, true);
    if (!W.bWrite && W.Ok())
    {
        if (Token == TEXT("Reject")) V = ELHMigrationPolicy::Reject;
        else if (Token == TEXT("RequireExplicitMigration")) V = ELHMigrationPolicy::RequireExplicitMigration;
        else if (Token == TEXT("NewCharacterRequired")) V = ELHMigrationPolicy::NewCharacterRequired;
        else W.Fail(ELHSaveReason::Malformed, TEXT("Unknown enum token"));
    }
}
static void Visit(FWire& W, ELHEquipmentSlot& V)
{
    FString Token;
    if (W.bWrite)
    {
        switch (V)
        {
        case ELHEquipmentSlot::Unspecified: Token = TEXT("Unspecified"); break;
        case ELHEquipmentSlot::MainHand: Token = TEXT("MainHand"); break;
        case ELHEquipmentSlot::OffHand: Token = TEXT("OffHand"); break;
        case ELHEquipmentSlot::Quiver: Token = TEXT("Quiver"); break;
        case ELHEquipmentSlot::Head: Token = TEXT("Head"); break;
        case ELHEquipmentSlot::Torso: Token = TEXT("Torso"); break;
        case ELHEquipmentSlot::Legs: Token = TEXT("Legs"); break;
        case ELHEquipmentSlot::Feet: Token = TEXT("Feet"); break;
        case ELHEquipmentSlot::Accessory: Token = TEXT("Accessory"); break;
        default: W.Fail(ELHSaveReason::Malformed, TEXT("Unknown enum")); return;
        }
    }
    W.String(Token, 128, true, true);
    if (!W.bWrite && W.Ok())
    {
        if (Token == TEXT("Unspecified")) V = ELHEquipmentSlot::Unspecified;
        else if (Token == TEXT("MainHand")) V = ELHEquipmentSlot::MainHand;
        else if (Token == TEXT("OffHand")) V = ELHEquipmentSlot::OffHand;
        else if (Token == TEXT("Quiver")) V = ELHEquipmentSlot::Quiver;
        else if (Token == TEXT("Head")) V = ELHEquipmentSlot::Head;
        else if (Token == TEXT("Torso")) V = ELHEquipmentSlot::Torso;
        else if (Token == TEXT("Legs")) V = ELHEquipmentSlot::Legs;
        else if (Token == TEXT("Feet")) V = ELHEquipmentSlot::Feet;
        else if (Token == TEXT("Accessory")) V = ELHEquipmentSlot::Accessory;
        else W.Fail(ELHSaveReason::Malformed, TEXT("Unknown enum token"));
    }
}
static void Visit(FWire& W, ELHEncounterLifeState& V)
{
    FString Token;
    if (W.bWrite)
    {
        switch (V)
        {
        case ELHEncounterLifeState::Unresolved: Token = TEXT("Unresolved"); break;
        case ELHEncounterLifeState::Alive: Token = TEXT("Alive"); break;
        case ELHEncounterLifeState::Dead: Token = TEXT("Dead"); break;
        case ELHEncounterLifeState::RespawnPending: Token = TEXT("RespawnPending"); break;
        case ELHEncounterLifeState::PermanentlyDefeated: Token = TEXT("PermanentlyDefeated"); break;
        default: W.Fail(ELHSaveReason::Malformed, TEXT("Unknown enum")); return;
        }
    }
    W.String(Token, 128, true, true);
    if (!W.bWrite && W.Ok())
    {
        if (Token == TEXT("Unresolved")) V = ELHEncounterLifeState::Unresolved;
        else if (Token == TEXT("Alive")) V = ELHEncounterLifeState::Alive;
        else if (Token == TEXT("Dead")) V = ELHEncounterLifeState::Dead;
        else if (Token == TEXT("RespawnPending")) V = ELHEncounterLifeState::RespawnPending;
        else if (Token == TEXT("PermanentlyDefeated")) V = ELHEncounterLifeState::PermanentlyDefeated;
        else W.Fail(ELHSaveReason::Malformed, TEXT("Unknown enum token"));
    }
}
static void Visit(FWire& W, ELHEffectSavePolicy& V)
{
    FString Token;
    if (W.bWrite)
    {
        switch (V)
        {
        case ELHEffectSavePolicy::Unresolved: Token = TEXT("Unresolved"); break;
        case ELHEffectSavePolicy::CompletedActionBoundaryOnly: Token = TEXT("CompletedActionBoundaryOnly"); break;
        default: W.Fail(ELHSaveReason::Malformed, TEXT("Unknown enum")); return;
        }
    }
    W.String(Token, 128, true, true);
    if (!W.bWrite && W.Ok())
    {
        if (Token == TEXT("Unresolved")) V = ELHEffectSavePolicy::Unresolved;
        else if (Token == TEXT("CompletedActionBoundaryOnly")) V = ELHEffectSavePolicy::CompletedActionBoundaryOnly;
        else W.Fail(ELHSaveReason::Malformed, TEXT("Unknown enum token"));
    }
}
static void Visit(FWire& W, FLHAreaId& V)
{
    FDepth Depth(W); W.Struct(1);
    Field(W, "Content", V.Content);
}
static void Visit(FWire& W, FLHAreaRecord& V)
{
    FDepth Depth(W); W.Struct(4);
    Field(W, "Area", V.Area);
    W.Name("Corpses"); Array(W, V.Corpses, 1024, false);
    W.Name("Encounters"); Array(W, V.Encounters, 4096, false);
    W.Name("Objects"); Array(W, V.Objects, 4096, false);
}
static void Visit(FWire& W, FLHAttributeBlock& V)
{
    FDepth Depth(W); W.Struct(5);
    Field(W, "Agility", V.Agility);
    Field(W, "Endurance", V.Endurance);
    Field(W, "Intelligence", V.Intelligence);
    Field(W, "Strength", V.Strength);
    Field(W, "Wisdom", V.Wisdom);
}
static void Visit(FWire& W, FLHBossRecord& V)
{
    FDepth Depth(W); W.Struct(5);
    Field(W, "Boss", V.Boss);
    W.Name("DialogueFlags"); Array(W, V.DialogueFlags, 256, false);
    W.Name("Marks"); Array(W, V.Marks, 256, false);
    Field(W, "UniqueClaim", V.UniqueClaim);
    Field(W, "bDefeated", V.bDefeated);
}
static void Visit(FWire& W, FLHCharacterId& V)
{
    FDepth Depth(W); W.Struct(1);
    Field(W, "Value", V.Value);
}
static void Visit(FWire& W, FLHCharacterRecord& V)
{
    FDepth Depth(W); W.Struct(21);
    Field(W, "ActiveEntrance", V.ActiveEntrance);
    W.Name("AppearanceIds"); Array(W, V.AppearanceIds, 16, false);
    Field(W, "BaseAttributes", V.BaseAttributes);
    Field(W, "Creation", V.Creation);
    Field(W, "CurrentHealth", V.CurrentHealth);
    Field(W, "CurrentMana", V.CurrentMana);
    W.Name("DisplayName"); W.String(V.DisplayName, 128);
    Field(W, "EarnedBaseHealth", V.EarnedBaseHealth);
    Field(W, "EarnedBaseMana", V.EarnedBaseMana);
    Field(W, "EarnedLevel", V.EarnedLevel);
    W.Name("Equipment"); Array(W, V.Equipment, 8, false);
    Field(W, "ExperienceBalance", V.ExperienceBalance);
    Field(W, "ExperienceDebt", V.ExperienceDebt);
    Field(W, "Gold", V.Gold);
    W.Name("GrowthAwards"); Array(W, V.GrowthAwards, W.GrowthLimit, true);
    W.Name("Inventory"); Array(W, V.Inventory, 512, false);
    W.Name("LearnedSkills"); Array(W, V.LearnedSkills, 256, false);
    W.Name("LearnedSpells"); Array(W, V.LearnedSpells, 256, false);
    Field(W, "RebirthCount", V.RebirthCount);
    Field(W, "UnspentAttributePoints", V.UnspentAttributePoints);
    Field(W, "UnspentSkillPoints", V.UnspentSkillPoints);
}
static void Visit(FWire& W, FLHCheckpoint& V)
{
    FDepth Depth(W); W.Struct(3);
    Field(W, "Entrance", V.Entrance);
    FTransform Canonical = W.bWrite && V.TransformResolution == ELHValueResolution::Unresolved ? FTransform::Identity : V.SafeTransform;
    Field(W, "SafeTransform", Canonical); if (!W.bWrite) V.SafeTransform = Canonical;
    Field(W, "TransformResolution", V.TransformResolution);
}
static void Visit(FWire& W, FLHContentId& V)
{
    FDepth Depth(W); W.Struct(1);
    Field(W, "Value", V.Value);
}
static void Visit(FWire& W, FLHCooldownRecord& V)
{
    FDepth Depth(W); W.Struct(3);
    Field(W, "Ability", V.Ability);
    Field(W, "Owner", V.Owner);
    Field(W, "RemainingSeconds", V.RemainingSeconds);
}
static void Visit(FWire& W, FLHCorpseLootRecord& V)
{
    FDepth Depth(W); W.Struct(8);
    Field(W, "CleanupRemainingSeconds", V.CleanupRemainingSeconds);
    Field(W, "Container", V.Container);
    Field(W, "RemainingGold", V.RemainingGold);
    W.Name("RemainingItems"); Array(W, V.RemainingItems, 512, false);
    Field(W, "Reward", V.Reward);
    Field(W, "SourceLife", V.SourceLife);
    Field(W, "bClaimed", V.bClaimed);
    Field(W, "bFinalized", V.bFinalized);
}
static void Visit(FWire& W, FLHCreationRecord& V)
{
    FDepth Depth(W); W.Struct(5);
    Field(W, "AcceptedAttributes", V.AcceptedAttributes);
    W.Name("AcceptedRollInputs"); Array(W, V.AcceptedRollInputs, 16, false);
    Field(W, "GenerationPolicy", V.GenerationPolicy);
    Field(W, "GenerationRevision", V.GenerationRevision);
    W.Name("QuestionAnswers"); Array(W, V.QuestionAnswers, 4, true);
}
static void Visit(FWire& W, FLHDurableEffectRecord& V)
{
    FDepth Depth(W); W.Struct(6);
    W.Name("CanonicalInputs"); Array(W, V.CanonicalInputs, 64, false);
    Field(W, "Effect", V.Effect);
    Field(W, "Owner", V.Owner);
    Field(W, "RemainingSeconds", V.RemainingSeconds);
    Field(W, "Source", V.Source);
    Field(W, "Stacks", V.Stacks);
}
static void Visit(FWire& W, FLHEncounterRecord& V)
{
    FDepth Depth(W); W.Struct(7);
    Field(W, "CurrentHealth", V.CurrentHealth);
    Field(W, "Definition", V.Definition);
    Field(W, "KillReward", V.KillReward);
    Field(W, "Life", V.Life);
    Field(W, "RespawnRemainingSeconds", V.RespawnRemainingSeconds);
    Field(W, "State", V.State);
    Field(W, "bRewardCommitted", V.bRewardCommitted);
}
static void Visit(FWire& W, FLHEntityId& V)
{
    FDepth Depth(W); W.Struct(3);
    Field(W, "Area", V.Area);
    Field(W, "InstanceId", V.InstanceId);
    Field(W, "RunId", V.RunId);
}
static void Visit(FWire& W, FLHEntranceId& V)
{
    FDepth Depth(W); W.Struct(2);
    Field(W, "Area", V.Area);
    Field(W, "LocalId", V.LocalId);
}
static void Visit(FWire& W, FLHEquipmentBinding& V)
{
    FDepth Depth(W); W.Struct(2);
    Field(W, "Item", V.Item);
    Field(W, "Slot", V.Slot);
}
static void Visit(FWire& W, FLHFieldProvenance& V)
{
    FDepth Depth(W); W.Struct(7);
    Field(W, "FieldPath", V.FieldPath);
    Field(W, "Notes", V.Notes);
    Field(W, "RetrievedDate", V.RetrievedDate);
    Field(W, "SourceBaseline", V.SourceBaseline);
    Field(W, "SourceUrl", V.SourceUrl);
    Field(W, "Status", V.Status);
    Field(W, "ValueAsRecorded", V.ValueAsRecorded);
}
static void Visit(FWire& W, FLHGrowthAward& V)
{
    FDepth Depth(W); W.Struct(10);
    Field(W, "AttributePoints", V.AttributePoints);
    Field(W, "AwardId", V.AwardId);
    Field(W, "FromLevel", V.FromLevel);
    Field(W, "GrowthInputs", V.GrowthInputs);
    Field(W, "HealthIncrement", V.HealthIncrement);
    Field(W, "ManaIncrement", V.ManaIncrement);
    W.Name("RollInputs"); Array(W, V.RollInputs, 16, false);
    Field(W, "Ruleset", V.Ruleset);
    Field(W, "SkillPoints", V.SkillPoints);
    Field(W, "ToLevel", V.ToLevel);
}
static void Visit(FWire& W, FLHInteger& V)
{
    FDepth Depth(W); W.Struct(3);
    Field(W, "Provenance", V.Provenance);
    Field(W, "Resolution", V.Resolution);
    int64 Canonical = W.bWrite && V.Resolution == ELHValueResolution::Unresolved ? 0 : V.Value;
    Field(W, "Value", Canonical); if (!W.bWrite) V.Value = Canonical;
}
static void Visit(FWire& W, FLHItemInstance& V)
{
    FDepth Depth(W); W.Struct(4);
    Field(W, "Definition", V.Definition);
    Field(W, "Id", V.Id);
    W.Name("PermanentRolledValues"); Array(W, V.PermanentRolledValues, 64, false);
    Field(W, "Quantity", V.Quantity);
}
static void Visit(FWire& W, FLHLearnedSkill& V)
{
    FDepth Depth(W); W.Struct(2);
    Field(W, "Skill", V.Skill);
    Field(W, "TrainedValue", V.TrainedValue);
}
static void Visit(FWire& W, FLHMechanicalField& V)
{
    FDepth Depth(W); W.Struct(2);
    Field(W, "Key", V.Key);
    Field(W, "Value", V.Value);
}
static void Visit(FWire& W, FLHNumber& V)
{
    FDepth Depth(W); W.Struct(3);
    Field(W, "Provenance", V.Provenance);
    Field(W, "Resolution", V.Resolution);
    double Canonical = W.bWrite && V.Resolution == ELHValueResolution::Unresolved ? 0 : V.Value;
    Field(W, "Value", Canonical); if (!W.bWrite) V.Value = Canonical;
}
static void Visit(FWire& W, FLHObjectRecord& V)
{
    FDepth Depth(W); W.Struct(6);
    Field(W, "Definition", V.Definition);
    Field(W, "Id", V.Id);
    W.Name("RemainingItems"); Array(W, V.RemainingItems, 512, false);
    Field(W, "bCollected", V.bCollected);
    Field(W, "bOpened", V.bOpened);
    Field(W, "bUnlocked", V.bUnlocked);
}
static void Visit(FWire& W, FLHQuestRecord& V)
{
    FDepth Depth(W); W.Struct(8);
    Field(W, "EligibleKillCount", V.EligibleKillCount);
    W.Name("Flags"); Array(W, V.Flags, 256, false);
    Field(W, "Quest", V.Quest);
    Field(W, "Stage", V.Stage);
    Field(W, "TurnInClaim", V.TurnInClaim);
    Field(W, "bAccepted", V.bAccepted);
    Field(W, "bCompleted", V.bCompleted);
    Field(W, "bRewarded", V.bRewarded);
}
static void Visit(FWire& W, FLHQuestionAnswer& V)
{
    FDepth Depth(W); W.Struct(2);
    Field(W, "Answer", V.Answer);
    Field(W, "Question", V.Question);
}
static void Visit(FWire& W, FLHRequestId& V)
{
    FDepth Depth(W); W.Struct(2);
    Field(W, "Epoch", V.Epoch);
    Field(W, "Value", V.Value);
}
static void Visit(FWire& W, FLHRequestReceipt& V)
{
    FDepth Depth(W); W.Struct(3);
    Field(W, "PayloadDigest", V.PayloadDigest);
    Field(W, "Request", V.Request);
    Field(W, "TransactionSequence", V.TransactionSequence);
}
static void Visit(FWire& W, FLHRewardId& V)
{
    FDepth Depth(W); W.Struct(1);
    Field(W, "Value", V.Value);
}
static void Visit(FWire& W, FLHRngState& V)
{
    FDepth Depth(W); W.Struct(4);
    Field(W, "Algorithm", V.Algorithm);
    Field(W, "AlgorithmRevision", V.AlgorithmRevision);
    W.Name("State"); Array(W, V.State, 64, true);
    Field(W, "StreamId", V.StreamId);
}
static void Visit(FWire& W, FLHRulesetRef& V)
{
    FDepth Depth(W); W.Struct(5);
    Field(W, "ContentHash", V.ContentHash);
    Field(W, "HashAlgorithm", V.HashAlgorithm);
    Field(W, "Id", V.Id);
    Field(W, "MigrationPolicy", V.MigrationPolicy);
    Field(W, "Revision", V.Revision);
}
static void Visit(FWire& W, FLHSaveHeader& V)
{
    FDepth Depth(W); W.Struct(11);
    Field(W, "BuildId", V.BuildId);
    Field(W, "CharacterId", V.CharacterId);
    Field(W, "ChecksumAlgorithm", V.ChecksumAlgorithm);
    Field(W, "ContentRevision", V.ContentRevision);
    Field(W, "Magic", V.Magic);
    Field(W, "PayloadChecksum", V.PayloadChecksum);
    Field(W, "PayloadCodec", V.PayloadCodec);
    Field(W, "PayloadLengthBytes", V.PayloadLengthBytes);
    Field(W, "Ruleset", V.Ruleset);
    Field(W, "SchemaVersion", V.SchemaVersion);
    Field(W, "TransactionSequence", V.TransactionSequence);
}
static void Visit(FWire& W, FLHSessionRecord& V)
{
    FDepth Depth(W); W.Struct(9);
    W.Name("Cooldowns"); Array(W, V.Cooldowns, 1024, false);
    W.Name("Diagnostics"); W.StringLimit=512; Array(W, V.Diagnostics, 64, true); W.StringLimit=4096;
    W.Name("DurableEffects"); Array(W, V.DurableEffects, 1024, false);
    Field(W, "EffectPolicy", V.EffectPolicy);
    W.Name("GameplayRng"); Array(W, V.GameplayRng, 4096, false);
    Field(W, "ManaRegenFractionalSeconds", V.ManaRegenFractionalSeconds);
    W.Name("RecentRequests"); Array(W, V.RecentRequests, 4096, false);
    Field(W, "RequestEpoch", V.RequestEpoch);
    Field(W, "SafeRespawn", V.SafeRespawn);
}
static void Visit(FWire& W, FLHSpawnLifeId& V)
{
    FDepth Depth(W); W.Struct(3);
    Field(W, "Area", V.Area);
    Field(W, "LifeGeneration", V.LifeGeneration);
    Field(W, "SpawnSlot", V.SpawnSlot);
}
static void Visit(FWire& W, FLHWorldRecord& V)
{
    FDepth Depth(W); W.Struct(6);
    W.Name("Areas"); Array(W, V.Areas, 5, false);
    W.Name("Bosses"); Array(W, V.Bosses, 64, false);
    W.Name("ClaimedUniqueRewards"); Array(W, V.ClaimedUniqueRewards, 4096, false);
    W.Name("Quests"); Array(W, V.Quests, 256, false);
    Field(W, "RunId", V.RunId);
    W.Name("UnlockedPortals"); Array(W, V.UnlockedPortals, 256, false);
}
