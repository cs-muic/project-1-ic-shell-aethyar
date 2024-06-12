[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/Cc2uuWhf)
[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/2Vc0gGZS)
[![Open in Visual Studio Code](https://classroom.github.com/assets/open-in-vscode-718a45dd9cf7e7f842a935f5ebbe5719a5e09af4491e668f4dbf3b35d5cca122.svg)](https://classroom.github.com/online_ide?assignment_repo_id=11089167&assignment_repo_type=AssignmentRepo)
# ICSH

Please describe your assumptions and/or implementation here. -AJ:




I have only **tested the inputs similar to the example usages** on the milestones. The program assumes that there is only a max
of 100 background processes to save my VM's life... I can handle `SIGTSTP` and `SIGINT` but the logic  with my `SIGTSTP` handler 
had problems with being incorporated to job handling which prevents my `bg %`<job_id> implementation  from being tested. 
Other than that, the rest of the implementations should work as expected.



### Description

`main()`
When launching the program with `./icsh`, the interface starts with "Starting IC shell" and then provides the prompt "icsh $ ".
This prompt will repeatedly show up with most inputs until the program ends with commands like `exit `#.
- Both `SIGTSTP` and `SIGINT` are handled wotj `signal_handler` such that they will not stop the main IC shell process.
- Any child process completion will signal `check_background_jobs()` for "asynchronous" printing.
- Using `fgets`, the user input is captured as a command and then this is processed with `process_cmd()`.

`process_cmd()`
Commands that were directly told to implement such as `echo $?`, `!!`, `exit `#, `jobs`, `fg %`#, and `bg %`#  have their 
own conditions explicitly. There is a `script_mode` used here to circumvent the feature shown in Milestone 2. There was an 
implementation for `echo` here previously but because it may use Output Redirection, this is now handled with `external_cmd()` 
which handles any external command that was not implemented.
- Using the command `jobs` iterates through the `jobs` collection and displays the programs along with their statuses. If the
`status` member of the `struct` was implemented perfectly, the if-statements inside could have been simpler.
- Using the command `fg %`# checks first if the input Job ID exists, and then properly uses it as `fg_process` while also
waiting for it to finish.
- If successful, using the command `bg %`# also checks first if the input Job ID exists, and then switches its status while
also calling `kill(job_pid, SIGCONT)` in order to continue the suspended process.
- Anything else is directly passed into `external_cmd()`

`external_cmd()`
For external commands, the example code for it was used and then modified heavily to support any default bash command, input
and output redirection, and background processes.
- The input and output redirection code was inspired from a StackOverflow post, not from the example given.
- Commands meant to be run in the background are identified with their last character '&'.
- Background processes are stored inside `jobs` using a `Job struct` that can hold their `pid`, `cmd`, and `status`.
  - indices will be used as the Process ID's that are meant to start at 1
  - `cmd` is the string that will be displayed when referring to the process
  - `status` is marked with 1 for Running, and 0 for Stopped

`check_background_jobs()`
This function readjusts the `jobs` collection whenever a `bg_process` completes. This allows for "asyncrhonous" printing.
This also reduces the number of `jobs` of course.

`signal_handler()`
Previously, I used `signal()` instead of `sigaction()` for Milestone 4 but then changing it to the latter to follow the example
worked out fine. However, either my logic fails or I barely understand signal handling which is why it cannot properly suspend a
process in order to send it to `jobs` while Stopped which also prevents with testing `bg %`#.
