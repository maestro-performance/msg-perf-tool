# In all project subdirectories
set(loader_tools mpt-loader.py)
set(loader_py_packages mpt)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/../../build/target/bin)
set(LIBRARY_OUPTUT_PATH ${CMAKE_BINARY_DIR}/../../build/target/lib)

# ... then traverse subdirectories
foreach(tool ${loader_tools}) 
    message("Copying script file ${CMAKE_CURRENT_SOURCE_DIR}/${tool} to runtime dir ${EXECUTABLE_OUTPUT_PATH}")
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${tool} DESTINATION ${EXECUTABLE_OUTPUT_PATH})
endforeach(tool)

# ... then traverse subdirectories
foreach(loader_py_package ${loader_py_packages})
    message("Copying loader package ${CMAKE_CURRENT_SOURCE_DIR}/${loader_py_package} to site-packages dir ${LIBRARY_OUPTUT_PATH}/${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages")
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${loader_py_package} DESTINATION ${LIBRARY_OUPTUT_PATH}/${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages)
endforeach(loader_py_package)


# In all project subdirectories
set(loader_config config/mpt-loader.conf config/sample-test-case.conf)

set(SHARED_DATA_OUTPUT_PATH ${CMAKE_BINARY_DIR}/../../build/target/share/mpt)

# ... then traverse subdirectories
foreach(config ${loader_config}) 
    message("Copying configuration data ${CMAKE_CURRENT_SOURCE_DIR}/${config} to shared data dir ${SHARED_DATA_OUTPUT_PATH}")
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${config} DESTINATION ${SHARED_DATA_OUTPUT_PATH})
endforeach(config)
