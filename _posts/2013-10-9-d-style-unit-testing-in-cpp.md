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
integration (well, aside from `#include "unittest.h"` in C++ source files). Something that
gives me D style unit testing in C++.

## Outcome/Example

[Download](/assets/unittest.h)  


I managed to end up getting exactly what I want. Before explaining what's going on and what the
(minor) limitations are, I'll give an example of how it's used. Once I include the
unittest.h header
in a C++ source file, I can write tests in the exact same way as I would in D. For asserting
I just use `assert(expr)` from \<cassert\>.

{% highlight c++ %}
{% raw %}
// demo.cc
#include <iostream>
#include "unittest.h"

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

Compilation is unchanged. Like dmd, unit tests aren't included/run
unless a flag is passed to the compiler. In this case, testing is controlled by the UNIT_TESTS
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

Your mileage may vary about what information is logged on assertion failures, but most
compilers should at least give a file and line number.

## Explanation

`unittest` is defined as a macro. When UNIT\_TESTS *isn't* defined by the compiler, the 
`unittest` macro simply expands to a function declaration that's the concatenation of
TEST and the current line in the file. The use of the line number is purely to keep names
unique. This expansion is as follows:

{% highlight c++ %}
{% raw %}
unittest { // this is line 35 of a source file
    // ... do a test
}

// after preprocessing with unit tests disabled
static void TEST35() {
    // ... do a test
}
{% endraw %}
{% endhighlight %}

As long as your code never mentions that particular name (given how odd it is the chances
are fairly slim), that code will never be run. In fact, most compilers will realise that
and strip the tests from the compiled binary, an optimization known as dead code elimination.
You can see this for yourself by comparing the filesizes of binaries compiled with and without
unit tests:

    bash-3.2$ clang++ -DUNIT_TESTS demo.cc -o demo_with_tests
    bash-3.2$ clang++ demo.cc -o demo_without_tests
    bash-3.2$ ls -l demo_*
    -rwxr-xr-x  1 matt  staff  9612  9 Oct 16:04 demo_with_tests
    -rwxr-xr-x  1 matt  staff  9228  9 Oct 16:04 demo_without_tests
    bash-3.2$

It's possible that the same test name will
be used in different files (i.e they're declared on the same line), but this is why the
function is declared as static - static functions sharing the same name won't cause linking
errors as they're local to a single compilation unit.

However, the real fun happens when UNIT\_TESTS *is* defined (whether it's by passing -DUNIT\_TESTS
as a compiler option or just by uncommenting the `#define UNIT_TESTS` line in unittest.h).
In C++, the constructors of global variables are called before main(). When UNIT_TESTS is
defined, the `unittest` macro takes advantage of this and expands into a structure declaration,
the declaration of an instance of that struct as a global variable,
and the beginning of the definition for that structure's default constructor. This expansion
is as follows:

{% highlight c++ %}
{% raw %}
unittest { // this is line 35 of a source file
    // ... do a test
}

// after preprocessing with unit tests *enabled*
static struct TEST35 {
       TEST35(); // declare the default constructor
} test35; // note the lower case name for the global variable

TEST35::TEST35() {
    // ... do a test
}
{% endraw %}
{% endhighlight %}

Much like the case where unit tests are disabled, the static keyword is used to make sure that
the global variable's name doesn't clash with any other tests in other C++ files that managed
to have the same name.

## Limitations

Because the tests depend on being declared as global variables, it means that (unlike D)
you're not able to define unit tests anywhere but at the global level (however within a namespace
should be fine). What this means is that this won't work:

{% highlight c++ %}
{% raw %}
class foo {
public:
    void bar() {
        // code...
    }
    unittest {
        foo myFoo;
        myFoo.bar();
        // some assertion
    }
    // more class stuff...
};
{% endraw %}
{% endhighlight %}

However this is OK and will work as expected, as the unit test is now in global scope:

{% highlight c++ %}
{% raw %}
// this is usually in a separate header file
class foo {
public:
    void bar();
    // more class stuff...
};

// and this is in a source file
void foo::bar() {
    // code...
}
unittest {
    foo myFoo;
    myFoo.bar();
    // some assertion
}
{% endraw %}
{% endhighlight %}

Note that the function declaration could have stayed within the class block (as long as
the unit test is outside it and global) but in D
code the common practice is to define unit tests immediately below the function you're
intending on testing. I agree with this practice, otherwise it's not obvious what you're testing.