# Mini-shell 2021

Mini-shell is an elementary version of the shell using C programming language.

## Installation

Clone the code to your computer.

```bash
git clone https://github.com/el-omari-soufyane/minishell-2021.git
```

Change to the project workspace and execute the Makefile to generate the executable file.

```bash
cd minishell-2021/
make
```

## Usage

We implement some of the built-in commands such us : ```cd ```, ```export ```, ```unset ```.

We also implement some of the reserved command like :
-  the pipe command ```command | command```.
- Standard Input (stdin) : ```command < file```.
- Standard Output (stdout) : ```command > file``` and ```command >> file```.
- Standard Error (stderr) : ```command 2> file``` and ```command 2>> file```.
- The conditional commands like AND and OR command : ```command && command2``` and ```command || command2```.
- The inconditional command : ```command1 ; command2 ; command3```

```bash
mini@shell:/home$ ls
a.out  main.c
mini@shell:/home$ mkdir test
mini@shell:/home$ cd test
mini@shell:/home/test$ ls -l
total 0
mini@shell:/home/test$ ls ..
a.out  main.c  test
mini@shell:/home/test$ cd ..
mini@shell:/home$ cd ..
mini@shell:/$ ls
bin   dev  home  lib32  libx32  mnt  proc  run   script  sys  usr
boot  etc  lib   lib64  media   opt  root  sbin  srv     tmp  var
mini@shell:/$ ls -l | wc -c
1156
mini@shell:/$ ls >> file.txt
open: Read-only file system
ls: cannot access '>>': No such file or directory
ls: cannot access 'file.txt': No such file or directory
mini@shell:/$ cd home
mini@shell:/home$ ls -l >> file.txt
mini@shell:/home$ cat file.txt
total 52
-rwxr-xr-x 1 runner15 runner15 27048 Nov 21 21:45 a.out
-rw-r--r-- 1 runner15 runner15     0 Nov 21 21:46 file.txt
-rwxrwxrwx 1 root     root     18987 Nov 21 21:45 main.c
drwxr-xr-x 2 runner15 runner15  4096 Nov 21 21:45 test
mini@shell:/home$ wc -c < file.txt
236
mini@shell:/home$ export VAR11=HELLO_SHELL
VAR11=HELLO_SHELL
mini@shell:/home$ echo $VAR11   
HELLO_SHELL
mini@shell:/home$
```

## Appreciation

This project were made with the help of our professor Mr. Gaël Le Mahec, we take this chance to give him a thanks for his effort to help us reach this level of knowledge.

## Contributors

- [EL OMARI Soufyane](https://github.com/el-omari-soufyane)
- [CHOUKRI Marouane](https://github.com/marouane1031)
- [DOUMI Saloua](https://github.com/doumiSaloua)


## License
[MIT](https://choosealicense.com/licenses/mit/)
