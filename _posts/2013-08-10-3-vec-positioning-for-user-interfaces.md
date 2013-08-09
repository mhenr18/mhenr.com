---
layout: post
title:  "3-Vec positioning for user interfaces"
---

_Note: I anticipate that I'll revisit this topic at a later point, most likely
around when I release the UI library I'm building on top of this new positioning
model._

## Context

I'm currently working on a native version of Line Rider (which I'll almost
certainly be posting more about at some point), that's cross-platform and
uses OpenGL for graphics. Like most other games, it needs a user
interface. 

As someone who doesn't like wasting time reinventing the wheel,
I first went looking online for existing libraries. My ideal UI library would be:

- lightweight (not a HTML/CSS monstrosity),
- capable of code-based layouts,
- self-contained (within reason, FreeType is OK as a dependency),
- modern (no immediate mode rendering),
- written in C++ (as that's what I'm using),
- and permissively licensed (no GPL or LGPL).

Unfortunately I wasn't able to find any libraries meeting that criteria.
So, I rolled my own. In doing so, I decided to improve on the way that
code-based layout is currently done.

## Current approaches to positioning

One of my criteria - "capable of code-based layouts" is something that's
a pain point for many other APIs. This isn't just OpenGL/game stuff I'm
talking about, it's something that desktop APIs haven't solved. By "solved",
I don't mean that it's impossible to do UI in code with an API like Cocoa. That's
far from the truth. 

However, it's not currently possible to do UI in code that handles scaling without
taking one of two approaches:

- Use layout managers which make you learn a new API whenever you use a new layout,
- or use constraint-based layout.

The layout manager approach is just an abstraction on top of constraints.
This is for good reason, as constraint-based layout is hard to get right 
even for simple layouts. Anyone who's used Cocoa's Auto Layout in Interface
Builder will have noticed the vast numbers of constraints that are created as
they lay out their interface.

As a simple example, I created a new Xcode project and jumped into the .xib. I placed
two buttons in the lower right corner of the window, with Xcode automatically
creating 4 constraints that kept the buttons there regardless of window size.

<p align="center"><img src="http://mhenr18.com/assets/simpleui-constraints.png" /></p>

If it were easy to insert those constraints in code then I wouldn't have bothered
with a different approach. Unfortunately it's not. [It's so hard that the
Auto Layout API gives developers the option of using _ASCII art_ just to depict
simple constraints.](http://developer.apple.com/library/ios/documentation/UserExperience/Conceptual/AutolayoutPG/Articles/formatLanguage.html#//apple_ref/doc/uid/TP40010853-CH3)

## My approach: 3-Vec positioning

I wanted something that's a lot simpler then mentally having to constrain the edges
of all of my UI elements to each other. Something that lets me position my UI elements
in one line instead of ten.

When you consider an `(x, y)` position in UI, it's the _offset_ from the top-left corner
of some parent element like a window or container. If you're using a more traditional
coordinate system then it'd be from the bottom-left corner but the same mechanics apply.
Because it's an offset from that corner, things work just fine for elements placed in
that corner without even needing to adjust their position when the parent element is resized.

Consider the parent element's area as a unit square. We'd represent its top-left corner as
`(0, 0)` and its bottom-right corner as `(1, 1)`. If we include a point in that square - the _anchor_ - with the `(x, y)`
offset then we can get the benefits of not needing to adjust positions upon resizes. The
position `(1, 1), (-12, -12)` will always represent a point that's 12px up and to the left of
the bottom right corner of the element's parent, regardless of the parent's size.

This is already a big win but it's not enough. Say we wanted to put an element of size `(10, 10)`
in the bottom-right corner of its parent, with a 12px margin between it and the corner. 
We'd need to include the element's size in that position and end up with `(1, 1), (-22, -22)`.
If we were to resize the element, we'd need to adjust that offset again. This is the same problem
as before, except coming from the other direction. Instead of failing when the parent changes size,
we now fail when the element itself changes size.

To solve this problem we can adopt the same solution that we used for the anchor and supply another
point in the unit square. This point is the _align_ of the element. An alignment of `(0, 0)` would
place the top-left corner of the element at the position, while `(1, 1)` puts its bottom-right corner
at the position. Centering it there is just a matter of using `(0.5, 0.5)`.

Going back to the previous example, we can now provide a position that's totally independent of the
element's size: `(1, 1), (-12, -12), (1, 1)`. Those three vectors are (respectively) the anchor, offset
and align. No matter what the size of the parent or the element is, it'll always sit with a 12px margin
between the bottom-right corner of it and the bottom-right corner of the parent.

## Getting the frame out of a 3-Vec

To actually render an element or hit test mouse coordinates with it, we need its frame, which provides
the location of the top-left corner of the element (inside its parent) and the element's size. The size is trivial as it's
a known quantity. All we need to do is calculate the position of the top-left corner.

That can be done using the following formula:

    topLeftPos = ((parentSize * anchor) + offset) - (elementSize * align)	

Note that _all_ operations in that formula are element-wise, including the multiply. There's no dot
or cross products going on here. Also note the judicious use of brackets to make it obvious what's
going on :) Continuing on with the example of an element of size `(10, 10)` with
position `(1, 1), (-12, -12), (1, 1)`, inside a parent of size `(100, 100)` we'd end up with:

    topLeftPos = ((parentSize * anchor) + offset) - (elementSize * align)	
	           = [[(100, 100) * (1, 1)] + (-12, -12)] - [(10, 10) * (1, 1)]
	           = [(100, 100) + (-12, -12)] - (10, 10)
               = (88, 88) - (10, 10)
			   = (78, 78)			    

<p></p>

## The next level: Targeted 3-Vec positioning

It's great to have positioning that works without needing to care about the size of
the parent or the element. However, consider the layout that I used as an example of
constraints in a .xib file. The right hand button sat with a fixed margin between its bottom-right
corner and the bottom-right corner of the window.
Replicating that in a 3-Vec is easy - it's what I've been doing with my example. To replicate
the layout for the left hand button, we could just use the same 3-Vec as the right one but
increase the x-offset, so that it always sits at a fixed offset to the right hand button.

However, if we did that then we'd just get back to the same problems that caused us to move
to 3-Vec positioning. If the right hand button were to change size, we'd have to adjust the offset
of the left one. If I was interested in adjusting offsets then I wouldn't have bothered with 3-Vec
positioning in the first place :)

It turns out that we don't need to add any more vectors to our position. We just need
to anchor the left button to somewhere in the right hand button, instead of anchoring it to
its parent. Our position now becomes `(0, 0.5), (-5, 0), (1, 0.5), rightHandButton`. Here,
`rightHandButton` is the _target_ of our position's anchor.

Let's break this down. Our anchor of `(0, 0.5)` means that we're starting from a position that's
on the middle of the left hand side of `rightHandButton`. The `(-5, 0)` offset just moves 5 pixels to
the left of that anchor. Finally, our align of `(1, 0.5)` will place the middle of the right hand side of
our left button at that position.

As that's a wordy explanation, I've thrown together a quick diagram to visualise what this
is depicting:

<p align="center"><img src="http://mhenr18.com/assets/targeted-3vec.png" /></p>

## Framing a targeted 3-Vec

Getting the frame from a targeted 3-Vec requires a minor change to our formula for a parented 3-Vec:

	elementTLPos = ((targetSize * anchor) + offset) - (elementSize * align) + targetTLPos	

Note the use of the target's top-left position, which is needed for a simple reason: If we're anchoring
ourselves to the frame of a target then clearly we need to know what that frame is in order to resolve
the anchor. Also note that if we want to anchor an element to its parent like before, `targetTLPos` is going
to need be `(0, 0)`. This means that some testing is required to differentiate between the parent and
`target` cases.

## Pulling it all together (sample code)

_Note that I'm using C++11 universal initializers in this sample._

{% highlight c++ %}
{% raw %}
// I assume the existence of a Vec2<T> struct with operators
// overloaded for element-wise operations

template <typename T>
struct Rect {
	Vec2<T> topLeft;
	Vec2<T> size;
};

class UIElement;

struct UIPosition {
	Vec2<double> anchor;
	Vec2<double> offset;
	Vec2<double> align;
	UIElement* target;
	// position is parented
	UIPosition(Vec2<double> anchor_, Vec2<double> offset_, Vec2<double> align_)
	: anchor(anchor_), offset(offset_), align(align_), target(nullptr) {}

	UIPosition(Vec2<double> anchor_, 
		Vec2<double> offset_, 
		Vec2<double> align_, 
		UIElement* target_)
  	: anchor(anchor_), offset(offset_), align(align_), target(target_) {}
};

class UIElement {
	// ... stuff ...
	UIElement* parent;
	UIPosition position;
	Vec2<double> size;
public:
	UIElement(UIPosition pos, Vec2<double> size_) : position(pos), size(size_) {}
	// ... more stuff ...
	Rect<double> frame() {
		Rect<double> f;
		f.size = size;
		
		if(position.target) {
			f.topLeft = ((position.target->size * position.anchor) + position.offset) 
						    - (size * position.align) + position.target->frame().topLeft;
		} else {
			f.topLeft = ((parent->size * position.anchor) + position.offset) 
						    - (size * position.align);
		}
		
		return f;
	}
};

int main(int argc, const char** argv) {
	UIElement root{ {{0, 0}, {0, 0}, {0, 0}}, {848, 480} };
	UIElement rightHandElem{ {{1, 1}, {-12, -12}, {1, 1}}, {40, 15} };
	UIElement leftHandElem{ {{0, 0.5}, {-5, 0}, {1, 0.5}, &rightHandElem}, {40, 15} };	
	
	// Add rightHandElem and leftHandElem to root (i.e set their parents to root).
	// rightHandElem is anchored to the bottom-right of root, regardless of
	// its or root's size. leftHandElem is anchored to the left hand side of
	// rightHandElem, regardless of either of their sizes.
	
	// All in 3 lines of code. Win!
	
	return 0;
}
{% endraw %}
{% endhighlight %}



