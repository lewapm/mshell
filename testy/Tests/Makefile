BIN=bin
SRC=src

all: $(BIN)/splitter $(BIN)/catcher $(BIN)/fdcounter $(BIN)/decho $(BIN)/testerOX $(BIN)/tsleep

$(BIN)/splitter : $(SRC)/splitter.c
	cc -o $@ $(SRC)/splitter.c

$(BIN)/catcher : $(SRC)/catcher.c
	cc -o $@ $(SRC)/catcher.c

$(BIN)/fdcounter : $(SRC)/fdcounter.c
	cc -o $@ $(SRC)/fdcounter.c

$(BIN)/decho : $(SRC)/decho.c
	cc -o $@ $(SRC)/decho.c

$(BIN)/testerOX : $(SRC)/testerOX.c
	cc -o $@ $(SRC)/testerOX.c -lm

$(BIN)/tsleep : $(SRC)/testerOX.c
	cc -o $@ $(SRC)/testerOX.c -lm

clean:
	rm -f $(BIN)/*

.PHONY: all clean
