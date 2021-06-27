project('luatex', 'c')

#git = find_program('git', native: true)
#git_tag = run_command([git, 'describe', '--tag'])
mmtex_version = '20210515'#git_tag.stdout().strip()
add_project_arguments('-DMMTEX_VERSION="@0@"'.format(mmtex_version), language: 'c')

mmtex_inc = include_directories('include')

subdir('src')

install_subdir('texmf', install_dir: 'share/mmtex', strip_directory: true)

mmtex_fmt = custom_target(
  'mmtex.fmt',
  input: files('texmf/tex/mmtex/mmtex.ini'),
  output: 'mmtex.fmt',
  command: ['env', 'TEXMF=' + meson.current_source_dir() + '/texmf', mmtex, '-ini', '@INPUT@'],
  install_dir: 'share/mmtex/web2c',
  install: true,
)
