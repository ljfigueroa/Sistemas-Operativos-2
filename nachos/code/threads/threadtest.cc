// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create several threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustrate the inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//
// Parts from Copyright (c) 2007-2009 Universidad de Las Palmas de Gran Canaria
//

#include "copyright.h"
#include "system.h"
#include "synch.h"

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 10 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"name" points to a string with a thread name, just for
//      debugging purposes.
//----------------------------------------------------------------------

#ifdef SEMAPHORE_TEST
#define SEMAPHORE_INITIAL_VALUE 3
static Semaphore sem = Semaphore("ej14",3);
#endif

void
SimpleThread(void* name)
{
    // Reinterpret arg "name" as a string
    char* threadName = (char*)name;
    
#ifdef SEMAPHORE_TEST
    sem.P();
    DEBUG('s',"%s did P()\n",threadName);
#endif
    
    // If the lines dealing with interrupts are commented,
    // the code will behave incorrectly, because
    // printf execution may cause race conditions.
    for (int num = 0; num < 10; num++) {
        //IntStatus oldLevel = interrupt->SetLevel(IntOff);
	printf("*** thread %s looped %d times\n", threadName, num);
	//interrupt->SetLevel(oldLevel);
        currentThread->Yield();
    }
    //IntStatus oldLevel = interrupt->SetLevel(IntOff);
    printf(">>> Thread %s has finished\n", threadName);
    //interrupt->SetLevel(oldLevel);

#ifdef SEMAPHORE_TEST
    DEBUG('s',"%s is going to do V()\n",threadName);
    sem.V();
#endif
    
}

//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between several threads, by launching
//	ten threads which call SimpleThread, and finally calling 
//	SimpleThread ourselves.
//----------------------------------------------------------------------

#define THREAD_NUM 5

void
ThreadTest()
{
    DEBUG('t', "Entering SimpleTest");

    for(int thread = 1; thread < THREAD_NUM; thread++) {
	char *threadname = new char[128];
	sprintf(threadname,"Thread %d",thread);
	Thread* newThread = new Thread (threadname);
	newThread->Fork (SimpleThread, (void*)threadname);
    }
    
    SimpleThread( (void*)"Thread 0");
	    
}
