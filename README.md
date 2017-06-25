# Generators

This is a C implementation of generators. Generators are a nice way of defining a range of values. Take the following python snippet:
```python
def fib(max):
    a, b = 0, 1
    while a < max:
        yield a
        a, b = b, a + b
gen = fib(10)
```
This will generate numbers from the Fibonacci sequence each time next(fib) is called. C doesn't have native support for generators, they are pretty straightforward to implement. The idea is to save the stack in the heap between calls to next() and restore it before the next call to next(), this can be done with malloc and memcpy. Then we also have to restore registers, the code pointer and other low-level, asm-y details but thankfully setjmp and longjmp take care of that for us.

This code is architecture-independent, probably platform independent too and should compile fine with any compiler.

