---
layout: post
title:  "3-Vec positioning for user interfaces"
---

In my current project (a native version of Line Rider), I've been using my own custom UI
system that I've found to be insanely powerful for code-based layouts. There's other UI
systems out there but I've yet to find one that works with the OpenGL Core Profile and also doesn't
introduce other dependencies like Boost. Furthermore, doing code-based layout with these systems
is a world of pain.

## Traditional (read: painful) positioning

Usually a UI element (or a view/widget/MovieClip, whatever you want to call it) has a position and
a size associated with it. Both are simple 2D vectors - the position is an `(x y)` pair and
the size a `(width height)` pair. This works for trivial layouts. However, it breaks down quickly
once a resizable window is brought into play.

Traditional approaches to this have been to use layout managers (which require you to learn a whole
new API whenever you want to do a different layout) or to go with fully constraint based layout.
While constraints are powerful, they're also incredibly complex and fiddly to get right. Apple's
constraint system for Auto Layout is so complex that the API includes a way to use _ASCII art_ to
describe your constraints.

I'm not interested in these approaches and I don't need complex constraints. I'm just looking for
a way to anchor elements to a part of the window and put other elements near them, without having
everything fall apart when something changes size. I'm also interested in doing this in a platform-
independent manner, which takes Auto Layout (and any of Apple's technologies) right out.

## Enter 3-Vec positioning

Instead of trying to force the traditional `(x y)` absolute positioning model into working for me, I decided
to take a totally new approach. I made everything relative and went with three vectors (hence the
name) that, in conjunction with a handle to a target UI element, are able to describe in one line of
code positions like:

- An element with its lower-right corner sitting 12pt away from the lower-right corner of its target.
- An element that's centered on the center of the window.
- An element that's horizontally aligned with its target with a 5pt gap between the two.

A 3-Vec position is composed of an _anchor_, an _offset_, and an _alignment_.
Note that each element also has a size as a 3-Vec position is designed to only
replace the `(x y)` position of an element.

The anchor is a point in normalised space (i.e from `(0 0)` to `(1 1)`) that
describes where in the target the position should initially be placed. `(0.5
0.5)` resolves to the middle of the target and `(1 0)` resolves to the top-right of
the target element, regardless of its size.

The offset is a vector that's then added to the resolved position of the anchor.
This offset is in points/pixels/etc - an offset that moves the position 12pt up
and 12pt to the right of an anchor would be `(12 -12)`.

The alignment is used to then place the element relative to this resolved and
offset position. An alignment of `(0 0)` puts the top-left corner of the
element there, `(1 1)` puts the lower-right corner of the element there and so
on (like the anchor, this is normalised space).

Putting it all together, a 3-Vec position that centers an element on the center of
the window is simply:

    anchor: (0.5 0.5) // anchor us to the center of the window, regardless of its size
    offset: (0 0) // no offset from there
    align: (0.5 0.5) // and put the center of us at that position
    target: window

In actual C++ code, I'd construct it with something like (using C++11 universal initializers):

{% raw  %}
    UIElement elem({{0.5, 0.5}, {0, 0}, {0.5, 0.5}, window},
                   {300, 300});			   
{% endraw  %}

Note that the `{300, 300}` parameter is just the size of the element.

## Getting back to the world of the absolute

While absolute positioning is hopeless for layout, it's needed for everything else in
a UI system like hit testing the mouse or setting transforms for rendering elements.
Getting an absolute position out of a 3-Vec position is relatively simple. Given a

	absPos = ((target.size * anchor) + offset) - (elem.size * align)

Note that these operations here are all element-wise, including the multiply. For
people familiar with MATLAB syntax the equivalent would be `.*`.
