==> Read the datasheet on Interupts available, where the vector addresses are stored, and the control registers to enable them.

==> Are the interupt vectors 8, 16 or 32-bit wide?
==> Is the memory addressing bytes or words?
==> Do we use rjmp or jmp? How does this differ to say the ATmega8/88?
==> How does this address space differ from the PORT Registers (say 0x0004 for DDRB)?
==> Are the interupt vectors in the same place for 328 / 32u4? How about the control registers / bits?
==> Do interupts fire if code has finished execution? C and ASM?

1. Set up the button to trigger an interupt which toggles the lamp value
