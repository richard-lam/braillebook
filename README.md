# braillebook

## Description
Braille literacy within the blind community has declined in recent decades with the advent of text-to-voice and other audio technologies. The braillebook is a protoype embedded device created as a Braille learning tool.  

## Hardware
The design implements pairs of 3D printed 8-sided rotating disks with Braille faces that combine to display any 6-dot Braille character. 

    20 x left 8-sided Braille Disk
    20 x right 8-sided Braille Disk

The disks are driven by two stepper motors: one for rotational positioning and one for horizontal positioning.

    2 x Stepper motor - NEMA-17 size - 200 steps/rev, 12V 350mA

Powered by a single microcontroller and motordriver.

    1 x Arduino Uno R3
    1 x Adafruit Proto Shield for Arduino Unassembled Kit - Stackable - Version R3

Miscellaneous parts.

    2 x 3D printed rotational spur gears (unknown gear ratio)
    2 x 3D printed horizontal rack and pinion gears (unknown gear ratio)
    1 x rotational stepper motor enclonsure
    1 x rotational axle mount
    4 x metal rods
    1 x drawer slide
    y x lumber

A constraint is the display refresh speed is limited by the sequential nature of the rotational positioning of disks.

## Braille reference
### A Braille character is a 2x3 cell with 2^6 = 64 possible configurations

    . = bump
    _ = blank

### Braille cell numbering
    
    14
    25
    36

### 26 Character English Alphabet

        ._	    ._	    ..	    ..	    ._	    ..	    ..	    ._	    _.	    _.	
    a = __	b = ._	c = __	d = _.	e = _.	f = ._	g = ..	h = ..	i = ._	j = ..
        __	    __	    __	    __	    __	    __	    __	    __	    __	    __

        ._	    ._	    ..	    ..	    ._	    ..	    ..	    ._	    _.	    _.
    k = __	l = ._	m = __	n = _.	o = _.	p = ._	q = ..	r = ..	s = ._	t = ..
        ._	    ._	    ._	    ._	    ._	    ._	    ._	    ._	    ._	    ._

        ._	    ._	    _.	    ..	    ..	    ._
    u = __	v = ._	w = ..	x = __	y = _.	z = _.
        ..	    ..	    _.	    ..	    ..	    ..

### Numbers are preceded by the number/hash/pound sign
*note that numbers 1-9,0 are the same symbols as a-i,j

        _.	    
    # = _.	
        ..	

        ._	    ._	    ..	    ..	    ._	    ..	    ..	    ._	    _.	    _.	
    1 = __	2 = ._	3 = __	4 = _.	5 = _.	6 = ._	7 = ..	8 = ..	9 = ._	0 = ..    
        __	    __	    __	    __	    __	    __	    __	    __	    __	    __


### Punctuation

        __	    __	    __	    __	    __	    __	    __	   
    , = ._	; = ._	: = ..	. = ..	? = ._	! = ..	' = __	
        __	    ._	    __	    _.	    ..	    ._	    ._	
