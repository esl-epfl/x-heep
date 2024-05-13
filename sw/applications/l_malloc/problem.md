So this simple program using dynamic memory allocation does give the following result:
```bash
Allocated memory range: [0xd590 - 0xd5a3]
allocate arrays
Allocated memory range: [0xd218 - 0xd23f]
Allocated memory range: [0xd218 - 0xd22b]
Allocated memory range: [0xd218 - 0xd253]
```
So after the second allocation all the new arrays are allocated at the same address which then obviously leads to memory corruption

If I coment out the call to the first function I get this output:
```bash
allocate arrays
Allocated memory range: [0xd590 - 0xd5b7]
Allocated memory range: [0xd218 - 0xd22b]
Allocated memory range: [0xd218 - 0xd253]
```
The issue persists...

I don't really know how this could occur and how to fix it.