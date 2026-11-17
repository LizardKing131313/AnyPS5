#include "prx/libSceFreeTypeOt/include/SlotCreation.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace {

using namespace FreeType;

void Require(bool condition, const char* message) {
    if (condition) return;
    std::fprintf(stderr, "FreeType Ot new glyph slot: %s\n", message);
    std::exit(1);
}

struct Fixture {
    GuestMemoryRec memory{};
    GuestSlotModuleClass module{};
    GuestSlotDriverClass clazz{};
    GuestSlotDriver driver{};
    GuestFaceRec face{};
    GuestGlyphSlotRec previous{};
    std::vector<void*> blocks;
    std::vector<std::int64_t> sizes;
    int failAt = 0;
    int initialized = 0;
    int finalized = 0;
    int initError = 0;
    bool allocateOnInit = false;
    GuestGlyphSlotRec* observedSlot = nullptr;
    unsigned char borrowed[8]{};

    Fixture();
    ~Fixture() {
        for (auto* block : blocks) std::free(block);
    }
};

void* APS5_VABI Allocate(GuestMemoryRec* memory, std::int64_t bytes) {
    auto& fixture = *static_cast<Fixture*>(memory->user);
    fixture.sizes.push_back(bytes);
    if (fixture.failAt == static_cast<int>(fixture.sizes.size())) return nullptr;
    void* block = std::malloc(static_cast<std::size_t>(bytes));
    Require(block != nullptr, "test allocator exhausted");
    std::memset(block, 0xAB, static_cast<std::size_t>(bytes));
    fixture.blocks.push_back(block);
    return block;
}

void APS5_VABI Free(GuestMemoryRec* memory, void* block) {
    auto& fixture = *static_cast<Fixture*>(memory->user);
    const auto found = std::find(fixture.blocks.begin(), fixture.blocks.end(), block);
    Require(found != fixture.blocks.end(), "freed an unowned block or freed twice");
    if (block == fixture.observedSlot) Require(fixture.observedSlot->internal == nullptr, "internal pointer not cleared before freeing slot");
    fixture.blocks.erase(found);
    std::free(block);
}

Fixture& Owner(GuestGlyphSlotRec* slot) {
    return *static_cast<Fixture*>(static_cast<GuestFaceRec*>(slot->face)->generic.data);
}

int APS5_VABI Initialize(GuestGlyphSlotRec* slot) {
    auto& fixture = Owner(slot);
    ++fixture.initialized;
    Require(fixture.face.glyph == &fixture.previous, "slot linked before initialization succeeded");
    Require(slot->library == &fixture, "library not set before callback");
    Require(slot->metrics.width == 0 && slot->bitmap.buffer == nullptr && slot->next == nullptr, "public slot fields not cleared");
    auto* extension = reinterpret_cast<unsigned char*>(slot) + sizeof(*slot);
    for (int i = 0; i < 32; ++i) Require(extension[i] == 0, "driver extension not cleared");
    auto* internal = static_cast<GuestSlotInternalRec*>(slot->internal);
    Require(internal && internal->flags == 0 && internal->glyph_hints == nullptr, "internal record not initialized");
    const bool outlines = (fixture.module.flags & 0x200) == 0;
    auto* loader = static_cast<GuestSlotGlyphLoader*>(internal->loader);
    Require((loader != nullptr) == outlines, "wrong outline loader selection");
    if (loader) {
        Require(loader->memory == &fixture.memory && loader->maxPoints == 0 && loader->base.outline.points == nullptr, "loader not initialized");
        if (fixture.allocateOnInit) {
            loader->base.outline.points = static_cast<GuestVector*>(Allocate(&fixture.memory, 32));
            loader->base.outline.tags = static_cast<unsigned char*>(Allocate(&fixture.memory, 2));
            loader->base.outline.contours = static_cast<std::uint16_t*>(Allocate(&fixture.memory, 2));
            loader->base.extraPoints = static_cast<GuestVector*>(Allocate(&fixture.memory, 64));
            loader->base.extraPoints2 = loader->base.extraPoints + 2;
            loader->base.subglyphs = Allocate(&fixture.memory, 48);
        }
    }
    slot->bitmap.buffer = fixture.allocateOnInit ? static_cast<unsigned char*>(Allocate(&fixture.memory, 8)) : fixture.borrowed;
    if (fixture.allocateOnInit) internal->flags |= GlyphOwnBitmap;
    return fixture.initError;
}

void APS5_VABI Finalize(GuestGlyphSlotRec* slot) {
    auto& fixture = Owner(slot);
    ++fixture.finalized;
    fixture.observedSlot = slot;
    Require(fixture.face.glyph == &fixture.previous, "failed slot changed the face list");
    Require(std::find(fixture.blocks.begin(), fixture.blocks.end(), slot) != fixture.blocks.end(), "slot freed before finalizer");
    if (fixture.initialized) Require(slot->bitmap.buffer != nullptr, "bitmap freed before finalizer");
}

Fixture::Fixture() {
    memory = {this, Allocate, Free, nullptr};
    module.flags = 0x200;
    clazz.slotSize = sizeof(GuestGlyphSlotRec) + 32;
    clazz.initSlot = Initialize;
    clazz.doneSlot = Finalize;
    driver = {&module, this, &memory, &clazz};
    face.driver = &driver;
    face.memory = &memory;
    face.glyph = &previous;
    face.generic.data = this;
}

void Creation(bool outlines, bool outputRequested, bool callback) {
    Fixture fixture;
    if (outlines) fixture.module.flags = 0;
    if (!callback) fixture.clazz.initSlot = nullptr;
    GuestGlyphSlotRec* output = &fixture.previous;
    Require(FT_New_GlyphSlot_nid_postfix(&fixture.face, outputRequested ? &output : nullptr) == 0, "creation failed");
    auto* slot = fixture.face.glyph;
    Require(slot != &fixture.previous && slot->next == &fixture.previous && slot->face == &fixture.face, "new slot not prepended");
    Require(!outputRequested || output == slot, "wrong returned slot");
    Require(fixture.initialized == (callback ? 1 : 0) && fixture.finalized == 0, "wrong lifecycle callbacks on success");
    Require(fixture.sizes.size() == (outlines ? 3u : 2u), "unexpected allocations on success");
    Require(fixture.sizes[0] == fixture.clazz.slotSize, "driver slot size ignored");
    Require(FT_New_GlyphSlot_nid_postfix(nullptr, &output) == 0x06 && output == (outputRequested ? slot : &fixture.previous), "null face changed output");
    fixture.clazz.initSlot = nullptr;
    Require(FT_New_GlyphSlot_nid_postfix(&fixture.face, &output) == 0, "second slot failed");
    Require(output == fixture.face.glyph && output->next == slot && slot->next == &fixture.previous, "multiple slot order broken");
}

void AllocationFailure(bool outlines, int failure) {
    Fixture fixture;
    if (outlines) fixture.module.flags = 0;
    fixture.failAt = failure;
    GuestGlyphSlotRec* output = &fixture.previous;
    Require(FT_New_GlyphSlot_nid_postfix(&fixture.face, &output) == 0x40, "allocation error not propagated");
    Require(fixture.face.glyph == &fixture.previous && fixture.blocks.empty(), "failed allocation leaked or changed list");
    Require(output == (failure == 1 ? nullptr : &fixture.previous), "failure output differs from FreeType");
    Require(fixture.initialized == 0 && fixture.finalized == (failure == 1 ? 0 : 1), "wrong callbacks after allocation failure");
}

void CallbackFailure(bool outlines, bool owned) {
    Fixture fixture;
    if (outlines) fixture.module.flags = 0;
    fixture.initError = 0x55;
    fixture.allocateOnInit = owned;
    GuestGlyphSlotRec* output = &fixture.previous;
    Require(FT_New_GlyphSlot_nid_postfix(&fixture.face, &output) == 0x55, "driver error not propagated");
    Require(output == &fixture.previous && fixture.face.glyph == &fixture.previous, "driver failure changed published state");
    Require(fixture.blocks.empty(), "driver failure leaked allocations");
    Require(fixture.initialized == 1 && fixture.finalized == 1, "driver cleanup not called exactly once");
}

}

int main() {
    for (bool outlines : {false, true}) {
        for (bool output : {false, true})
            for (bool callback : {false, true}) Creation(outlines, output, callback);
        for (int failure = 1; failure <= (outlines ? 3 : 2); ++failure) AllocationFailure(outlines, failure);
        for (bool owned : {false, true}) CallbackFailure(outlines, owned);
    }
    Fixture fixture;
    fixture.face.driver = nullptr;
    GuestGlyphSlotRec* output = &fixture.previous;
    Require(FT_New_GlyphSlot_nid_postfix(&fixture.face, &output) == 0x06 && output == &fixture.previous, "missing driver not rejected");
    fixture.face.driver = &fixture.driver;
    fixture.clazz.slotSize = -1;
    Require(FT_New_GlyphSlot_nid_postfix(&fixture.face, &output) == 0x06 && output == nullptr && fixture.sizes.empty(), "negative allocation size not rejected");
    std::puts("Guest FreeType Ot new glyph slot: 19 lifecycle and failure scenarios passed");
    return 0;
}
