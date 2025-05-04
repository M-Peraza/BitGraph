#########################
#headers for BITSCAN lib
#########################
set(useful_headers_for_IDE
 ${CMAKE_CURRENT_SOURCE_DIR}/bbconfig.h
 ${CMAKE_CURRENT_SOURCE_DIR}/bbtypes.h
 ${CMAKE_CURRENT_SOURCE_DIR}/bitblock.h
 ${CMAKE_CURRENT_SOURCE_DIR}/bbset.h
 ${CMAKE_CURRENT_SOURCE_DIR}/bbset_sparse.h
 ${CMAKE_CURRENT_SOURCE_DIR}/bbscan.h
 ${CMAKE_CURRENT_SOURCE_DIR}/bbscan_sparse.h
 ${CMAKE_CURRENT_SOURCE_DIR}/bbsentinel.h
 ${CMAKE_CURRENT_SOURCE_DIR}/bbobject.h
 ${CMAKE_CURRENT_SOURCE_DIR}/bbalgorithm.h
 ${CMAKE_CURRENT_SOURCE_DIR}/bitscan.h
 ${CMAKE_CURRENT_SOURCE_DIR}/mainpage.dox
 ${CMAKE_CURRENT_SOURCE_DIR}/doxygen_template.h
)

###################
#creates BITSCAN lib
###################
add_library (bitscan STATIC
bbsentinel.cpp 
bitblock.cpp 
bbset.cpp  
bbset_sparse.cpp 
tables.cpp
${useful_headers_for_IDE}
)

set_target_properties(bitscan 
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    #PUBLIC_HEADER   "${useful_headers_for_IDE}"        #does not preserve internal structure  
)


# Make sure the compiler can find include files for our bitscan library
# when other libraries or executables link to bitscan
#target_include_directories (bitscan 
#    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
#)

# Make sure the compiler can find include files for our bitscan library
# when other libraries or executables link to bitscan
target_include_directories(bitscan PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${include_install_dir}>    # <prefix>/include/<PROJECT_NAME>
)

###################
# DOXYGEN DOCUMENTATION
###################
# Look for Doxygen package
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ON)
if(BUILD_DOCUMENTATION)
  find_package(Doxygen)
  if(DOXYGEN_FOUND)
    # Set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # Configure the Doxyfile
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    # Add a custom target for documentation
    add_custom_target(doc
      COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating API documentation with Doxygen"
      VERBATIM
    )

    # Install the documentation if requested
    if(INSTALL_BITGRAPH)
      install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs/html
        DESTINATION share/doc/bitscan
        OPTIONAL
      )
    endif()
  else()
    message(STATUS "Doxygen not found, documentation will not be built")
  endif()
endif()

#############
# Install BITSCAN lib
# BITSCAN lib header files are deployed manually
# to preserve internal structure
#############

if(INSTALL_BITGRAPH)

#Install BITSCAN library
install(
   TARGETS bitscan
   EXPORT  "${TARGETS_EXPORT_NAME}"
   ARCHIVE DESTINATION lib
   LIBRARY DESTINATION lib
   RUNTIME DESTINATION bin
   INCLUDES DESTINATION include
)

#install header files
file(GLOB_RECURSE DEPLOY_FILES_AND_DIRS "${source_code_dir}/bitscan/*.h")
foreach(ITEM ${DEPLOY_FILES_AND_DIRS})
    file(RELATIVE_PATH REL_PATH ${source_code_dir} ${ITEM})
    get_filename_component(DIR_PATH ${REL_PATH} DIRECTORY)
    install(FILES ${ITEM} DESTINATION ${include_install_dir}/${DIR_PATH})	 
 endforeach()

endif(INSTALL_BITGRAPH)

###################
# UNIT TESTS for BITSCAN lib  
###################
#option(bitscan_build_tests "Build bitscan tests." ON)
if (BUILD_TESTS)

	add_subdirectory(tests)

endif()


