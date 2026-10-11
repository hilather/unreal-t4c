#!/usr/bin/env python3
"""Audit three full-frame B1 PNGs; requires ImageMagick (no image alteration).

Prototype W5-09e presentation targets: exported sRGB Rec.709 luma mean .15-.25,
share below .02 <= .20 (baseline .375/.540/.627). Do not mask exterior void or
UI to pass this check. Pixel acceptance complements the native pool/gap audit;
neither establishes subjective appearance or gameplay readability alone.
"""
import argparse
import json
import subprocess


def measure(path):
    raw = subprocess.check_output(["magick", str(path), "-depth", "8", "rgb:-"])
    if not raw or len(raw) % 3:
        raise ValueError("Expected nonempty RGB8 image: " + str(path))
    luma = [(.2126*r + .7152*g + .0722*b)/255
            for r, g, b in zip(raw[::3], raw[1::3], raw[2::3])]
    mean = sum(luma)/len(luma)
    black = sum(value < .02 for value in luma)/len(luma)
    return {"image": str(path), "mean": mean, "near_black_below_002": black,
            "passes": .15 <= mean <= .25 and black <= .20}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("captures", nargs=3, help="Same room1/room2/room3 views from rebuilt package")
    args = parser.parse_args()
    rows = [measure(path) for path in args.captures]
    print(json.dumps({"metric": "Full-frame exported sRGB Rec.709 luma",
                      "mean_target": [.15, .25], "near_black_max": .20,
                      "captures": rows}, indent=2))
    raise SystemExit(0 if all(row["passes"] for row in rows) else 1)
