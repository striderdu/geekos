/*
 * Common user mode functions
 * Copyright (c) 2001,2003,2004 David H. Hovemeyer <daveho@cs.umd.edu>
 * $Revision: 1.50 $
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/errno.h>
#include <geekos/ktypes.h>
#include <geekos/kassert.h>
#include <geekos/int.h>
#include <geekos/mem.h>
#include <geekos/malloc.h>
#include <geekos/kthread.h>
#include <geekos/vfs.h>
#include <geekos/tss.h>
#include <geekos/user.h>

/*
 * This module contains common functions for implementation of user
 * mode processes.
 */

/*
 * Associate the given user context with a kernel thread.
 * This makes the thread a user process.
 */
void Attach_User_Context(struct Kernel_Thread* kthread, struct User_Context* context)
{
    KASSERT(context != 0);
    kthread->userContext = context;

    Disable_Interrupts();

    /*
     * We don't actually allow multiple threads
     * to share a user context (yet)
     */
    KASSERT(context->refCount == 0);

    ++context->refCount;
    Enable_Interrupts();
}

/*
 * If the given thread has a user context, detach it
 * and destroy it.  This is called when a thread is
 * being destroyed.
 */
void Detach_User_Context(struct Kernel_Thread* kthread)
{
    struct User_Context* old = kthread->userContext;

    kthread->userContext = 0;

    if (old != 0) {
	int refCount;

	Disable_Interrupts();
        --old->refCount;
	refCount = old->refCount;
	Enable_Interrupts();

	/*Print("User context refcount == %d\n", refCount);*/
        if (refCount == 0)
            Destroy_User_Context(old);
    }
}

/*
 * Spawn a user process.
 * Params:
 *   program - the full path of the program executable file
 *   command - the command, including name of program and arguments
 *   pThread - reference to Kernel_Thread pointer where a pointer to
 *     the newly created user mode thread (process) should be
 *     stored
 * Returns:
 *   The process id (pid) of the new process, or an error code
 *   if the process couldn't be created.  Note that this function
 *   should return ENOTFOUND if the reason for failure is that
 *   the executable file doesn't exist.
 */
int Spawn(const char *program, const char *command, struct Kernel_Thread **pThread)
{
    /*
     * Hints:
     * - Call Read_Fully() to load the entire executable into a memory buffer
     * - Call Parse_ELF_Executable() to verify that the executable is
     *   valid, and to populate an Exe_Format data structure describing
     *   how the executable should be loaded
     * - Call Load_User_Program() to create a User_Context with the loaded
     *   program
     * - Call Start_User_Thread() with the new User_Context
     *
     * If all goes well, store the pointer to the new thread in
     * pThread and return 0.  Otherwise, return an error code.
     */
    int rc;
    char *exeFileData=0;
    ulong_t exeFileLength;
    struct User_Context *userContext=0;
    struct Kernel_Thread *process=0;
    struct Exe_Format Exe_Format;
    if((result=Read_Fully(program,(void **) &exeFileData, &exeFileLength)) != 0)
    {
        Print("Failed to read file %s\n", program);
        goto fail;
    }
    if((result=Parse_ELF_Executable(exeFileData, exeFileLength, &exeFormat)) != 0)
    {
        Print("Failed to parse ELF file \n");
        goto fail;
    }
    if((result=Load_User_Program(exeFileData, exeFileLength, &exeFormat, command,&UserContext)) != 0)
    {
        Print("Failed to load user program\n");
        goto fail;
    }
    Free(exeFileData);
    exeFileData=0;
    process=Start_User_Thread(userContext,false);
    if (process != 0)
    {
        KASSERT(process->refCount==2);
        *pThread=process;
    }
    else
    {
        rc=ENOMEM;
    }
    return rc;

fail:
    if(exeFileData!=0) Free(exeFileData);
    if(userContext!=0) Destroy_User_Context(userContext);
    return rc;
    //TODO("Spawn a process by reading an executable from a filesystem");
}

/*
 * If the given thread has a User_Context,
 * switch to its memory space.
 *
 * Params:
 *   kthread - the thread that is about to execute
 *   state - saved processor registers describing the state when
 *      the thread was interrupted
 */
void Switch_To_User_Context(struct Kernel_Thread* kthread, struct Interrupt_State* state)
{
    /*
     * Hint: Before executing in user mode, you will need to call
     * the Set_Kernel_Stack_Pointer() and Switch_To_Address_Space()
     * functions.
     */
    static struct User_Context* s_currentUserContext;
    extern int userDebug;
    struct User_Context* userContext=kthread->userContext;
    KASSERT(!Interrupts_Enabled());
    if(userContext==0)
    {
        return ;
    }
    if(userContext!=s_currentUserContext)
    {
        ulong_t esp0;
        if(userDebug) Print("A[%p]\n", kthread);
        Switch_To_Address_Space(userContext);
        esp0=((ulong_t)kthread->stackPage)+PAGE_SIZE;
        if(userDebug) Print("s[%lx]\n", esp0);
        Set_Kernel_Stack_Pointer(esp0);
        s_currentUserContext=userContext;
    }
    /*if(kthread->userContext==NULL)
    {
        return ;
    }
    TODO("Switch to a new user address space, if necessary");*/
}

