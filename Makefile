#	Copyright Affonso Amendola 2019					#
#													#
#	ffLibEGA											#
#---------------------------------------------------#

LIBNAME = FFLIBEGA
CC = tcc
RM = rm -f

INCLUDE = include;D:\tc\include
LIB = D:\tc\lib
CCFLAGS = -c

all: $(LIBNAME).LIB

$(LIBNAME).LIB:
	$(CC) "-I$(INCLUDE) -L$(LIB) -o$(LIBNAME).OBJ $(CCFLAGS)" *.c
	$(RM) $(LIBNAME).LIB
	tlib ".\$(LIBNAME).LIB + $(LIBNAME).OBJ"

clean:
	$(RM) *.OBJ *.AAA *.EXE *.LOG *.BAT *.LIB *.BAK

#run:
#	dosbox -conf ~/.dosbox/tcc.conf -c "$(LIBNAME)"