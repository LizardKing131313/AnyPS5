/***************************************************************************/
/*                                                                         */
/*  ftdriver.h                                                             */
/*                                                                         */
/*    FreeType font driver interface (specification).                      */
/*                                                                         */
/*  Copyright 1996-2003, 2006, 2008, 2011-2012 by                          */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/
// https://github.com/freetype/freetype/blob/cecf93ef90c660a8b0b45e5adbbfb5ea443fb6b9/include/freetype/internal/ftdriver.h

/***************************************************************************/
/*                                                                         */
/*  ftobjs.h                                                               */
/*                                                                         */
/*    The FreeType private base classes (specification).                   */
/*                                                                         */
/*  Copyright 1996-2006, 2008, 2010, 2012 by                               */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/
// https://github.com/freetype/freetype/blob/cecf93ef90c660a8b0b45e5adbbfb5ea443fb6b9/include/freetype/internal/ftobjs.h

/***************************************************************************/
/*                                                                         */
/*  ftgloadr.h                                                             */
/*                                                                         */
/*    The FreeType glyph loader (specification).                           */
/*                                                                         */
/*  Copyright 2002, 2003, 2005, 2006 by                                    */
/*  David Turner, Robert Wilhelm, and Werner Lemberg                       */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/
// https://github.com/freetype/freetype/blob/cecf93ef90c660a8b0b45e5adbbfb5ea443fb6b9/include/freetype/internal/ftgloadr.h

/***************************************************************************/
/*                                                                         */
/*  ftmodapi.h                                                             */
/*                                                                         */
/*    FreeType modules public interface (specification).                   */
/*                                                                         */
/*  Copyright 1996-2003, 2006, 2008-2010, 2012 by                          */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/
// https://github.com/freetype/freetype/blob/cecf93ef90c660a8b0b45e5adbbfb5ea443fb6b9/include/freetype/ftmodapi.h

#pragma once

#include "prx/libSceFreeTypeOt/include/GlyphSlot.hpp"

namespace FreeType {

struct GuestSlotModuleClass {
    std::uint64_t flags;
    std::int64_t size;
    const char* name;
    std::int64_t version;
    std::int64_t requiresVersion;
    const void* moduleInterface;
    void* init;
    void* done;
    void* getInterface;
};

struct GuestSlotDriverClass {
    GuestSlotModuleClass root;
    std::int64_t faceSize;
    std::int64_t sizeSize;
    std::int64_t slotSize;
    void* initFace;
    void* doneFace;
    void* initSize;
    void* doneSize;
    int (APS5_VABI *initSlot)(GuestGlyphSlotRec*);
    void (APS5_VABI *doneSlot)(GuestGlyphSlotRec*);
};

struct GuestSlotDriver {
    const GuestSlotModuleClass* moduleClass;
    void* library;
    GuestMemoryRec* memory;
    const GuestSlotDriverClass* clazz;
};

struct GuestSlotGlyphLoad {
    GuestOutline outline;
    GuestVector* extraPoints;
    GuestVector* extraPoints2;
    std::uint32_t numSubglyphs;
    void* subglyphs;
};

struct GuestSlotGlyphLoader {
    GuestMemoryRec* memory;
    std::uint32_t maxPoints;
    std::uint32_t maxContours;
    std::uint32_t maxSubglyphs;
    std::uint8_t useExtra;
    GuestSlotGlyphLoad base;
    GuestSlotGlyphLoad current;
    void* other;
};

static_assert(offsetof(GuestSlotDriverClass, slotSize) == 88);
static_assert(offsetof(GuestSlotDriverClass, initSlot) == 128 && offsetof(GuestSlotDriverClass, doneSlot) == 136);
static_assert(offsetof(GuestSlotDriver, memory) == 16 && offsetof(GuestSlotDriver, clazz) == 24);
static_assert(sizeof(GuestSlotGlyphLoad) == 72 && sizeof(GuestSlotGlyphLoader) == 176);
static_assert(offsetof(GuestSlotGlyphLoader, base) == 24 && offsetof(GuestSlotGlyphLoader, current) == 96);

}

extern "C" {
int APS5_VABI FT_New_GlyphSlot_nid_postfix(FreeType::GuestFaceRec* face, FreeType::GuestGlyphSlotRec** output);
}
