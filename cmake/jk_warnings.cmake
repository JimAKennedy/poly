# jk_warnings.cmake — jk.digital shared compiler warning configuration
#
# Usage (after smtg_enable_vst3_sdk()):
#   include(cmake/jk_warnings.cmake)
#   jk_suppress_sdk_warnings()
#   jk_target_warnings(MyPlugin)

function(jk_target_warnings target)
    get_target_property(_type ${target} TYPE)

    if(_type STREQUAL "INTERFACE_LIBRARY")
        set(_scope INTERFACE)
    else()
        set(_scope PRIVATE)
    endif()

    if(MSVC)
        target_compile_options(${target} ${_scope} /W4)
    else()
        target_compile_options(${target} ${_scope} -Wall -Wextra)
    endif()
endfunction()

function(jk_suppress_sdk_warnings)
    set(_sdk_targets
      vstgui vstgui_uidescription vstgui_support
      sdk sdk_common sdk_hosting base pluginterfaces)

    foreach(_t ${_sdk_targets})
        if(TARGET ${_t})
            get_target_property(_opts ${_t} COMPILE_OPTIONS)
            if(_opts)
                list(REMOVE_ITEM _opts "-Werror")
                set_target_properties(${_t} PROPERTIES COMPILE_OPTIONS "${_opts}")
            endif()
        endif()
    endforeach()

    if(TARGET vstgui)
        if(MSVC)
            target_compile_options(vstgui PRIVATE /wd4996)
        else()
            target_compile_options(vstgui PRIVATE -Wno-deprecated-declarations)
        endif()
    endif()
endfunction()
