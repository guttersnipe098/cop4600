/*******************************************************************************
* File:      obj4.c
* Version:   0.1
* Purpose:   Calculates the OS's statistics
* Template:  Dr. David Workman, Time Hughey, Mark Stephens, Wade Spires, and
*            Sean Szumlanski
* Coded by:  Michael Altfield <maltfield@knights.ucf.edu>
* Course:    COP 4600 <http://www.cs.ucf.edu/courses/cop4600/spring2012>
* Objective: 5
* Created:   2012-04-11
* Updated:   2012-04-11
* Notes:     This program was written to be compiled against the gnu99 standard.
*            Please execute the following commands to build correctly:
*
*  CFLAGS="-O0 -g3 -I/home/eurip/ossim2010 --std=gnu99"
*  export CFLAGS
*  make -e sim
*******************************************************************************/
/**
	obj5.c


	Simulator Project for COP 4600

	Objective 5 project that calculates OS statistics. 

	Revision List:
		Original Design:  Dr. David Workman, 1990
		Revised by Tim Hughey and Mark Stephens, 1993
		Revised by Wade Spires, Spring 2005
		Minor Revisions by Sean Szumlanski, Spring 2010
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "osdefs.h"
#include "externs.h"

/**
	Compute all simulation statistics and store them in the appropriate
	variables and data structures for display.  It is called by Clean_Up().

	<b> Algorithm: </b>
	\verbatim
	For each user
		Calculate total processing time
		Calculate total job blocked time
		Calculate total job wait time
		Calculate total job execution time
		Calculate efficiency for each process

	For each device
		Calculate response time for each device
		Calculate total idle time for each device
		Calculate utilization for each device

	Calculate average user execution time
	Calculate average user logon time
	Calculate average user blocked time
	Calculate average user wait time
	Calculate response time for CPU
	Calculate total idle time for CPU
	Calculate total utilization for CPU
	\endverbatim
 */
void
Calc_Stats( )
{
}
