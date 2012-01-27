/**
	obj1.c


	Simulator Project for COP 4600

	Objective 1 project that implements loading the event list for
	the OS simulator.

	Revision List:
		Original Design:  Dr. David Workman, 1990
		Revised by Tim Hughey and Mark Stephens, 1993
		Revised by Wade Spires, Spring 2005
		Minor Revisions by Sean Szumlanski, Spring 2010
*/


#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "osdefs.h"
#include "externs.h"

int  get_agent_id( char* );
int  get_event_id( char* );

/**
	Initialize the event list from the file logon.dat. This file normally
	contains only logon events for all terminals. However, for debugging
	purposes, logon.dat can contain events of any type.

	<b> Algorithm: </b>
	\verbatim
	Open logon file given by constant LOGON_FILENAME
	While not end of file
		Read event name, agent name, and time from each line in file
		Convert event name to event ID--call get_event_id() (must be written)
		Convert agent name to agent ID--call get_agent_id() (must be written)
		Convert time to simulation time--call Uint_to_time()
		Add event to event list using event ID, agent ID, and simulation time
	Close file
	\endverbatim

	@retval None
 */
void
Load_Events( )
{
}

/**
	Add a new event into the list of operating system events.

	This function inserts a future event into the list Event_List at the proper
	time sequence. Event_List points to the event in the list having the
	smallest time (as defined by the function Compare_time).

	<b> Algorithm: </b>
	\verbatim
	Allocate a new event node
	Set the new event node's fields to the event, agent, and time passed in
	If the event list is empty
		Set the event list header node to the new event node
	Else, if the new event node should precede the event list header node
		Place the new node at the start of the list
	Otherwise, the new node goes in the middle or at the end of the list
		Traverse the event list until reaching the node that should precede the new node
		Add the new node after the node reached in the traversal--handle special case of new node being at the end of the list
	\endverbatim

	@param[in] event -- event to simulate
	@param[in] agent -- user agent causing event
	@param[in] time -- time event occurs
	@retval None
 */
void
Add_Event( int event, int agent, struct time_type* time )
{
}

/**
	Generate clock interrupt.

	<b> Algorithm: </b>
	\verbatim
	Remove an event from Event_List
	Set Clock, Agent and Event to the respective fields in the removed event
	Write the event to the output file--call write_event()
	Deallocate the removed event item
	Save CPU.mode and CPU.pc into Old_State.
	Change New_State to CPU.mode and CPU.pc
	\endverbatim 

	@retval None
 */
void
Interrupt( )
{
}

/**
	Write an event to the output file.
	
	Call print_out() for all output to file. The format of the output is:
	"  EVENT  AGENT  TIME (HR:xxxxxxxx MN:xx SC:xx MS:xxx mS:xxx NS:xxx )"

	<b> Algorithm: </b>
	\verbatim
	Convert the seconds field of time_type to hours, minutes, and seconds
	Convert the nanoseconds field to milliseconds, microseconds and nanoseconds
	Determine type of agent--user terminal or device:
		If agent ID <= Num_Terminals, then agent is user terminal:
			Agent name is of the form 'U0#' where # = agent ID
		Otherwise, agent is a device:
			Agent name is stored in Dev_Table[ agent - Num_Terminals - 1]
	Print formatted message using event name from Event_Names, agent name, and event times--use print_out()
	\endverbatim 

	@param[in] event -- event
	@param[in] agent -- agent
	@param[in] time -- time
	@retval None
 */
void
Write_Event( int event, int agent, struct time_type *time )
{
}

/**
	Convert event name into event ID number.

	<b> Algorithm: </b>
	\verbatim
	Capitalize event name so that case does not matter
	Verify that name's length is shorter than constant the EVENT_NAME_LENGTH_MAX
	For event ID = 0 to NUM_EVENTS
		Compare event name to Event_Names[ event ID ]
		If they are equal
			Return event ID
		Otherwise,
			Continue in loop
	Report error if no event matched
	\endverbatim

	@param[in] event_name -- name of event
	@retval event -- event ID corresponding to event_name
 */
int
get_event_id( char* event_name )
{
	// temporary return value
	return( 0 );
}

/**
	Convert agent name into agent ID number.

	<b> Algorithm: </b>
	\verbatim
	Capitalize agent name so that case does not matter
	Verify that the name's length is shorter than the constant DEV_NAME_LENGTH
	If name starts with 'U', then agent is a user terminal:
		Convert the name (except the initial 'U') into an integer--the agent ID
		Return the agent ID
	Otherwise, agent is a device:
		For agent ID = 0 to Num_Devices
			Compare agent name to the name at Dev_Table[ agent ID ]
			If they are equal
				Return agent ID + Num_Terminals + 1 since device agent IDs follow user agent IDs
			Otherwise,
				Continue in loop
		Report error if no agent name matched
	\endverbatim

	@param[in] agent_name -- name of agent
	@retval agent_id -- agent ID corresponding to agent_name
 */
int
get_agent_id( char* agent_name )
{
	// temporary return value
	return( 0 );
}

/**
	Print debugging message about Event_List.

	<b> Important Notes:</b>
	\li The debugging message is only printed if the variable DEBUG_EVT is
	turned on (set to 1). This is done by adding the following line to
	config.dat: <br>
	DEBUG_EVT= ON
	\li The function is not necessary to implement as it serves only to help
	the programmer in debugging the implementation of common data structures.

	@retval None
 */
void
Dump_evt( )
{
	// if debug flag is turned on
	if( DEBUG_EVT )
	{
		/* print debugging message */
	}
	else // do not print any message
	{ ; }
}
