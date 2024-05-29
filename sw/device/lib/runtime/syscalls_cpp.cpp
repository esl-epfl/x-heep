// This function is required to register the call for the destructor
// of static object
extern "C" int __aeabi_atexit(void *object, void (*destructor)(void *),void *dso_handle)
{
    static_cast<void>(object);
    static_cast<void>(destructor);
    static_cast<void>(dso_handle);
    return 0;
}

void operator delete(void *,unsigned int)
{
}

// Required when there is pure virtual function
extern "C" void __cxa_pure_virtual()
{
    while (true) {}
}


void *__dso_handle = 0;