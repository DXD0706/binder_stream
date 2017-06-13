.SUFFIXES: .o .C .cpp

CC=g++ -g
VPATH=../src

INCDIR=
LIBDIR=

C++FLAGS+=-Wall -Wno-deprecated

LIBS=-lpthread -lrt

.cpp.o:
	$(CC) $(C++FLAGS) $(INCDIR) -c $<

all:dfes_binder dfes_binder_client

OBJS=dfes_binder.o
dfes_binder:$(OBJS)
	$(CC) -o $@ $^ $(LIBDIR) $(LIBS)


OBJS2=dfes_binder_client.o
dfes_binder_client:$(OBJS2)
	$(CC) -o $@ $^ $(LIBDIR) $(LIBS)
#make clean
clean:
	$(RM) $(OBJS) $(OBJS2)
