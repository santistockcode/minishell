# Refactorización de Lexing: De Vectores a Listas

## 📋 Resumen de Cambios

Esta refactorización transforma el sistema de lexing original (que usaba `t_vector`) a uno basado en listas enlazadas (`t_list`), con una nueva estructura `t_lexing` para mantener el estado.

---

## 🏗️ Estructura Nueva: `t_lexing`

```c
typedef struct s_lexing
{
	char	*buff;			// Buffer de entrada (lexing_buff de shell)
	char	*current;		// Puntero actual en el buffer
	int		token_id;		// ID secuencial del token
	t_list	*tokens;		// Lista de tokens
}	t_lexing;
```

### Comparación con la estructura anterior:

**ANTES (con vector):**
```c
typedef struct s_term_token
{
	char		*term_line;
	t_vector	*token_array;  // Requería ft_vector_init, push, get
}	t_term_token;
```

**AHORA (con listas):**
```c
typedef struct s_shell
{
	char 		*lexing_buff;	// De readline
	t_list		*tokens_list;	// Nueva lista de tokens
}	t_shell;
```

---

## 📁 Archivos Creados

### 1. **lexing_list.c** - Núcleo del lexing
- `get_tokens_list()` - Procesa buffer completo, itera, salta espacios
- `add_token_list()` - Crea un token y lo añade a la lista
- `token_switch()` - Determina tipo de token según carácter
- `lexing()` - Punto de entrada principal desde shell

### 2. **init_lexing_list.c** - Inicialización y utilidades
- `init_new_token()` - Crea estructura vacía de token
- `reval_assign_token()` - Reconoce VAR=valor después de análisis
- `syntax_quotes()` - Valida comillas balanceadas
- `free_tokens_list()` - Libera lista completa

### 3. **assing_lexing_list.c** - Asignación de tipos específicos
- `assign_redir_token()` - Detecta <<, >>, <, >
- `assign_pipevar_token()` - Detecta |
- `assign_var_token()` - Marca VAR=valor
- `assign_word_token()` - Procesa palabras respetando comillas

### 4. **automa_lexing_list.c** - Máquina de estados para palabras
- `state_noquote()` - Estado sin comillas
- `state_singlequote()` - Dentro de comillas simples
- `state_doublequote()` - Dentro de comillas dobles
- `state_switch()` - Máquina de cambio de estados

### 5. **lexing_new.h** - Header con todas las definiciones

---

## 🔄 Flujo de Ejecución

```
readline() → lexing_buff
    ↓
lexing(shell)
    ↓
get_tokens_list(lexing_ctx)
    ↓
while (*current) → add_token_list() × N
    ↓
token_switch() → assign_redir/pipe/word/var
    ↓
ft_lstadd_back(&tokens, nuevo_token)
    ↓
Validar sintaxis → syntax_quotes()
    ↓
Reasignar VAR=valor → reval_assign_token()
    ↓
minishell->tokens_list = lexing_ctx.tokens
    ↓
Parser recibe lista de tokens
```

---

## 📊 Comparación: Vector vs Lista

| Aspecto | Vector Anterior | Lista Nueva |
|---------|-----------------|-------------|
| **Estructura** | `t_vector` con malloc manual | `t_list` de libft |
| **Inicialización** | `ft_vector_init()` | `NULL` (crece dinámico) |
| **Agregar** | `ft_vector_push()` con realloc | `ft_lstadd_back()` |
| **Acceso** | `ft_vector_get(i)` - O(1) | Iterar - O(n) |
| **Iteración** | Por índice con loop | `while (node)` |
| **Memoria** | Contiguous (mejor cache) | Dispersa (flexibilidad) |
| **Libertad** | Manual (ft_vector_free) | Automática (sin vector) |

---

## 🔑 Cambios Clave en Funciones

### `get_tokens()` → `get_tokens_list()`

**ANTES:**
```c
int get_tokens(t_term_token *term_token)
{
    init_lexing_utils(term_token, &term_copy, &term_i, &token_id);
    while (*term_i) {
        if (ft_isspace(*term_i)) { term_i++; continue; }
        add_token(term_token, token_id, &term_i);
        token_id++;
    }
    free(term_copy);
    init_new_token(&token_eof, token_id);
    ft_vector_push(term_token->token_array, &token_eof);
}
```

**AHORA:**
```c
int get_tokens_list(t_lexing *lexing)
{
    lexing->current = lexing->buff;
    lexing->token_id = 0;
    lexing->tokens = NULL;
    
    while (*(lexing->current)) {
        if (ft_isspace(*(lexing->current))) {
            (lexing->current)++;
            continue;
        }
        add_token_list(lexing, &(lexing->current));
        (lexing->token_id)++;
    }
    
    token_eof = NULL;
    init_new_token(&token_eof, lexing->token_id);
    ft_lstadd_back(&(lexing->tokens), ft_lstnew(token_eof));
}
```

**CAMBIOS:**
- No necesita copiar buffer (usa directo lexing_buff)
- No necesita liberar copia
- `ft_lstadd_back()` en lugar de `ft_vector_push()`
- Estructura `t_lexing` en lugar de `t_term_token`

---

## 🧩 Integración en Shell

### Cambio en `minishell.h`:

```c
typedef struct s_shell
{
    // ... otros campos ...
    char *lexing_buff;          // De readline
    t_list *tokens_list;        // NUEVO: reemplaza term_token->token_array
    // ... otros campos ...
}   t_shell;
```

### Cambio en el REPL principal:

```c
while (1)
{
    minishell->lexing_buff = readline("> ");
    if (!minishell->lexing_buff) break;
    
    if (lexing(minishell) != SUCCESS)
    {
        fprintf(stderr, "Lexing error\n");
        continue;
    }
    
    // minishell->tokens_list ahora contiene los tokens
    if (parsing(minishell) != SUCCESS)
    {
        fprintf(stderr, "Parsing error\n");
        continue;
    }
    
    // ... ejecutar ...
    
    free_tokens_list(minishell->tokens_list);
    free(minishell->lexing_buff);
}
```

---

## ✨ Ventajas de esta Refactorización

1. **Simplificación**: Usa libft's `t_list` en lugar de vector custom
2. **Menor overhead**: No necesita copiar buffer
3. **Integración natural**: lexing_buff viene directo de readline
4. **Mejor limpieza**: `free_tokens_list()` limpia todo
5. **Mismo comportamiento**: Máquina de estados idéntica
6. **Mejor documentación**: Cada función tiene comentarios extensos

---

## 🚀 Cómo Usar

### Compila con:
```bash
gcc -Wall -Wextra -Werror \
    src/lexing/lexing_list.c \
    src/lexing/init_lexing_list.c \
    src/lexing/assing_lexing_list.c \
    src/lexing/automa_lexing_list.c \
    ... (otros archivos)
```

### Desde tu código:
```c
t_shell *minishell;
minishell->lexing_buff = readline("> ");

if (lexing(minishell) == SUCCESS)
{
    t_list *tokens = minishell->tokens_list;
    // Procesar tokens...
}
```

---

## 📝 Notas Importantes

1. **No usa t_vector**: Todo trabajo con `t_list` de libft
2. **Buffer directo**: No copia ni necesita liberar copia
3. **Máquina de estados**: Idéntica a original (compatibilidad)
4. **Headers**: Define en `lexing_new.h`, no modifica existentes
5. **Frecuencia**: Considerar compilar solo archivos necesarios

