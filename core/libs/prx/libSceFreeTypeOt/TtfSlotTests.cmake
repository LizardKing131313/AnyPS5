if(NOT BUILD_TESTING OR NOT Python3_Interpreter_FOUND)
    return()
endif()

set(slotTestSource "${LIBS_INCLUDE_DIR}/tests/freetype-slot")
set(slotReferenceSource "${CMAKE_SOURCE_DIR}/3rdparty/freetype-slot-tests")
set(slotGenerated "${CMAKE_CURRENT_BINARY_DIR}/ttf-slot/generated")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_custom_command(OUTPUT ${slotGenerated}/ftbase-slot-import.c ${slotGenerated}/ftobjs-slot-import.c
            ${slotGenerated}/slot-font.h ${slotGenerated}/guest-modules.h
        COMMAND ${Python3_EXECUTABLE} ${slotTestSource}/prepare.py --source ${slotReferenceSource} --output ${slotGenerated}
        DEPENDS ${slotTestSource}/prepare.py ${slotReferenceSource}/src/base/ftobjs.c ${slotReferenceSource}/src/base/ftbase.c)
    add_custom_target(freetype_slot_font DEPENDS ${slotGenerated}/slot-font.h ${slotGenerated}/guest-modules.h)
    include(${slotTestSource}/Reference.cmake)
    add_freetype_slot_reference(guest_freetype_slot_reference "${slotReferenceSource}" "${slotGenerated}")
    add_dependencies(guest_freetype_slot_reference freetype_slot_font)
    target_compile_definitions(guest_freetype_slot_reference PRIVATE FT_New_GlyphSlot=FT_New_GlyphSlot_nid_postfix)
    target_link_libraries(guest_freetype_slot_reference PRIVATE libSceFreeTypeOt libc)
    add_test_executable(guest_freetype_ot_ttf_tests ${slotTestSource}/Ttf.c)
    target_include_directories(guest_freetype_ot_ttf_tests PRIVATE ${slotReferenceSource}/include ${slotGenerated})
    target_link_libraries(guest_freetype_ot_ttf_tests PRIVATE guest_freetype_slot_reference libSceFreeTypeOt libc)
    add_dependencies(guest_freetype_ot_ttf_tests freetype_slot_font)
    add_test(NAME guest_freetype_ot_ttf_hinting COMMAND guest_freetype_ot_ttf_tests)
elseif(WIN32)
    set(ANYPS5_FREETYPE_SLOT_ZIG "" CACHE FILEPATH "Zig compiler for the LP64 FreeType slot integration test on Windows")
    if(NOT ANYPS5_FREETYPE_SLOT_ZIG)
        message(STATUS "FreeType slot TTF integration test: set ANYPS5_FREETYPE_SLOT_ZIG to enable the LP64 guest build on Windows")
        return()
    endif()
    set(slotGuest "${CMAKE_CURRENT_BINARY_DIR}/ttf-slot/guest")
    get_filename_component(slotHostBin "${CMAKE_CXX_COMPILER}" DIRECTORY)
    file(GLOB_RECURSE slotReferenceInputs CONFIGURE_DEPENDS ${slotReferenceSource}/*.c ${slotReferenceSource}/*.h)
    add_custom_command(OUTPUT ${slotGuest}/SlotTtf.exe
        COMMAND ${Python3_EXECUTABLE} ${slotTestSource}/build_guest.py
            --source ${slotReferenceSource} --test-source ${slotTestSource} --output ${slotGuest}
            --zig ${ANYPS5_FREETYPE_SLOT_ZIG} --cmake ${CMAKE_COMMAND} --ninja ${CMAKE_MAKE_PROGRAM}
            --host-bin ${slotHostBin} --relinker $<TARGET_FILE:relinker> --patcher $<TARGET_FILE:nid_patcher>
            --runtime ${UNPATCHED_DIR} --exports ${LIBC_EXPORT_LIST}
        DEPENDS libSceFreeTypeOt libSceFreeType libc libkernel relinker nid_patcher ${LIBC_EXPORT_LIST}
            ${slotTestSource}/build_guest.py ${slotTestSource}/prepare.py ${slotTestSource}/Ttf.c
            ${slotTestSource}/CMakeLists.txt ${slotTestSource}/Reference.cmake ${slotReferenceInputs}
        VERBATIM)
    add_custom_target(guest_freetype_ot_ttf_tests ALL DEPENDS ${slotGuest}/SlotTtf.exe)
    add_test(NAME guest_freetype_ot_ttf_hinting COMMAND ${slotGuest}/SlotTtf.exe)
    set_tests_properties(guest_freetype_ot_ttf_hinting PROPERTIES WORKING_DIRECTORY ${slotGuest})
else()
    return()
endif()
set_tests_properties(guest_freetype_ot_ttf_hinting PROPERTIES TIMEOUT 20)
