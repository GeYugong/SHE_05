if(DEFINED CONFIG_SAMPLE_SUPPORT_DEMO01)
    set(SOURCES "${SOURCES}" "${CMAKE_CURRENT_SOURCE_DIR}/demo01.c"
                            "${CMAKE_CURRENT_SOURCE_DIR}/bh1750.c"
                            "${CMAKE_CURRENT_SOURCE_DIR}/cw2015.c" PARENT_SCOPE)
endif()