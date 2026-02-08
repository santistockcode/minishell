- echo << HEREDOC hola
    
    → should print hola after executing heredoc
    
- << HERE << DOC
    
    → should open two consecutive instances of heredoc
    
- env variables should be read from a variable or struct (shouldn’t use getenv() after shell initialisation)
- export variable and exec `./minishell` inside minishell, variable should exist in `env`
- `$HOME/../../bin/ls`
- `> ret ls < doesntexist` should create ret,  `< doesntexist ls > ret` shouldn’t. Neither should execute ls
- `env -i ./minishell` should create basic env and not leak if non-existing command
- CTRL+C into heredoc
- export "test=ici"=coucou, echo $test
-


use cases :

$ < Makefile
- return 0

$ < doesnt_exist.txt
doesnt_exist.txt: No such file or directory
- return 1

$ < Makefile ls
*does ls*
- return 0

$ ls < Makefile -l
*does ls -l*
- return 0

$ -l ls
-l: command not found
- return 127


export/unset use cases :

$ export var1 var2
*exports both variables without a value*

$ export var1 var1=hey
*exports var1="hey"*

$ export var1=hey var1=hola
*exports var1="hola"*

$ unset var1 var2
*deletes both variables from export list*

$ cat << EOF

Parser :
$ cd /home/"$USER/.local"/bin
$ cd /home/'$USER'/.local/bin
$ echo hey '$USER' this is your cwd: $CWD and shell:'$SHELL'
$ echo $PWD/.local/bin
$ echo "hey '$PWD/var.log'"
$ cat>>~/.zshrc     <<EOF|wc -l
$ echo $USER$?~$SHELL
$ echo $.hey $~
$ echo /hey$var___var/hey