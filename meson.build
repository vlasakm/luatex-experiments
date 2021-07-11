project('mmtex', 'c')

#git = find_program('git', native: true)
#git_tag = run_command([git, 'describe', '--tag'])
mmtex_version = '20210515'#git_tag.stdout().strip()
add_project_arguments('-DMMTEX_VERSION="@0@"'.format(mmtex_version), language: 'c')

subdir('src')
#install_subdir('texmf', install_dir: 'share', strip_directory: true)
