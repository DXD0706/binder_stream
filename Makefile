
all:
	$(MAKE) -C obj

.PHONY:clean
clean:
	$(RM) src/*.o src/*~
