common = common/controls.o common/shader.o common/texture.o network.o
CC = g++
flag = -g
glFlag = -lGL -lGLEW -lglfw 
output = draw

$(output): draw.o $(common)
		$(CC) draw.o $(common) -o $(output) $(glFlag) $(flag)

draw.o: draw.cpp layout.h particle.h
	$(CC) -c draw.cpp $(flag)


$(common): %.o: %.cpp
	$(CC) -c $< -o $@ $(glFlag) $(flag)

clean:
	rm *.o common/*.o


