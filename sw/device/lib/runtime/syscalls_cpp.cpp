/*
* Copyright 2024 Anestis Athanasiadis
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*     http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/


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