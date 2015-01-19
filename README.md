# Rainbow-Table-Generator
This was developed during my computer security class.

For information about rainbow tables see the RainbowTables.pdf document.

At the current state the code can generate rainbow tables based on the BLAKE hash function. http://en.wikipedia.org/wiki/BLAKE_(hash_function)

It was developed for fast, in-memory rainbow tables because of limited search time available for each password to be found.
There are functions to save the rainbow table to a file and load it from there but this is just to enable precomputation of the rainbow table prior to the attack on the passwords.


