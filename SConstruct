cxx_flags = ["-std=c++11", "-fPIC", "-O3"]

VariantDir('build', 'src', duplicate=0)
env = Environment()

env.Program('fdinject', [
	'build/fdinject.cpp',
	'build/dbpp.cpp',
	'build/signal.cpp',
	'build/syscall.cpp'
	], CXXFLAGS=cxx_flags)

# vi: set ft=python:
