project('armadillo', 'cpp',
    version: '14.0.x',
    license: 'Apache',
    license_files: ['LICENSE.txt'],
)

# Header-only library
armadillo_inc = include_directories('include')

if get_option('lapack') == 'none'
  warning('Disabled Lapack dependencies. SVD and LU algorithms will be undefined.')
  lapack_dep = dependency('', required: false)
elif get_option('lapack') == 'openblas64'
  lapack_dep = dependency('openblas64')
elif get_option('lapack') == 'openblas'
  lapack_dep = dependency('openblas')
elif get_option('lapack') == 'lapack64'
  lapack_dep = dependency('lapack64')
else # lapack or altas
  lapack_dep = dependency('lapack')
endif


armadillo_dep = declare_dependency(
    include_directories: armadillo_inc,
    compile_args: [
      '-DARMA_DONT_USE_BLAS',
      '-DARMA_DONT_USE_ARPACK',
    ],
    dependencies: [
      lapack_dep,
    ],
)
