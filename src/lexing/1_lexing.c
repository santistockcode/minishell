/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   1_lexing.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/29 22:27:44 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/01/29 22:27:55 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../../include/minishell.h"

int lexing(t_shell	* minishell)
{

	if(!get_tokens(minishell->term_token))
		return(INPUT_ERROR);
	if(!syntax_quotes(minishell->term_token))
		return(INPUT_ERROR);
	if (reval_assign_token(minishell->term_token) != SUCCESS)
		return (INPUT_ERROR);
	return(SUCCESS);

}