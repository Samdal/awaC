# AWA 5.0 interpreter in C (with debugging utils)
currently tested on both hello world and the fibonacci script, hopefully it works.
### Compiling
The program only needs libc as a dependency, it's a terminal program.

So just compile with for example:
- unix: `cc main.c -o awaparser`
- windows: `cl /Fe: awaparser.exe main.c`

### usage
```
command line options:
	'--quiet'               : disables printing inbetween operations
	
	'--verbose-parser'     : prints opcodes and parameters of the awatalk
	'--verbose-interpreter' : prints current instruction and stack while executing
	'--verbose'             : enables both of the above
	
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
