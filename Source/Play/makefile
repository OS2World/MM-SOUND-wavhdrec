#
#  play.exe makefile for NMAKE and VAC++
#

CC = icc
AS = wasm

.ASM.OBJ:
   $(AS) $(AFLAGS) $*.ASM

all = play.exe

#INCLUDE = $(INCLUDE);some more
#LIB = $(LIB);some more

CFLAGS = /Gm /Ss /Q /Tdp

!IFDEF DEBUG
CFLAGS = /Ti $(CFLAGS)
!ENDIF

!IFNDEF NOTOPT
CFLAGS = /B "/PACKCODE /PACKDATA" /Gf /Gi /O /Ol /G5 $(CFLAGS)
!ENDIF

DEF =

OBJECTS =  play.obj dart.obj wav.obj $(DEF)

LIBS =  mmpm2.lib

play.exe: $(OBJECTS)
   $(CC) $(CFLAGS) /Fm$* /Fe$@ $** $(LIBS)

play.obj : play.h
dart.obj : play.h wav.hpp
wav.obj  : wav.hpp
