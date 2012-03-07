/*******************************************************************************
* File:      obj3.c
* Version:   0.8
* Purpose:   Read all remaining *.dat files, build PCB, load programs & service
*            event interruption.
* Template:  Dr. David Workman, Time Hughey, Mark Stephens, Wade Spires, and
*            Sean Szumlanski
* Coded by:  Michael Altfield <maltfield@knights.ucf.edu>
* Course:    COP 4600 <http://www.cs.ucf.edu/courses/cop4600/spring2012>
* Objective: 3
* Created:   2012-02-25
* Updated:   2012-03-06
* Notes:     This program was written to be compiled against the gnu99 standard.
*            Please execute the following commands to build correctly:
*
*  CFLAGS="-g -I/home/eurip/ossim2010 --std=gnu99"
*  export CFLAGS
*  make -e sim
*******************************************************************************/
/**
	obj3.c


	Simulator Project for COP 4600

	Objective 3 project that implements loading program scripts into
	memory for the OS simulator.

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

void print_free_list( );

/**
	Service user logon events.

	<b> Important Notes:</b>
	\li Events of type LOGON_EVT will result in this call via 
	Interrupt_Handler() in simulator.c after interrupt() in obj1.c sets the
	global Event variable.

	<b> Algorithm: </b>
	\verbatim
	Allocate new Process Control Block with all fields initialized to 0

	Initialize specific PCB fields:
		Mark status as new
		Save terminal table position
		Set logon time
		Set user name 

	Save PCB in global terminal table

	Initialize list of programs user will run--call Get_Script()
	Execute first program of PCB--call Next_pgm()

	If objective 4 or higher, set scheduling flags:
		If no process is active in the CPU currently
			Turn off scheduling and CPU switches
		Otherwise,
			Turn on switches
	\endverbatim

	@retval None
 */
void
Logon_Service( )
{
	// TODO remove debug prints
	//printf( "\tAgent:|%d|\n", Agent );

	// DECLARE VARIABLES
	char* buf[BUFSIZ];
	pcb_type* pcb;

	pcb = (pcb_type*) calloc( 1, sizeof(struct pcb_type) );

	// STATUS
	pcb->status= NEW_PCB;

	// TERMINAL TABLE POS
	pcb->term_pos = Agent;

	// LOGON TIME
	pcb->logon_time = Clock;

	// USER NAME
	pcb->username[0] = 'U';
	pcb->username[1] = '\0';
	sprintf( (char*) buf, "%03d", Agent ); // TODO: use something more secure than sprintf
	strncat( pcb->username, (char*) buf, BUFSIZ-1 );

	// TODO remove debug prints
	printf( "User Logon:|%s|\n", (char*) pcb->username );
	//printf( "\tpcb->term_pos:|%d|\n", pcb->term_pos );
	//printf( "%s", Event );

	Term_Table[ pcb->term_pos ] = pcb;

	Get_Script( pcb );
	Next_pgm( pcb );

}

/**
	Read set of all programs that a user will run.
	
	<b> Important Notes:</b>
	\li The global variable Script_fp is a file pointer to the script file to
	be read. It is opened by the simulator when it is initialized. Hence, it
	should neither be opened nor closed in this function--only read from.
	\li The script file has no specific format, that is, the names for all
	programs that all users will run appear together in this file and are not
	necessarily on different lines. Script names for a single user must be
	repeatedly read from the file until encountering the script name "LOGOFF",
	which marks the last program the user will run.
	\li Since the number of scripts is unknown, it should be allocated to have
	Max_Num_Scripts elements.
	\li The given PCB's 'scripts' array, after allocation, must set each entry
	to the script ID corresponding to each script name. Even after Get_Script()
	stores all script IDs, the end of the scripts array is not explicitly
	stored; however, this can still be determined as the last valid entry of the
	array has the LOGOFF ID.

	<b> Algorithm: </b>
	\verbatim
	Allocate PCB's array of scripts
	Mark index of first script

	Read each script name in file
		Capitalize script name so that case does not matter

		Output name of the script to output file

		Determine script ID for script name read
			Compare script name to each name in Prog_Names array
				If names match
				Mark ID in PCB's scripts array
		Stop reading script names when "LOGOFF" script name encountered
	\endverbatim

	@param[in,out] pcb -- process control block for the script being read
	@retval None
 */
void
Get_Script( pcb_type *pcb )
{

	// DECLARE VARIABLES
	int i;
	char* line[BUFSIZ];
	char* script_name[BUFSIZ];

	// Allocate PCB's array of scripts

	// TODO: matching free()
	pcb->script = (struct prog_type*) calloc(
	 Max_Num_Scripts,
	 sizeof( prog_type )
	);

	// Mark index of first script
	pcb->current_prog = 0;

	print_out( "\tScript for user %s is:\n\t", pcb->username );

	// Read each script name in file

	// loop line-by-line until EOF
	i = 0;
	//while( fgets( (char*) line, sizeof(line), Script_fp) ){
	while( fscanf( Script_fp, "%s", script_name ) ){

		//sscanf( (char*) line, "%s", &script_name );

		// Capitalize script name so that case does not matter
		script_name[-1] = strupr( (char*) script_name );

		// TODO: remove debug
		//printf( "\tscript_name:|%s|\n", script_name );

		// Output name of the script to output file
		print_out( "%s  ", script_name );

		// Stop reading script names when "LOGOFF" script name encountered
		if( strcmp( (char*) script_name, "LOGOFF" ) == 0 ){
			break;
		}

		// Determine script ID for script name read

		// loop though each possible program name
		for( int j=0; j<NUM_PROGRAMS; j++ ){

			// does this iteration's program name match the this pcb's script_name?
			if( strcmp( (char*) script_name, Prog_Names[j] ) == 0 ){

				pcb->script[i] = (prog_type) j;

				// TODO: remove debug
				//printf( "\t\tmatch! pcb->script[%d]:|%s|\n", j, pcb->script[j] );

				break;
			}

		}
		// increment counter
		i = i+1;

	}

	print_out( "\n\n" );

}

/**
	Load the next program script for the given PCB into memory.

	<b> Important Notes:</b>
	\li Despite the name of the function, the "next" program to run may
	actually be the very first program script that the user wishes to run--hence
	the initial check as the first step of the algorithm.

	This function makes the transition to the next program in the script
	for (pcb), if there is one. A 1 is returned if the transition to the
	next program is successful, a 0 otherwise. The transition is possible
	only if the RB-list for the current program is empty and the script
	is not empty.

	<b> Algorithm: </b>
	\verbatim
	If the next program is not the first program the user will run
		And
	No unserviced I/O request blocks exist
		Deallocate memory used for the previous program--call Dealloc_pgm()

	If process has an unserviced I/O request block
		Do not load a new program

	If encountered last program script
		Mark PCB as terminated and do not load another program
		Calculate total time logged on

	Allocate and initialize a new segment table for the PCB's program--call Get_Memory()
	If failed to allocate space (pcb->seg_table is NULL)
		Do not load another program

	Load the next program into memory--call Loader()

	Clear (set to 0) PCB's area for saving CPU information during an interrupt

	Mark PCB as ready to run

	If objective 4 or greater
		Insert process into CPU's ready queue 

	Increment index position of program script for next call to Next_pgm()
	\endverbatim

	@param[in,out] pcb -- PCB for which to load the next program
	@retval was_new_prog_loaded -- whether a new program was loaded (1) or
	not (0)
 */
int
Next_pgm( pcb_type* pcb )
{
	// TODO: remove debug print`
	printf( "Just executed Next_pgm()\n" );

	// If the next program is not the first program the user will run
	//  && No unserviced I/O request blocks exist
	// 	Deallocate memory used for the previous program--call Dealloc_pgm()

	// TODO: change this back to "pcb->current_prog != 1" (?)
	if( pcb->current_prog != 0 && pcb->rb_q == NULL && pcb->current_prog != NUM_PROGRAMS+1 ){

		// TODO: remove debug print
		//printf( "omw to Dealloc dt first Next_Pgm() exec\n" );

		Dealloc_pgm( pcb );
	}

	// If process has an unserviced I/O request block
	//  Do not load a new program

	// is the process's IORB queue empty?
	if( pcb->rb_q != NULL ){
		// the queue is not empty; there exist unserviced IORB(s)
		return 0;
	}

	// If encountered last program script
	//  Mark PCB as terminated and do not load another program
	//  Calculate total time logged on

	// are we currently executing the last program?
	if( pcb->current_prog >= NUM_PROGRAMS ){

		// set the pcb's status to TERMINATED
		pcb->status = TERMINATED_PCB;

		// calculate the difference between the logon time and the current time;
		// save the result to pcb->total_logon_time
		Diff_time( &Clock, &pcb->total_logon_time );

		// do not load another program; simply return.
		return 0;
	}

	// Allocate and initialize a new segment table for the PCB's program--call Get_Memory()

	Get_Memory( pcb );

	// If failed to allocate space (pcb->seg_table is NULL)
	//  Do not load another program
	if( pcb->seg_table == NULL ){
		// TODO: remove debug print
		printf( "\tfailed to get memory!\n");

		return 0;
	}

	// Load the next program into memory--call Loader()

	Loader( pcb );

	// Clear (set to 0) PCB's area for saving CPU information during an interrupt

	pcb->cpu_save.mode = 0;
	pcb->cpu_save.pc.segment = 0;
	pcb->cpu_save.pc.offset = 0;

	// Mark PCB as ready to run
	pcb->status = READY_PCB;

	//If objective 4 or greater
	// Insert process into CPU's ready queue 

	// TODO: something wicked this way comes in Obj 4

	// Increment index position of program script for next call to Next_pgm()
	pcb->current_prog = pcb->current_prog + 1;

	return 1;

}

/**
	Allocate space for a new by program by reading segment table information.

	<b> Important Notes:</b>
	\li The global variables Prog_Files and Prog_File_Names are arrays holding
	the file pointers to all program files and the respective program names.
	To access the needed program file, use the given PCB's current script ID to
	index into the PCB's script array. Then use this value to index into each
	array as each script entry holds a program ID of program the user is
	running. More simply put, just use pcb->script[ pcb->current_prog ] to
	index into either array (Prog_Files or Prog_File_Names) for the needed
	program.
	\li The file pointers held in Prog_Files are opened by the simulator when it
	is initialized. Hence, they should neither be opened nor closed in this
	function--only read from.
	\li The format of each program is the same as the other "program.dat"
	files as described in "intro.doc", so it is divided into two sections:
	segment table information and instructions. This is the same format as
	boot.dat in objective 2, except multiple programs may be in the same file
	as the same program type, such as the editor program, might be used by
	multiple users or multiple times by the same user.
	\li Once the segment table has been allocated and constructed, memory is
	allocated to each segment.
	\li Note that the call to Compact_mem() in the algorithm is guaranteed to
	free enough memory to load a new program since the total amount of free
	space is checked prior to the function call.

	<b> Algorithm: </b>
	\verbatim
	Read the fields "PROGRAM" and number of segments from program file

	Allocate pcb's segment table

	Read segment table information from program file:
		For each segment in user's program
			Read "SEGMENT size_of_segment access_bits" from program file
			Set size of current segment
			Set access bits of current segment
			Increment amount of memory used
			Go to next segment in user's segment table

	If program cannot fit into memory (total amount of free space < needed size)
		Clear user's segment table, which marks allocation failure
		Return without allocating a segment

	Allocate each segment:
		For each segment
			Try to allocate a new segment and save returned base pointer position--call Alloc_seg()

			If no space was available (invalid position returned)
				Compact memory to eliminate external fragmentation and try to get the base pointer again--call Alloc_seg()
	}
	\endverbatim

	@param[in,out] pcb -- process for which to allocate segment
	@retval None
 */
void
Get_Memory( pcb_type* pcb )
{
	// TODO: remove debug
	printf( "entered Get_Memory()!\n" );

	// DECLARE VARIABLES
	unsigned int num_segments;
	int base_ptr;

	// TODO: remove debug prints
	//printf( "%d\n", pcb->current_prog );
	//printf( "%d\n", pcb->script[ pcb->current_prog ] );

	// Read the fields "PROGRAM" and number of segments from program file

	printf( "about to call skipBlankLines(%d)\n", pcb->script[pcb->current_prog] );
	if( skipBlankLines( pcb->script[pcb->current_prog] ) ){
		printf( "\t***detected EOF; returning NOW!\n" );
		pcb->current_prog = NUM_PROGRAMS+1;
		return;
	}

	// get the number of segments defined for this program
	fscanf(
	 Prog_Files[ pcb->script[pcb->current_prog] ],
	 "PROGRAM %d\n",
	 &num_segments
	);

	// TODO: remove debug prints
	//printf( "***line:|%s|\n", Prog_Files[ pcb->script[pcb->current_prog] ] );
	//printf( "***num_segments:|%d|\n", num_segments );
	printf( "\t***PROGRAM %d\n", num_segments );

	// Allocate pcb's segment table
	pcb->seg_table = (struct segment_type*) calloc(
	 num_segments,
	 sizeof(struct segment_type)
	);

	pcb->num_segments = num_segments;

// TODO remove debug block
if( pcb->seg_table == NULL ){
	err_quit( "malloc() returned a null pointer. Try compiling on a non-eustis machine.\n" );
}

	// Read segment table information from program file:
	//  For each segment in user's program

	// loop through each SEGMENT definition
	for( int i=0; i<num_segments; i++ ){

		//		Read "SEGMENT size_of_segment access_bits" from program file
		//		Set size of current segment
		//		Set access bits of current segment

		// get this segment's size & access bit
		fscanf( Prog_Files[ pcb->script[pcb->current_prog] ], "SEGMENT %d %x\n",
		 &pcb->seg_table[i].size,
		 &pcb->seg_table[i].access
		);

		if( i==0 ){
			pcb->seg_table[i].base = 0;
		} else {
			pcb->seg_table[i].base = pcb->seg_table[i-1].base + pcb->seg_table[i-1].size;
		}

		// TODO: remove debug hard code
		//pcb->seg_table[i].access = 'X';
		//printf( "*** segment[%d].access:|%x|\n", i, pcb->seg_table[i].access );
		printf( "\t***SEGMENT %d %02X\n", pcb->seg_table[i].size, pcb->seg_table[i].access );

		//		Increment amount of memory used

		Total_Free = Total_Free - pcb->seg_table[i].size;

		// If program cannot fit into memory (total amount of free space < needed size)
		// 	Clear user's segment table, which marks allocation failure
		// 	Return without allocating a segment

		if( Total_Free < pcb->seg_table[i].size ){

			// TODO: remove debug print
			printf( "\tinsufficient Total_Free\n" );

			free( pcb->seg_table );
			pcb->seg_table = NULL;
			return;
		}

		//		Go to next segment in user's segment table
	}

	// Allocate each segment:
	// 	For each segment

	// loop through each SEGMENT definition
	for( int i=0; i<num_segments; i++ ){
		
		// 		Try to allocate a new segment and save returned base pointer position--call Alloc_seg()

		base_ptr = Alloc_seg( pcb->seg_table[i].size );

		// 		If no space was available (invalid position returned)
		// 			Compact memory to eliminate external fragmentation and try to get the base pointer again--call Alloc_seg()

		// did Alloc_seg() finish the request as asked?
		if( base_ptr == -1 ){
			// return pointer was -1; there was an error.

			// free some more mem!
			Compact_mem();

			// try one last time (this is supposedly guranteed to work this time;
			// see documentation for more info)
			base_ptr = Alloc_seg( pcb->seg_table[i].size );

		}

		//pcb->seg_table[i].base = base_ptr;
			
	}

}

/**
	Allocate segment of given size and return base memory pointer to this
	segment.

	This function searches the list of free memory segments for a segment
	of at least length 'size' and returns the address of this free segment.
	If the request cannot be satisfied, -1 is returned.

	<b> Important Notes:</b>
	\li The head of the free list is the pointed to by the global variable
	Free_Mem. The list is in order by base address of each free segment in the
	list. That is, if free node A has base address 5 and free node B has base
	address 10, then A precedes B in the list.
	\li The base addresses held by free list nodes are integer values denoting
	indices into memory. Hence, the index (array position) into memory (Mem)
	held by the free list node to be used should be returned, not '&Mem[i]'.

	<b> Algorithm: </b>
	\verbatim
	Search free list for a block large enough to hold the segment
		If block is large enough to hold segment
			Stop searching

	If no block was large enough
		Return invalid position

	Else, if block is an exact fit
		Use free block's base address for segment

		If block is first in list
			Set the head of the free list to the next node
		Else, segment is in middle or at end of list
			By-pass removed node in list

		Delete node
		Decrement total amount of free memory

	Else, if block is larger than needed
		Use free block's base address for segment

		Adjust node's size and starting position:
			Increment base address and decrement block size by given size

		Decrement total amount of free memory
	\endverbatim

	@param[in] size -- size of segment to allocate
	@retval segment_base -- base address segment should begin at in memory
 */
int
Alloc_seg( int size )
{

	// DECLARE VARIABLES
	struct seg_list* current;
	struct seg_list* last;
	int base;

	// Search free list for a block large enough to hold the segment
	// 	If block is large enough to hold segment
	//		Stop searching

	// iterate through Free_Mem list until we find a block large enough
	current = (struct seg_list*) Free_Mem;
	last = (struct seg_list*) Free_Mem;
	while( current->size < size ){

		// If no block was large enough
		//	Return invalid position

		// if we are still iterating, we haven't found a block large enough, so
		// we get the next one...
		last = current;
		current = current->next;

		// ...until we reach the end of the list, at which point, return invalid
		if( current == NULL ){
			return -1;
		}

	}

	// Else, if block is an exact fit
	//	Use free block's base address for segment

	// if we made it this far, we found a block large enough for the given size
	// is the block the exact same size as requested?
	if( current->size == size ){

		//	If block is first in list
		//		Set the head of the free list to the next node
		//	Else, segment is in middle or at end of list
		//		By-pass removed node in list

		if( current == Free_Mem ){
			Free_Mem = Free_Mem->next;
		} else {
			last->next = current->next;
		}

		//	Delete node
		base = current->base;
		free( current );
		current = NULL;

		//	Decrement total amount of free memory
		Total_Free = Total_Free - size;

		return base;

	}

	// if we made it this far, we found a block large enough for the given size
	// and it's larger than the size needed.

	//Else, if block is larger than needed
	//	Use free block's base address for segment

	//	Adjust node's size and starting position:
	//		Increment base address and decrement block size by given size
	current->base = current->base + size;
	current->size = current->size - size;

	//	Decrement total amount of free memory
	Total_Free = Total_Free - size;

	return current->base;

}

/**
	Load user program from file into memory.

	<b> Important Notes:</b>
	\li Get_Instr() from objective 2 is used to read each instruction from the
	program file. Recall that Get_Instr() accepts two parameters: a program ID
	and an instruction. The program ID is an index into the array Prog_Files for
	the program file to read. This index is obtained in the same manner as in
	Get_Memory()--use pcb->script[ pcb->current_prog ]. The instruction is a
	pointer to memory at the position in which to store the instruction for the
	segment, i.e., &Mem[ base + offset ], where the base and offset are from
	the user's segment, should be passed to Get_Instr().
	\li The segment table, pcb->seg_table, contains the length of each segment
	and where it should be loaded into memory. The number of segments is given
	by pcb->num_segments.

	<b> Algorithm: </b>
	\verbatim
	For each segment in the user's segment table
		For each instruction in the segment
			Read instruction from program file into memory--call Get_Instr()
		Display each segment of program
	\endverbatim

	@retval None

	@param[in,out] pcb -- process for which to load segments
 */
void
Loader( pcb_type* pcb )
{
	// TODO: remove debug code
	printf( "\n *** called Loader() ***\n" );
	printf( "\tpcb->num_segments:|%d|\n", pcb->num_segments );

	// For each segment in the user's segment table

	for( int i=0; i < (pcb->num_segments); i++ ){

		//	For each instruction in the segment
		//for( int j=0; j < (pcb->seg_table[j].size); j++ ){
		for( int j=0; j < (pcb->seg_table[i].size); j++ ){

			//		Read instruction from program file into memory--call Get_Instr()
			Get_Instr(
			 pcb->script[ pcb->current_prog ],
			 &Mem[ pcb->seg_table[i].base + j ]
			);

		}

			// TODO: remove debug line
			printf( "\tabout to call Display_pgm()\n" );

			//	Display each segment of program
			Display_pgm( pcb->seg_table, i, pcb );
	}

	print_out( "\t\tProgram number %d, %s, has been loaded for user %s\n\n",
	 pcb->current_prog + 1,
	 Prog_Names[pcb->script[pcb->current_prog]],
	 pcb->username
	);

}

/**
	Deallocate user's segments and segment table for current program.

	<b> Algorithm: </b>
	\verbatim
	For each segment
		Deallocate the segment--call Dealloc_seg()
	Free segment table
	\endverbatim

	@param[in,out] pcb -- process for which to deallocate segments
	@retval None
 */
void
Dealloc_pgm( pcb_type* pcb )
{

	// TODO: remove debug print
	//printf( "\n\t***|%d|***\n", pcb->seg_table->base );
	//printf( "\n\t***|%u|***\n", pcb->seg_table->size );

	// loop though each segment
	for( int i=0; i < (pcb->num_segments); i++ ){
		Dealloc_seg( pcb->seg_table[i].base, pcb->seg_table[i].size );
	}

	free( pcb->seg_table );
	pcb->seg_table = NULL;

}

/**
	Deallocate segment at given base position and size. 

	<b> Important Notes:</b>
	\li The elements in the free list are ordered by the address of the segment
	they represent.

	<b> Algorithm: </b>
	\verbatim
	Allocate and initialize a new free segment
	Increment total amount of free memory

	If free list is empty
		Set new segment as the start of the free list

	Else, if new segment goes at start of list
		Set new segment as the start of the free list

	Otherwise, new segment goes in the middle or at the end of the list:
		Search free list to find segments to the left and right of the segment being freed
			If previous segment's base < new segment's base < next segment's base
				Insert new segment between two segments in list

		If search failed, then the new segment belongs at end of the list
			Add new segment to end of the list

	Merge adjacent segments into a single, larger segment--call Merge_seg()
	\endverbatim

	@param[in] base -- position in memory (Mem array)
	@param[in] size -- size of memory block being freed
	@retval None
 */
void
Dealloc_seg( int base, int size )
{

	// DECLARE VARIABLES
	struct seg_list* new_segment;
	struct seg_list* current;
	struct seg_list* last;

	current = Free_Mem;
	last = current;

	// Allocate and initialize a new free segment

	// allocate
	new_segment = (struct seg_list*) malloc( sizeof(struct seg_list) );

	// initalize
	new_segment->base = base;
	new_segment->size = size;
	new_segment->next = NULL;

	// Increment total amount of free memory
	Total_Free = Total_Free + size;
	
	//If free list is empty
	if( Free_Mem == NULL ){
		//	Set new segment as the start of the free list

		Free_Mem = new_segment;
	
	//Else, if new segment goes at start of list
	} else if( new_segment->base < Free_Mem->base ){
		//	Set new segment as the start of the free list

		new_segment->next = Free_Mem;
		Free_Mem = new_segment;

	//Otherwise, new segment goes in the middle or at the end of the list:
	} else {
		//	Search free list to find segments to the left and right of the segment being freed
		while( current != NULL ){

			//		If previous segment's base < new segment's base < next segment's base

			// is our new block between the last iteration & the current iteration?
			if(
			 (new_segment->base) > (last->base)
			 && (new_segment->base) < (current->base)
			){
				//			Insert new segment between two segments in list
				last->next = new_segment;
				new_segment->next = current;
				break;
			}

			// skip to next block
			last = current;
			current = current->next;

		}

		//	If search failed, then the new segment belongs at end of the list
		//		Add new segment to end of the list

		last->next = new_segment;

	}

	// Merge adjacent segments into a single, larger segment--call Merge_seg()

	Merge_seg( last, new_segment, new_segment->next );

}

/**
	Merge adjacent free blocks into a single, larger block (if possible).

	For example, suppose the newly freed segment starts at position 5 and ends
	at 10 while the next free segment starts at 11 and ends at 20. Then these
	two segments would be merged into a single segment starting at 5 and ending
	at 20.

	<b> Important Notes:</b>
	\li The elements in the free list are ordered by the address of the segment
	they represent.
	\li The total amount of free memory does not change due to merging.
	\li The new segment added to memory, via Dealloc_seg(), is given as the
	second parameter. Either prev_seg or next_seg (or both) may be NULL, but
	the algorithm accounts for this and does not attempt to merge.

	<b> Algorithm: </b>
	\verbatim

	If the previous free segment is not invalid
		And
	The new free segment follows directly after the previous segment
		Add the new segment's size to the previous segment
		Set previous segment's next link to new segment's next link
		Free node representing new segment
		Set new segment pointer to previous segment

	If the next free segment is not invalid
		And
	The new free segment directly precedes the next segment
		Add the next segment's size to the new segment
		Set new segment's next link to next segment's next link
		Free node representing next segment
	\endverbatim

	@param[in,out] prev_seg -- segment before new_seg to potentially merge
	with new_seg
	@param[in,out] new_seg -- new segment added to memory
	@param[in,out] next_seg -- segment after new_seg to potentially merge with
	new_seg
	@retval None
 */
void
Merge_seg( seg_list* prev_seg, seg_list* new_seg, seg_list* next_seg )
{

	// If the previous free segment is not invalid
	//	 And
	// The new free segment follows directly after the previous segment

	if( prev_seg != NULL && (prev_seg->base)+(prev_seg->size) == new_seg->base ){

		// Add the new segment's size to the previous segment
		// Set previous segment's next link to new segment's next link
		// Free node representing new segment
		// Set new segment pointer to previous segment

		prev_seg->size = prev_seg->size + new_seg->size;
		prev_seg->next = new_seg->next;
		free( new_seg );
		new_seg = prev_seg;

	}
		
	// If the next free segment is not invalid
	//	And
	// The new free segment directly precedes the next segment

	if( next_seg != NULL && (new_seg->base)+(new_seg->size) == next_seg->base ){

		// Add the next segment's size to the new segment
		// Set new segment's next link to next segment's next link
		// Free node representing next segment

		new_seg->size - new_seg->size + next_seg->size;
		new_seg->next = next_seg->next;
		free( next_seg );
		next_seg = NULL;

	}

}

/**
	Service end program events.

	<b> Important Notes:</b>
	\li Events of type END_EVT will result in this call via 
	Interrupt_Handler() in simulator.c after interrupt() in obj1.c sets the
	global Event variable.
	\li The PCB status DONE_PCB is not the same as TERMINATED_PCB. Only the
	current program is ended, but the user's PCB may still be active as the user
	may be running multiple programs.

	<b> Algorithm: </b>
	\verbatim
	Retrieve PCB associated with program from terminal table
	Mark pcb as done
	Remove PCB from CPU's active process

	Calculate active time for process and busy time for CPU
   Record time process became blocked

	If objective 4 or higher
		Deallocate all I/O request blocks associated with PCB--call Purge_rb()

	If the PCB has no outstanding I/O request blocks
		Load the next program for the user--call Next_pgm()

	If objective 4 or higher
		Turn both the scheduling and CPU switches on in order to retrieve the next process to run
	\endverbatim

	@retval None
 */
void
End_Service( )
{

	// DECLARE VARIABLES
	struct pcb_type* pcb;
	struct time_type* active_time;

	// Retrieve PCB associated with program from terminal table
	// TODO: change back from "Agent - 1" to "Agent" (?)
	pcb = Term_Table[ Agent ];

	// Mark pcb as done
	pcb->status = DONE_PCB;

	// Remove PCB from CPU's active process
	CPU.active_pcb = NULL;

	// Calculate active time for process and busy time for CPU
   // Record time process became blocked

	active_time = &pcb->run_time;
	Diff_time( &Clock, active_time );
	Add_time( active_time, &CPU.total_busy_time );

	// TODO: remove (?)
	pcb->block_time = Clock;
	pcb->status = BLOCKED_PCB;

// TODO: remove debug
//printf( "\t***pcb->script:|%d|\n", pcb->script[ pcb->current_prog ] );
//printf( "\t***pcb->current_prog:|%d|\n", pcb->current_prog );

	print_out( "\t\tProgram number %d, %s, has ended for user %s\n",
	 pcb->current_prog, Prog_Names[pcb->script[pcb->current_prog]], pcb->username
	);
	print_out( "\t\tCPU burst was %d instructions.\n", CPU.CPU_burst );

	// If objective 4 or higher
	//  Deallocate all I/O request blocks associated with PCB--call Purge_rb()
	if( Objective >= 4 ){
		// TODO: more work for obj#4
		;
	}

	// If the PCB has no outstanding I/O request blocks
	//	Load the next program for the user--call Next_pgm()
	if( pcb->rb_q == NULL ){
		Next_pgm( pcb );
	}

	// If objective 4 or higher
	//	Turn both the scheduling and CPU switches on in order to retrieve the next process to run
	if( Objective >= 4 ){
		// TODO: more work for obj#4
		;
	}

}

/**
	Service segmentation fault and address fault events.

	<b> Important Notes:</b>
	\li This function services causes an abnormal end to the simulated program.
	\li Events of type SEGFAULT_EVT and ADRFAULT_EVT will result in this call
	via Interrupt_Handler() in simulator.c after interrupt() in obj1.c sets the
	global Event variable.
	\li Use print_out() function for output.

	<b> Algorithm: </b>
	\verbatim
	Print output message indicating that process was terminated due to some abnormal condition
	Call End_Service() to handle terminating process.
	\endverbatim

	@retval None
 */
void
Abend_Service( )
{

	print_out( "ERROR: Process terminated due to an abnormal condition\n" );
	End_Service();

}

/**
	Print contents of free list.

	<b> Important Notes:</b>
	\li This function should be useful for debugging and verifying memory
	allocation and deallocation functions, especially Merge_seg().

	<b> Algorithm: </b>
	\verbatim
	For each free block
		Print base position and last position in block
	\endverbatim
 */
void print_free_list( )
{
}

/**
	Print debugging message about Term_Table.

	<b> Important Notes:</b>
	\li The debugging message is only printed if the variable DEBUG_PCB is
	turned on (set to 1). This is done by adding the following line to
	config.dat: <br>
	DEBUG_PCB= ON
	\li The function is not necessary to implement as it serves only to help
	the programmer in debugging the implementation of common data structures.

	@retval None
 */
void
Dump_pcb( )
{
	// if debug flag is turned on
	if( DEBUG_PCB )
	{
		/* print debugging message */
	}
	else // do not print any message
	{ ; }
}
