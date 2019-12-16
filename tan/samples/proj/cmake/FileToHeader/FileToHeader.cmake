function(markGenerated InputDirectories OutputNames OutHeaders)
  list(LENGTH ${InputDirectories} CL_FilesCount)
  math(EXPR CL_Files_MaxIndex ${CL_FilesCount}-1)

  # create empty list
  list(APPEND Headers "")
  #set(Headers ${InputHeaders})

  foreach(CL_FILE_INDEX RANGE ${CL_Files_MaxIndex})
    list(GET ${InputDirectories} ${CL_FILE_INDEX} CL_DIRECTORY)
    list(GET ${OutputNames} ${CL_FILE_INDEX} CL_OUTPUT)

    set(GeneratedFileName ${CL_DIRECTORY}/${CL_OUTPUT})
    #message("process: ${GeneratedFileName}")

    set_source_files_properties(
      ${GeneratedFileName}
      PROPERTIES
      GENERATED TRUE
      )

    list(APPEND Headers ${GeneratedFileName})
  endforeach()

  SET(OutHeaders ${Headers} PARENT_SCOPE)

endfunction()

function(generateCLKernelHeader OutputTarget InputDirectories InputFiles OutputNames)

  list(LENGTH ${InputFiles} CL_FilesCount)
  math(EXPR CL_Files_MaxIndex ${CL_FilesCount}-1)

  foreach(CL_FILE_INDEX RANGE ${CL_Files_MaxIndex})
    list(GET ${InputDirectories} ${CL_FILE_INDEX} CL_DIRECTORY)
    list(GET ${InputFiles} ${CL_FILE_INDEX} CL_FILE)
    list(GET ${OutputNames} ${CL_FILE_INDEX} CL_OUTPUT)
    #message("create commands to convert file ${CL_DIRECTORY}/${CL_FILE} to ${CL_OUTPUT}")

    set(TARGET_NAME ${CL_FILE})

    if(NOT TARGET ${TARGET_NAME})

      add_custom_command(
        OUTPUT
        ${CL_OUTPUT}
        WORKING_DIRECTORY
        ${CL_DIRECTORY}
        DEPENDS
        ${CL_FILE}
        COMMAND
        #CLKernelPreprocessor ${CL_FILE} ${CL_OUTPUT}
        FileToHeader ${CL_FILE} ${CL_OUTPUT}
        #"${TAN_ROOT}/thirdparty/file_to_header/file_to_header" ${CL_FILE} ${CL_OUTPUT}
        COMMENT
        "Building binary resource header (${CL_OUTPUT}) for  ${CL_DIRECTORY}/${CL_FILE}..."
        VERBATIM
        )

      add_custom_target(
        ${TARGET_NAME}

        WORKING_DIRECTORY
        ${CL_DIRECTORY}
        DEPENDS
        ${CL_OUTPUT}
        COMMENT
        "TARGET ${CL_FILE}"
        #BYPRODUCTS ${CL_FILE}
        )

    endif()

    add_dependencies(${TARGET_NAME} FileToHeader)
    add_dependencies(${OutputTarget} ${TARGET_NAME})

  endforeach()

endfunction()