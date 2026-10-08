#pragma once
#include "CoreMinimal.h"

// V-03 authored prototype design values, 2026-10-08. No font assets embedded.
// Opaque sRGB converted once for rendering. Engine default font until asset approval.
struct LIGHTHAVEN_API FLHUIStyle
{
    FLinearColor Background = Color(0x171717), Surface = Color(0x242424), Panel = Color(0x242424);
    FLinearColor Raised = Color(0x333333), TextPrimary = Color(0xF2F2F2);
    FLinearColor TextSecondary = Color(0xC4C4C4), TextDisabled = Color(0xC4C4C4);
    FLinearColor Edge = Color(0x909090), HoverEdge = Color(0xC4C4C4);
    FLinearColor Accent = Color(0xE9BF79), FocusRing = Color(0xE9BF79);
    FLinearColor Warning = Color(0xE9BF79), Error = Color(0xE9BF79);
    FLinearColor Success = Color(0xA9C4CC), Info = Color(0xA9C4CC), SelectedMark = Color(0xF2F2F2);
    FLinearColor BarTrack = Color(0x171717), BarHealth = Color(0xC4C4C4);
    FLinearColor BarMana = Color(0xA9C4CC), BarXP = Color(0xE9BF79), UnknownMark = Color(0xC4C4C4);
    float Title = 48, Section = 36, Body = 30, Control = 30, Numeric = 30, Metadata = 24;
    float TitleLine = 60, SectionLine = 48, BodyLine = 42, MetadataLine = 36;
    float Unit = 12, ControlInset = 12, PanelInset = 24, Gutter = 24, TargetMin = 72;
    float ControlTwoLine = 108, RowGap = 12, CornerRadius = 6, EdgeStroke = 3;
    float FocusStroke = 3, FocusGap = 6, Icon = 36, IconGap = 12, HintGroupGap = 36;
    float BarHeight = 12, BarLabelGap = 12;
    FVector4 Safe = FVector4(72,54,1776,972), Header = FVector4(72,54,1776,96);
    FVector4 BodyBounds = FVector4(72,174,1776,720), Status = FVector4(72,900,1776,48);
    FVector4 Footer = FVector4(72,954,1776,72), Modal = FVector4(372,282,1176,516);
    static float Scale(FVector2D Viewport) { return FMath::Min(Viewport.X / 1920.f, Viewport.Y / 1080.f); }
    static FLinearColor Color(uint32 RGB)
    { return FLinearColor::FromSRGBColor(FColor((RGB >> 16) & 255, (RGB >> 8) & 255, RGB & 255, 255)); }
};
