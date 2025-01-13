all: regex2fsm fsm-dump fsm-merge fsm-simplify

example: example.c generated.c

generated.c: regex2fsm fsm-dump fsm-merge fsm-simplify space.fsm identifier.fsm integer.fsm float.fsm
	./fsm-merge space.fsm identifier.fsm integer.fsm float.fsm | ./fsm-simplify | ./fsm-dump c > $@

%.fsm: %.regex
	./regex2fsm < $^ > $@

