/*******************************************************************************
* File:      obj1.c
* Version:   0.2
* Purpose:   Implements loading the event list
* Template:  Dr. David Workman, Time Hughey, Mark Stephens, Wade Spires, and
*            Sean Szumlanski
* Coded by:  Michael Altfield <maltfield@knights.ucf.edu>
* Course:    COP 4600 <http://www.cs.ucf.edu/courses/cop4600/spring2012>
* Objective: 1
* Created:   2012-01-27
* Updated:   2012-02-05
* Notes:     This program was written to be compiled against the gnu99 standard.
*            Please execute the following commands to build correctly:
*
*  CFLAGS="-g -I/home/eurip/ossim2010 --std=gnu99"
*  export CFLAGS
*  make -e sim
*******************************************************************************/

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
void printList();

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

	// DECLARE VARIBLES
	FILE* logon;       // file pointer to our input file
	char line[BUFSIZ]; // buffer for each line in our input file

	int event_id;
	int agent_id;
	struct time_type sim_time;

	// open logon file for reading
	logon = fopen( LOGON_FILENAME, "r" );

	// loop line-by-line until EOF
	while( fgets( line, sizeof(line), logon) ){

		// DECLARE VARIABLES
		char event_name[BUFSIZ];
		char agent_name[BUFSIZ];
		unsigned long time;

		sscanf( line, "%s %s %lu", &event_name, &agent_name, &time );

		agent_id = get_agent_id( agent_name );
		event_id = get_event_id( event_name );
		Uint_to_time( time, &sim_time );

		// add event to the event list
		Add_Event( event_id, agent_id, &sim_time );

	}

	// close logon file
	fclose( logon );

	return;

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

	// DECLARE VARIABLES
	struct event_list* new_node;  // ptr to our new event node
	struct event_list* cur_node;  // ptr to the current node (when iterating)
	struct event_list* prev_node; // ptr to the previous node (when iterating)

	// allocate memory for the new new_node
	new_node = (struct event_list*) malloc( sizeof(struct event_list) );

	// store argments to the new new_node's fields
	new_node->event = event;
	new_node->agent = agent;
	new_node->time  = *time;
	new_node->prev = NULL;
	new_node->next = NULL;

	// is the Event_List empty?
	if( Event_List == NULL ){
		// Event_List is empty; make new_node our first event in Event_List
		Event_List = new_node;
	} else {
		// Event_List is not empty

		prev_node = Event_List;
		cur_node = Event_List;
		// traverse it until new_node finds its place
		while(
		 cur_node != NULL // stop looping when we reach the end of the list
		 // stop looping if the new node does not occur after the next node 
		 && Compare_time( &new_node->time, &cur_node->time ) >= 0
		){

			prev_node = cur_node;
			cur_node = cur_node->next;

		}

		// where did we just insert the node in the list?
		if( cur_node == Event_List ){
			// we just added a node to the front of the list; update Event_List

			new_node->next = Event_List;
			Event_List = new_node;
			new_node->prev = NULL;

		} else {
			// we did not add the node to the front of the list

			// now we insert the new node just before the current node
			prev_node->next = new_node;
			new_node->next = cur_node;
			new_node->prev = prev_node;

		}

		// did we just update the end of the list?
		if( cur_node != NULL ){
			// we did not just update the end of the list; update last node
			cur_node->prev = new_node;
		}

	}

	return;

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

	// DECLARE VARIABLES
	struct event_list* event;

	// remove first element from the list
	event = Event_List;
	Event_List = Event_List->next;

	if( Event_List != NULL ){
		Event_List->prev = NULL;
	}

	// update OS fields for this event
	Clock = event->time;
	Agent = event->agent;
	Event = event->event;

	// write event to output file
	Write_Event( (int) event->event, event->agent, &(event->time) );

	// deallocate this event from memory
	free( event );

	return;

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

	// DECLARE VARIABLES
	int hours, minutes, seconds; // to be calculated from time_type.seconds
	int milli, micro, nano;      // to be calculated from time_type.nanoseconds
	char agent_name[BUFSIZ];
	char agent_id[BUFSIZ];       // string to convert agent integer to string

	hours =       (time->seconds) / 3600; // 60*60 = 3600 seconds in an hour
	minutes =   ( (time->seconds) % 3600 ) / MIN;
	seconds = ( ( (time->seconds) % 3600 ) % MIN );

	milli =       (time->nanosec) / MSEC;
	micro =     ( (time->nanosec) % MSEC ) / mSEC;
	nano  =     ( (time->nanosec) % MSEC ) % mSEC;

	// is the given agent a terminal or device?
	if( agent <= Num_Terminals ){
		// agent is a user terminal
		strncpy( agent_name, "U", BUFSIZ );
		sprintf( agent_id, "%03d", agent ); // TODO: use something more secure than sprintf
		strncat( agent_name, agent_id , BUFSIZ );
	} else {
		// agent is a device
		strncpy( agent_name, Dev_Table[ agent - Num_Terminals - 1 ].name, BUFSIZ );
	}

	print_out(
	 " %-5s %5s   HR:%8d MN:%2d SC:%2d MS:%3d mS:%3d NS:%3d\n\n",
	 Event_Names[event], agent_name, hours, minutes, seconds, milli, micro, nano
	);

	return;

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
	// convert to upper case
	event_name = strupr(event_name);

	// is the Event Name too long?
	if( strlen(event_name) > EVENT_NAME_LENGTH_MAX ){
		// Event Name is too long; warn
		err_warn(
		 "Event Name (%s) is too long (>%d)", event_name, EVENT_NAME_LENGTH_MAX
		);
	}

	// loop through every known event
	for( int i=0; i<NUM_EVENTS; i++ ){

		// does this known event match this event_name?
		if( strncmp( event_name, Event_Names[i], EVENT_NAME_LENGTH_MAX ) == 0 ){
			// this event matches our event_name; return its index
			return i;
		}

	}

	// if we made it this far, there was an error
	err_quit( "Failed to get Event ID from Event Name (%s)", event_name );


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

	// convert to upper case
	agent_name = strupr(agent_name);

	// is the Agent Name too long?
	if( strlen(agent_name) > DEV_NAME_LENGTH ){
		// Agent Name is too long; warn
		err_warn(
		 "Agent Name (%s) is too long (>%d)", agent_name, DEV_NAME_LENGTH
		);
	}

	// what type of agent is this?
	if( agent_name[0] == 'U' ){
		// agent is a terminal

		// convert all but first letter of agent_name to integer
		return atoi( &agent_name[1] );

	}

	// if we made it this far, agent is a device
	// loop through every known device
	for( int i=0; i<Num_Devices; i++ ){

		// does this device match this agent?
		if( strncmp( agent_name, Dev_Table[i].name, DEV_NAME_LENGTH ) == 0 ){
			// this device is our agent

			// we return this device's index + the number of terminals + 1
			// because the devices agents IDs follow the user agent IDs
			return (i + Num_Terminals + 1);
		}

	}

	// if we made it this far, there was an error
	err_quit( "Failed to get Agent ID from Agent Name (%s)", agent_name );

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

// print the doubly linked list for debugging
void printList(){

	// DECLARE VARIABLES
	struct event_list* node;

	node = Event_List;
	while( node != NULL ){

		printf( "(%d|%d) -> ", node->event, node->agent );
		printf( "(%d|%d)\n", node->time.seconds, node->time.nanosec );
		node = node->next;

	}
	printf( "\n" );

}
