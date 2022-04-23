project('gamegirl', 'c',
  version : '0.1',
  default_options : ['warning_level=3', 'c_std=c89', 'werror=true'])

base_src = [
  'src/bus.c',
  'src/cartridge.c',
  'src/cpu.c',
  'src/cpu_utils.c',
  'src/gamegirl.c',
  'src/ppu.c',
  'src/utils.c',
]

sdl = dependency('SDL2')

src = base_src + 'src/main.c'
exe = executable('gamegirl', src,
      dependencies : [sdl], install : true)

test_src = base_src + 'src/unit_test.c'
test_exe = executable('gamegirl_test', test_src,
           dependencies : [sdl])

test('cpu', test_exe)
