Group 71 
Members: Henry Matthys - matth637, Shridhar Vashishtha - vashi024, Owen Keating - keati090
Tested on csel-kh1250-01.cselabs.umn.edu
No changes to makefile, used port 9637 in server.c and client.c
Contributions: 
Henry - Intermediate submission
Shridhar - Remaining client.c
Owen - Remaining server.c
Plan: The package will be sent by each client to the server. Each client will construct a package
and add to it the operation, flags, image size, and image data. The server will call the clientHandler function
for each client it receives, which will handle receiving the package. The clientHandler will check the
operation and flags given with the package, and then process the image data accordingly.
It will then send a package containing the processed image data back to the client.