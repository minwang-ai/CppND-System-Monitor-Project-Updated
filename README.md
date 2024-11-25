# CppND-System-Monitor

My solution for System Monitor Project in the Object Oriented Programming Course of the [Udacity C++ Nanodegree Program](https://www.udacity.com/course/c-plus-plus-nanodegree--nd213). 

![System Monitor](images/monitor.png)

## Project Overvew

- **`System` Class**: Represents the overall system and provides information about the system's state, such as the list of processes, memory utilization, and CPU utilization.
- **`Process` Class**: Represents an individual process running on the system and provides information about that process, such as its ID, CPU usage, memory usage, and command.
- **`Processor` Class**: Represents the CPU and provides information about its utilization.
- **`LinuxParser` Namespace**: Contains functions that parse information from the Linux filesystem (primarily from the `/proc` directory) to provide data needed by the `System`, `Process`, and `Processor` classes.

### Responsibilities and Relationships

1. **`System` Class**:
   - Uses functions from the `LinuxParser` namespace to gather system-wide information.
   - Maintains a list of `Process` objects, representing all running processes.
   - Contains a `Processor` object to represent the CPU.

2. **`Process` Class**:
   - Uses functions from the `LinuxParser` namespace to gather information specific to a process, such as its CPU and memory usage.
   - Each `Process` object corresponds to a single process on the system.

3. **`Processor` Class**:
   - Uses functions from the `LinuxParser` namespace to gather information about CPU utilization.
   


## Make
This project uses [Make](https://www.gnu.org/software/make/). The Makefile has four targets:
* `build` compiles the source code and generates an executable
* `format` applies [ClangFormat](https://clang.llvm.org/docs/ClangFormat.html) to style the source code
* `debug` compiles the source code and generates an executable, including debugging symbols
* `clean` deletes the `build/` directory, including all of the build artifacts

## Instructions

1. Clone the project repository: `git clone <project_url>`

2. Install ncurses within your own Linux environment: `sudo apt install libncurses5-dev libncursesw5-dev`. [ncurses](https://www.gnu.org/software/ncurses/) is a library that facilitates text-based graphical output in the terminal. This project relies on ncurses for display output.

3. Build the project: `make build`

4. Run the resulting executable: `./build/monitor`
![Starting System Monitor](images/starting_monitor.png)

