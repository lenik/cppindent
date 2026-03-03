project('cppindent', 'c',
  version : '0.1.0',
  default_options : [
    'c_std=c99',
    'warning_level=3',
    'optimization=2'
  ]
)

cppindent_exe = executable(
  'cppindent',
  'src/main.c',
  'src/formatter.c',
  'src/io.c',
  install : true
)

install_man('cppindent.1')

install_data(
  'completions/cppindent',
  install_dir : get_option('datadir') / 'bash-completion' / 'completions'
)

