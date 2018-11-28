#CC=gcc
CC=aarch64-linux-gnu-gcc
#CC_FLAG=-Wall
 
PRG=test
OBJ=my_jpeg.o jpg_header.o jpg_data.o
 
$(PRG):$(OBJ)
	$(CC) $(INC) $(LIB) -o $@ $(OBJ)
	
.SUFFIXES: .c .o .cpp
.cpp.o:
	$(CC) -O0 $(INC) -c $*.cpp -o $*.o
 
.PRONY:clean
clean:
	@echo "Removing linked and compiled files......"
	rm -f $(OBJ) $(PRG)
