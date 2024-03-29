/*
History
=======


Notes
=======

*/

#include "../clib/stl_inc.h"
#include "../clib/logfile.h"
#include "../clib/passert.h"
#include "../clib/strexcpt.h"
#include "../clib/threadhelp.h"
#include "../clib/tracebuf.h"

#include "checkpnt.h"
#include "polsem.h"

#ifdef _WIN32
DWORD locker;
void polsem_lock()
{
    DWORD tid = GetCurrentThreadId();
    EnterCriticalSection( &cs );
    passert_always( locker == 0 );
    locker = tid;
	//std::cout << "LK: " << tid << " ";
}

void polsem_unlock()
{
    DWORD tid = GetCurrentThreadId();
    passert_always( locker == tid );
	//std::cout << "UL: " << tid << " ";
    locker = 0;
    LeaveCriticalSection( &cs );
}
#else
pid_t locker;
void polsem_lock()
{
    pid_t pid = getpid();
    int res = pthread_mutex_lock( &polsem );
    if (res != 0 || locker != 0)
    {
        Log( "pthread_mutex_lock: res=%d, pid=%d, locker=%d\n", res, pid, locker );
    }
    passert_always( res == 0 );
    passert_always( locker == 0 );
    locker = pid;
}
void polsem_unlock()
{
    pid_t pid = getpid();
    passert_always( locker == pid );
    locker = 0;
    int res = pthread_mutex_unlock( &polsem );
    if (res != 0)
    {
        Log( "pthread_mutex_unlock: res=%d,pid=%d", res, pid );
    }
    passert_always( res == 0 );
}

#endif



//static void (*tmp_thread_func1)(void);
//static void (*tmp_thread_func2)(void*);
//static const char* tmp_thread_name;
//static void* tmp_thread_arg;

#ifdef _WIN32
#include <process.h>
CRITICAL_SECTION cs;
HANDLE hEvPulse;

HANDLE hEvTasksThread;

CRITICAL_SECTION csThread;
HANDLE hSemThread;

void init_ipc_vars()
{
    InitializeCriticalSection( &cs );
    hEvPulse = CreateEvent( NULL, TRUE, FALSE, NULL );

    hEvTasksThread = CreateEvent( NULL, FALSE, FALSE, NULL );

    InitializeCriticalSection( &csThread );
    hSemThread = CreateSemaphore( NULL, 0, 1, NULL );
}

void deinit_ipc_vars()
{
    CloseHandle( hSemThread );
    DeleteCriticalSection( &csThread );

    CloseHandle( hEvTasksThread );
    hEvTasksThread = NULL;

    CloseHandle( hEvPulse );
    DeleteCriticalSection( &cs );
}
void send_pulse()
{
    TRACEBUF_ADDELEM( "Pulse", 1 );
    PulseEvent( hEvPulse );
}

void wait_for_pulse( unsigned long millis )
{
    WaitForSingleObject( hEvPulse, millis );
}

void wake_tasks_thread()
{
    SetEvent( hEvTasksThread );
}

void tasks_thread_sleep( unsigned long millis )
{
    WaitForSingleObject( hEvTasksThread, millis );
}
#else

#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

pthread_mutexattr_t polsem_attr;
pthread_mutex_t polsem;
//pthread_mutex_t polsem = PTHREAD_MUTEX_INITIALIZER;
// pthread_mutex_t polsem = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP; 

pthread_mutex_t pulse_mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t pulse_cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t task_pulse_mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t task_pulse_cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t threadstart_mut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t threadstart_pulse_mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t threadstart_pulse_cond = PTHREAD_COND_INITIALIZER;
bool thread_started;


pthread_mutex_t polsemdbg_mut = PTHREAD_MUTEX_INITIALIZER;

pthread_attr_t thread_attr;

void init_ipc_vars()
{
    int res;
    res = pthread_mutexattr_init( &polsem_attr );
    passert_always( res == 0 );
    
/*
    res = pthread_mutexattr_setkind_np( &polsem_attr, PTHREAD_MUTEX_ERRORCHECK_NP );
    passert_always( res == 0 );

    res = pthread_mutexattr_settype( &polsem_attr, PTHREAD_MUTEX_ERRORCHECK );
    passert_always( res == 0 );
*/
    
    res = pthread_mutex_init( &polsem, &polsem_attr );
    passert_always( res == 0 );

    pthread_attr_init( &thread_attr );
    pthread_attr_setdetachstate( &thread_attr, PTHREAD_CREATE_DETACHED );
}

void deinit_ipc_vars()
{
}

void send_pulse()
{
    pthread_mutex_lock( &pulse_mut );
    pthread_cond_broadcast( &pulse_cond );
    pthread_mutex_unlock( &pulse_mut );
}

void calc_abs_timeout( struct timespec* ptimeout, unsigned long millis )
{
    struct timeval now;
    struct timezone tz;
    
    gettimeofday(&now, &tz);
    long add_sec = 0;
    if (millis > 1000)
    {
        add_sec = millis / 1000;
        millis -= (add_sec * 1000);
    }
    ptimeout->tv_sec = now.tv_sec + add_sec;

    ptimeout->tv_nsec = now.tv_usec * 1000 + millis * 1000000L;
    if (ptimeout->tv_nsec >= 1000000000)
    {
        ++ptimeout->tv_sec;
        ptimeout->tv_nsec -= 1000000000;
    }
}

void wait_for_pulse( unsigned long millis )
{
    struct timespec timeout;
    
    pthread_mutex_lock(&pulse_mut);
   
    calc_abs_timeout( &timeout, millis );
    
    pthread_cond_timedwait(&pulse_cond, &pulse_mut, &timeout);

    pthread_mutex_unlock(&pulse_mut);
}

void wake_tasks_thread()
{
    pthread_mutex_lock( &task_pulse_mut );
    pthread_cond_broadcast( &task_pulse_cond );
    pthread_mutex_unlock( &task_pulse_mut );
}

void tasks_thread_sleep( unsigned long millis )
{
    struct timespec timeout;
    
    pthread_mutex_lock(&task_pulse_mut);
   
    calc_abs_timeout( &timeout, millis );
    
    pthread_cond_timedwait(&task_pulse_cond, &task_pulse_mut, &timeout);

    pthread_mutex_unlock(&task_pulse_mut);
}

#endif
