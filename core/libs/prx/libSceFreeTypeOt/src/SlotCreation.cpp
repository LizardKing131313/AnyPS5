/***************************************************************************/
/*                                                                         */
/*  ftobjs.c                                                               */
/*                                                                         */
/*    The FreeType private base classes (body).                            */
/*                                                                         */
/*  Copyright 1996-2013 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/
// https://github.com/freetype/freetype/blob/cecf93ef90c660a8b0b45e5adbbfb5ea443fb6b9/src/base/ftobjs.c
// https://github.com/freetype/freetype/blob/cecf93ef90c660a8b0b45e5adbbfb5ea443fb6b9/src/base/ftgloadr.c

/***************************************************************************/
/*                                                                         */
/*  ftgloadr.c                                                             */
/*                                                                         */
/*    The FreeType glyph loader (body).                                    */
/*                                                                         */
/*  Copyright 2002-2006, 2010, 2013 by                                     */
/*  David Turner, Robert Wilhelm, and Werner Lemberg                       */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

#include "prx/libSceFreeTypeOt/include/SlotCreation.hpp"
#include "prx/libSceFreeTypeOt/include/Memory.hpp"
#include "prx/libc/include/General.hpp"

using namespace FreeType;

namespace {

template <typename T>
void Release(GuestMemoryRec* memory, T*& pointer) {
    ft_mem_free_nid_postfix(memory, pointer);
    pointer = nullptr;
}

void DestroyLoader(GuestSlotGlyphLoader* loader) {
    if (!loader) return;
    auto* memory = loader->memory;
    if (!memory || !memory->free) NotImplemented_nid_no_patch(__func__);
    Release(memory, loader->base.outline.points);
    Release(memory, loader->base.outline.tags);
    Release(memory, loader->base.outline.contours);
    Release(memory, loader->base.extraPoints);
    Release(memory, loader->base.subglyphs);
    loader->base.extraPoints2 = nullptr;
    loader->maxPoints = 0;
    loader->maxContours = 0;
    loader->maxSubglyphs = 0;
    loader->base.outline.n_points = 0;
    loader->base.outline.n_contours = 0;
    loader->base.numSubglyphs = 0;
    loader->current = loader->base;
    ft_mem_free_nid_postfix(memory, loader);
}

void DestroySlot(GuestGlyphSlotRec* slot, const GuestSlotDriver& driver, const GuestSlotDriverClass& clazz, bool outlines) {
    auto* memory = driver.memory;
    if (clazz.doneSlot) clazz.doneSlot(slot);
    ft_glyphslot_free_bitmap_nid_postfix(slot);
    if (slot->internal) {
        auto* internal = static_cast<GuestSlotInternalRec*>(slot->internal);
        if (outlines) {
            DestroyLoader(static_cast<GuestSlotGlyphLoader*>(internal->loader));
            internal->loader = nullptr;
        }
        ft_mem_free_nid_postfix(memory, internal);
        slot->internal = nullptr;
    }
    ft_mem_free_nid_postfix(memory, slot);
}

}

extern "C" {

int APS5_VABI FT_New_GlyphSlot_nid_postfix(GuestFaceRec* face, GuestGlyphSlotRec** output) {
    if (!face || !face->driver) return 0x06;
    const auto& driver = *static_cast<const GuestSlotDriver*>(face->driver);
    if (!driver.clazz || !driver.moduleClass || !driver.memory || !driver.memory->alloc || !driver.memory->free) {
        NotImplemented_nid_no_patch(__func__);
    }
    const auto& clazz = *driver.clazz;
    if (clazz.slotSize < 0) {
        if (output) *output = nullptr;
        return 0x06;
    }
    if (clazz.slotSize < static_cast<std::int64_t>(sizeof(GuestGlyphSlotRec))) NotImplemented_nid_no_patch(__func__);
    const bool outlines = (driver.moduleClass->flags & 0x200) == 0;
    std::int32_t error = 0;
    auto* slot = static_cast<GuestGlyphSlotRec*>(ft_mem_alloc_nid_postfix(driver.memory, clazz.slotSize, &error));
    if (error) {
        if (output) *output = nullptr;
        return error;
    }
    slot->face = face;
    slot->library = driver.library;
    auto* internal = static_cast<GuestSlotInternalRec*>(ft_mem_alloc_nid_postfix(driver.memory, sizeof(GuestSlotInternalRec), &error));
    slot->internal = internal;
    if (!error && outlines) {
        auto* loader = static_cast<GuestSlotGlyphLoader*>(ft_mem_alloc_nid_postfix(driver.memory, sizeof(GuestSlotGlyphLoader), &error));
        if (!error) {
            loader->memory = driver.memory;
            internal->loader = loader;
        }
    }
    if (!error && clazz.initSlot) error = clazz.initSlot(slot);
    if (error) {
        DestroySlot(slot, driver, clazz, outlines);
        return error;
    }
    slot->next = face->glyph;
    face->glyph = slot;
    if (output) *output = slot;
    return 0;
}

}
