# Pruebas manuales vistas en 42

1. "cat | cat | ls"
Debe funcionar como bash, se queda escuchando despues de imprimir ls.

En este punto hacemos control+C

Repetir lo mismo pero con control+D

2. Control+D en un caso de varios here_docs

Debe salir solo de unos de los here_docs, no de todos.

3. Pruebas de no pasar ningún comando, o pasar espacios y tabulaciones

4. Echo con y sin "-n" y llamado multiples vveces seguidas con distintos argumentos

5. "<ok.txt cat | <ok.txt cat"

6. "cat lol.c | cat lol.c"

7. expr $? + $? (va a sumar el último código de salida a si pispo)

8. Desde bash llamar a ./minishell tal que "env -i ./minishell"

Aquí buscamos que no crashee por no tener PWD basicamente. 

9. Resto de https://wormav.github.io/42_eval/Cursus/Minishell/index.html


