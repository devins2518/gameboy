project('gamegirl', 'c',
  version : '0.1',
  default_options : ['warning_level=3', 'c_std=c89'])

src = [
  'src/bus.c',
  'src/cartridge.c',
  'src/cpu.c',
  'src/gamegirl.c',
  'src/main.c',
  'src/utils.c',
]

if get_option('buildtype') == 'debug'
  # add_project_arguments('-gline-tables-only',  language: 'c')
  add_project_link_arguments('-rdynamic', '-no-pie', language: 'c')
endif

sdl = dependency('SDL2')

exe = executable(
  'gamegirl',
  src,
  dependencies : [sdl],
  install : true
)

test('basic', exe)
