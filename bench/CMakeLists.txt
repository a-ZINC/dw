function(dw_bench name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE dw_core)
endfunction()

dw_bench(bench_snapshot)
