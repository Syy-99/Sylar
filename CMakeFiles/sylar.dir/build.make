# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.18

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/gaoxiaoyu/syy/Sylar

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/gaoxiaoyu/syy/Sylar

# Include any dependencies generated for this target.
include CMakeFiles/sylar.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/sylar.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/sylar.dir/flags.make

CMakeFiles/sylar.dir/sylar/log.cc.o: CMakeFiles/sylar.dir/flags.make
CMakeFiles/sylar.dir/sylar/log.cc.o: sylar/log.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gaoxiaoyu/syy/Sylar/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/sylar.dir/sylar/log.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sylar.dir/sylar/log.cc.o -c /home/gaoxiaoyu/syy/Sylar/sylar/log.cc

CMakeFiles/sylar.dir/sylar/log.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sylar.dir/sylar/log.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gaoxiaoyu/syy/Sylar/sylar/log.cc > CMakeFiles/sylar.dir/sylar/log.cc.i

CMakeFiles/sylar.dir/sylar/log.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sylar.dir/sylar/log.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gaoxiaoyu/syy/Sylar/sylar/log.cc -o CMakeFiles/sylar.dir/sylar/log.cc.s

CMakeFiles/sylar.dir/sylar/util.cc.o: CMakeFiles/sylar.dir/flags.make
CMakeFiles/sylar.dir/sylar/util.cc.o: sylar/util.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gaoxiaoyu/syy/Sylar/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/sylar.dir/sylar/util.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sylar.dir/sylar/util.cc.o -c /home/gaoxiaoyu/syy/Sylar/sylar/util.cc

CMakeFiles/sylar.dir/sylar/util.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sylar.dir/sylar/util.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gaoxiaoyu/syy/Sylar/sylar/util.cc > CMakeFiles/sylar.dir/sylar/util.cc.i

CMakeFiles/sylar.dir/sylar/util.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sylar.dir/sylar/util.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gaoxiaoyu/syy/Sylar/sylar/util.cc -o CMakeFiles/sylar.dir/sylar/util.cc.s

CMakeFiles/sylar.dir/sylar/config.cc.o: CMakeFiles/sylar.dir/flags.make
CMakeFiles/sylar.dir/sylar/config.cc.o: sylar/config.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gaoxiaoyu/syy/Sylar/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/sylar.dir/sylar/config.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sylar.dir/sylar/config.cc.o -c /home/gaoxiaoyu/syy/Sylar/sylar/config.cc

CMakeFiles/sylar.dir/sylar/config.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sylar.dir/sylar/config.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gaoxiaoyu/syy/Sylar/sylar/config.cc > CMakeFiles/sylar.dir/sylar/config.cc.i

CMakeFiles/sylar.dir/sylar/config.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sylar.dir/sylar/config.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gaoxiaoyu/syy/Sylar/sylar/config.cc -o CMakeFiles/sylar.dir/sylar/config.cc.s

CMakeFiles/sylar.dir/sylar/thread.cc.o: CMakeFiles/sylar.dir/flags.make
CMakeFiles/sylar.dir/sylar/thread.cc.o: sylar/thread.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gaoxiaoyu/syy/Sylar/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/sylar.dir/sylar/thread.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sylar.dir/sylar/thread.cc.o -c /home/gaoxiaoyu/syy/Sylar/sylar/thread.cc

CMakeFiles/sylar.dir/sylar/thread.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sylar.dir/sylar/thread.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gaoxiaoyu/syy/Sylar/sylar/thread.cc > CMakeFiles/sylar.dir/sylar/thread.cc.i

CMakeFiles/sylar.dir/sylar/thread.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sylar.dir/sylar/thread.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gaoxiaoyu/syy/Sylar/sylar/thread.cc -o CMakeFiles/sylar.dir/sylar/thread.cc.s

CMakeFiles/sylar.dir/sylar/fiber.cc.o: CMakeFiles/sylar.dir/flags.make
CMakeFiles/sylar.dir/sylar/fiber.cc.o: sylar/fiber.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gaoxiaoyu/syy/Sylar/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/sylar.dir/sylar/fiber.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sylar.dir/sylar/fiber.cc.o -c /home/gaoxiaoyu/syy/Sylar/sylar/fiber.cc

CMakeFiles/sylar.dir/sylar/fiber.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sylar.dir/sylar/fiber.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gaoxiaoyu/syy/Sylar/sylar/fiber.cc > CMakeFiles/sylar.dir/sylar/fiber.cc.i

CMakeFiles/sylar.dir/sylar/fiber.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sylar.dir/sylar/fiber.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gaoxiaoyu/syy/Sylar/sylar/fiber.cc -o CMakeFiles/sylar.dir/sylar/fiber.cc.s

CMakeFiles/sylar.dir/sylar/mutex.cc.o: CMakeFiles/sylar.dir/flags.make
CMakeFiles/sylar.dir/sylar/mutex.cc.o: sylar/mutex.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gaoxiaoyu/syy/Sylar/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/sylar.dir/sylar/mutex.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sylar.dir/sylar/mutex.cc.o -c /home/gaoxiaoyu/syy/Sylar/sylar/mutex.cc

CMakeFiles/sylar.dir/sylar/mutex.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sylar.dir/sylar/mutex.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gaoxiaoyu/syy/Sylar/sylar/mutex.cc > CMakeFiles/sylar.dir/sylar/mutex.cc.i

CMakeFiles/sylar.dir/sylar/mutex.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sylar.dir/sylar/mutex.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gaoxiaoyu/syy/Sylar/sylar/mutex.cc -o CMakeFiles/sylar.dir/sylar/mutex.cc.s

CMakeFiles/sylar.dir/sylar/scheduler.cc.o: CMakeFiles/sylar.dir/flags.make
CMakeFiles/sylar.dir/sylar/scheduler.cc.o: sylar/scheduler.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gaoxiaoyu/syy/Sylar/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/sylar.dir/sylar/scheduler.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sylar.dir/sylar/scheduler.cc.o -c /home/gaoxiaoyu/syy/Sylar/sylar/scheduler.cc

CMakeFiles/sylar.dir/sylar/scheduler.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sylar.dir/sylar/scheduler.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gaoxiaoyu/syy/Sylar/sylar/scheduler.cc > CMakeFiles/sylar.dir/sylar/scheduler.cc.i

CMakeFiles/sylar.dir/sylar/scheduler.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sylar.dir/sylar/scheduler.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gaoxiaoyu/syy/Sylar/sylar/scheduler.cc -o CMakeFiles/sylar.dir/sylar/scheduler.cc.s

CMakeFiles/sylar.dir/sylar/iomanager.cc.o: CMakeFiles/sylar.dir/flags.make
CMakeFiles/sylar.dir/sylar/iomanager.cc.o: sylar/iomanager.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gaoxiaoyu/syy/Sylar/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object CMakeFiles/sylar.dir/sylar/iomanager.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sylar.dir/sylar/iomanager.cc.o -c /home/gaoxiaoyu/syy/Sylar/sylar/iomanager.cc

CMakeFiles/sylar.dir/sylar/iomanager.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sylar.dir/sylar/iomanager.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gaoxiaoyu/syy/Sylar/sylar/iomanager.cc > CMakeFiles/sylar.dir/sylar/iomanager.cc.i

CMakeFiles/sylar.dir/sylar/iomanager.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sylar.dir/sylar/iomanager.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gaoxiaoyu/syy/Sylar/sylar/iomanager.cc -o CMakeFiles/sylar.dir/sylar/iomanager.cc.s

CMakeFiles/sylar.dir/sylar/timer.cc.o: CMakeFiles/sylar.dir/flags.make
CMakeFiles/sylar.dir/sylar/timer.cc.o: sylar/timer.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gaoxiaoyu/syy/Sylar/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building CXX object CMakeFiles/sylar.dir/sylar/timer.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sylar.dir/sylar/timer.cc.o -c /home/gaoxiaoyu/syy/Sylar/sylar/timer.cc

CMakeFiles/sylar.dir/sylar/timer.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sylar.dir/sylar/timer.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gaoxiaoyu/syy/Sylar/sylar/timer.cc > CMakeFiles/sylar.dir/sylar/timer.cc.i

CMakeFiles/sylar.dir/sylar/timer.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sylar.dir/sylar/timer.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gaoxiaoyu/syy/Sylar/sylar/timer.cc -o CMakeFiles/sylar.dir/sylar/timer.cc.s

CMakeFiles/sylar.dir/sylar/hook.cc.o: CMakeFiles/sylar.dir/flags.make
CMakeFiles/sylar.dir/sylar/hook.cc.o: sylar/hook.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gaoxiaoyu/syy/Sylar/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building CXX object CMakeFiles/sylar.dir/sylar/hook.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sylar.dir/sylar/hook.cc.o -c /home/gaoxiaoyu/syy/Sylar/sylar/hook.cc

CMakeFiles/sylar.dir/sylar/hook.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sylar.dir/sylar/hook.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gaoxiaoyu/syy/Sylar/sylar/hook.cc > CMakeFiles/sylar.dir/sylar/hook.cc.i

CMakeFiles/sylar.dir/sylar/hook.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sylar.dir/sylar/hook.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gaoxiaoyu/syy/Sylar/sylar/hook.cc -o CMakeFiles/sylar.dir/sylar/hook.cc.s

CMakeFiles/sylar.dir/sylar/fd_manager.cc.o: CMakeFiles/sylar.dir/flags.make
CMakeFiles/sylar.dir/sylar/fd_manager.cc.o: sylar/fd_manager.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gaoxiaoyu/syy/Sylar/CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Building CXX object CMakeFiles/sylar.dir/sylar/fd_manager.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sylar.dir/sylar/fd_manager.cc.o -c /home/gaoxiaoyu/syy/Sylar/sylar/fd_manager.cc

CMakeFiles/sylar.dir/sylar/fd_manager.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sylar.dir/sylar/fd_manager.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gaoxiaoyu/syy/Sylar/sylar/fd_manager.cc > CMakeFiles/sylar.dir/sylar/fd_manager.cc.i

CMakeFiles/sylar.dir/sylar/fd_manager.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sylar.dir/sylar/fd_manager.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gaoxiaoyu/syy/Sylar/sylar/fd_manager.cc -o CMakeFiles/sylar.dir/sylar/fd_manager.cc.s

CMakeFiles/sylar.dir/sylar/address.cc.o: CMakeFiles/sylar.dir/flags.make
CMakeFiles/sylar.dir/sylar/address.cc.o: sylar/address.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gaoxiaoyu/syy/Sylar/CMakeFiles --progress-num=$(CMAKE_PROGRESS_12) "Building CXX object CMakeFiles/sylar.dir/sylar/address.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sylar.dir/sylar/address.cc.o -c /home/gaoxiaoyu/syy/Sylar/sylar/address.cc

CMakeFiles/sylar.dir/sylar/address.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sylar.dir/sylar/address.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gaoxiaoyu/syy/Sylar/sylar/address.cc > CMakeFiles/sylar.dir/sylar/address.cc.i

CMakeFiles/sylar.dir/sylar/address.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sylar.dir/sylar/address.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gaoxiaoyu/syy/Sylar/sylar/address.cc -o CMakeFiles/sylar.dir/sylar/address.cc.s

CMakeFiles/sylar.dir/sylar/socket.cc.o: CMakeFiles/sylar.dir/flags.make
CMakeFiles/sylar.dir/sylar/socket.cc.o: sylar/socket.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gaoxiaoyu/syy/Sylar/CMakeFiles --progress-num=$(CMAKE_PROGRESS_13) "Building CXX object CMakeFiles/sylar.dir/sylar/socket.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sylar.dir/sylar/socket.cc.o -c /home/gaoxiaoyu/syy/Sylar/sylar/socket.cc

CMakeFiles/sylar.dir/sylar/socket.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sylar.dir/sylar/socket.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gaoxiaoyu/syy/Sylar/sylar/socket.cc > CMakeFiles/sylar.dir/sylar/socket.cc.i

CMakeFiles/sylar.dir/sylar/socket.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sylar.dir/sylar/socket.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gaoxiaoyu/syy/Sylar/sylar/socket.cc -o CMakeFiles/sylar.dir/sylar/socket.cc.s

CMakeFiles/sylar.dir/sylar/bytearray.cc.o: CMakeFiles/sylar.dir/flags.make
CMakeFiles/sylar.dir/sylar/bytearray.cc.o: sylar/bytearray.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gaoxiaoyu/syy/Sylar/CMakeFiles --progress-num=$(CMAKE_PROGRESS_14) "Building CXX object CMakeFiles/sylar.dir/sylar/bytearray.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sylar.dir/sylar/bytearray.cc.o -c /home/gaoxiaoyu/syy/Sylar/sylar/bytearray.cc

CMakeFiles/sylar.dir/sylar/bytearray.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sylar.dir/sylar/bytearray.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gaoxiaoyu/syy/Sylar/sylar/bytearray.cc > CMakeFiles/sylar.dir/sylar/bytearray.cc.i

CMakeFiles/sylar.dir/sylar/bytearray.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sylar.dir/sylar/bytearray.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gaoxiaoyu/syy/Sylar/sylar/bytearray.cc -o CMakeFiles/sylar.dir/sylar/bytearray.cc.s

CMakeFiles/sylar.dir/sylar/http/http.cc.o: CMakeFiles/sylar.dir/flags.make
CMakeFiles/sylar.dir/sylar/http/http.cc.o: sylar/http/http.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gaoxiaoyu/syy/Sylar/CMakeFiles --progress-num=$(CMAKE_PROGRESS_15) "Building CXX object CMakeFiles/sylar.dir/sylar/http/http.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sylar.dir/sylar/http/http.cc.o -c /home/gaoxiaoyu/syy/Sylar/sylar/http/http.cc

CMakeFiles/sylar.dir/sylar/http/http.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sylar.dir/sylar/http/http.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gaoxiaoyu/syy/Sylar/sylar/http/http.cc > CMakeFiles/sylar.dir/sylar/http/http.cc.i

CMakeFiles/sylar.dir/sylar/http/http.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sylar.dir/sylar/http/http.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gaoxiaoyu/syy/Sylar/sylar/http/http.cc -o CMakeFiles/sylar.dir/sylar/http/http.cc.s

CMakeFiles/sylar.dir/sylar/http/http11_parser.rl.cc.o: CMakeFiles/sylar.dir/flags.make
CMakeFiles/sylar.dir/sylar/http/http11_parser.rl.cc.o: sylar/http/http11_parser.rl.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gaoxiaoyu/syy/Sylar/CMakeFiles --progress-num=$(CMAKE_PROGRESS_16) "Building CXX object CMakeFiles/sylar.dir/sylar/http/http11_parser.rl.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sylar.dir/sylar/http/http11_parser.rl.cc.o -c /home/gaoxiaoyu/syy/Sylar/sylar/http/http11_parser.rl.cc

CMakeFiles/sylar.dir/sylar/http/http11_parser.rl.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sylar.dir/sylar/http/http11_parser.rl.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gaoxiaoyu/syy/Sylar/sylar/http/http11_parser.rl.cc > CMakeFiles/sylar.dir/sylar/http/http11_parser.rl.cc.i

CMakeFiles/sylar.dir/sylar/http/http11_parser.rl.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sylar.dir/sylar/http/http11_parser.rl.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gaoxiaoyu/syy/Sylar/sylar/http/http11_parser.rl.cc -o CMakeFiles/sylar.dir/sylar/http/http11_parser.rl.cc.s

CMakeFiles/sylar.dir/sylar/http/httpclient_parser.rl.cc.o: CMakeFiles/sylar.dir/flags.make
CMakeFiles/sylar.dir/sylar/http/httpclient_parser.rl.cc.o: sylar/http/httpclient_parser.rl.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gaoxiaoyu/syy/Sylar/CMakeFiles --progress-num=$(CMAKE_PROGRESS_17) "Building CXX object CMakeFiles/sylar.dir/sylar/http/httpclient_parser.rl.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sylar.dir/sylar/http/httpclient_parser.rl.cc.o -c /home/gaoxiaoyu/syy/Sylar/sylar/http/httpclient_parser.rl.cc

CMakeFiles/sylar.dir/sylar/http/httpclient_parser.rl.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sylar.dir/sylar/http/httpclient_parser.rl.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gaoxiaoyu/syy/Sylar/sylar/http/httpclient_parser.rl.cc > CMakeFiles/sylar.dir/sylar/http/httpclient_parser.rl.cc.i

CMakeFiles/sylar.dir/sylar/http/httpclient_parser.rl.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sylar.dir/sylar/http/httpclient_parser.rl.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gaoxiaoyu/syy/Sylar/sylar/http/httpclient_parser.rl.cc -o CMakeFiles/sylar.dir/sylar/http/httpclient_parser.rl.cc.s

CMakeFiles/sylar.dir/sylar/http/http_parser.cc.o: CMakeFiles/sylar.dir/flags.make
CMakeFiles/sylar.dir/sylar/http/http_parser.cc.o: sylar/http/http_parser.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gaoxiaoyu/syy/Sylar/CMakeFiles --progress-num=$(CMAKE_PROGRESS_18) "Building CXX object CMakeFiles/sylar.dir/sylar/http/http_parser.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sylar.dir/sylar/http/http_parser.cc.o -c /home/gaoxiaoyu/syy/Sylar/sylar/http/http_parser.cc

CMakeFiles/sylar.dir/sylar/http/http_parser.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sylar.dir/sylar/http/http_parser.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gaoxiaoyu/syy/Sylar/sylar/http/http_parser.cc > CMakeFiles/sylar.dir/sylar/http/http_parser.cc.i

CMakeFiles/sylar.dir/sylar/http/http_parser.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sylar.dir/sylar/http/http_parser.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gaoxiaoyu/syy/Sylar/sylar/http/http_parser.cc -o CMakeFiles/sylar.dir/sylar/http/http_parser.cc.s

CMakeFiles/sylar.dir/sylar/tcp_server.cc.o: CMakeFiles/sylar.dir/flags.make
CMakeFiles/sylar.dir/sylar/tcp_server.cc.o: sylar/tcp_server.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gaoxiaoyu/syy/Sylar/CMakeFiles --progress-num=$(CMAKE_PROGRESS_19) "Building CXX object CMakeFiles/sylar.dir/sylar/tcp_server.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sylar.dir/sylar/tcp_server.cc.o -c /home/gaoxiaoyu/syy/Sylar/sylar/tcp_server.cc

CMakeFiles/sylar.dir/sylar/tcp_server.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sylar.dir/sylar/tcp_server.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gaoxiaoyu/syy/Sylar/sylar/tcp_server.cc > CMakeFiles/sylar.dir/sylar/tcp_server.cc.i

CMakeFiles/sylar.dir/sylar/tcp_server.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sylar.dir/sylar/tcp_server.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gaoxiaoyu/syy/Sylar/sylar/tcp_server.cc -o CMakeFiles/sylar.dir/sylar/tcp_server.cc.s

CMakeFiles/sylar.dir/sylar/stream.cc.o: CMakeFiles/sylar.dir/flags.make
CMakeFiles/sylar.dir/sylar/stream.cc.o: sylar/stream.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gaoxiaoyu/syy/Sylar/CMakeFiles --progress-num=$(CMAKE_PROGRESS_20) "Building CXX object CMakeFiles/sylar.dir/sylar/stream.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sylar.dir/sylar/stream.cc.o -c /home/gaoxiaoyu/syy/Sylar/sylar/stream.cc

CMakeFiles/sylar.dir/sylar/stream.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sylar.dir/sylar/stream.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gaoxiaoyu/syy/Sylar/sylar/stream.cc > CMakeFiles/sylar.dir/sylar/stream.cc.i

CMakeFiles/sylar.dir/sylar/stream.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sylar.dir/sylar/stream.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gaoxiaoyu/syy/Sylar/sylar/stream.cc -o CMakeFiles/sylar.dir/sylar/stream.cc.s

CMakeFiles/sylar.dir/sylar/socket_stream.cc.o: CMakeFiles/sylar.dir/flags.make
CMakeFiles/sylar.dir/sylar/socket_stream.cc.o: sylar/socket_stream.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gaoxiaoyu/syy/Sylar/CMakeFiles --progress-num=$(CMAKE_PROGRESS_21) "Building CXX object CMakeFiles/sylar.dir/sylar/socket_stream.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sylar.dir/sylar/socket_stream.cc.o -c /home/gaoxiaoyu/syy/Sylar/sylar/socket_stream.cc

CMakeFiles/sylar.dir/sylar/socket_stream.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sylar.dir/sylar/socket_stream.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gaoxiaoyu/syy/Sylar/sylar/socket_stream.cc > CMakeFiles/sylar.dir/sylar/socket_stream.cc.i

CMakeFiles/sylar.dir/sylar/socket_stream.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sylar.dir/sylar/socket_stream.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gaoxiaoyu/syy/Sylar/sylar/socket_stream.cc -o CMakeFiles/sylar.dir/sylar/socket_stream.cc.s

CMakeFiles/sylar.dir/sylar/http/http_session.cc.o: CMakeFiles/sylar.dir/flags.make
CMakeFiles/sylar.dir/sylar/http/http_session.cc.o: sylar/http/http_session.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gaoxiaoyu/syy/Sylar/CMakeFiles --progress-num=$(CMAKE_PROGRESS_22) "Building CXX object CMakeFiles/sylar.dir/sylar/http/http_session.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sylar.dir/sylar/http/http_session.cc.o -c /home/gaoxiaoyu/syy/Sylar/sylar/http/http_session.cc

CMakeFiles/sylar.dir/sylar/http/http_session.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sylar.dir/sylar/http/http_session.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gaoxiaoyu/syy/Sylar/sylar/http/http_session.cc > CMakeFiles/sylar.dir/sylar/http/http_session.cc.i

CMakeFiles/sylar.dir/sylar/http/http_session.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sylar.dir/sylar/http/http_session.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gaoxiaoyu/syy/Sylar/sylar/http/http_session.cc -o CMakeFiles/sylar.dir/sylar/http/http_session.cc.s

CMakeFiles/sylar.dir/sylar/http/http_server.cc.o: CMakeFiles/sylar.dir/flags.make
CMakeFiles/sylar.dir/sylar/http/http_server.cc.o: sylar/http/http_server.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gaoxiaoyu/syy/Sylar/CMakeFiles --progress-num=$(CMAKE_PROGRESS_23) "Building CXX object CMakeFiles/sylar.dir/sylar/http/http_server.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sylar.dir/sylar/http/http_server.cc.o -c /home/gaoxiaoyu/syy/Sylar/sylar/http/http_server.cc

CMakeFiles/sylar.dir/sylar/http/http_server.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sylar.dir/sylar/http/http_server.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gaoxiaoyu/syy/Sylar/sylar/http/http_server.cc > CMakeFiles/sylar.dir/sylar/http/http_server.cc.i

CMakeFiles/sylar.dir/sylar/http/http_server.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sylar.dir/sylar/http/http_server.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gaoxiaoyu/syy/Sylar/sylar/http/http_server.cc -o CMakeFiles/sylar.dir/sylar/http/http_server.cc.s

# Object files for target sylar
sylar_OBJECTS = \
"CMakeFiles/sylar.dir/sylar/log.cc.o" \
"CMakeFiles/sylar.dir/sylar/util.cc.o" \
"CMakeFiles/sylar.dir/sylar/config.cc.o" \
"CMakeFiles/sylar.dir/sylar/thread.cc.o" \
"CMakeFiles/sylar.dir/sylar/fiber.cc.o" \
"CMakeFiles/sylar.dir/sylar/mutex.cc.o" \
"CMakeFiles/sylar.dir/sylar/scheduler.cc.o" \
"CMakeFiles/sylar.dir/sylar/iomanager.cc.o" \
"CMakeFiles/sylar.dir/sylar/timer.cc.o" \
"CMakeFiles/sylar.dir/sylar/hook.cc.o" \
"CMakeFiles/sylar.dir/sylar/fd_manager.cc.o" \
"CMakeFiles/sylar.dir/sylar/address.cc.o" \
"CMakeFiles/sylar.dir/sylar/socket.cc.o" \
"CMakeFiles/sylar.dir/sylar/bytearray.cc.o" \
"CMakeFiles/sylar.dir/sylar/http/http.cc.o" \
"CMakeFiles/sylar.dir/sylar/http/http11_parser.rl.cc.o" \
"CMakeFiles/sylar.dir/sylar/http/httpclient_parser.rl.cc.o" \
"CMakeFiles/sylar.dir/sylar/http/http_parser.cc.o" \
"CMakeFiles/sylar.dir/sylar/tcp_server.cc.o" \
"CMakeFiles/sylar.dir/sylar/stream.cc.o" \
"CMakeFiles/sylar.dir/sylar/socket_stream.cc.o" \
"CMakeFiles/sylar.dir/sylar/http/http_session.cc.o" \
"CMakeFiles/sylar.dir/sylar/http/http_server.cc.o"

# External object files for target sylar
sylar_EXTERNAL_OBJECTS =

lib/libsylar.so: CMakeFiles/sylar.dir/sylar/log.cc.o
lib/libsylar.so: CMakeFiles/sylar.dir/sylar/util.cc.o
lib/libsylar.so: CMakeFiles/sylar.dir/sylar/config.cc.o
lib/libsylar.so: CMakeFiles/sylar.dir/sylar/thread.cc.o
lib/libsylar.so: CMakeFiles/sylar.dir/sylar/fiber.cc.o
lib/libsylar.so: CMakeFiles/sylar.dir/sylar/mutex.cc.o
lib/libsylar.so: CMakeFiles/sylar.dir/sylar/scheduler.cc.o
lib/libsylar.so: CMakeFiles/sylar.dir/sylar/iomanager.cc.o
lib/libsylar.so: CMakeFiles/sylar.dir/sylar/timer.cc.o
lib/libsylar.so: CMakeFiles/sylar.dir/sylar/hook.cc.o
lib/libsylar.so: CMakeFiles/sylar.dir/sylar/fd_manager.cc.o
lib/libsylar.so: CMakeFiles/sylar.dir/sylar/address.cc.o
lib/libsylar.so: CMakeFiles/sylar.dir/sylar/socket.cc.o
lib/libsylar.so: CMakeFiles/sylar.dir/sylar/bytearray.cc.o
lib/libsylar.so: CMakeFiles/sylar.dir/sylar/http/http.cc.o
lib/libsylar.so: CMakeFiles/sylar.dir/sylar/http/http11_parser.rl.cc.o
lib/libsylar.so: CMakeFiles/sylar.dir/sylar/http/httpclient_parser.rl.cc.o
lib/libsylar.so: CMakeFiles/sylar.dir/sylar/http/http_parser.cc.o
lib/libsylar.so: CMakeFiles/sylar.dir/sylar/tcp_server.cc.o
lib/libsylar.so: CMakeFiles/sylar.dir/sylar/stream.cc.o
lib/libsylar.so: CMakeFiles/sylar.dir/sylar/socket_stream.cc.o
lib/libsylar.so: CMakeFiles/sylar.dir/sylar/http/http_session.cc.o
lib/libsylar.so: CMakeFiles/sylar.dir/sylar/http/http_server.cc.o
lib/libsylar.so: CMakeFiles/sylar.dir/build.make
lib/libsylar.so: CMakeFiles/sylar.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/gaoxiaoyu/syy/Sylar/CMakeFiles --progress-num=$(CMAKE_PROGRESS_24) "Linking CXX shared library lib/libsylar.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/sylar.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/sylar.dir/build: lib/libsylar.so

.PHONY : CMakeFiles/sylar.dir/build

CMakeFiles/sylar.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/sylar.dir/cmake_clean.cmake
.PHONY : CMakeFiles/sylar.dir/clean

CMakeFiles/sylar.dir/depend:
	cd /home/gaoxiaoyu/syy/Sylar && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/gaoxiaoyu/syy/Sylar /home/gaoxiaoyu/syy/Sylar /home/gaoxiaoyu/syy/Sylar /home/gaoxiaoyu/syy/Sylar /home/gaoxiaoyu/syy/Sylar/CMakeFiles/sylar.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/sylar.dir/depend

