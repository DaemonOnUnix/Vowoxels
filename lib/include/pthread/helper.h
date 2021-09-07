#ifndef PTHREAD_HELPER
#define PTHREAD_HELPER

#define DETACHED_THREAD(NAME, FUNC, PARAM) {pthread_t NAME; pthread_create(&NAME, (void*)0, FUNC, &PARAM); (void)(pthread_detach(NAME)); }

#define PTHREAD_PROTO(FUNC_NAME) void* FUNC_NAME(void* __pthread_parameter)
#define PTHREAD_GET_PARAMETERS(T) (*((T*)__pthread_parameter))

#endif