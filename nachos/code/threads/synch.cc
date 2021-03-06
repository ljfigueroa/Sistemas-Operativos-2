// synch.cc
//      Routines for synchronizing threads.  Three kinds of
//      synchronization routines are defined here: semaphores, locks
//      and condition variables (the implementation of the last two
//      are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
//      Initialize a semaphore, so that it can be used for synchronization.
//
//      "debugName" is an arbitrary name, useful for debugging.
//      "initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(const char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List<Thread*>;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
//      De-allocate semaphore, when no longer needed.  Assume no one
//      is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
//      Wait until semaphore value > 0, then decrement.  Checking the
//      value and decrementing must be done atomically, so we
//      need to disable interrupts before checking the value.
//
//      Note that Thread::Sleep assumes that interrupts are disabled
//      when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts

    while (value == 0) {                        // semaphore not available
        queue->Append(currentThread);           // so go to sleep
        currentThread->Sleep();
    }
    value--;                                    // semaphore available,
                                                // consume its value

    interrupt->SetLevel(oldLevel);              // re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
//      Increment semaphore value, waking up a waiter if necessary.
//      As with P(), this operation must be atomic, so we need to disable
//      interrupts.  Scheduler::ReadyToRun() assumes that threads
//      are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = queue->Remove();
    if (thread != NULL)    // make thread ready, consuming the V immediately
        scheduler->ReadyToRun(thread);
    value++;
    interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments
// Note -- without a correct implementation of Condition::Wait(),
// the test case in the network assignment won't work!
Lock::Lock(const char* debugName) {

    sem  = new Semaphore(debugName,1);
    name = debugName;
    held_by = NULL;
}

bool Lock::isHeldByCurrentThread() {
    return held_by == currentThread;
}

Lock::~Lock() {
    delete sem;
}

void Lock::Acquire() {

    ASSERT(!isHeldByCurrentThread());

    sem->P();
    held_by = currentThread;
}

void Lock::Release() {
    ASSERT(isHeldByCurrentThread());
    held_by = NULL;
    sem->V();
}

Condition::Condition(const char* debugName, Lock* conditionLock) {
    name = debugName;
    lock = conditionLock;
}

Condition::~Condition() { }

void Condition::Wait() {

    ASSERT(lock->isHeldByCurrentThread());

    Semaphore* sem = new Semaphore(name, 0);
    sem_list.Append(sem);
    lock->Release();

    DEBUG('s',"CONDITION: %s is waiting.\n",name);
    sem->P();
    DEBUG('s',"CONDITION: %s is fulfield.\n",name);

    delete sem;
    lock->Acquire();

}
void Condition::Signal() {

    ASSERT(lock->isHeldByCurrentThread());

    Semaphore* tmp;
    lock->Release();
    if (!sem_list.IsEmpty()) {
        tmp = sem_list.Remove();
        DEBUG('s',"CONDITION: %s is signaling.\n",name);
        tmp->V();
    }
    lock->Acquire();

}
void Condition::Broadcast() {

    ASSERT(lock->isHeldByCurrentThread());

    Semaphore* tmp;
    lock->Release();
    while(!sem_list.IsEmpty()) {
        DEBUG('s',"CONDITION: %s is broadcasting.\n",name);
        tmp = sem_list.Remove();
        tmp->V();
    }
    lock->Acquire();
}

Port::Port(const char* debugName)
{
    name = debugName;
    lock = new Lock("Port lock");
    emptyBuffer = new Condition("Empty buffer",lock);
    consumeBuffer = new Condition("Consume buffer",lock);
    buffer = new List<int>();
}

Port::~Port()
{
    delete lock;
    delete emptyBuffer;
    delete consumeBuffer;
    delete buffer;
}

void Port::Send(int message)
{
    lock->Acquire();
    buffer->Append(message);
    emptyBuffer->Signal();
    DEBUG('s',"PORT: %s sended %d.\n",name,message);
    consumeBuffer->Wait();
    lock->Release();
}

void Port::Receive(int* message)
{
    lock->Acquire();
    while (buffer->IsEmpty()) {
        DEBUG('s',"PORT: %s has an empty buffer.\n",name);
        emptyBuffer->Wait();
    }
    *message = buffer->Remove();
    DEBUG('s',"PORT: %s consumed %d.\n",name,*message);
    consumeBuffer->Signal();
    lock->Release();
}
