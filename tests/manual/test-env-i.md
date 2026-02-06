An evaluator can execute our minishell like
```bash
env -i ./minishell
```
That means no env variables. 


So, it is recommended that parsing part have some devault env variables. 


/*


c1r9s6% env -i bash
saalarco@c1r9s6:/home/saalarco/.local/share/Trash/files/delete-me$ env
PWD=/home/saalarco/.local/share/Trash/files/delete-me
SHLVL=1
_=/usr/bin/env
saalarco@c1r9s6:/home/saalarco/.local/share/Trash/files/delete-me$ ls
script.sh
saalarco@c1r9s6:/home/saalarco/.local/share/Trash/files/delete-me$ which ls
/usr/bin/ls
saalarco@c1r9s6:/home/saalarco/.local/share/Trash/files/delete-me$ declare -p
declare -- BASH="/usr/bin/bash"
declare -r BASHOPTS="checkwinsize:cmdhist:complete_fullquote:expand_aliases:extquote:force_fignore:globasciiranges:hostcomplete:interactive_comments:progcomp:promptvars:sourcepath"
declare -i BASHPID
declare -A BASH_ALIASES=()
declare -a BASH_ARGC=([0]="0")
declare -a BASH_ARGV=()
declare -- BASH_ARGV0
declare -A BASH_CMDS=()
declare -- BASH_COMMAND
declare -a BASH_LINENO=()
declare -a BASH_SOURCE=()
declare -- BASH_SUBSHELL
declare -ar BASH_VERSINFO=([0]="5" [1]="1" [2]="16" [3]="1" [4]="release" [5]="x86_64-pc-linux-gnu")
declare -- BASH_VERSION="5.1.16(1)-release"
declare -- COLUMNS="137"
declare -- COMP_WORDBREAKS
declare -a DIRSTACK=()
declare -- EPOCHREALTIME
declare -- EPOCHSECONDS
declare -ir EUID="11237"
declare -a FUNCNAME
declare -a GROUPS=()
declare -i HISTCMD
declare -- HISTFILE="/home/saalarco/.bash_history"
declare -- HISTFILESIZE="500"
declare -- HISTSIZE="500"
declare -- HOSTNAME="c1r9s6.42madrid.com"
declare -- HOSTTYPE="x86_64"
declare -- IFS=" 	
"
declare -- LINENO
declare -- LINES="62"
declare -- MACHTYPE="x86_64-pc-linux-gnu"
declare -i MAILCHECK="60"
declare -x OLDPWD
declare -- OPTERR="1"
declare -i OPTIND="1"
declare -- OSTYPE="linux-gnu"
declare -- PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
declare -a PIPESTATUS=([0]="0")
declare -ir PPID="2669497"
declare -- PS1="\${debian_chroot:+(\$debian_chroot)}\\u@\\h:\\w\\\$ "
declare -- PS2="> "
declare -- PS4="+ "
declare -x PWD="/home/saalarco/.local/share/Trash/files/delete-me"
declare -i RANDOM
declare -- SECONDS
declare -- SHELL="/bin/zsh"
declare -r SHELLOPTS="braceexpand:emacs:hashall:histexpand:history:interactive-comments:monitor"
declare -x SHLVL="1"
declare -i SRANDOM
declare -- TERM="dumb"
declare -ir UID="11237"
declare -- _="ls"
saalarco@c1r9s6:/home/saalarco/.local/share/Trash/files/delete-me$ 

*/