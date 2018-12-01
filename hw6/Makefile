C=g++ -g -Wall --std=c++98
E=.cpp
O=main.o
P=A6
all: $(P)
$(P):$(O)
	$(C) -o $(P) $(O) $(SFML)
$(E).o:
	$(C) -c $<  -o $@
clean:
	rm $(O) $(P)

.PHONY: clean all
