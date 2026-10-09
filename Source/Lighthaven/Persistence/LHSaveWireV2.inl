// Frozen rev-2 save layout is byte-identical to the bounded rev-1 allowlist.
// Future saved-shape changes must retain PayloadV1 and give v2 its own decoder.
static void PayloadV2(FWire& W, FLHSaveSnapshot& V) { PayloadV1(W,V); }

// LHRequest1 / UseItem: explicit ASCII field ordering, no reflection.
static void RequestFields(FWire& W, FLHUseItemRequest& R)
{
    FDepth Depth(W); W.Struct(3);
    Field(W,"Item",R.Item); Field(W,"Request",R.Request); Field(W,"Target",R.Target);
}
