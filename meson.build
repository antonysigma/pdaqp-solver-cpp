project('pdaqp-cpp', 'cpp',
  version : '0.1',
  default_options : [
    'buildtype=release',
    'cpp_std=c++20',
    'cpp_eh=none',
    'b_ndebug=if-release',
])

problem_def_inc = include_directories('examples' / get_option('name'))

if meson.is_cross_build()
  hardware_capability = []
  avr_stdcpp_dep = subproject('avr-libstdcpp').get_variable('avr_stdcpp_dep')
else
  hardware_capability = '-march=native'
  avr_stdcpp_dep = []
endif

subdir('common')
subdir('tree_walker')
subdir('apply_feedback')

pdaqp_solver_dep = declare_dependency(
    sources: [
      'pdaqp-solver.cpp',
    ],
    include_directories: [
      '.',
      problem_def_inc,
    ],
    dependencies: [
      avr_stdcpp_dep,
      tree_walker_heap_dep,
      apply_feedback_dep,
    ]
)

if not meson.is_cross_build()
demo_exe = executable('pdaqp-cpp',
    sources: [
      'demo.cpp',
    ],
    cpp_args: [
      hardware_capability,
      '-ffunction-sections',
      '-fdata-sections',
    ],
    link_args: [
      '-Wl,-gc-sections',
      '-Wl,-Map=demo.map',
    ],
    dependencies: [
      pdaqp_solver_dep,
      subproject('fmt').get_variable('fmt_dep'),
    ],
)
endif