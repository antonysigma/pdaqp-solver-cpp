project('pdaqp-cpp', 'cpp',
  version : '0.1',
  default_options : [
    'buildtype=release',
    'cpp_std=c++20',
    'cpp_eh=none',
    'b_ndebug=if-release',
])

problem_def_inc = include_directories('examples' / get_option('name'))

subdir('common')
subdir('tree_walker')
subdir('apply_feedback')

main_exe = executable('pdaqp-cpp',
    sources: [
      'main.cpp',
    ],
    include_directories: [
      problem_def_inc,
    ],
    cpp_args: [
      '-march=native',
      '-ffunction-sections',
      '-fdata-sections',
    ],
    link_args: [
      '-Wl,-gc-sections',
      '-Wl,-Map=pdaqp-cpp.map',
    ],
    dependencies: [
      tree_walker_heap_dep,
      apply_feedback_dep,
      subproject('fmt').get_variable('fmt_dep'),
    ]
)
