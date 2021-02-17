CXX=g++
CXXFLAGS=-O3 -Wall -Wextra -std=c++17 \
	 -Wno-unused-result -Wno-sign-compare -Wno-return-type \
	 -I shared -I third-party
MAINFILE=main.cpp

CPLEX_INCLUDE=/opt/ibm/ILOG/CPLEX_Studio128/cplex/include
CPLEX_LIBRARY=/opt/ibm/ILOG/CPLEX_Studio128/cplex/lib/x86-64_linux/static_pic

EX1=ex1-cplex
EX2=ex2-metaheuristic
RANDOM=random-baseline

OBJ_EX1=main.o

OUT_DIR="./build"
EXT=".out"

all: dir algs

algs: ${EX1} ${EX2} ${RANDOM}

dir:
	mkdir -p ${OUT_DIR}


${EX1}: dir
	${CXX} ${CXXFLAGS} -I ${CPLEX_INCLUDE} -c ./${EX1}/main.cpp -o ${OUT_DIR}/${OBJ_EX1}
	${CXX} ${CXXFLAGS} $(addprefix ${OUT_DIR}/,$(OBJ_EX1)) -o "${OUT_DIR}/${EX1}${EXT}" -L ${CPLEX_LIBRARY} -lcplex -lm -pthread -ldl
	rm -rf ${OUT_DIR}/${OBJ_EX1}

${EX2}: dir
	${CXX} ${CXXFLAGS} -pthread "${EX2}/${MAINFILE}" -o "${OUT_DIR}/${EX2}${EXT}"


${RANDOM}: dir
	${CXX} ${CXXFLAGS} -pthread "${RANDOM}/${MAINFILE}" -o "${OUT_DIR}/${RANDOM}${EXT}"

clean:
	rm -rf ${OUT_DIR}

.PHONY: all clean
