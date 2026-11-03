#include "PackType.h"

#include "StringUtil.h"

namespace jc8 {

namespace {
struct Entry {
    PackType type;
    const char* name;
    const char* label;
};

const Entry kEntries[] = {
    {PackType::Piece, "PIECE", "Piece"},
    {PackType::Set, "SET", "Set"},
    {PackType::Roll, "ROLL", "Roll"},
    {PackType::Box, "BOX", "Box"},
    {PackType::Pack, "PACK", "Pack"},
    {PackType::Ream, "REAM", "Ream"},
    {PackType::Bundle, "BUNDLE", "Bundle"},
    {PackType::Gallon, "GALLON", "Gallon"},
    {PackType::Kilo, "KILO", "Kilo"},
    {PackType::Other, "OTHER", "Other"}};
}

std::string packLabel(PackType type) {
    for (const Entry& e : kEntries) {
        if (e.type == type) {
            return e.label;
        }
    }
    return "Other";
}

std::string packName(PackType type) {
    for (const Entry& e : kEntries) {
        if (e.type == type) {
            return e.name;
        }
    }
    return "OTHER";
}

PackType parsePackType(const std::string& text) {
    std::string key = toUpper(trim(text));
    if (key.empty()) {
        return PackType::Piece;
    }
    for (const Entry& e : kEntries) {
        if (key == e.name) {
            return e.type;
        }
    }
    return PackType::Other;
}

const std::vector<PackType>& allPackTypes() {
    static const std::vector<PackType> all = [] {
        std::vector<PackType> v;
        for (const Entry& e : kEntries) {
            v.push_back(e.type);
        }
        return v;
    }();
    return all;
}

}
