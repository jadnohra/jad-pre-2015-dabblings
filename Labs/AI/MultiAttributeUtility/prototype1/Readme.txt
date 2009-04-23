Motivation
----------
It was sunday, I had not much to do, after watching 'Applying Behavioral Mathematics to Left 4 Dead with Dave Mark' on AIGamedev.com,
I thought about how much time it would take to wip out a very rudimentary prototype, so I took a one day shot at it.

Behavioral mathematics
----------------------
The usage of response curves as such is not new and ideas very closed to what is presented can be found per example in 'AI Game Programming Wisdom', 
There is a chapter about the 'Beauty of a Response Curve' written by 'Bob Alexander' and how to use it as a heuristic to decide if an enemy soldier is to be attacked or not.
The author does not call it Behavioral mathematics, I know I wouldn't have either, but that's a very important thing to do! 
We have seen in many cases that simply coining a term is already an important step in branching a new stand alone technique.
For those who do not own the book, a lucky google books hit takes directly to our chapter:
http://books.google.de/books?id=4f5Gszjyb8EC&pg=PA81&lpg=PA81&dq=AI+response+curve&source=bl&ots=9ASRtsJwmh&sig=l-amw71jk-eP6VP_FaOhJ0y9tfQ&hl=en&ei=fXXmSfrODcjdsgbYkr2VBw&sa=X&oi=book_result&ct=result&resnum=1#PPA79,M1

Super-short term planner
------------------------
I think of this whole approach as a super-short term planner. 
What we are doing here is comparable to per example to Q-Learning in Reinforcement learning, 
with the difference of manually learning the values of state/action values, by using response curves and weights and combining them.
We could even plan based on this... but for starters, it seems to be a good alternative to State machines, it still remains to be seen 
how easy it is to tune all of this, Dave repeats that we would base ourselves on compartementalized confidence, which is crucial, 
but I am not sure it achieves what he says about this allowing us to sit and design instead of sitting at the keyboard, I am not sure 
it is possible to design those curves with confidence irresepective of the confidene in how we build blocks from simpler ones, 
the answers are proably in his book :) but it is still difficult to imagine that thinking and logic alone will cut it without lots of tweaking.

1st prototype
-------------
In any case, this 1st version is the work of 1.5 days, I decided to go for the Love2d engine, there are many alternatives, HGE was a good candidate, 
unlike Love2D it had a built in GUI, but tight Lua intergration made me choose Love2d.

The prototype is based on some base source code (Vector, Box, Circle, BaseEntity, basic response curves) an engine and scenarios, 
currently only one scenario is loaded from a hard coded path.

The scenario sets up the scene and fills it with entities, it also assigns drives to entities, those drives have their values displayed and
updated as the user drags entities around with the mouse.


Extensibility
-------------
With Lua it should be very easy to extend and add new drives, make drives get their values from other ones, etc...
It should be easy to add a console (there are some love2d implementations which I did not look at yet) and this would allow 
changing properties such as health, weapon in real time.

The prototype also allows right-dragging to move the camera, zooming is also possible, but only programmatically for now.
 
Death??
-------
It could very well be that this short burst of work will die a fast death, but who knows ...

Jad Nohra.




