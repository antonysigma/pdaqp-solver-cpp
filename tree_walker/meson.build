tree_walker_inc = include_directories('inc')

tree_walker_sml_lib = static_library('tree-walker-sml',
    sources: 'src/tree_walker_sml.cpp',
    include_directories: [
        common_inc,
        problem_def_inc,
        tree_walker_inc,
        subproject('boost-sml').get_variable('boost_sml_inc'),
    ],
    cpp_args: [
        hardware_capability,
        '-ftemplate-depth=5000',
        '-ffunction-sections',
        '-fdata-sections',
    ],
    dependencies: [
        avr_stdcpp_dep,
    ],
    build_by_default: false,
)

tree_walker_lut_lib = static_library('tree-walker-lut',
    sources: 'src/tree_walker_lut.cpp',
    include_directories: [
        common_inc,
        problem_def_inc,
        tree_walker_inc,
    ],
    cpp_args: [
        hardware_capability,
        '-ffunction-sections',
        '-fdata-sections',
    ],
    dependencies: [
        avr_stdcpp_dep,
    ],
)

tree_walker_heap_lib = static_library('tree-walker-heap',
    sources: 'src/tree_walker_heap.cpp',
    include_directories: [
        common_inc,
        problem_def_inc,
        tree_walker_inc,
    ],
    cpp_args: [
        hardware_capability,
        '-ffunction-sections',
        '-fdata-sections',
    ],
    dependencies: [
        avr_stdcpp_dep,
    ],
)

tree_walker_heap_dep = declare_dependency(
    link_with: tree_walker_heap_lib,
    include_directories: tree_walker_inc,
)

tree_walker_lut_dep = declare_dependency(
    link_with: tree_walker_lut_lib,
    include_directories: tree_walker_inc,
)

tree_walker_sml_dep = declare_dependency(
    link_with: tree_walker_sml_lib,
    include_directories: tree_walker_inc,
)
