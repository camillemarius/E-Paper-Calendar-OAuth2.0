#pragma once

// Wert = Anzahl Farben; die Indizes entsprechen den Paletten in FlashImage.cpp und der Webseite
enum class ImagePalette : uint8_t
{
    TwoColor = 2,
    ThreeColor = 3,
    SixColor = 6
};