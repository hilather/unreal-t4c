#include "Misc/AutomationTest.h"
#include "Core/LHCommands.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHSchemaDefaultsTest, "Lighthaven.Core.Schema.InvalidDefaults", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FLHSchemaDefaultsTest::RunTest(const FString&)
{
    TestTrue(TEXT("Hash algorithm defaults None"), FLHRulesetRef().HashAlgorithm.IsNone());
    TestFalse(TEXT("Request epoch defaults invalid"), FLHRequestId().Epoch.IsValid());
    TestFalse(TEXT("Session epoch defaults invalid"), FLHSessionRecord().RequestEpoch.IsValid());
    TestFalse(TEXT("Preview token defaults invalid"), FLHCreateCharacterRequest().PreviewToken.IsValid());
    TestTrue(TEXT("Loot kind defaults Unspecified"), FLHTakeLootRequest().Kind == ELHLootTransferKind::Unspecified);
    TestTrue(TEXT("Payload codec defaults None"), FLHSaveHeader().PayloadCodec.IsNone());
    const FLHEntityId Owners[] = {FLHCooldownRecord().Owner, FLHDurableEffectRecord().Owner};
    for (const FLHEntityId& Owner : Owners)
    {
        TestFalse(TEXT("Owner run defaults invalid"), Owner.RunId.IsValid());
        TestTrue(TEXT("Owner area defaults None"), Owner.Area.Content.Value.IsNone());
        TestFalse(TEXT("Owner instance defaults invalid"), Owner.InstanceId.IsValid());
    }
    const ULHAbilityDefinition* Ability = GetDefault<ULHAbilityDefinition>();
    TestTrue(TEXT("Presentation defaults None"), Ability->PresentationId.Value.IsNone());
    TestTrue(TEXT("Execution class defaults null"), Ability->ExecutionClass.IsNull());
    TestTrue(TEXT("ImpactSeconds defaults unresolved"), Ability->ImpactSeconds.Resolution == ELHValueResolution::Unresolved);
    const ULHPresentationDefinition* Presentation = GetDefault<ULHPresentationDefinition>();
    TestTrue(TEXT("Presentation Id defaults None"), Presentation->Id.Value.IsNone());
    TestTrue(TEXT("Visual defaults null"), Presentation->Visual.IsNull());
    TestEqual(TEXT("Presentation primary type is fixed"), Presentation->GetPrimaryAssetId().PrimaryAssetType.GetName(), FName(TEXT("Presentation")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHSchemaLootTest, "Lighthaven.Core.Schema.LootPayload", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FLHSchemaLootTest::RunTest(const FString&)
{
    FLHTakeLootRequest Request;
    Request.Quantity.Resolution = ELHValueResolution::Resolved;
    Request.Quantity.Value = 1; // Synthetic payload, not gameplay tuning.
    TestTrue(TEXT("Unspecified rejects even with positive resolved quantity"), LHValidateLootTransferPayload(Request) == ELHCommandReason::InvalidRequest);
    Request.Kind = ELHLootTransferKind::Gold;
    TestTrue(TEXT("Explicit gold with default item sentinel passes payload check"), LHValidateLootTransferPayload(Request) == ELHCommandReason::None);
    Request.Item.InstanceId = FGuid(1, 2, 3, 4);
    TestTrue(TEXT("Gold with partial item identity rejects"), LHValidateLootTransferPayload(Request) == ELHCommandReason::InvalidRequest);
    Request.Kind = ELHLootTransferKind::Item;
    TestTrue(TEXT("Incomplete item identity rejects"), LHValidateLootTransferPayload(Request) == ELHCommandReason::InvalidRequest);
    Request.Item.RunId = FGuid(5, 6, 7, 8);
    Request.Item.Area.Content.Value = TEXT("Area.TempleB1");
    TestTrue(TEXT("Explicit valid item passes payload check"), LHValidateLootTransferPayload(Request) == ELHCommandReason::None);
    Request.Quantity.Value = 0;
    TestTrue(TEXT("Zero quantity rejects"), LHValidateLootTransferPayload(Request) == ELHCommandReason::InvalidRequest);
    Request.Quantity.Value = -1;
    TestTrue(TEXT("Negative quantity rejects"), LHValidateLootTransferPayload(Request) == ELHCommandReason::InvalidRequest);
    Request.Quantity.Value = 1;
    Request.Quantity.Resolution = ELHValueResolution::Unresolved;
    TestTrue(TEXT("Unresolved quantity rejects"), LHValidateLootTransferPayload(Request) == ELHCommandReason::InvalidRequest);
    return true;
}
#endif
