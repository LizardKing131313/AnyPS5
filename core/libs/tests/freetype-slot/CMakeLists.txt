cmake_minimum_required(VERSION 3.20)

if(DEFINED FREETYPE_SLOT_IMPORTS)
    project(FreeTypeSlotGuest C)
    include(Reference.cmake)
    add_freetype_slot_reference(slotFreeType "${FREETYPE_SLOT_SOURCE}" "${FREETYPE_SLOT_GENERATED}")
    set_target_properties(slotFreeType PROPERTIES PREFIX "" SUFFIX ".prx")
    target_link_options(slotFreeType PRIVATE -nostdlib "-Wl,--hash-style=sysv" "-Wl,--no-as-needed")
    target_link_libraries(slotFreeType PRIVATE "${FREETYPE_SLOT_IMPORTS}/libSceFreeTypeOt.so" "${FREETYPE_SLOT_IMPORTS}/libc.so")
    return()
endif()

project(FreeTypeSlotLinux C CXX)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux" OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "The standalone slot tests require 64-bit Linux; use the main build and ANYPS5_FREETYPE_SLOT_ZIG on Windows")
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
enable_testing()
get_filename_component(slotRoot "${CMAKE_CURRENT_LIST_DIR}/../../../.." ABSOLUTE)
set(slotSource "${slotRoot}/3rdparty/freetype-slot-tests")
set(slotGenerated "${CMAKE_CURRENT_BINARY_DIR}/generated")
add_custom_command(OUTPUT ${slotGenerated}/ftbase-slot-import.c ${slotGenerated}/ftobjs-slot-import.c
        ${slotGenerated}/slot-font.h ${slotGenerated}/guest-modules.h
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/prepare.py --source ${slotSource} --output ${slotGenerated}
    DEPENDS ${CMAKE_CURRENT_LIST_DIR}/prepare.py ${slotSource}/src/base/ftobjs.c ${slotSource}/src/base/ftbase.c)
add_custom_target(slot_font DEPENDS ${slotGenerated}/slot-font.h ${slotGenerated}/guest-modules.h)
add_library(slot_production SHARED
    ${slotRoot}/core/libs/prx/libSceFreeTypeOt/src/SlotCreation.cpp
    ${slotRoot}/core/libs/prx/libSceFreeTypeOt/src/GlyphSlot.cpp
    ${slotRoot}/core/libs/prx/libSceFreeTypeOt/src/Memory.cpp
    Unsupported.cpp)
target_include_directories(slot_production PRIVATE ${slotRoot}/core/libs)
include(Reference.cmake)
add_freetype_slot_reference(slotFreeType "${slotSource}" "${slotGenerated}")
add_dependencies(slotFreeType slot_font)
target_compile_definitions(slotFreeType PRIVATE FT_New_GlyphSlot=FT_New_GlyphSlot_nid_postfix)
target_link_libraries(slotFreeType PRIVATE slot_production)
add_executable(guest_freetype_ot_ttf_tests Ttf.c)
target_include_directories(guest_freetype_ot_ttf_tests PRIVATE ${slotSource}/include ${slotGenerated})
target_link_libraries(guest_freetype_ot_ttf_tests PRIVATE slotFreeType)
add_dependencies(guest_freetype_ot_ttf_tests slot_font)
add_test(NAME guest_freetype_ot_ttf_hinting COMMAND guest_freetype_ot_ttf_tests)
add_executable(guest_freetype_ot_new_glyph_slot_tests ../GuestFreeTypeOtNewGlyphSlot.cpp)
target_include_directories(guest_freetype_ot_new_glyph_slot_tests PRIVATE ${slotRoot}/core/libs)
target_link_libraries(guest_freetype_ot_new_glyph_slot_tests PRIVATE slot_production)
add_test(NAME guest_freetype_ot_new_glyph_slot COMMAND guest_freetype_ot_new_glyph_slot_tests)
set_tests_properties(guest_freetype_ot_ttf_hinting guest_freetype_ot_new_glyph_slot PROPERTIES TIMEOUT 20)
