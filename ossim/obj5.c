/*******************************************************************************
* File:      obj4.c
* Version:   0.2
* Purpose:   Calculates the OS's statistics
* Template:  Dr. David Workman, Time Hughey, Mark Stephens, Wade Spires, and
*            Sean Szumlanski
* Coded by:  Michael Altfield <maltfield@knights.ucf.edu>
* Course:    COP 4600 <http://www.cs.ucf.edu/courses/cop4600/spring2012>
* Objective: 5
* Created:   2012-04-11
* Updated:   2012-04-13
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

	// DECLARE VARIABLES
	struct time_type time; // throw-away/local var for calculations

	//For each user
	for( int i=0; i<Num_Terminals; i++ ){

		// skip if this user serviced 0 IORBs (nothing to calculate)
		if( Dev_Table[i].num_served == 0 ){
			continue;
		}

		//Calculate total processing time
		Add_time( &( Term_Table[i]->total_logon_time ), &Tot_Logon );

		//Calculate total job blocked time
		Add_time( &( Term_Table[i]->total_block_time ), &Tot_Block );

		//Calculate total job wait time
		Add_time( &( Term_Table[i]->total_ready_time ), &Tot_Wait );

		//Calculate total job execution time
		Add_time( &( Term_Table[i]->total_run_time ), &Tot_Run );

		//Calculate efficiency for each process

		time = Term_Table[i]->total_run_time;
		Add_time( &( Term_Table[i]->total_block_time ), &time );

		Term_Table[i]->efficiency =
		 Divide_time( &time, &( Term_Table[i]->total_logon_time ) )
		 * 100;

	}

	//For each device
	for( int i=0; i<Num_Devices; i++ ){

		//Calculate response time for each device
		time = Dev_Table[i].total_q_time;
		Add_time( &(Dev_Table[i].total_busy_time), &time );
		Average_time( &time, Dev_Table[i].num_served, &(Dev_Table[i].response) );

		//Calculate total idle time for each device
		time = Clock;
		Diff_time( &( Dev_Table[i].total_busy_time ), &time );
		Dev_Table[i].total_idle_time = time;

		//Calculate utilization for each device
		Dev_Table[i].utilize =
		 Divide_time( &( Dev_Table[i].total_busy_time ), &Clock )
		 * 100;

	}

	//Calculate average user execution time
	Average_time( &Tot_Run, CPU.num_served, &Avg_Run );

	//Calculate average user logon time
	Average_time( &Tot_Logon, CPU.num_served, &Avg_Logon );

	//Calculate average user blocked time
	Average_time( &Tot_Block, CPU.num_served, &Avg_Block );

	//Calculate average user wait time
	Average_time( &Tot_Wait, CPU.num_served, &Avg_Wait );

	//Calculate response time for CPU
	time = CPU.total_q_time;
	Add_time( &( CPU.total_busy_time ), &time );
	Average_time( &time, CPU.num_served, &( CPU.response ) );

	//Calculate total idle time for CPU
	time = Clock;
	Diff_time( &( CPU.total_busy_time ), &time );
	CPU.total_idle_time = time;

	//Calculate total utilization for CPU
	CPU.utilize =
	 Divide_time( &( CPU.total_busy_time ), &Clock )
	 * 100;

}
