# In all project subdirectories
set(runner_tools mpt-runner.sh)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/../../build/target/bin)

# ... then traverse subdirectories
foreach(tool ${runner_tools}) 
    message("Copying script file ${CMAKE_CURRENT_SOURCE_DIR}/${tool} to runtime dir ${EXECUTABLE_OUTPUT_PATH}")
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${tool} DESTINATION ${EXECUTABLE_OUTPUT_PATH})
endforeach(tool)


# In all project subdirectories
set(plot_scripts latency.ps throughput.ps report.html)

set(SHARED_DATA_OUTPUT_PATH ${CMAKE_BINARY_DIR}/../../build/target/share/mpt)

# ... then traverse subdirectories
foreach(plot_data ${plot_scripts}) 
    message("Copying plot data ${CMAKE_CURRENT_SOURCE_DIR}/${plot_data} to shared data dir ${SHARED_DATA_OUTPUT_PATH}")
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${plot_data} DESTINATION ${SHARED_DATA_OUTPUT_PATH})
endforeach(plot_data)
