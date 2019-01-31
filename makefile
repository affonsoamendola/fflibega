#	Copyright Affonso Amendola 2019					#
#													#
#	LibEGA											#
#---------------------------------------------------#

LIBNAME = libega
CC = tcc
RM = rm -f

INCLUDE = include;D:\tc\include
LIB = D:\tc\lib
CCFLAGS = 

all: $(LIBNAME).exe

$(LIBNAME).exe:
# 	$(CC) "-I$(INCLUDE) -L$(LIB) -o$(LIBNAME).OBJ $(CCFLAGS)" *.c 
	$(CC) "-I$(INCLUDE) -L$(LIB) -e$(LIBNAME) $(CCFLAGS)" *.c 
#	tlib ".\$(LIBNAME).LIB + $(LIBNAME).OBJ"

clean:
	$(RM) *.OBJ *.EXE *.LOG *.BAT *.LIB *.BAK

run:
	dosbox -conf ~/.dosbox/tcc.conf -c "$(LIBNAME)"