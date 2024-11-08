#
#	configuration variables for the example

## Main application file
MAIN = purelylocalmotifclustermain
DEPH = $(EXSNAPADV)/purelylocalmotifcluster.h 
DEPCPP = $(EXSNAPADV)/purelylocalmotifcluster.cpp 

# Set the suffix _ if the fortran77 routines are named that way
CXXFLAGS += -DF77_POST
