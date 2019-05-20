PROJECT_NAME=FrustumCulling
TEST_SCENE=asianDragon
# r -vp 615.894 146.908 564.519 -vd -0.726047 -0.173158 -0.665486 -vu 0 1 0 -vf 0.691836 -s ../data/A10.obj

test: build
	#cd build && vblank_mode=0 primusrun ./gTemplate -s ../data/conference.obj
	cd build && vblank_mode=0 primusrun ./${PROJECT_NAME} -no-plane-coherency -s scenes/$(TEST_SCENE).obj

debug: build
	cd build && vblank_mode=0 primusrun gdb ./${PROJECT_NAME} -s ../data/A10.obj 

build: .FORCE
	cd build && make ${PROJECT_NAME}

init:
	rm -rf build
	mkdir build && cd build && GLM_DIR=/usr/lib/cmake/glm/ cmake -DCMAKE_BUILD_TYPE=Release ../src

pack: .FORCE
	cd build && make pack

doc: .FORCE
	cd build && make doc

.FORCE:
