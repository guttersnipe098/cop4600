/*******************************************************************************
* File:      obj2.c
* Version:   0.8
* Purpose:   Implements loading the event list
* Template:  Dr. David Workman, Time Hughey, Mark Stephens, Wade Spires, and
*            Sean Szumlanski
* Coded by:  Michael Altfield <maltfield@knights.ucf.edu>
* Course:    COP 4600 <http://www.cs.ucf.edu/courses/cop4600/spring2012>
* Objective: 2
* Created:   2012-02-07
* Updated:   2012-03-24
* Notes:     This program was written to be compiled against the gnu99 standard.
*            Please execute the following commands to build correctly:
*
*  CFLAGS="-g -I/home/eurip/ossim2010 --std=gnu99"
*  export CFLAGS
*  make -e sim
*******************************************************************************/

/**
	obj2.c


	Simulator Project for COP 4600

	Objective 2 project that implements loading and executing the boot
	program for the OS simulator.

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

int skipBlankLines( int );

/**
	Simulate booting the operating system by loading the boot program into
	kernel memory.

	This function is called from simulator.c (the simulator driver)
	before processing the event list. Boot() reads in the file boot.dat to load
	the kernel and initialize Mem_Map for the first program execution. Note that
	boot.dat is already open when Boot() is called and can immediately be read
	using the file pointer Prog_Files[BOOT] (Prog_Files is an array of file
	pointers opened by the simulator).

	The upper half of Mem_Map always holds the address map for the operating
	system, i.e., Mem_Map[Max_Segments ... 2*Max_Segments - 1] (the lower half
	is reserved for user programs). The PROGRAM and SEGMENT statements in the
	boot file are used to initialize the appropriate entries in Mem_Map. The
	actual kernel script is loaded into Mem (array representing physical
	memory) starting at absolute memory location 0. The Free_Mem list
	and Total_Free must be updated as the program is loaded to account for
	the region of Mem that is occupied by the kernel. 

	The format of boot.dat is the same as the other "program.dat"
	files, so it is divided into two sections: segment table information and
	instructions. To load the OS program, Boot() calls the Get_Instr() routine
	to read and process each line in the data file. Once the line is read, it is
	then stored in Mem at the appropriate absolute memory address. Once
	the entire OS program is loaded, Boot() should call Display_pgm() to echo 
	the contents of Mem to the output file. 

	<b> Important Notes:</b>
	\li Boot() is only active during objective 2, so if the current objective is not 2, then simply return from the function.
	\li The boot file, boot.dat, is already open when Boot() is called and can immediately be read using the file pointer Prog_Files[BOOT] (Prog_Files is an array of file pointers opened by the simulator).
	\li Get_Instr() is used by the function Loader() in objective 3.

	<b> Algorithm: </b>
	\verbatim
	Read fields the "PROGRAM" and number of segments from boot file

	Read segment information from boot file:
		For each segment in boot file
			Read "SEGMENT size_of_segment access_bits" from boot file
			Set size of current segment
			Set access bits of current segment
			Go to next segment in Mem_Map

	Read instructions from boot file:
		Current position in main memory--incremented for each instruction read

		For each segment
			Set base pointer of segment to its location in main memory
			For each instruction in the segment
				Read instruction into memory--call Get_Instr()
				Increment current position in main memory
			Go to next segment in Mem_Map

		Adjust size of memory since boot program is loaded into memory:
			Decrement total amount of free memory (Total_Free)
			Decrement size of free memory block (Free_Mem->size)
			Move location of first free memory block (Free_Mem->base)

		Display each segment of memory
			Display segment Mem_Map[i + Max_Segments] since kernel resides in
			Upper half of Mem_Map; pass NULL as PCB since OS has no PCB
	\endverbatim

	@retval None
 */
void
Boot( )
{

	// is this objective 2?
	if( Objective != 2 ){
		return;
	}

	// DECLARE VARIABLES
	char line[BUFSIZ]; // buffer for each line in our input file
	int num_segments;

	// get the number of segments defined in this file
	fscanf( Prog_Files[BOOT], "PROGRAM %d\n", &num_segments );
	// TODO: remove debug print
	//printf( "%d\n",num_segments );

	/**************************
	* GET SEGMENT DEFINITIONS *
	**************************/

	// loop through each SEGMENT definition
	for( int i=0; i<num_segments; i++ ){

		// DECLARE VARIABLES
		int element; // the current element in our Mem_Map array
		int size_of_segment, access_bit;

		// our current element in Mem_Maps[] is this iteration's segment number
		// plus the offset Max_Segments. Mem_Maps[] length = (Max_Segments * 2),
		// where the first half is for program data. We store this boot data in
		// the second half of Mem_Maps, hence the offset.
		element = Max_Segments + i;

		// get this segment's size & access bit
		fscanf( Prog_Files[BOOT], "SEGMENT %d %x\n",
		 &size_of_segment, &access_bit
		);
		// TODO: remove debug print
		//printf( "%d %x\n", size_of_segment, access_bit );

		// Mem_Map[].size
		Mem_Map[element].size   = size_of_segment;

		// Mem_Map[].element
		Mem_Map[element].access = access_bit;

		// Mem_Map[].base
		// is this the first element being stored to Mem_Map?
		if( i == 0 ){
			// this is the first element of Mem_Map; set the base to 0
			Mem_Map[element].base = 0;

		} else {
			// this is not the first element of Mem_Map

			// set this base to the last segment's base + the last segment's size
			Mem_Map[element].base =
			 Mem_Map[ element - 1 ].base + Mem_Map[ element - 1 ].size;
		}

		// TODO: remove debug
		//printf( "setting base of Mem_Map[ %d ] = %d\n", element, Mem_Map[element].base );

		// update memory pointers
		Total_Free     = Total_Free     - size_of_segment;
		Free_Mem->size = Free_Mem->size - size_of_segment;
		Free_Mem->base = Free_Mem->base + size_of_segment;

	} // end for( each SEGMENT definition )

	/***************
	* GET SEGMENTS *
	***************/

	// loop through each segment
	for( int i=0; i<num_segments; i++ ){

		// DECLARE VARIABLES
		int element; // the current element in our Mem_Map array

		// element in Mem_Maps[] is segment number + offset (see above)
		element = Max_Segments + i;

		// skip blank lines that may be used for formatting boot.dat
		skipBlankLines( BOOT );

		// loop through each instruction in this segment
		for( int j=0; j<Mem_Map[element].size; j++ ){

			// call Get_Instr() where the pointer to the next instruction is the
			// BOOT stream and the pointer to the instruction's future home in mem
			// is the base of the current segment + the current instruction number
			// in this segment

			// TODO: remove debug
			//printf( "calling Get_Instr() w Mem[ %d ]", Mem_Map[element].base+j );

			Get_Instr( BOOT, &Mem[ Mem_Map[element].base + j ] );

		}

		/**************************
		* DISPLAY MEMORY SEGMENTS *
		**************************/
		Display_pgm( &Mem_Map[element], i, NULL );

	}

	return;

}

/**
	Read the next instruction from the file Prog_Files[prog_id] into
	instruction.  

	The format of the file is a series of statements of the form
	'opcode operand'. opcode may be an actual instruction, such as SIO, or a
	device, such as DISK. The operand will vary depending on the type of opcode
	read. Each instruction starts on a new line, but consecutive instructions
	may be separated by empty lines. 

	<b> Important Notes:</b>
	\li The operand field of an instr_type structure is defined as a C union, so
	it may represent one of many values but only one at a time. For example,
	instruction->operand.burst is set for an SIO instruction, but
	instruction->operand.count is set for a SKIP instruction.
	\li For device instruction, assign unique opcodes to each device by adding
	NUM_OPCODES to its corresponding dev_id so device codes do not conflict with
	regular op_codes
	\li Get_Instr() is used by Boot() in objective 2 and Loader() in objective
	3.

	<b> Algorithm: </b>
	\verbatim
	Read opcode and operand from file--store as strings initially Convert to uppercase so that case does not matter

	Search all op_code IDs to determine which op_code that the opcode_str matches with
		If found the correct op_code
			Save the opcode in the instruction

			Process the operand differently depending on the op_code type:
				If SIO, WIO, or END instruction
					Convert operand into burst count
				Else, if REQ or JUMP instruction
					Convert operand into segment and offset pair
				Else, if SKIP instruction
					Convert operand into skip count

	If no op_codes matched, the op_code may correspond to a device
		Search device table for the particular device
			If found the correct device ID
				Assign a unique opcode for the device (See note above)
				Convert operand into number of bytes to transfer
		If no devices matched, quit program
	\endverbatim 


	@param[in] prog_id -- index into Prog_Files to read instruction from
	@param[out] instruction -- location to store instruction read
 */
void
Get_Instr( int prog_id, struct instr_type* instruction )
{

	// DECLARE VARIABLES
	char opcode_str[BUFSIZ], operand_str[BUFSIZ];

	// get the opcode & operands as strings from the given location
	fscanf( Prog_Files[prog_id], "%s %s", &opcode_str, &operand_str );

	// convert to upper case
	//strncpy( opcode_str,  strupr( opcode_str ),  BUFSIZ-1 );
	opcode_str[-1] = strupr( (char*) opcode_str );

	/**************************
	* DETERMINE THE OPCODE ID *
	**************************/
	instruction->opcode = -1;

	// loop through all possible opcode names
	for( int i=0; i<NUM_OPCODES; i++ ){

		// does the opcode match this iteration's opcode name?
		if( strncmp( opcode_str, Op_Names[i], sizeof(Op_Names[i])-1 ) == 0 ){
			// this opcode is matches this iteration's opcode name
			instruction->opcode = i;
		}

	}

	/************************
	* DETERMINE THE OPERAND *
	************************/

	// TODO; remove debug lines
	//printf( "\tstrings:\t%s|%s\n", opcode_str, operand_str );

	// did we find an opcode for this instruction?
	if( instruction->opcode != -1 ){
		// we found an opcode for this instruction

		switch( instruction->opcode ){
			case SIO_OP:
			case WIO_OP:
			case END_OP:
				sscanf( operand_str, "%lu", &instruction->operand.burst );
				break;

			case REQ_OP:
			case JUMP_OP:
				sscanf( operand_str, "[%d,%u]",
			 	&instruction->operand.address.segment,
			 	&instruction->operand.address.offset
				);
				break;

			default:
				sscanf( operand_str, "%u", &instruction->operand.count );
				break;
		}

	} else {
		// we could not find an opcode for this instruction; might be a device..

		// loop through every known device
		for( int i=0; i<Num_Devices; i++ ){
	
			// TODO: remove debug
			//printf( "comparing:|%s|%s|\n", opcode_str, Dev_Table[i].name );

			// does the opcode match this device?
			if(
			 strncmp(
			  opcode_str, Dev_Table[i].name, DEV_NAME_LENGTH-1
			 ) == 0
			){
				// this device is our opcode
	
				// we assign device's opcodes to be NUM_OPCODES + the device's id
				// to prevent conflict with real operation instructions
				instruction->opcode = NUM_OPCODES + i;
				sscanf( operand_str, "%lu", &instruction->operand.bytes );

				// exit the loop (there's no need to keep looking for a match)
				break;
			}

		}

	} // endif( did we find an opcode for this instruction? )

	// TODO: remove debug
	//printf( "\topcode id:%d\n", instruction->opcode );

	// did we determine the instruction yet?
	if( instruction->opcode == -1 ){
		// we did not determine the instruction! error & exit.
		err_quit( "Failed to determine instruction opcode!\n" );
	}

	return;

}

/**
	Simulate the Central Processing Unit.

	The CPU, in general, does the following:
	\li Fetches the instruction from memory (Mem)
	\li Interprets the instruction.
	\li Creates a future event based on the type of instruction and
	inserts the event into the event queue.

	Set_MAR() is called to define the next memory location for a Fetch().
	The first fetch from memory is based on CPU.state.pc. Instructions that
	create events are:
	\li WIO   							
	\li SIO							
	\li END							
	The SKIP and JUMP instructions are executed in "real-time" until
	one of the other instructions causes a new event.

	An addressing fault could be generated by a call to Fetch() or Write(), so
	Cpu() should just return in this case (the simulator will later call
	Abend_Service() from the interrupt handler routine to end the simulated
	program).

	<b> Important Notes:</b>
	\li Use a special agent ID, i.e., 0, to identify the boot program. For
	all other objectives, the agent ID should be retrieved from the CPU's
	currently running process--use this process's terminal position and add 1
	so that it does not conflict with boot.
	\li The only instructions processed by Cpu() are SIO, WIO, END, SKIP, and
	JUMP. No other instructions, such as REQ and device instructions, are
	encountered in Cpu(). This is by incrementing the program counter by 2 to
	bypass these unused instructions.
	\li A while(1) loop is needed to correctly evaluate SKIP and JUMP
	instructions since multiple SKIP and JUMP instructions may be encountered
	in a row. The loop is exited when an SIO, WIO, or END instruction is
	encountered.
	\li The function Burst_time() should be used to convert CPU cycles for SIO,
	WIO, and END instructions to simulation time (time_type).

	<b> Algorithm: </b>
	\verbatim
	Identify the agent ID for the currently running program:
		If CPU has no active PCB, then the program is boot
		Otherwise, the program is the active process running in the CPU, so use its ID

	Loop forever doing the following:
		Set MAR to CPU's program counter
		Fetch instruction at this address
		If fetch returns a negative value, a fault has occurred, so
			Return

		Determine type of instruction to execute
			If SIO, WIO, or END instruction
				If the objective is 3 or higher
					Increment total number of burst cycles for PCB
				Calculate when I/O event will occur using current time + burst time
				Add event to event list
				Increment PC by 2 to skip the next instruction--device instruction
				Return from Cpu() (exit from loop)

			If SKIP instruction
				If count > 0,
					Decrement count by 1
					Write this change to memory by calling Write()
					If write returns a negative value, a fault has occurred, so
						Return
					Increment the CPU's PC by 2 since the next instruction is to be skipped
				Otherwise, instruction count equals 0, so
					Increment the CPU's PC by 1 since the next instruction, JUMP, is to be executed
				Continue looping

			If JUMP instruction
				Set the PC for the CPU so that the program jumps to the address determined by the operand of instruction
				Continue looping
	\endverbatim 

	\retval None
 */
void
Cpu( )
{

	// DECLARE VARIABLES
	int agent_id;
	struct instr_type instruction;
	struct time_type sim_time;
	int result; // stores results from functions to check for errors

	/********************
	* Identify Agent ID *
	********************/

	// is the program boot?
	if( CPU.active_pcb == NULL ){
		// there is no active PCB; the program is boot (0 is our special case)
		agent_id = 0;
	} else {
		// there *is* an active PCB; get its terminal id (+1 because boot is 0)
		agent_id = CPU.active_pcb->term_pos + 1;
	}

	/*************
	* Iterate PC *
	*************/

	while( 1 ){

		// setting MAR to the CPU's program counter
		Set_MAR( &CPU.state.pc );

		// fetch next instruction
		result = Fetch( &instruction );

		// did fetch finish OK?
		if( result < 0 ){
			return;
		}

		// what kind of operation are we preforming?
		switch( instruction.opcode ){

			case SIO_OP:

				//If the objective is 3 or higher
				if( Objective >= 3 ){
					//Increment total number of burst cycles for PCB
					CPU.active_pcb->sjnburst = CPU.active_pcb->sjnburst + instruction.operand.burst;
				}

				Burst_time( instruction.operand.burst, &sim_time );
				Add_time( &Clock, &sim_time );
				Add_Event( SIO_EVT, agent_id, &sim_time );
				CPU.state.pc.offset = CPU.state.pc.offset + 2; // skip next instr
				return;

			case WIO_OP:

				//If the objective is 3 or higher
				if( Objective >= 3 ){
					//Increment total number of burst cycles for PCB
					CPU.active_pcb->sjnburst = CPU.active_pcb->sjnburst + instruction.operand.burst;
				}

				Burst_time( instruction.operand.burst, &sim_time );
				Add_time( &Clock, &sim_time );
				Add_Event( WIO_EVT, agent_id, &sim_time );
				CPU.state.pc.offset = CPU.state.pc.offset + 2; // skip next instr
				return;

			case END_OP:

				//If the objective is 3 or higher
				if( Objective >= 3 ){
					//Increment total number of burst cycles for PCB
					CPU.active_pcb->sjnburst = CPU.active_pcb->sjnburst + instruction.operand.burst;
				}

				Burst_time( instruction.operand.burst, &sim_time );
				Add_time( &Clock, &sim_time );
				Add_Event( END_EVT, agent_id, &sim_time );
				CPU.state.pc.offset = CPU.state.pc.offset + 2; // skip next instr
				return;

			case SKIP_OP:
				if( instruction.operand.count > 0 ){
					instruction.operand.count = instruction.operand.count - 1;

					result = Write( &instruction );
					// did Write() finish OK?
					if( result != 0 ){
						// Write() encountered a fault
						return;
					}

					CPU.state.pc.offset = CPU.state.pc.offset + 2; // skip next instr
				}

				CPU.state.pc.offset = CPU.state.pc.offset + 1; // execute next JUMP
				break;

			case JUMP_OP:
				CPU.state.pc.segment = instruction.operand.address.segment;
				CPU.state.pc.offset = instruction.operand.address.offset;
				break;

		}

	}

/*
			If JUMP instruction
				Set the PC for the CPU so that the program jumps to the address determined by the operand of instruction
				Continue looping
*/

}

/**
	Simulate a context switch that places a user program in execution.

	<b> Algorithm: </b>
	\verbatim
	Copy the given state into the CPU's state
	Resume execution of the program at the new PC--just call Cpu()
	\endverbatim

	@param[in] state -- program state to load into CPU
 */
void
Exec_Program( struct state_type* state )
{

	// copy paramater data to CPU struct
	CPU.state.mode = state->mode;
	CPU.state.pc.segment = state->pc.segment;
	CPU.state.pc.offset = state->pc.offset;

	// awake the great Central Processing Unit!
	Cpu();

}

/**
	Simulate the Memory Management Unit (MMU) that translates the contents of
	the Memory Address Register (MAR) to a real physical address.

	Use the contents of MAR = [segment,offset] as the logical address to be
	translated to a physical address.

	<b> Algorithm: </b>
	\verbatim
	Set segment to the segment saved in the MAR:
		If in kernel mode (CPU's mode equals 1)
			Set segment to be in upper half of Mem_Map (MAR.segment+Max_Segments)
		Otherwise, in user mode (CPU's mode equals 0)
			Just use lower half of Mem_Map (MAR.segment)

	If illegal memory access (access == 0)
		Create seg. fault event at the current time using the CPU's active process's terminal position (+ 1) as the agent ID
		Return -1

	If address is out of the bounds
		Create address fault event at the current time using the CPU's active process's terminal position (+ 1) as the agent ID
		Return -1

	Compute physical memory address:
		Address = base address from segment in Mem_Map + offset saved in MAR

	Return address
	\endverbatim

	@retval address -- physical address into Mem obtained from logical address
		stored in MAR
 */
int
Memory_Unit( )
{

	// DECLARE VARIABLES
	int segment;

	// are we in kernel mode?
	if( CPU.state.mode == 1 ){
		// we are in kernel mode; setting segment to the upper half of Mem_Map
		segment = MAR.segment + Max_Segments;
	} else {
		// we are in user mode
		segment = MAR.segment;
	}

	// illegal memory address check
	if( Mem_Map[segment].access == 0 ){
		Add_Event( SEGFAULT_EVT, CPU.active_pcb->term_pos + 1, &Clock );
		return -1;
	}

	// address out of bounds check
	if( MAR.offset > Mem_Map[segment].size ){
		Add_Event( ADRFAULT_EVT, CPU.active_pcb->term_pos + 1, &Clock );
		return -1;
	}

	// calculate physical memory address
	return (Mem_Map[segment].base + MAR.offset);

}

/**
	Set Memory Address Register (MAR) to the value of the given address.

	This function must be called to define the logical memory address of the
	next Fetch(), Read(), or Write() operation in memory.

	<b> Algorithm: </b>
	\verbatim
	Set MAR to given address
	\endverbatim

	@param[in] addr -- address to set MAR to
 */
void
Set_MAR( struct addr_type* addr )
{

	MAR.segment = addr->segment;
	MAR.offset = addr->offset;

}

/**
	Fetch next instruction from memory. 

	<b> Important Notes:</b>
	\li Fetch() is identical to Read()

	<b> Algorithm: </b>
	\verbatim
	Get current physical memory address--call Memory_Unit()
	If returned address < 0, then a fault occurred
		Return -1

	Set given instruction to instruction in memory at the current memory address
	Return 1 to signify that no errors occurred
	\endverbatim

	@param[out] instruction -- location to store instruction fetched from memory
	@retval status -- whether an error occurred or not (-1 or 1)
 */
int
Fetch( struct instr_type* instruction )
{

	// DECLARE VARIABLES
	int physical_address;

	// get the current physical memory address
	physical_address = Memory_Unit();

	// is this physical_address sane?
	if( physical_address < 0 ){
		// this address is not sane; return fail
		return -1;
	}

	// OPCODE
	instruction->opcode = Mem[ physical_address ].opcode;

	// OPERAND
	switch( instruction->opcode ){
		case SIO_OP:
		case WIO_OP:
		case END_OP:
			instruction->operand.burst = Mem[ physical_address ].operand.burst;
			break;

		case REQ_OP:
		case JUMP_OP:
			instruction->operand.address.segment =
			 Mem[ physical_address ].operand.address.segment;
			instruction->operand.address.offset =
			 Mem[ physical_address ].operand.address.offset;
			break;

		case SKIP_OP:
			instruction->operand.count = Mem[ physical_address ].operand.count;
			break;

		default:
			instruction->operand.bytes = Mem[ physical_address ].operand.bytes;
			break;
	}

	// temporary return value
	return 1;
}

/**
	Read next instruction from memory. 

	<b> Important Notes:</b>
	\li Read() is identical to Fetch()

	<b> Algorithm: </b>
	\verbatim
	Get current physical memory address--call Memory_Unit()
	If returned address < 0, then a fault occurred
		Return -1

	Set given instruction to instruction in memory at the current memory address
	Return 1 to signify that no errors occurred
	\endverbatim

	@param[out] instruction -- location to store instruction read from memory
	@retval status -- whether an error occurred or not (-1 or 1)
 */
int
Read( struct instr_type* instruction )
{
	// temporary return value
	return( 0 );
}

/**
	Write instruction to memory. 

	<b> Algorithm: </b>
	\verbatim
	Get current physical memory address--call Memory_Unit()
	If returned address < 0, then a fault occurred
		Return -1

	Set instruction in memory at the current memory address to given instruction
	Return 1 to signify that no errors occurred
	\endverbatim

	@param[in] instruction -- instruction to write to memory
	@retval status -- whether an error occurred or not (-1 or 1)
 */
int
Write( struct instr_type* instruction )
{

	// DECLARE VARIALBES
	int physical_address;

	physical_address = Memory_Unit();

	// is this address sane?
	if( physical_address < 0 ){
		// this address is illegal; return fault
		return -1;
	}

	// OPCODE
	Mem[ physical_address ].opcode = instruction->opcode;

	// OPERAND
	switch( instruction->opcode ){
		case SIO_OP:
		case WIO_OP:
		case END_OP:
			Mem[ physical_address ].operand.burst = instruction->operand.burst;
			break;

		case REQ_OP:
		case JUMP_OP:
			Mem[ physical_address ].operand.address.segment =
			 instruction->operand.address.segment;
			Mem[ physical_address ].operand.address.offset =
			 instruction->operand.address.offset;
			break;

		case SKIP_OP:
			Mem[ physical_address ].operand.count = instruction->operand.count;
			break;

		default:
			Mem[ physical_address ].operand.bytes = instruction->operand.bytes;
			break;
	}

	return 1;

}

/**
	Display contents of program to output file.

	Refer to intro.doc for the proper output format.
	Only a single segment (seg_num) of the segment table is displayed per
	call of Display_pgm(). Multiple calls of Display_pgm() must be made to
	display the entire table.

	<b> Important Notes:</b>
	\li Call print_out() to print to the output file.
	\li For objective 2, the given pcb will be NULL, so print the name as
	"BOOT".
	\li For objectives other than 2, use
	Prog_Names[ pcb->script[ pcb->current_prog ] ] to get the program name.
	Prog_Names is a global array holding the names of all programs the
	simulator can run. pcb->script is an array of all the programs that the
	process has (or will) run, and pcb->current_prog is the index into
	pcb->script of the program that the user is currently running, that is, the
	one that needs to be displayed.
	\li If instruction is not a regular instruction, e.g., not SIO, WIO, etc.,
	then it is a device. The location of the device in Dev_Table (to get its
	name) is obtained by taking the opcode of the current memory position -
	NUM_OPCODES since NUM_OPCODES was added to device op_codes in Get_Instr().

	<b> Algorithm: </b>
	\verbatim
	Print segment header:
		If boot process
			Print "BOOT"
		Else, if user process
			Print name of user's program
	Print additional header information
 Display current segment of memory: Get base memory position of the first instruction in the segment

		For each instruction in the segment
			If opcode is an instruction, look for the appropriate one in Op_Names
				Display memory position and instruction code

				Determine instruction type to display operand differently:
					If SIO, WIO, or END instruction
						Display burst count

					Else, if REQ or JUMP instruction
						Display segment/offset pair

					Else, if SKIP instruction
						Display skip count
			Else, opcode is a device
				Look for it in devtable
				Display memory position, device name, and byte count

			Update base memory position to go to next instruction in segment
	\endverbatim

	@param[in] seg_table -- segment table to display
	@param[in] seg_num -- segment within table to display (seg_table[ seg_num ])
	@param[in] pcb -- process control block that the segment table belongs to
	(NULL if table belongs to boot process)
 */
void
Display_pgm( segment_type* seg_table, int seg_num, pcb_type* pcb )
{

	// TODO remove debug print
	//printf("CALLED DISPLAY_PGM()\n");

	// DELCARE VARIABLES
	int counter;

	// TODO: remove debug block
	//printf( "start\n" );
	//printf( "0 opcode:%s\n", Op_Names[ Mem[0].opcode ] );
	//printf( "0 operand:%s\n", Mem[0].operand.burst );
	//printf( "end\n" );

	// loop through each segment in memory
	counter = 0;
	print_out( "\tSEGMENT #%d OF PROGRAM %s OF PROCESS %s\n", seg_num, Prog_Names[pcb->script[pcb->current_prog]], pcb->username );
	print_out( "\tACCBITS: %02X  LENGTH: %u\n",
	 seg_table[seg_num].access, seg_table[seg_num].size
	);
	print_out( "\tMEM ADDR  OPCODE  OPERAND\n" );
	print_out( "\t--------  ------  -------\n" );

	// loop through each instruction in this segment
	for( int i=0; i<(seg_table[seg_num].size); i++ ){

		// DECLARE VARIABLES
		int element = seg_table[seg_num].base + i;
		int opcode;

		// TODO: remove debug
		//printf( "\t***seg_table[%d].base:|%d|\n", seg_num, seg_table[seg_num].base );
		//printf( "\t***i:|%d|\n", i );

		// MEM ADDR
		print_out( "\t%8d ", element );

		// OPCODE
		opcode = Mem[element].opcode;

		// is this an operation or a device?
		if( opcode < NUM_OPCODES ){
			// this is an operation; print its name
			print_out( " %-6s ", Op_Names[ opcode ] );
		} else {
			// this is a device; print its name
			print_out( " %-6s ", Dev_Table[ opcode - NUM_OPCODES ].name );
		}

		// OPERAND
		switch( opcode ){
			case SIO_OP:
			case WIO_OP:
			case END_OP:
				// burst
				print_out( " %lu", Mem[element].operand.burst );
				break;

			case REQ_OP:
			case JUMP_OP:
				// address
				print_out( " [%d,%u]", 
		 		 Mem[element].operand.address.segment,
		 		 Mem[element].operand.address.offset
				);
				break;

			case SKIP_OP:
				// count
				print_out( " %u", Mem[element].operand.count );
				break;

			default:
				// bytes
				print_out( " %lu", Mem[element].operand.bytes );
		}

		print_out( "\n" );

		//printf( "\t%d %s %s\n",
		//printf( "\t%d %s\n",
		 //counter,
		 //Op_Names[ Mem[ Mem_Map[i].base ].opcode ],
		 //Mem[ Mem_Map[element].base ].opcode,
		 //Mem[ Mem_Map[element].base ].operand
		 //Op_Names[ Mem[ Mem_Map[element].base ].opcode ],
		 //Mem[ Mem_Map[element].base + j ].opcode,
		 //Op_Names[ Mem[ element ].opcode ]
		 //Mem[ element ].operand
		//);
		counter++;

	}

	print_out("\n");

}

/**
	Print debugging message about Mem.

	<b> Important Notes:</b>
	\li The debugging message is only printed if the variable DEBUG_MEM is
	turned on (set to 1). This is done by adding the following line to
	config.dat: <br>
	DEBUG_MEM= ON
	\li The function is not necessary to implement as it serves only to help
	the programmer in debugging the implementation of common data structures.

	@retval None
 */
void
Dump_mem( segment_type* seg_tab )
{
	// if debug flag is turned on
	if( DEBUG_MEM )
	{
		/* print debugging message */
	}
	else // do not print any message
	{ ; }
}

int
skipBlankLines( int prog_id )
{

	// TODO: remove debug print
	//printf( "called skipBlankLines(%d)\n", prog_id );

	// DECLARE VARIABLES
	char line[BUFSIZ]; // buffer for each line in our input file
	fpos_t seek_pos;   // stores current position of the stream
	fpos_t initial;   // stores initial position of the stream
	int sentinel = 1;

	if( feof( Prog_Files[prog_id] ) ){
		//printf( "\t***EOF DETECTED\n" );
		return 1;
	} else {
		//printf( "\t***EOF not DETECTED\n" );
	}
	fgetpos( Prog_Files[prog_id], &seek_pos );
	fgetpos( Prog_Files[prog_id], &initial );

	// loop until we detect a non-blank line
	while( sentinel ){

		if( feof( Prog_Files[prog_id] ) ){
			//printf( "\t***EOF DETECTED\n" );
			fsetpos( Prog_Files[prog_id], &initial );
			return 1;
		}

		// store the current position of the stream
		//fgetpos( Prog_Files[prog_id], &seek_pos );

		// TODO remove debug lines
		//printf( "\t***success getting seek_pos:|%d|\n", seek_pos );

		// get the next line from the stream
		fgets( line, BUFSIZ-1, Prog_Files[prog_id] );

		// is this line blank?
		if( strncmp( line, "\n", 2 ) == 0  || strncmp( line, "", 1 ) == 0 ){

//TODO remove debug lines
//printf( "\t***skipping line:|%s|\n", line );

			// this line is blank; do nothing so we'll keep gobbling up blank lines
			fgetpos( Prog_Files[prog_id], &seek_pos );

		} else {
//TODO remove debug lines
//printf( "\t***NOT skipping line:|%s|\n", line );
			// this line is *not* blank; backup to the previous line and return
			fsetpos( Prog_Files[prog_id], &seek_pos );
			sentinel = 0;
		}

	}

	return 0;

}
