/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: smclacke <smclacke@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/21 17:38:18 by smclacke      #+#    #+#                 */
/*   Updated: 2024/10/22 15:05:28 by smclacke      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

# include "../include/web.hpp"

bool	validConf(char *arg)
{
	return arg[0] == '!' ? 0 : 1;
}

int	main(int argc, char **argv)
{
	// use catch block .. whatever Julius wants here
	if (argc != 2 && !validConf(argv[1]))
		exit (EXIT_FAILURE);


	// parse config

	// setup

	// run -> server -> poll loop

	// close/clean etc

	//basicSocket(); // will move later

}
