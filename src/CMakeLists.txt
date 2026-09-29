add_library(dw_core STATIC proc_self.cpp log.cpp)
target_link_libraries(dw_core PUBLIC dw_options)