apply_feedback_lib = static_library('apply-feedback',
    sources: 'src/apply_feedback.cpp',
    include_directories: [
        common_inc,
        problem_def_inc,
        'inc',
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

apply_feedback_dep = declare_dependency(
    link_with: apply_feedback_lib,
    include_directories: [
        'inc',
        common_inc,
    ],
)