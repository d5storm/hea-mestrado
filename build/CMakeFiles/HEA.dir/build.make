# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.11

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/guerine/eclipse-workspace/hea-mestrado

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/guerine/eclipse-workspace/hea-mestrado/build

# Include any dependencies generated for this target.
include CMakeFiles/HEA.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/HEA.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/HEA.dir/flags.make

CMakeFiles/HEA.dir/source/hea.cpp.o: CMakeFiles/HEA.dir/flags.make
CMakeFiles/HEA.dir/source/hea.cpp.o: ../source/hea.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/guerine/eclipse-workspace/hea-mestrado/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/HEA.dir/source/hea.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/HEA.dir/source/hea.cpp.o -c /home/guerine/eclipse-workspace/hea-mestrado/source/hea.cpp

CMakeFiles/HEA.dir/source/hea.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/HEA.dir/source/hea.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/guerine/eclipse-workspace/hea-mestrado/source/hea.cpp > CMakeFiles/HEA.dir/source/hea.cpp.i

CMakeFiles/HEA.dir/source/hea.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/HEA.dir/source/hea.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/guerine/eclipse-workspace/hea-mestrado/source/hea.cpp -o CMakeFiles/HEA.dir/source/hea.cpp.s

# Object files for target HEA
HEA_OBJECTS = \
"CMakeFiles/HEA.dir/source/hea.cpp.o"

# External object files for target HEA
HEA_EXTERNAL_OBJECTS =

HEA: CMakeFiles/HEA.dir/source/hea.cpp.o
HEA: CMakeFiles/HEA.dir/build.make
HEA: CMakeFiles/HEA.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/guerine/eclipse-workspace/hea-mestrado/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable HEA"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/HEA.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/HEA.dir/build: HEA

.PHONY : CMakeFiles/HEA.dir/build

CMakeFiles/HEA.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/HEA.dir/cmake_clean.cmake
.PHONY : CMakeFiles/HEA.dir/clean

CMakeFiles/HEA.dir/depend:
	cd /home/guerine/eclipse-workspace/hea-mestrado/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/guerine/eclipse-workspace/hea-mestrado /home/guerine/eclipse-workspace/hea-mestrado /home/guerine/eclipse-workspace/hea-mestrado/build /home/guerine/eclipse-workspace/hea-mestrado/build /home/guerine/eclipse-workspace/hea-mestrado/build/CMakeFiles/HEA.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/HEA.dir/depend

