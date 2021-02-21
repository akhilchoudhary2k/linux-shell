# Linux-Shell
## A rudimentary shell written in C/C++ as a part of Operating Systems Course.

<img src="https://i.imgur.com/RKUsy7S.png" width="500" height="450">


## Functionalities
Some extra features are implemented:
- Input-Output Redirection  
> ./a.out < input.txt > output.txt
- Use of Pipes             
> ls -al | sort | head
- Auto-Completion using ```TAB``` key
- Previous command using ```UP``` arrow key

Last 2 are implemented using ```readline``` library of C/C++



## Execution
```
make clean
make
./shell
```
Output:

<img src="https://i.imgur.com/aLFvKAO.png" width="500" height="300">
