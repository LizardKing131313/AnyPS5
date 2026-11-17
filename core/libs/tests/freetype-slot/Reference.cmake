function(add_freetype_slot_reference target source generated)
    add_library(${target} SHARED EXCLUDE_FROM_ALL
        ${generated}/ftbase-slot-import.c
        ${source}/src/base/ftinit.c ${source}/src/base/ftsystem.c ${source}/src/base/ftdebug.c
        ${source}/src/base/ftbitmap.c ${source}/src/base/ftglyph.c
        ${source}/src/truetype/truetype.c ${source}/src/sfnt/sfnt.c
        ${source}/src/smooth/smooth.c ${source}/src/raster/raster.c
        ${source}/src/gzip/ftgzip.c ${source}/src/lzw/ftlzw.c ${source}/src/bzip2/ftbzip2.c)
    target_include_directories(${target} PRIVATE ${source}/include ${source}/src/base ${generated})
    target_compile_definitions(${target} PRIVATE FT2_BUILD_LIBRARY FT_CONFIG_MODULES_H="guest-modules.h")
endfunction()
