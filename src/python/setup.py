from distutils.core import setup, Extension

extension = Extension('atsha204', ['atsha204.c'], libraries=['atsha204'])
setup(name='atsha204', version='0.1', ext_modules=[extension])
