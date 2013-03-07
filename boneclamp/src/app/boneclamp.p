//THIS STILL HAS TO BE IMPLEMENTED!!

.origin 0

//set ARM such that PRU can write to GPIO
//TEST IF THIS IS ACTUALLY NECESSARY!!
LBCO r0, C4, 4, 4
CLR r0, r0, 4
SBCO r0, C4, 4, 4

MOV r1, 10


