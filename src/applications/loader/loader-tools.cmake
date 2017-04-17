# In all project subdirectories
set(loader_tools mpt-loader.py)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/../../build/target/bin)

# ... then traverse subdirectories
foreach(tool ${loader_tools}) 
    message("Copying script file ${CMAKE_CURRENT_SOURCE_DIR}/${tool} to runtime dir ${EXECUTABLE_OUTPUT_PATH}")
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${tool} DESTINATION ${EXECUTABLE_OUTPUT_PATH})
endforeach(tool)


# In all project subdirectories
set(loader_config config/mpt-loader.conf config/sample-test-case.conf)

set(SHARED_DATA_OUTPUT_PATH ${CMAKE_BINARY_DIR}/../../build/target/${CMAKE_INSTALL_DATAROOTDIR}/mpt)

# ... then traverse subdirectories
foreach(config ${loader_config}) 
    message(STATUS "Copying configuration data ${CMAKE_CURRENT_SOURCE_DIR}/${config} to shared data dir ${SHARED_DATA_OUTPUT_PATH}")
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${config} DESTINATION ${SHARED_DATA_OUTPUT_PATH})
endforeach(config)
