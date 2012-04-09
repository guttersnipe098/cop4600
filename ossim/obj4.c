/*******************************************************************************
* File:      obj4.c
* Version:   0.3
* Purpose:   Implements programs and handles I/O requests.
* Template:  Dr. David Workman, Time Hughey, Mark Stephens, Wade Spires, and
*            Sean Szumlanski
* Coded by:  Michael Altfield <maltfield@knights.ucf.edu>
* Course:    COP 4600 <http://www.cs.ucf.edu/courses/cop4600/spring2012>
* Objective: 3
* Created:   2012-03-24
* Updated:   2012-04-09
* Notes:     This program was written to be compiled against the gnu99 standard.
*            Please execute the following commands to build correctly:
*
*  CFLAGS="-O0 -g3 -I/home/eurip/ossim2010 --std=gnu99"
*  export CFLAGS
*  make -e sim
*******************************************************************************/
/**
	obj4.c


	Simulator Project for COP 4600

	Objective 4 project that implements running all programs and handles
	I/O requests.

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
	Add given process to the end of the CPU ready queue.
	(i.e., pcb is added to the end of CPU.ready_q)

	<b> Important Notes:</b>
	\li The queue is doubly and circularly linked to allow efficient access to
	both the front and the end of the list. That is, the first node's previous
	link points to the last node in the list, and the last node's next link
	points to the first node in the list. For the special case in which the
	queue has a single node, all links point to itself.

	<b> Algorithm: </b>
	\verbatim
	Create and initialize pcb node

	If ready queue is empty
		Set ready queue to new node
		All links point to itself since it is circularly linked
	Otherwise, queue is non-empty
		Place node at end of queue
	\endverbatim

	@param pcb -- PCB to add to CPU's ready queue
	@retval None
 */
void
Add_cpuq( pcb_type *pcb )
{

	// DECLARE VARIABLES
	struct pcb_list* pcbListNode;

	//Create and initialize pcb node
	pcbListNode = (pcb_list*) calloc( 1, sizeof(struct pcb_list) );
	pcbListNode->pcb = pcb;

	//If ready queue is empty
	if( CPU.ready_q == NULL ){

		//Set ready queue to new node
		CPU.ready_q = pcbListNode;

		//All links point to itself since it is circularly linked
		pcbListNode->next = pcbListNode;
		pcbListNode->prev = pcbListNode;

	//Otherwise, queue is non-empty
	} else {

		//Place node at end of queue

		// CPU.ready_q points to the head of a doubly, circularly linked list
		// therefore, the head's previous node is the last node of the list

		// set the CPU ready queue's last node's next ptr to the new pcb_list node
		CPU.ready_q->prev->next = pcbListNode;

		// set the new pcb_list node's prev ptr to the current last node
		pcbListNode->prev = CPU.ready_q->prev;

		// put the new pcb_list node at the end of the CPU ready queue
		CPU.ready_q->prev = pcbListNode;

		// set the new pcb_list node's next ptr to the head of the CPU ready queue
		pcbListNode->next = CPU.ready_q;

	}

}

/**
	Add an I/O request block to the device's waiting queue.
	(i.e., rb is added to end of Dev_Table[dev_id].wait_q)

	<b> Important Notes:</b>
	\li The queue is doubly and circularly linked to allow efficient access to
	both the front and the end of the list. That is, the first node's previous
	link points to the last node in the list, and the last node's next link
	points to the first node in the list. For the special case in which the
	queue has a single node, all links point to itself.

	<b> Algorithm: </b>
	\verbatim
	Create and initialize request block node

	If device queue is empty
		Set device queue to new node
		All links point to itself since it is circularly linked
	Otherwise, queue is non-empty
		Place node at end of queue

	Record time when rb is enqueued
	\endverbatim

	@param dev_id -- index into device table to the device that I/O is
		being requested from
	@param rb -- request block to add to device's ready queue
	@retval None
 */
void
Add_devq( int dev_id, rb_type* rb )
{
	// DECLARE VARIABLES
	struct rb_list* rbListNode;

	//Create and initialize request block node
	rbListNode = (struct rb_list*) calloc( 1, sizeof(struct rb_list) );
	rbListNode->rb = rb;

	//If device queue is empty
	if( Dev_Table[ dev_id ].wait_q == NULL ){

		//Set device queue to new node
		Dev_Table[ dev_id ].wait_q = rbListNode;

		//All links point to itself since it is circularly linked
		rbListNode->next = rbListNode;
		rbListNode->prev = rbListNode;

	//Otherwise, queue is non-empty
	} else {

		//Place node at end of queue

		// Dev_Table[].wait_q points to the head of a doubly, circularly linked list
		// therefore, the head's previous node is the last node of the list

		// set the device queue's last node's next ptr to the new rb_list node
		Dev_Table[ dev_id ].wait_q->prev->next = rbListNode;

		// set the new rb_list node's prev ptr to the current last node
		rbListNode->prev = Dev_Table[ dev_id ].wait_q->prev;

		// put the new rb_list node at the end of the device ready queue
		Dev_Table[ dev_id ].wait_q->prev = rbListNode;

		// set the new rb_list node's next ptr to the head of the device ready queue
		rbListNode->next = Dev_Table[ dev_id ].wait_q;

	}

	//Record time when rb is enqueued
	rb->q_time = Clock;

}

/**
	Add an I/O request block to the process's request block queue.
	(i.e., rb is added to end of pcb->rb_q)

	<b> Algorithm: </b>
	\verbatim
	Create and initialize request block node

	If request block queue is empty
		Set request block queue to new node
		All links point to itself since it is circularly linked
	Otherwise, queue is non-empty
		Place node at end of queue
	\endverbatim

	<b> Important Notes:</b>
	\li The queue is doubly and circularly linked to allow efficient access to
	both the front and the end of the list. That is, the first node's previous
	link points to the last node in the list, and the last node's next link
	points to the first node in the list. For the special case in which the
	queue has a single node, all links point to itself.

	@param pcb -- pcb requesting I/O service
	@param rb -- request block to add to pcb's ready queue
	@retval None
 */
void
Add_rblist( pcb_type* pcb, rb_type* rb)
{

	// DECLARE VARIABLES
	struct rb_list* rbNode;

	//Create and initialize request block node
	rbNode = (struct rb_list*) calloc( 1, sizeof(struct rb_list) );
	rbNode->rb = rb;

	//If request block queue is empty
	if( pcb->rb_q == NULL ){

		//Set request block queue to new node
		pcb->rb_q = rbNode;

		//All links point to itself since it is circularly linked
		rbNode->next = rbNode;
		rbNode->prev = rbNode;

	//Otherwise, queue is non-empty
	} else {

		//Place node at end of queue

		// pcb->rb_q points to the head of a doubly, circularly linked list
		// therefore, the head's previous node is the last node of the list

		// set the request block queue's last node's next ptr to the new pcb_list node
		pcb->rb_q->prev->next = rbNode;

		// set the new rb_list node's prev ptr to the current last node
		rbNode->prev = pcb->rb_q->prev;

		// put the new rb_list node at the end of the request block queue
		pcb->rb_q->prev = rbNode;

		// set the new rb_list node's next ptr to the head of the request block queue
		rbNode->next = pcb->rb_q;
	}

}

/**
	Select the next process for allocation to the CPU according to the currently
	active scheduling algorithm.

	<b> Important Notes:</b>
	\li Currently, only first-come first-serve scheduling is being used.

	<b> Algorithm (FCFS): </b>
	\verbatim
		Update the number of processes serviced by the CPU

		Get first pcb in CPU's ready queue
		Remove first node in queue

		If not removing last pcb in ready queue
			Change head of ready queue

		Calculate time process was ready
		Increment total ready time for process
		Increment total CPU queue waiting time

		Reset the burst count for the next program
	\endverbatim

	@retval pcb -- next active process
 */
pcb_type*
Scheduler( )
{

	// DECLARE VARIABLES
	struct pcb_list* nextPcbListNode;
	struct pcb_type* nextPcb;

	//Update the number of processes serviced by the CPU
	CPU.num_served = CPU.num_served + 1;

	//Get first pcb in CPU's ready queue
	nextPcbListNode = CPU.ready_q;

	if( nextPcbListNode == NULL ){
		return NULL;
	}

	//Remove first node in queue
	nextPcb = nextPcbListNode->pcb;

	//If not removing last pcb in ready queue
	if( nextPcbListNode != nextPcbListNode->next ){

		//Change head of ready queue

		// set the last node's next ptr to the second node in the list
		CPU.ready_q->prev->next = CPU.ready_q->next;

		// set the second node in the list's prev ptr to the last node
		CPU.ready_q->next->prev = CPU.ready_q->prev;

		// set the head of the ready queue to be the second node in the list
		CPU.ready_q = CPU.ready_q->next;

	// TODO: add below else for "removing last pcb in ready queue" case
	} else {
		CPU.ready_q->next = NULL;
		CPU.ready_q->prev = NULL;
		CPU.ready_q = NULL;
	}

	// TODO: free(CPU.ready_q) here?

	//Calculate time process was ready
	Diff_time( &Clock, &nextPcb->ready_time );

	//Increment total ready time for process
	Add_time( &nextPcb->ready_time, &nextPcb->total_ready_time );

	//Increment total CPU queue waiting time
	Add_time( &nextPcb->ready_time, &CPU.total_q_time );

	//Reset the burst count for the next program
	nextPcb->sjnburst = 0;

	return nextPcb;
}

/**
	Service event to start an I/O operation.

	<b> Important Notes:</b>
	\li Events of type SIO_EVT will result in this call via 
	Interrupt_Handler() in simulator.c after interrupt() in obj1.c sets the
	global Event variable.
	\li SIO is a non-blocking I/O operation. As such, the program causing the
	event will continue running after the event is serviced.
	\li Control is returned to the program after servicing this request since
	the process is not waiting for I/O to complete.
	\li An event to end I/O may or may not have been placed in the event list
	depending on whether or not the device was busy processing some other I/O
	request at the time Start_IO() was called.

	<b> Algorithm: </b>
	\verbatim
	Allocate and initialize new I/O request block--call Alloc_rb()

	Add rb to both the device's and pcb's queues--call both Add_devq() and Add_rblist()

	Initiate I/O operation on requested device--call Start_IO()

	Return control to interrupted process--turn CPU switch on and scheduling switch off
	\endverbatim

	@retval None
 */
void
Sio_Service( )
{

	// DECLARE VARIABLES
	struct rb_type* newIORB;
	
	//Allocate and initialize new I/O request block--call Alloc_rb()
	newIORB = Alloc_rb();

	//Add rb to both the device's and pcb's queues--call both Add_devq() and Add_rblist()
	Add_devq(    newIORB->dev_id, newIORB );
	Add_rblist(  newIORB->pcb,   newIORB );

	//Initiate I/O operation on requested device--call Start_IO()
	Start_IO( newIORB->dev_id );

	//Return control to interrupted process--turn CPU switch on and scheduling switch off
	CPU_SW = ON;
	SCHED_SW = OFF;

}

/**
	Allocate and initialize a new I/O request block.

	<b> Important Notes:</b>
	\li Calculating the device ID (i.e., its index in Dev_Table) for the rb is
	rather difficult. The calculation is complicated as the instruction for the
	device being requested must be retrieved from its location in Mem, which
	involves a great deal of indirection. For example, we may have the
	following:
	<pre>
			SIO       32  <-- instruction requesting I/O service
			DISK     300  <-- instruction with device ID that we need
			WIO        5  <-- next instruction to be fetched for process
	</pre>
	The device instruction must be found in memory using only the current
	pcb with the pcb's saved program counter incremented passed the point
	needed.
	\li We subtract 1 from the offset when calculating the device instruction
	since we increment offset by 2 in Cpu() after executing each SIO
	instruction. Hence, offset gives the instruction AFTER the device, while
	offset - 1 gives us the device.  For example, the following shows a sample
	situation
	<pre>
			SIO       32  <-- instruction requesting I/O service
			DISK     300  <-- instruction with device we need (offset - 1)
			WIO        0  <-- current offset postion
	</pre>
	\li When calculating the request ID for the rb, one is subtracted from the
	offset since 2 was added to PC in Cpu(). This is similar to the situation
	above for calculating the device ID.
	\li The number of bytes to transfer is obtained by using the operand of the
	device instruction.

	<b> Algorithm: </b>
	\verbatim
	Allocate request block

	Initialize request block's fields:
		Mark status as pending
		Retrieve process that owns this rb using the current agent ID

		Calculate and save the device's ID for the rb:
			Let saved_pc be the program counter from the pcb's saved CPU state
			Let device_seg be the segment from the pcb's segment table of the last executed instruction (retrieved from saved_pc)
			Let device_instr be the instruction from Mem at postion device_seg.base + saved_pc.offset - 1
			Finally, set rb->dev_id be the opcode at device_instr converted to its corresponding device ID (i.e., opcode - NUM_OPCODES)

		Set number of bytes to transfer

		Save logical address (segment/offset pair) of the device instruction using process's saved PC
	\endverbatim

	@retval rb -- new I/O request block
 */
rb_type*
Alloc_rb( )
{

	// DECLARE VARIABLES
	struct rb_type* newIORB;
	struct addr_type saved_pc;
	struct segment_type device_seg;
	struct instr_type device_instr;

	//Allocate request block
	newIORB = (struct rb_type*) calloc( 1, sizeof(struct rb_type) );

	//Initialize request block's fields:

		//Mark status as pending
	newIORB->status = PENDING_RB;

		//Retrieve process that owns this rb using the current agent ID
	newIORB->pcb = Term_Table[ Agent - 1 ];

		//Calculate and save the device's ID for the rb:

			//Let saved_pc be the program counter from the pcb's saved CPU state
		saved_pc = newIORB->pcb->cpu_save.pc;

			//Let device_seg be the segment from the pcb's segment table of the last executed instruction (retrieved from saved_pc)
		device_seg = newIORB->pcb->seg_table[saved_pc.segment];

			//Let device_instr be the instruction from Mem at postion device_seg.base + saved_pc.offset - 1
		device_instr = Mem[ device_seg.base + saved_pc.offset - 1 ];

			//Finally, set rb->dev_id be the opcode at device_instr converted to its corresponding device ID (i.e., opcode - NUM_OPCODES)
	newIORB->dev_id = device_instr.opcode - NUM_OPCODES;

		//Set number of bytes to transfer
	newIORB->bytes = device_instr.operand.bytes;

	// TODO: be aware of addition
	saved_pc.offset = saved_pc.offset - 1;

		//Save logical address (segment/offset pair) of the device instruction using process's saved PC
	newIORB->req_id.segment = saved_pc.segment;
	newIORB->req_id.offset = saved_pc.offset;

	return newIORB;
}

/**
	Start processing I/O request for given device.

	<b> Important Notes:</b>
	\li Add_time() and Diff_time() should be used for calculating and
	incrementing many of the simulation time fields, such as the device's busy
	time. 
	\li The time at which I/O completes is obtained using rb->bytes and the
	speed of the device: device->bytes_per_sec and device->nano_per_byte.
	Using all these values, the seconds and nanoseconds fields of a simulation
	time structure can be filled in. Note that bytes per sec and nano per byte
	are, to some degree, reciprocals, which implies that one should divide by
	the former but multiply with the latter.
	\li When finally adding the ending I/O event to the event list, the
	device's ID must be converted into an agent ID. Recall in objective 1 that
	agent IDs for devices were set to directly follow terminal user IDs.

	<b> Algorithm: </b>
	\verbatim
	If the device is busy
		Do nothing

	If the device has no requests to service in its waiting queue
		Do nothing

	Get and remove first node from waiting list

	Mark rb as the current request block in the device; set rb's status as active

	Update time spent waiting in queue--calculate difference between current time and time it was enqueued and add this value to the current queue wait time--use Add_time() and Diff_time()

	Compute length of time that I/O will take (transfer time)--compute number of seconds and then use the remainder to calculate the number of nanoseconds

	Increment device's busy time by the transfer time--use Add_time()

	Compute time I/O ends--ending time is the current time + transfer time--use Add_time() and Diff_time()

	Update the number of IO requests served by the device

	Add ending I/O device interrupt to event list
	\endverbatim

	@param[in] dev_id -- ID of device
	@retval None
 */
void
Start_IO( int dev_id )
{

	// DECLARE VARIABLES
	struct rb_list* rbListNode;
	struct rb_type* rb;
	struct time_type transferTime;
	struct time_type time;

	// TODO: verify discrepancy of: if(){ return } conditions

	// TODO: change to "==" (?)
	//If the device is busy
	if( Dev_Table[ dev_id ].current_rb != NULL ){
		//Do nothing
		return;
	}

	//If the device has no requests to service in its waiting queue
	if( Dev_Table[ dev_id ].wait_q == NULL ){
		//Do nothing
		return;
	}

	// TODO: revisit discrepancy
	//Get and remove first node from waiting list
	rbListNode = Dev_Table[ dev_id ].wait_q;

	// also store the node's request block to rb since we're freeing the node
	rb = rbListNode->rb;

	// does wait_q contain >1 node?
	if( rbListNode->next != rbListNode ){
		// wait_q contains more than 1 node
		rbListNode->prev->next = rbListNode->next;
		rbListNode->next->prev = rbListNode->prev;
		Dev_Table[ dev_id ].wait_q = Dev_Table[ dev_id ].wait_q->next;
	} else {
		// wait_q contains exactly 1 node
		rbListNode->next = NULL;
		rbListNode->prev = NULL;
		Dev_Table[ dev_id ].wait_q = NULL;
	}

	// TODO: free rbListNode (?)

	//Mark rb as the current request block in the device; set rb's status as active
	Dev_Table[ dev_id ].current_rb = rb;
	rb->status = ACTIVE_RB;

	// TODO: revisit discrepancy on all time calculations below

	//Update time spent waiting in queue--calculate difference between current time and time it was enqueued and add this value to the current queue wait time--use Add_time() and Diff_time()
	time = Clock;
	Diff_time( &( rb->q_time ), &time );
	Add_time( &time, &( Dev_Table[ dev_id ].total_q_time ) );

	//Compute length of time that I/O will take (transfer time)--compute number of seconds and then use the remainder to calculate the number of nanoseconds
	transferTime.seconds =   rb->bytes / Dev_Table[ dev_id ].bytes_per_sec;
	transferTime.nanosec = ( rb->bytes % Dev_Table[ dev_id ].bytes_per_sec )
	 * Dev_Table[ dev_id ].nano_per_byte;

	//Increment device's busy time by the transfer time--use Add_time()
	Add_time( &transferTime, &( Dev_Table[ dev_id ].total_busy_time ) );

	//Compute time I/O ends--ending time is the current time + transfer time--use Add_time() and Diff_time()
	Add_time( &Clock, &transferTime );

	//Update the number of IO requests served by the device
	Dev_Table[ dev_id ].num_served = Dev_Table[ dev_id ].num_served + 1;

	//Add ending I/O device interrupt to event list
	Add_Event( EIO_EVT, dev_id + Num_Terminals + 1, &transferTime );

}

/**
	Service event to wait on an I/O operation.

	This function services requests to check an I/O operation for completion. 
	If the operation is complete, the process resumes execution immediately;
	otherwise, the process is blocked.

	<b> Important Notes:</b>
	\li Events of type WIO_EVT will result in this call via 
	Interrupt_Handler() in simulator.c after interrupt() in obj1.c sets the
	global Event variable.
	\li WIO, unlike SIO, is a blocking I/O operation. As such, the program
	causing the event will be blocked (removed from the ready queue or from the
	CPU) after this event is serviced. However, if I/O had already been
	started from an earlier SIO event, then the process may be able to
	continue, but only if the I/O request has already been completed.
	\li Retrieving the request instruction (REQ) from memory is done similarly
	to finding the device ID in Alloc_rb(). The saved CPU information is used
	to find the necessary segment in the PCB's segment table. Once the segment
	has been found, the segment base is added to the current PC offset (minus
	1) to get the position in memory that the request instruction is located
	at.
	\li The request instruction's operand, which is an address, should be
	passed to Find_rb() to find the needed request block as the request address
	holds information pointing to the device being waited on.

	<b> Algorithm: </b>
	\verbatim
	Retrieve pcb from the terminal table that is waiting for I/O

	Retrieve request instruction using saved CPU information

	Locate request block that process wants to wait for--call Find_rb()

	Determine status of request block
		If rb is still active or pending
			Block process since I/O not finished:
				Mark PCB as blocked; mark it 

				Turn CPU and scheduling switches on to schedule a new process

				Calculate active time for process and busy time for CPU

				Record time process was blocked

				Print output message giving blocked procese and burst count

		If rb has finished
			Delete it and keep running current process:
				Delete rb from pcb's list

				Turn CPU switch on to execute a process
				Turn scheduling flag off to use current process (i.e., not schedule a new process)

	\endverbatim
 */
void
Wio_Service( )
{

	// DECLARE VARIABLES	
	struct pcb_type* pcb;
	struct instr_type requestInstr;
	struct rb_type* rb;
	struct time_type time;

	//Retrieve pcb from the terminal table that is waiting for I/O
	pcb = Term_Table[ Agent - 1 ];

	//Retrieve request instruction using saved CPU information
	requestInstr = Mem[
	 pcb->seg_table[pcb->cpu_save.pc.segment].base + pcb->cpu_save.pc.offset - 1
	];

	//Locate request block that process wants to wait for--call Find_rb()
	rb = Find_rb( pcb, &requestInstr.operand.address );

	//Determine status of request block
		//If rb is still active or pending
	if( rb->status == ACTIVE_RB || rb->status == PENDING_RB ){
			//Block process since I/O not finished:
				//Mark PCB as blocked; mark it 
		pcb->status = BLOCKED_PCB;
		pcb->wait_rb = rb;
		CPU.active_pcb = NULL;

				//Turn CPU and scheduling switches on to schedule a new process
		CPU_SW   = ON;
		SCHED_SW = ON;

				//Calculate active time for process and busy time for CPU
		time = Clock;
		Diff_time( &(pcb->run_time), &time );
		Add_time( &time, &( pcb->total_run_time ) );
		Add_time( &time, &( CPU.total_busy_time ) );

				//Record time process was blocked
		pcb->block_time = Clock;

		// TODO: change sjnburst?

				//Print output message giving blocked procese and burst count
		print_out( "\t\tUser %s is blocked for I/O.\n", pcb->username );
		print_out( "\t\tCPU burst was %d instructions.\n\n", pcb->sjnburst );

	}

		//If rb has finished
	if( rb->status == DONE_RB ){
			//Delete it and keep running current process:
				//Delete rb from pcb's list
		Delete_rb( rb, pcb );

				//Turn CPU switch on to execute a process
		CPU_SW = ON;

				//Turn scheduling flag off to use current process (i.e., not schedule a new process)
		SCHED_SW = OFF;

	}

}

/**
	Search the PCB's request block list to find the rb with the matching
	request ID.

	<b> Algorithm: </b>
	\verbatim
	Traverse the pcb's request block list 
		If the rb's request address matches the given requested address
			The needed rb has been found
	\endverbatim

  @param[in] pcb -- process whose request blocks are being searched
  @param[in] req_id -- address of request block searching for
  @retval rb -- request block with address matching req_id 
 */
rb_type*
Find_rb( pcb_type* pcb, struct addr_type* req_id )
{

	// DECLARE VARIABLES
	struct rb_list* result;

	//Traverse the pcb's request block list 

	// first, start at the head of the list
	result = pcb->rb_q;
	do {

		//If the rb's request address matches the given requested address
		if( result->rb->req_id.segment == req_id->segment
		 && result->rb->req_id.offset == req_id->offset ){
			//The needed rb has been found
			return result->rb;
		}

		// increment to the next node for the next iteration
		result = result->next;

	// loop until we've reached the head of the circularly linked list again
	} while( result != pcb->rb_q );

	// if we made it this far, we didn't find the rb; return NULL
	return( NULL );
}

/**
  Delete given request block from process's list of request blocks. The
  request block itself is also deallocated if found.

	<b> Algorithm: </b>
	\verbatim
	Traverse the pcb's request block list 
		If the rb has been found
			If deleting last remaining node
				Delete list
			Otherwise, remove node from middle of list
				By-pass node in list

				If deleting front of queue
					Change head of list
			Deallocate both the list node and the request block
	\endverbatim

  @param[in] rb -- request block to remove from process's list
  @param[in] pcb -- process from which to remove request block
 */
void
Delete_rb( rb_type* rb, pcb_type* pcb )
{

	// DECLARE VARIABLES
	struct rb_list* result;

	//Traverse the pcb's request block list 
	// first, start at the head of the list
	result = pcb->rb_q;
	do {

		//If the rb has been found
		if( result->rb == rb ){

			//If deleting last remaining node
			if( result == result->next ){

				//Delete list

				free( pcb->rb_q->rb );
				pcb->rb_q->rb = NULL;

				free( pcb->rb_q );
				pcb->rb_q = NULL;

				return;

			//Otherwise, remove node from middle of list
			} else {

				//By-pass node in list
				result->prev->next = result->next;
				result->next->prev = result->prev;

				//If deleting front of queue
				if( result == pcb->rb_q ){
					//Change head of list
					pcb->rb_q = result->next;
				}

				//Deallocate both the list node and the request block
				free( result->rb );
				result->rb = NULL;

				free( result );
				result = NULL;

				return;

			}

		}

		// increment to the next node for the next iteration
		result = result->next;

	// loop until we've reached the head of the circularly linked list again
	} while( result != pcb->rb_q );

}

/**
	Service an I/O interrupt event from a device.

	<b> Important Notes:</b>
	\li Events of type EIO_EVT will result in this call via 
	Interrupt_Handler() in simulator.c after interrupt() in obj1.c sets the
	global Event variable.

	<b> Algorithm: </b>
	\verbatim
	Retrieve device that caused the I/O interrupt from the device table
	Retrieve request block issued to device that is now complete
	Retrieve process that issued the request block to the device

	Mark that device no longer has a current request block
	Mark rb's status as done

	Service next I/O request block for this device--call Start_IO()

	Determine current status of process
		If process is ready to be ran or is already running
			Turn off both switches

		If process is blocked due to I/O
			If process was waiting on this rb 
				The process can now run:
					Delete rb from pcb's waiting list--call Delete_rb()
					Mark pcb as ready to run
					Add pcb to CPU's ready queue--call Add_cpuq()

			Record time process became ready
			Calculate and record time process was blocked

			If CPU is has no currently active process
				Turn on both switches to schedule a new process to run
			Otherwise, CPU is already busy, so
				Turn off both switches

		If the process is done, but only waiting for I/O to complete.

			Delete rb from pcb's waiting list--call Delete_rb()

			Load next program for pcb--call Next_pgm()
				If one was available
					Mark pcb's status as ready to run

			Record time process became ready
			Calculate and record time process was blocked

			If CPU is idle
				Turn on CPU and scheduling switches to run a new process
			Otherwise, CPU is already busy, so
				Turn off both switches

	\endverbatim
}
 */
void
Eio_Service( )
{

	// TODO: remove debug print
	printf( "called Eio_Service()" );

	// DECLARE VARIABLES
	struct device_type* device;
	struct rb_type* rb;
	struct pcb_type* pcb;

	//Retrieve device that caused the I/O interrupt from the device table
	device = &( Dev_Table[ Agent - Num_Terminals - 1 ] );

	//Retrieve request block issued to device that is now complete
	rb = device->current_rb;

	//Retrieve process that issued the request block to the device
	pcb = rb->pcb;

	//Mark that device no longer has a current request block
	device->current_rb = NULL;

	//Mark rb's status as done
	rb->status = DONE_RB;

	//Service next I/O request block for this device--call Start_IO()
	Start_IO( Agent - Num_Terminals - 1 );

	//Determine current status of process
		//If process is ready to be ran or is already running
	if( pcb->status == READY_PCB || pcb->status == ACTIVE_PCB ){
			//Turn off both switches
		SCHED_SW = OFF;
		CPU_SW   = OFF;
	}

		//If process is blocked due to I/O
	if( pcb->status == BLOCKED_PCB ){

			//If process was waiting on this rb 
		if( pcb->wait_rb == rb ){

				//The process can now run:
					//Delete rb from pcb's waiting list--call Delete_rb()
			Delete_rb( rb, pcb );

					//Mark pcb as ready to run
			pcb->wait_rb = NULL;
			pcb->status = READY_PCB;

					//Add pcb to CPU's ready queue--call Add_cpuq()
			Add_cpuq( pcb );

		}

			//Record time process became ready
		pcb->ready_time = Clock;

			//Calculate and record time process was blocked
		Diff_time( &Clock, &( pcb->block_time ) );
		Add_time( &( pcb->block_time ), &( pcb->total_block_time ) );

			//If CPU is has no currently active process
		if( CPU.active_pcb == NULL ){
				//Turn on both switches to schedule a new process to run
			SCHED_SW = ON;
			CPU_SW   = ON;

			//Otherwise, CPU is already busy, so
		} else {
				//Turn off both switches
			SCHED_SW = OFF;
			CPU_SW   = OFF;
		}

	}

	// TODO: revisit discrepancy
		//If the process is done, but only waiting for I/O to complete.
	if( pcb->status == DONE_PCB ){

			//Delete rb from pcb's waiting list--call Delete_rb()
		Delete_rb( rb, pcb );

			//Load next program for pcb--call Next_pgm()
				//If one was available
		if( Next_pgm( pcb ) ){
					//Mark pcb's status as ready to run
			pcb->status = READY_PCB;
		}

			//Record time process became ready
		pcb->ready_time = Clock;

			//Calculate and record time process was blocked
		Diff_time( &Clock, &( pcb->block_time ) );
		Add_time( &( pcb->block_time ), &( pcb->total_block_time ) );

			//If CPU is idle
		if( CPU.active_pcb == NULL ){
				//Turn on CPU and scheduling switches to run a new process
			CPU_SW   = ON;
			SCHED_SW = ON;

			//Otherwise, CPU is already busy, so
		} else {
				//Turn off both switches
			CPU_SW   = OFF;
			SCHED_SW = OFF;
		}

	}

}

/**
	Remove all I/O request blocks from the PCB's list that have completed.

	<b> Important Notes:</b>
	\li Request blocks that have been processed will have the status DONE_RB.
	\li The request block list is circularly linked.

	<b> Algorithm: </b>
	\verbatim
	If list is not empty
		Traverse pcb's list of request blocks removing all finished rbs:
			Start at second node in the list and end when wraps around to first
				Save previous node and current node

				If rb is done
					Remove node from middle of list

		Handle special removal cases:
			If first rb in list is done
				If deleting last remaining node
					Delete list
				Remove node from front of list
				Change head of list
	\endverbatim

  @param pcb -- process from which to remove completed I/O request blocks
 */
void
Purge_rb( pcb_type* pcb )
{

	// TODO: free( rb ) in this function as well?

	// DECLARE VARIABLES
	struct rb_list* curr;
	struct rb_list* last;

	// if the list of request blocks is empty, there is nothing to traverse
	if( pcb->rb_q == NULL ){
		return;
	}

		//Traverse pcb's list of request blocks removing all finished rbs:

			//Start at second node in the list and end when wraps around to first
	last = pcb->rb_q;
	curr = pcb->rb_q->next;

	// loop until we've reached the head of the circularly linked list again
	while( curr != pcb->rb_q ){

				//If rb is done
		if( curr->rb->status == DONE_RB ){
					//Remove node from middle of list
			last->next = curr->next;
			curr->next->prev = last;
			free( curr );

			// iterate to the next node (last stays the same!)
			curr = last->next;

		} else {

			// iterate to the next node
			last = curr;
			curr = curr->next;

		}

	}

		//Handle special removal cases:
	curr = pcb->rb_q;
			//If first rb in list is done
	if( curr->rb->status == DONE_RB ){

				//If deleting last remaining node
		if( pcb->rb_q->next == pcb->rb_q ){
					//Delete list
			free( curr );
			curr = NULL;
			pcb->rb_q = NULL;

		} else {
				//Remove node from front of list
			pcb->rb_q->next->prev = pcb->rb_q->prev;
			pcb->rb_q->prev->next = pcb->rb_q->next;

			//Change head of list
			pcb->rb_q = pcb->rb_q->next;

			free( curr );
			curr = NULL;

		}

	}

}

/**
	Prepare the scheduled program for execution and transfer control to it.

	<b> Important Notes:</b>
	\li As a result of this function, the next active program, CPU.active_pcb,
	is loaded into memory and executed. The program will run until either it
	issues an end event to exit the program or an I/O instruction is issued.

	<b> Algorithm: </b>
	\verbatim
	Initialize and load the lower half of the kernel segment table with the segment table of the current user (located at CPU.active_pcb)--call Load_Map()

	Execute the program associated with the saved CPU state of the CPU's active process--call Exec_Program()
	\endverbatim

	@retval None
 */
void
Dispatcher( )
{

	//Initialize and load the lower half of the kernel segment table with the segment table of the current user (located at CPU.active_pcb)--call Load_Map()
	Load_Map( CPU.active_pcb->seg_table, CPU.active_pcb->num_segments );

	//Execute the program associated with the saved CPU state of the CPU's active process--call Exec_Program()
	Exec_Program( &( CPU.active_pcb->cpu_save ) );

} 

/**
	Load the dispatched program's segment table into the kernel's segment table.

	<b> Important Notes:</b>
	\li Recall that the kernel's segment table is represented by the global
	variable Mem_Map.
	\li The first half of Mem_Map is used for user programs (the second half is
	reserved for kernel processes, such as the boot program).

	<b> Algorithm: </b>
	\verbatim
	For each user segment in the kernel's segment table
		Reset segment's access mode to kernel mode so that any attempted access will result in segmentation fault (in simulated program)
	For each segment in user's program
		Copy segment into user segment in kernel's segment table
	\endverbatim

	@retval None
 */
void
Load_Map( segment_type *seg_tab, int num_segments )
{

	// TODO: revisit discrepancy

	//For each user segment in the kernel's segment table
	for( int i=0; i < Max_Segments; i++ ){
		//Reset segment's access mode to kernel mode so that any attempted access will result in segmentation fault (in simulated program)
		Mem_Map[i].access = 0;
	}

	//For each segment in user's program
	for( int i=0; i < Max_Segments; i++ ){
		//Copy segment into user segment in kernel's segment table
		Mem_Map[i].access = seg_tab[i].access;
		Mem_Map[i].size = seg_tab[i].size;
		Mem_Map[i].base = seg_tab[i].base;
	}

}

/**
	Print debugging message about pcb.rb_q.

	<b> Important Notes:</b>
	\li The debugging message is only printed if the variable DEBUG_RBLIST is
	turned on (set to 1). This is done by adding the following line to
	config.dat: <br>
	DEBUG_RBLIST= ON
	\li The function is not necessary to implement as it serves only to help
	the programmer in debugging the implementation of common data structures.

	@retval None
 */
void
Dump_rblist( pcb_type* pcb )
{
	// if debug flag is turned on
	if( DEBUG_RBLIST )
	{

		// DECLARE VARIABLES
		struct rb_list* curr;
		int i;

		curr = pcb->rb_q;

		if( curr == NULL ){
			printf( "DEBUG: pcb->rb_q is NULL\n" );
			return;
		}

		// iterate through each node in the rb_list
		i = 0;
		do{

			//printf( "DEBUG: pcb->rb_q node #%d, rT:|%d| pT:|%d| id:|%d| b:|%d|\n",
			 //i, curr->rb->status, curr->rb->pcb->status, curr->rb->dev_id,
			 //curr->rb->bytes
			//);
			printf( "DEBUG: pcb->rb_q node #%d s:|%d| o:|%d|\n",
			 i, curr->rb->req_id.segment, curr->rb->req_id.offset
			);

			// iterate to the next node in the rb_list
			curr = curr->next;
			i++;

		} while( curr != NULL && curr != pcb->rb_q );
	}
	else // do not print any message
	{ ; }
}

/**
	Print debugging message about device.wait_q.

	<b> Important Notes:</b>
	\li The debugging message is only printed if the variable DEBUG_DEVQ is
	turned on (set to 1). This is done by adding the following line to
	config.dat: <br>
	DEBUG_DEVQ= ON
	\li The function is not necessary to implement as it serves only to help
	the programmer in debugging the implementation of common data structures.

	@retval None
 */
void
Dump_devq( )
{
	// if debug flag is turned on
	if( DEBUG_DEVQ )
	{
		/* print debugging message */
	}
	else // do not print any message
	{ ; }
}

/**
	Print debugging message about CPU.ready_q.

	<b> Important Notes:</b>
	\li The debugging message is only printed if the variable DEBUG_CPUQ is
	turned on (set to 1). This is done by adding the following line to
	config.dat: <br>
	DEBUG_CPUQ= ON
	\li The function is not necessary to implement as it serves only to help
	the programmer in debugging the implementation of common data structures.

	@retval None
 */
void
Dump_cpuq( )
{
	// if debug flag is turned on
	if( DEBUG_CPUQ )
	{
		// DECLARE VARIABLES
		struct pcb_list* curr;
		int i;

		curr = CPU.ready_q;

		if( curr == NULL ){
			printf( "DEBUG: CPU.ready_q is NULL\n" );
			return;
		}

		// iterate through each node in the rb_list
		i = 0;
		do{

			printf( "DEBUG: CPU.ready_q node #%d term_pos:|%d|\n",
			 i, curr->pcb->term_pos
			);

			// iterate to the next node in the rb_list
			curr = curr->next;
			i++;

		} while( curr != NULL && curr != CPU.ready_q );
	}
	else // do not print any message
	{ ; }
}
