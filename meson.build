project('gameboy', 'c',
  version : '0.1',
  default_options : ['warning_level=3', 'c_std=c89', 'werror=true'])

base_src = [
  'src/bus.c',
  'src/cartridge.c',
  'src/cpu.c',
  'src/cpu_utils.c',
  'src/gameboy.c',
  'src/ppu.c',
  'src/utils.c',
]

sdl = dependency('SDL2')

if get_option('buildtype').startswith('debug')
  add_project_arguments('-DDEBUG', language : 'c')
endif

src = base_src + 'src/main.c'
exe = executable('gameboy', src,
      dependencies : [sdl], install : true)

test_src = base_src + 'test/test_cpu.c'
test_exe = executable('gameboy_test', test_src,
           dependencies : [sdl])

test('cpu', test_exe)
