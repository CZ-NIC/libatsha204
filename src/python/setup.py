from distutils.core import setup, Extension

extension = Extension('libatsha204', ['atsha204.c'], include_dirs=['../libatsha204'], library_dirs=['../libatsha204'], libraries=['crypto', 'ssl', 'unbound', 'atsha204'])
setup(name='libatsha204', version='0.1', ext_modules=[extension])
