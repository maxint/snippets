# create vcproj.user file for Visual Studio to set debug working directory
function(create_vcproj_userfile TARGETNAME)
    if (MSVC)
        # set default work directory and path directory
        if (CT_ENV_DIR_ADDED)
            set(CT_ENV_DIR "PATH=$PATH;${CT_ENV_DIR_ADDED}")
        endif()
        if (NOT CT_WORK_DIR)
            set(CT_WORK_DIR "${CMAKE_SOURCE_DIR}")
        endif ()

        set(TEMPLATE_FILE ${CMAKE_SOURCE_DIR}/cmake/VisualStudioUserFile.vcproj.user.in)
        set(OLD_VCPROJ_USERFILE ${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}.vcproj.$ENV{USERDOMAIN}.$ENV{USERNAME}.user)
        set(NEW_VCPROJ_USERFILE ${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}.vcproj.user)

        file(REMOVE ${OLD_VCPROJ_USERFILE})
        message(STATUS "Deleted ${OLD_VCPROJ_USERFILE}")
        
        configure_file(
            ${TEMPLATE_FILE}
            ${NEW_VCPROJ_USERFILE}
            @ONLY # only @VAR@ will be replaced
            )
        message(STATUS "Generated ${NEW_VCPROJ_USERFILE}")
    endif ()
endfunction(create_vcproj_userfile)

# create resources.cfg files for OGRE project
function(create_ogre_resources_cfgfile OVERWRITE)
    # 1. OGRE media directory
    string(REPLACE "\\" "/" OGRE_MEDIA_DIR $ENV{OGRE_SOURCE}/Samples/Media)
    set(TEMPLATE_FILE ${CMAKE_SOURCE_DIR}/cmake/ogre_resources.cfg.in)
    foreach(CONF_TYPE ${CMAKE_CONFIGURATION_TYPES} "") # add a cfg file for common use
        set(NEW_RESOURCES_CFGFILE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONF_TYPE}/resources.cfg)
        if (OVERWRITE OR NOT EXISTS ${NEW_RESOURCES_CFGFILE})
            configure_file(
                ${TEMPLATE_FILE}
                ${NEW_RESOURCES_CFGFILE}
                @ONLY
                )
            message(STATUS "Generated ${NEW_RESOURCES_CFGFILE}")
        endif ()
    endforeach ()
endfunction(create_ogre_resources_cfgfile OVERWRITE)

# create plugins.cfg files for OGRE project
function(create_ogre_plugins_cfgfile)
    foreach(CONF_TYPE ${CMAKE_CONFIGURATION_TYPES})
        # 1. OGRE runtime directory
        string(REPLACE "\\" "/" OGRE_BIN_DIR $ENV{OGRE_BUILD}/bin/${CONF_TYPE})
        # 2. debug or release
        if (${CONF_TYPE} MATCHES "[Dd]ebug")
            set(OGRE_DEBUG_POSTFIX "_d")
        else ()
            set(OGRE_DEBUG_POSTFIX "")
        endif ()
        set(TEMPLATE_FILE ${CMAKE_SOURCE_DIR}/cmake/ogre_plugins.cfg.in)
        set(NEW_PLUGINS_CFGFILE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONF_TYPE}/plugins.cfg)
        if (OVERWRITE OR NOT EXISTS ${NEW_PLUGINS_CFGFILE})
            configure_file(
                ${TEMPLATE_FILE}
                ${NEW_PLUGINS_CFGFILE}
                @ONLY
                )
            message(STATUS "Generated ${NEW_PLUGINS_CFGFILE}")
        endif ()
    endforeach ()
endfunction(create_ogre_plugins_cfgfile)

# a convenient call for two functions above
function(create_ogre_config_files OVERWRITE)
    create_ogre_resources_cfgfile(${OVERWRITE})
    create_ogre_plugins_cfgfile(${OVERWRITE})
endfunction(create_ogre_config_files)
