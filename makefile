
MODE:=64

CC=g++ -m$(MODE) -std=c++11 -I"include"

src/glew.o:src/glew.c
	$(CC) -c  $^ -o $@ -I"include" 

%.o:%.c
	$(CC) -c  $^ -o $@ -I"include" -I"shader"

ch04/%.o:ch04/%.c
	$(CC) -c  $^ -o $@ -I"include" -I"shader" -I"ch04"

req_o= esShader.o esShapes.o esTransform.o esUtil.o win32/esUtil_win32.o

%.exe: ch04/%.o $(req_o)
	$(CC) $^ -o bin/win$(MODE)/$@  -L"lib/Windows_x86_$(MODE)" -llibEGL -llibGLESv2  -lwinmm -lgdi32 -Wl,--subsystem,windows
	# -Wl,--subsystem,windows

ParticleSystemTransformFeedback.exe:ch14/ParticleSystemTransformFeedback.o $(req_o) ch14/Noise3D2.o
	$(CC) $^ -o bin/win$(MODE)/$@  -L"lib/Windows_x86_$(MODE)" -llibEGL -llibGLESv2  -lwinmm -lgdi32 -Wl,--subsystem,windows

clean:
	rm -f *.o */*.o *.exe