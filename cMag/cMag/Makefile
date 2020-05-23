#---------------------------------------------------------------------
# Define the C compiler
#---------------------------------------------------------------------

        CC = cc

        CFLAGS = -c 

#---------------------------------------------------------------------
# Define rm & mv  so as not to return errors
#---------------------------------------------------------------------

        RM =  rm -f
        MV =  mv -f
        MKDIR = mkdir -p

#---------------------------------------------------------------------
# Define (relative to the source dir) the object dir
#---------------------------------------------------------------------

        OBJDIR = ../obj

#---------------------------------------------------
# Program and library names
#---------------------------------------------------

     PROGRAM = magtest
     LIBNAME = magfield.a

#---------------------------------------------------------------------
# The source files that make up the application
#---------------------------------------------------------------------

      SRCS = \
             magfield.c\
             magfieldutil.c\
             main.c


#--------------------------------------------------------------------
# required libraries
#--------------------------------------------------------------------
     LIBS = -lm

#---------------------------------------------------------------------
#the lib source files are those that have code that changes whe we
#are making a library
#---------------------------------------------------------------------

      LIBSRCS = main.c

#---------------------------------------------------------------------
# This variable lists the subdirectories to search for include files.
#---------------------------------------------------------------------

      INCLUDES = 

#---------------------------------------------------------------------
# The object files (via macro substitution)
#---------------------------------------------------------------------

      OBJS = ${SRCS:.c=.o}
      LIBOBJS = ${LIBSRCS:.c=.o}

#---------------------------------------------------------------------
# How to make a .o file from a .c file 
# Note: move the .o files back as they are made
#---------------------------------------------------------------------

.c.o :
	$(CC) $(CFLAGS) $(INCLUDES) $<
	cp -p $@ $(OBJDIR)

#--------------------------------------------------------
# Default target
# Object files are copied from object
# directory with -p to preserve date
#--------------------------------------------------------

     all:
	$(RM) *.o
	$(MKDIR) $(OBJDIR)
	touch $(OBJDIR)/tmpfle
	cp -p $(OBJDIR)/* .
	$(RM) $(OBJDIR)/tmpfle
	make $(PROGRAM)
	$(RM) *.o

#---------------------------------------------------------------------
# This rule generates the executable using the object files and libraries.
#---------------------------------------------------------------------

     $(PROGRAM): $(OBJS)


	$(CC) -o $@ $(OBJS) $(LIBS) 
	cp -p  $(PROGRAM) $(OBJDIR)

#---------------------------------------------------------------------
# This rule creates the library libmagreader.a
#---------------------------------------------------------------------

     lib: $(OBJS)
	$(RM) $(LIBNAME)
	$(RM) $(LIBOBJS)
	$(CC) $(CFLAGS) $(LIBFLAGS) $(INCLUDES) $(LIBSRCS)
	$(AR) r $(LIBNAME) $(OBJS)
	$(HV_RANLIB) $(LIBNAME)
	$(RM) $(LIBOBJS)


#---------------------------------------------------------------------
#additional dependencies
#---------------------------------------------------------------------

INCLUDEFILES =  magfield.h
magfield.o:      $(INCLUDEFILES)
magfieldutil.o:  $(INCLUDEFILES)
main.o:          $(INCLUDEFILES)
