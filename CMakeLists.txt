add_executable(microwetland_planner
    src/CliPlanner.cpp
    src/WetlandKernel.cpp
)

target_include_directories(microwetland_planner PRIVATE include)
