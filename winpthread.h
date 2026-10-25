/* 
			pthread simple wrapper 2012-2019
			Edouard  BERGE             v0.04

changes v004:
- bugfix trylock

changes v003:
- win64 compliant

changes v002:
- pthread_mutex_init
- pthread_mutex_trylock now really tries
- pthread_create now can detached a thread if requested
			
initial release v001:
- pthread_mutex_lock, pthread_mutex_trylock, pthread_mutex_unlock
- pthread_attr_init, pthread_attr_destroy
- pthread_attr_setstacksize, pthread_attr_getstacksize
- pthread_attr_setdetachstate, pthread_attr_getdetachstate
- pthread_create, pthread_join, pthread_exit

*/

#define pthread_t       HANDLE
#define pthread_mutex_t HANDLE

#include<process.h>

enum e_detached_state {
PTHREAD_CREATE_DETACHED,
PTHREAD_CREATE_JOINABLE
};

struct pthread_attr_t {
	int	stack_size;
	enum e_detached_state detached_state;
};
typedef struct pthread_attr_t pthread_attr_t;

int pthread_mutex_init(pthread_mutex_t *mutex)
{
	HANDLE createdmutex;
	char mutex_addr[32 + 1] = { 0 };
	/* using hex address of mutex as string id */
	snprintf(mutex_addr, 32, "%LX", mutex);
	createdmutex=CreateMutex(NULL,FALSE,mutex_addr);
	*mutex=createdmutex;
	if (*mutex==NULL) {
		printf("impossible d'initialiser le mutex\n");
		return 1;
	} else return 0;
}
int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	pthread_t zehand;
	char mutex_addr[32+1]={0};
	snprintf(mutex_addr,32,"%LX",mutex);
	while ((zehand=OpenMutex(MUTEX_ALL_ACCESS,TRUE,mutex_addr))==NULL) {
		WaitForSingleObject(*mutex,INFINITE);
	}
	return 0;
}
int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
	pthread_mutex_t local_mutex;
	char mutex_addr[32+1];
	snprintf(mutex_addr, 32, "%LX", mutex);
	local_mutex=OpenMutex(MUTEX_ALL_ACCESS,FALSE,mutex_addr);
	if (local_mutex==NULL) return 1; else return 0;
}
int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	ReleaseMutex(*mutex);
	return 0;
}


int pthread_attr_init(pthread_attr_t *attr)
{
	attr->stack_size=1024*1024;
	attr->detached_state=PTHREAD_CREATE_JOINABLE;
	return 0;
}
int pthread_attr_destroy(pthread_attr_t *attr)
{
	return 0;
}
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize)
{
	attr->stack_size=stacksize;
	return 0;
}
int pthread_attr_getstacksize(pthread_attr_t *attr, size_t *stacksize)
{
	*stacksize=attr->stack_size;
	return 0;
}
int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate)
{
	if (detachstate!=PTHREAD_CREATE_DETACHED && detachstate!=PTHREAD_CREATE_JOINABLE) return 1;
	attr->detached_state=detachstate;
	return 0;
}
int pthread_attr_getdetachstate(pthread_attr_t *attr, int *detachstate)
{
	*detachstate=attr->detached_state;
	return 0;
}
int pthread_create(pthread_t *thread, pthread_attr_t *attr,void *(*start_routine) (void *), void *arg)
{
	unsigned int stack_size=1024*1024;
	int detached_state=PTHREAD_CREATE_JOINABLE;
	void (*bounce)(void *);
	
	if (attr) {
		stack_size=attr->stack_size;
		detached_state=attr->detached_state;
	}
	
	bounce=(void (__cdecl *)(void *))start_routine;
	*thread=(pthread_t)_beginthread(bounce,stack_size,arg);

	if (detached_state==PTHREAD_CREATE_DETACHED) {
		CloseHandle(*thread);
		_ReadWriteBarrier();
		*thread=0;
	}
	
	return 0;
}

int pthread_join(pthread_t thread, void **retval)
{
	DWORD exitcode;
	DWORD *retptr;
	WaitForSingleObject(thread,INFINITE);
	GetExitCodeThread(thread,&exitcode);
	CloseHandle(thread);
	retptr=(DWORD *)*retval;
	if (retptr) *retptr=exitcode;
	return 0;
}

void pthread_exit(void *retval)
{
	DWORD *retptr;
	retptr=(DWORD *)retval;
	if (retptr) ExitThread(*retptr); else ExitThread(0);
}

/*EOF*/