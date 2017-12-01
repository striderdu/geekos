/*
 * GeekOS C code entry point
 * Copyright (c) 2001,2003,2004 David H. Hovemeyer <daveho@cs.umd.edu>
 * Copyright (c) 2003, Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 * Copyright (c) 2004, Iulian Neamtiu <neamtiu@cs.umd.edu>
 * $Revision: 1.51 $
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/bootinfo.h>
#include <geekos/string.h>
#include <geekos/screen.h>
#include <geekos/mem.h>
#include <geekos/crc32.h>
#include <geekos/tss.h>
#include <geekos/int.h>
#include <geekos/kthread.h>
#include <geekos/trap.h>
#include <geekos/timer.h>
#include <geekos/keyboard.h>

void print_key(ulong_t arg)
{
    Keycode keycode;
    int flag = 0;
    while(1)
    {
        keycode = Wait_For_Key();
        if( keycode == 276 || ! ((keycode & KEY_SPECIAL_FLAG) || (keycode & KEY_RELEASE_FLAG)) )
        {
            int ascii = keycode & 0xff;
            //Print("%d\n", ascii);
            if( (keycode & KEY_CTRL_FLAG) && ascii == 'd')
            {
                Print("\nReceived ctrl+d, input is terminated!\n");
                break;
            }
            else if(keycode == 276)
            {
                if(!flag) Print("\nEnabled caps lock!\n");
                else Print("\nDisabled caps lock!\n");
                flag=!flag;
            }
            else
            {
                Print("%c", ascii=='\r' ? '\n' : ascii-32*flag);
            }
        }
        /*if( keycode == ('d'|KEY_CTRL_FLAG))
        {
            Print("\nReceived ctrl+d, input is terminated!\n");
            break;
        }
        else if(keycode == 276)
        {
            if(!flag) Print("\nEnabled caps lock!\n");
            else Print("\nDisabled caps lock!\n");
            flag=!flag;
        }
        else
        {
            Print("%c", keycode=='\r' ? '\n' : keycode-32*flag);
        }*/
    }
}

static void print_A(ulong_t arg)
{
    while(1)
    {
        Print("A");
    }
}

static void print_B(ulong_t arg)
{
    while(1)
    {
        Print("B");
    }
}
/*
 * Kernel C code entry point.
 * Initializes kernel subsystems, mounts filesystems,
 * and spawns init process.
 */
void Main(struct Boot_Info* bootInfo)
{
    Init_BSS();
    Init_Screen();
    Init_Mem(bootInfo);
    Init_CRC32();
    Init_TSS();
    Init_Interrupts();
    Init_Scheduler();
    Init_Traps();
    Init_Timer();
    Init_Keyboard();


    Set_Current_Attr(ATTRIB(BLACK, GREEN|BRIGHT));
    Print("Welcome to GeekOS!\n");
    Set_Current_Attr(ATTRIB(BLACK, GRAY));


    //TODO("Start a kernel thread to echo pressed keys and print counts");
    Start_Kernel_Thread(print_key, 0, PRIORITY_NORMAL, true);
    Start_Kernel_Thread(print_A, 0, PRIORITY_NORMAL, true);
    Start_Kernel_Thread(print_B, 0, PRIORITY_NORMAL, true);


    /* Now this thread is done. */
    Exit(0);
}









