# PSEARCH
Psearch is a command line tool for searching a keyword in multiple files and find the matched lines.
- Psearch uses different Interprocess Communication (IPC) methods to optimize the search time:
  - fork (parent-child processes)
  - anonymous pipelines
  - shared memory
  - semaphores
- Psearch uses standard POSIX API targeting **Linux**.

## Compile 
- run `make` command in the project directory to compile all the programs.

## Run
### Psearch1
`psearch1` uses `parent-child` paradigm to concurrently read each input file. The parent process creates child processes with `fork()`. Each child process gets an input file and finds the matched lines for given search keyword. Then each child process creates a buffer output file and prints the matched lines. Finally, the parent process reads all the buffer output files and writes the results into a single output file that is specified by the user.  

- run `./psearch1 <search_keyword> <input_file_count> <input_file1.txt> <input_file2.txt> <output_file.txt>`
- e.g., `./psearch1 love 1 input1.txt output1.txt`

### Psearch2
`psearch2` uses `parent-child` paradigm to read each input file concurrently. Each child process gets an input file and finds the matched lines for given search keyword. Then each child process writes the results to `anonymous pipes`. Finally, the parent process reads all pipes and writes the results into a single output file that is specified by the user 

- run `./psearch2 <search_keyword> <input_file_count> <input_file1.txt> <input_file2.txt> <output_file.txt>`
- e.g., `./psearch2 love 1 input1.txt input2.txt output2.txt`

### Psearch3
This program uses `master-slave` paradigm. `psearch3` only creates child processes and execute (`exec()`) them while `psearch3slave` reads the input file specified by the `psearch3` and sends the results to the master. The communication between the master and the slaves is maintained by named shared memory(`mmap()`) with file mapping.  

- run `./psearch3 <search_keyword> <inputfile1.txt> <inputfile2.txt> <output_file.txt>`
- e.g., `./psearch3 love input1.txt input2.txt output3.txt`

### Psearch4
This program uses the same concept with `psearch3` with some modifications.`psearch4` uses POSIX shared memory without file mapping (`shm_open()`). `psearch4` uses semaphores (`sem_open()`) to synchronize the shared memory usage of the master and the slaves.  

- run `./psearch4 <search_keyword> <inputfile1.txt> <inputfile2.txt> <output_file.txt>`
- e.g., `./psearch4 love input1.txt input2.txt output3.txt`

## Learning Outcomes 
- create child processes and concurrently solve problems with `fork()`.
- Interprocess communication on Linux with POSIX API: parent-child, master-slave, pipelines, semaphores.
- use shared memory to provide communication between processes:
  - shared anonymous mapping
  - shared file mapping
  - named and unnamed semaphores
- File management (e.g., CRUD operations)
- measure execution times of concurrent processes.
- memory management (still needs review :D)
