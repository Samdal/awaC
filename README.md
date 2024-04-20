# AWA 5.0 interpreter in C (with debugging utils)
Implementation of AWA5.0:
- https://github.com/TempTempai/AWA5.0

currently tested on hello world, fibonacci script, binary->awatalk, and string->awatalk code. Hopefully it works.
### compiling
The program only needs libc as a dependency, it's a terminal program.

So just compile with for example:
- unix: `cc main.c -o awaparser`
- windows: `cl /Fe: awaparser.exe main.c`

### additional opcodes:
- syscall 'sys u10' (0x15)\
'~wa awawa awawa **awa awa awa awa awa awa awa awawa**' (syscall 1, write)
> The provided u10 is which syscall that will be used\
> The instruction looks at the top most bubble, it must be a double bubble.\
> The double bouble contains the arguments (arg1, arg2, arg3...arg6).\
> The number of arguments depends on the syscall.\
> A double bouble inside a double bouble is interpreted as a raw byte array\
> And a pointer to that raw byte array is provided as the argument.\
> Example: (arg1, (byte1, byte2, byte3, byte4), arg3)\
> The double bubble used as the argument is NOT popped, as return values might be placed in byte arrays (NOT IMPLEMENTED)\
> The return value of the syscall is placed at the top of the stack.
- double pop 'p0p' (0x16)\
'~wa awawa awa awa'
> Same as pop except double boubles are deleted instead of being released into the bubble abyss
### usage
```
command line options:
	'--quiet'               : disables printing inbetween operations
	'--step'                : steps through the program [press enter]
	
	'--verbose-parser'      : prints opcodes and parameters of the awatalk
	'--verbose-interpreter' : prints current instruction and stack while executing
	'--verbose'             : enables both of the above
	'--allow-syscall'       : enables syscalls
	
	'--help'                : prints this message
	
	'-string'               : the next provided argument will be awatalk to be parsed
	'-file'                 : the next provided argument will be awatalk file to be parsed
example usage:
	awaparser --verbose -string 'awa awa awawa awawa awa awa awa awa awa awawa awa awa awawa awawa awa awa awa awa awa awawawa awa awawa awawa awa awa awa awa awa awawa awa awawa awa awawa awa awa awawawa awa awa awa awawa'
	awaparser --quiet -file hello_world.awa
	awaparser --verbose-parser -file hello_world.awa
```
### debug output example
```
$ ./awaparser --verbose -string 'awa awa awawa awawa awa awa awa awa awa awawa awa awa awawa awawa awa awa awa awa awa awawawa awa awawa awawa awa awa awa awa awa awawa awa awawa awa awawa awa awa awawawa awa awa awa awawa'
blo 2 [a]
blo 3 [w]
blo 2 [a]
srn 3
prn
parsed file succesfully
running program:
-
	<<<() | blo 2 [a]>>>
	<<<(2) | blo 3 [w]>>>
	<<<(3, 2) | blo 2 [a]>>>
	<<<(2, 3, 2) | srn 3>>>
	<<<((2, 3, 2)) | prn>>>
awa
-
program done!
```

### error messages
```
$ ./awaparser --verbose -string 'awa awawawww'
0:10 error: awa parser got unexpected input starting here
awa awawawww
          ^~~
```
```
$ ./awaparser --verbose -file test.awa
blo 0 [A]
blo 0 [A]
blo 0 [A]
blo 0 [A]
blo 0 [A]
blo 0 [A]
blo 0 [A]
blo 0 [A]
blo 39 [f]
prn
blo 21 [i]
prn
blo 41 [r]
prn
14:32 error: awa parser got unexpected input starting here
 awa__hello_ebebiben!__ awawa awwa awawa awawa awa awa
                                ^~~
NOTE: awa parser error above happened while parsing an opcode
error during parsing, exiting.
```
