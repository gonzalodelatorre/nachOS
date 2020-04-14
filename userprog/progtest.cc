// progtest.cc
// Test routines for demonstrating that Nachos can load a user program and execute it.
//
// Also, routines for testing the Console hardware device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation of liability
// and disclaimer of warranty provisions.
//----------------------------------------------------------------------------------------
// Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------


#include "copyright.h"
#include "system.h"
#include "console.h"
#include "addrspace.h"
#include "synch.h"
#include "syscall.h"


//----------------------------------------------------------------------------------------
// StartProcess
// Run a user program. Open the executable, load it into memory, and jump to it.
//----------------------------------------------------------------------------------------

void StartProcess(const char *filename)
{
	OpenFile *executable = fileSystem->Open(filename);
	AddrSpace *space;

	if (executable == NULL)
	{
		printf("[%s]: Unable to open file %s.\n", __FUNCTION__, filename);
		return;
    }

	space = new AddrSpace(executable);

	if (space == NULL)
	{
		printf("[%s]: Unable to create the address space!\n", __FUNCTION__);
		return;
	}

	currentThread->space = space;

#if !defined(DEMAND_LOADING) && !defined(PAGINATION)
	// Close the executable file.
	delete executable;
#endif

	// Add thread to the process table.

	SpaceId id = processTable->addProcess(currentThread); //TODO

	if (id < 0)
	{
		printf("[%s]: Unable to attach the thread into process table!\n", __FUNCTION__);
		delete space;
		return;
	}

	// Set the initial register values. And load page table register.

	space->InitRegisters();
	space->RestoreState();

	// Jump to the user program, machine->Run never returns; the address space
	// exits by doing the syscall "exit".

	machine->Run();
	ASSERT(false);
}

// Data structures needed for the console test. Threads making I/O requests wait on
// a Semaphore to delay until the I/O completes.

static Console *console;
static Semaphore *readAvail;
static Semaphore *writeDone;

//----------------------------------------------------------------------------------------
// ConsoleInterruptHandlers
// Wake up the thread that requested the I/O.
//----------------------------------------------------------------------------------------

static void ReadAvail(void* arg) { readAvail->V(); }
static void WriteDone(void* arg) { writeDone->V(); }

//----------------------------------------------------------------------------------------
// ConsoleTest
// Test the console by echoing characters typed at the input onto the output.
// Stop when the user types a 'q'.
//----------------------------------------------------------------------------------------

void ConsoleTest (const char *in, const char *out)
{
	char ch;

	console = new Console(in, out, ReadAvail, WriteDone, 0);
	readAvail = new Semaphore("read avail", 0);
	writeDone = new Semaphore("write done", 0);

	for (;;)
	{
		readAvail->P();				// Wait for character to arrive.
		ch = console->GetChar();
		console->PutChar(ch);		// Echo it!
		writeDone->P();				// Wait for write to finish.
		if (ch == 'q') return;		// If q, quit.
	}

	delete console;
	delete readAvail;
	delete writeDone;
}

