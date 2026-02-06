/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 16:17:49 by saalarco          #+#    #+#             */
/*   Updated: 2026/02/02 18:08:44 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOG_H
# define LOG_H

typedef struct s_shell	t_shell;
typedef struct s_cmd    t_cmd;
typedef struct s_list	t_list;


#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"

# ifdef DEBUG
#  define LOG 1
# else
#  define LOG 0
# endif

void	logger(const char *tag, const char *message);
void	logger_ctx(t_shell *sh, t_list *cmd, const char *tag, const char *message);
void    logger_ctx_simple(t_shell *sh, t_cmd *cmd, const char *tag, const char *message);
void	logger_open_fds(const char *starttag, const char *endtag);

#endif