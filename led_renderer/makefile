SOURCEDIR=source/

NAME=led_renderer
LIBS=-lm 
FLAGS=-O2 -Wall -Wextra

EXESOURCE=$(SOURCEDIR)main.c $(SOURCEDIR)renderer.c $(SOURCEDIR)pwm.c $(SOURCEDIR)pcm.c $(SOURCEDIR)dma.c $(SOURCEDIR)mailbox.c $(SOURCEDIR)rpihw.c $(SOURCEDIR)ws2811.c
EXEOUTDIR=build/

DLLSOURCE=$(SOURCEDIR)renderer.c $(SOURCEDIR)pwm.c $(SOURCEDIR)pcm.c $(SOURCEDIR)dma.c $(SOURCEDIR)mailbox.c $(SOURCEDIR)rpihw.c $(SOURCEDIR)ws2811.c
DLLOUTDIR=dll_build/

exe:
	gcc $(FLAGS) $(LIBS) $(EXESOURCE) -o$(EXEOUTDIR)$(NAME).exe

dll:
	gcc -c $(FLAGS) $(LIBS) $(DLLSOURCE)
	mv *.o $(DLLOUTDIR)
	gcc -shared $(DLLOUTDIR)*.o -o$(DLLOUTDIR)$(NAME).dll

