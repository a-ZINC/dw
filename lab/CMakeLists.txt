function(dw_lab name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE dw_options)
endfunction()

dw_lab(hello)
dw_lab(leak_bounded)
dw_lab(bugs)
dw_lab(fd_leak)
dw_lab(fd_raii)
dw_lab(observe_pid)
# dw_lab(bench)

