---
layout: post
title:  "D style unit testing for C++"
---

I'm somewhat fond of D but tend to use C++ more in practice. However, [D's inbuilt
unit testing](http://dlang.org/unittest.html) is something I've longed for in C++.
No complex xUnit frameworks, just a simple `unittest { }` block that you can put 
below a function to test it. When compiled with a flag (in dmd's case it's `-unittest`),
the unit tests are included with the binary and run before the main function.

So, I decided to try some preprocessor hackery to make it work.

## Motivations

One thing that I dislike about C++ unit testing frameworks is how annoying it can
be to integrate with them. It's not particuarly *hard* and for large-scale projects
it's probably worth the time investment. However for something small I just can't
be bothered compiling, linking and then integrating a framework. For example,
once you've compiled and linked with Google's testing framework, integration is as simple
as [starting from a 50 line bolierplate!](https://code.google.com/p/googletest/wiki/Primer#Writing_the_main(\)_Function)

What I want is something that needs no compilation, no linking with my project and no
integration (well, aside from `#include "testing.h"` in .cpp files). I want something
that's as true to D as possible.

## Outcome

I managed to end up getting exactly what I want. Once I include the [testing.h header](../assets/testing.h)
in a C++ source file, I can write tests in the exact same way as I would in D. For asserting
I just use `assert(expr)` from \<cassert\>. Here's an example of testing.h in action:

{% highlight c++ %}
{% raw %}
// demo.cc
#include <iostream>
#include "testing.h"

unittest {
    assert(1 + 1 == 2); // this will obviously pass
}

unittest {
    assert(1 + 1 == 3); // this won't
}

int main(int argc, const char** argv) {
    std::cout << "Hello World!" << std::endl;
    return 0;
}
{% endraw %}
{% endhighlight %}

Compilation can be done as per usual. Like dmd, unit tests aren't included/run
unless a flag is passed. In this case, testing is controlled by the UNIT_TESTS
define. So, regular compilation will produce this:

    bash-3.2$ clang++ demo.cc && ./a.out
    Hello World!
    bash-3.2$ 

While compiling with unit tests enabled causes main to never get reached as there's
an assertion failure in one of the tests:

    bash-3.2$ clang++ -DUNIT_TESTS demo.cc && ./a.out
    Assertion failed: (1 + 1 == 3), function test_main, file demo.cc, line 9.
    Abort trap: 6
    bash-3.2$ 

