C=g++ -g -Wall --std=c++98
E=.cpp
O=A4.o
P=A4
all: $(P)
$(P):$(O)
	$(C) -o $(P) $(O) -pthread
$(E).o:
	$(C) -c $<  -o $@
clean:
	rm $(O) $(P)


.PHONY: clean all
