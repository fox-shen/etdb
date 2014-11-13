#Macros
SUBDIRS = src

#Actions
all : clean
	@for X in $(SUBDIRS); \
	do \
	    cd $$X; make VERSION=$(VERSION); cd -; \
	done

clean :
	@for X in $(SUBDIRS); \
	do \
	    cd $$X; make clean; cd -; \
	done

