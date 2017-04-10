
#THREAD_NUM 5
void condicionTest()
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

