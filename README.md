## Usage
In order to build the C executables simply <make all>
In order to enter the mini shell: <./shell>. NOTE: You are able to enter into any number of child shells from the current shell.

# Built in Commands
- cd <directory> : This command should change the current working directory of the shell to the path specified as the argument.
- source <file> : Takes a filename as an argument and processes each line of the file as a command, including built-ins.
- prev : Prints the previous command line and executes it again
- help : Explains all the built-in commands available in your shell
- exit : Exits the current mini shell you are in

# Non built in Commands
All basic linux commands are available as we call them with execvp in a child process.

# Advanced Shell Features
- Sequencing (;): Commands separated by ; will be executed from left to right
- Input Redirection (<): A command may be followed by < and a file name. The command shall be run with the contents of the file replacing the standard input.
- Output Redirection (>): A command may be followed by > and a file name. The command shall be run as normal, but the standard output should be captured in the given file. If the file exists, its original contents should be deleted (“truncated”) before output is written to it. If it does not exist, it should be created automatically. You do not need to implement output redirection for built-ins.
- Piping (|): The pipe operator | runs the command on the left hand side and the command on the right-hand side simultaneously and the standard output of the LHS command is redirected to the standard input of the RHS command. 
