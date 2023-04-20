src=$(wildcard *.cpp)
obj=$(patsubst %.cpp,%.o,$(src))

libweb.a:$(obj)
	ar -rc $@ $^
%.o:%.cpp
	g++ -c $< -I ./include -Wall

.PHONY:clean
clean:
	rm *.o -rf

.PHONY:output
output:
	mkdir output
	cp *.h libweb.a ./out