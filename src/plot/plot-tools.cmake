# In all project subdirectories
set(plot_tools mpt-parse.sh)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/../../build/target/bin)

# ... then traverse subdirectories
foreach(tool ${plot_tools}) 
    message("Copying script file ${CMAKE_CURRENT_SOURCE_DIR}/${tool} to runtime dir ${EXECUTABLE_OUTPUT_PATH}")
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${tool} DESTINATION ${EXECUTABLE_OUTPUT_PATH})
endforeach(tool)
