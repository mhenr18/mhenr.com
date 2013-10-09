/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/

#ifndef testing_h_
#define testing_h_

// to include/run unit tests without using a compiler switch (i.e -DUNIT_TESTS),
// you'll want to uncomment the next line.
// #define UNIT_TESTS

#define ACTUALLY_CONCAT(a, b) a ## b
#define CONCAT(a, b) ACTUALLY_CONCAT(a, b)

#ifdef UNIT_TESTS
#define unittest							\
	static struct CONCAT(TEST, __LINE__) {				\
		void test_main();					\
		CONCAT(TEST, __LINE__) () {				\
			test_main();					\
		}							\
	}} CONCAT(test, __LINE__);					\
	void CONCAT(TEST, __LINE__)::test_main()
#else
#define unittest void CONCAT(TEST, __LINE__)
#endif

#endif