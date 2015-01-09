This folder contains code projects to be loaded onto the glasses platform, separated out by hardware revision. Most of the folders contain a number of discrete projects for performing different tasks or driving different hardware functions, there is (should be) an index in each folder briefly describing all of the projects.

The following folders are exceptions to that:

- common_libs: Contains the libraries (mostly official ones from ST) that many of the v3+ projects reference

- rev0-x: The rev0 prototypes were our initial proof-of-concept and we broke out the code bases very differently for these platforms than we do now for the Cortex-based hardware. This is poorly documented, and because it is legacy at this point it will probably be left that way.