/*
 * Copyright (C) 2001 MontaVista Software Inc.
 * Author: Jun Sun, jsun@mvista.com or jsun@junsun.net
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#ifndef _print_h_
#define _print_h_

#include <stdarg.h>

/* this is the maximum width for a variable */
#define		LP_MAX_BUF	1000

#define LIGHT_BLUE(str)				"\033[0m\033[1;34m" # str "\033[m"
#define BLUE(str)					"\033[0m\033[34m"	# str "\033[m"
#define DARK_BLUE(str)				"\033[0m\033[2;34m"	# str "\033[m"
#define RED(str)					"\033[0m\033[31m"	# str "\033[m"
#define LIGHT_RED(str)				"\033[0m\033[1;31m"	# str "\033[m"
#define DARK_RED(str)				"\033[0m\033[2;31m"	# str "\033[m"
#define GREEN(str)					"\033[0m\033[32m"	# str "\033[m"
#define LIGHT_GREEN(str)			"\033[0m\033[1;32m"	# str "\033[m"
#define DARK_GREEN(str)				"\033[0m\033[2;32m"	# str "\033[m"
#define YELLOW(str)					"\033[0m\033[33m"	# str "\033[m"
#define LIGHT_YELLOW(str)			"\033[0m\033[1;33m"	# str "\033[m"
#define DARK_YELLOW(str)			"\033[0m\033[2;33m"	# str "\033[m"
#define MEG(str)					"\033[0m\033[35m"	# str "\033[m"
#define LIGHT_MEG(str)				"\033[0m\033[1;35m"	# str "\033[m"
#define DARK_MEG(str)				"\033[0m\033[2;35m"	# str "\033[m"
#define CIY(str)					"\033[0m\033[36m"	# str "\033[m"
#define LIGHT_CIY(str)				"\033[0m\033[1;36m"	# str "\033[m"
#define DARK_CIY(str)				"\033[0m\033[2;36m"	# str "\033[m"
/* -*-
 * output function takes an void pointer which is passed in as the
 * second argument in lp_Print().  This black-box argument gives output
 * function a way to track state.
 *
 * The second argument in output function is a pointer to char buffer.
 * The third argument specifies the number of chars to outputed.
 *
 * output function cannot assume the buffer is null-terminated after
 * l number of chars.
 */
void lp_Print(void (*output)(void *, char *, int), 
	      void * arg,
	      char *fmt, 
	      va_list ap);

#endif
