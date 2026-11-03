#pragma once

#include <string>
#include <vector>

namespace jc8 {

enum class PackType {
    Piece,
    Set,
    Roll,
    Box,
    Pack,
    Ream,
    Bundle,
    Gallon,
    Kilo,
    Other
};

std::string packLabel(PackType type);
std::string packName(PackType type);
PackType parsePackType(const std::string& text);
const std::vector<PackType>& allPackTypes();

}
