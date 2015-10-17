all: git-store

git-store: command.hpp subprocess.hpp
	g++ -std=c++11 -o $@ -I./ ./libs/program_options/src/*.cpp ./libs/filesystem/src/*.cpp ./libs/system/src/*.cpp main.cpp
