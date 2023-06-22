#ifndef _LOCKWRAPPER_H_
#define _LOCKWRAPPER_H_
/*
#ifndef PTHREAD_MUTEX_INITIALIZER
# define PTHREAD_MUTEX_INITIALIZER   { { 0, 0, 0, 0, 0, 0, { 0, 0 } } }
#endif
*/
#ifdef _WIN32
	#include <windows.h>
	typedef CRITICAL_SECTION cs_lock_t;

	inline void init_lock(cs_lock_t* lock)
	{
		::InitializeCriticalSection(lock);
	}

	inline void cs_lock(cs_lock_t* lock)
	{
		 ::EnterCriticalSection(lock);
	}

	inline void cs_unlock(cs_lock_t* lock)
	{
		 ::LeaveCriticalSection(lock);
	}

	inline void destroy_lock(cs_lock_t* lock)
	{
		::DeleteCriticalSection(lock);
	}
#else
	#include <pthread.h>

	typedef pthread_mutex_t cs_lock_t;

	inline void init_lock(cs_lock_t* lock)
	{
//		*lock = PTHREAD_MUTEX_INITIALIZER;
		pthread_mutex_init ( lock, NULL);
	}

	inline void cs_lock(cs_lock_t* lock)
	{
		 pthread_mutex_lock(lock);
	}

	inline void cs_unlock(cs_lock_t* lock)
	{
		 pthread_mutex_unlock(lock);
	}

	inline void destroy_lock(cs_lock_t* lock)
	{

	}
#endif

#endif

