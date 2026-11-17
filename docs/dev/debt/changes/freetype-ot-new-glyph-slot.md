# libSceFreeTypeOt

## add Unknown function info

- [FT_New_GlyphSlot](../../../core/libs/prx/libSceFreeTypeOt/src/SlotCreation.cpp) (libSceFreeTypeOt) - PS5 prototype, LP64 layouts, provider version and results are unmeasured. Follows FreeType 2.4.12 [ftobjs.c](https://github.com/freetype/freetype/blob/cecf93ef90c660a8b0b45e5adbbfb5ea443fb6b9/src/base/ftobjs.c#L249-L439) and [ftgloadr.c](https://github.com/freetype/freetype/blob/cecf93ef90c660a8b0b45e5adbbfb5ea443fb6b9/src/base/ftgloadr.c#L44-L145) using guest module, driver, slot and loader records with System V callbacks, not native Windows FreeType objects. Missing allocator callbacks or driver metadata and a slot smaller than its public record throw instead of invalid accesses; other non-null pointers are unchecked. A generated-TTF integration test checks native SCFS hinting, rendering and destruction with the same slot. Closes when: compared with the console provider. Labels: console, tested, windows.
