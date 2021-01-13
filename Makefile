CXX=g++
CXXFLAGS=-O3 -Wall -Wextra -std=c++17 -Wno-return-type -I shared -I third-party
MAINFILE=main.cpp

CPLEX_INCLUDE=/opt/ibm/ILOG/CPLEX_Studio128/cplex/include
CPLEX_LIBRARY=/opt/ibm/ILOG/CPLEX_Studio128/cplex/lib/x86-64_linux/static_pic

EX1=ex1-cplex
EX2=ex2-metaheuristic
RANDOM=random-baseline

OUT_DIR="."
EXT=".exe"

all: algs

algs: ${EX1} ${EX2} ${RANDOM}

${EX1}:
	${CXX} ${CXXFLAGS} -I ${CPLEX_INCLUDE} -L ${CPLEX_LIBRARY} -lm -pthread -lcplex  "${EX1}/${MAINFILE}" -o "${OUT_DIR}/${EX1}${EXT}"

${EX2}:
	${CXX} ${CXXFLAGS} "${EX2}/${MAINFILE}" -o "${OUT_DIR}/${EX2}${EXT}"


${RANDOM}:
	${CXX} ${CXXFLAGS} -pthread "${RANDOM}/${MAINFILE}" -o "${OUT_DIR}/${RANDOM}${EXT}"

.PHONY: all algs clear
.PHONY: ${EX1} ${EX2} ${RANDOM}

clear:
	rm "${OUT_DIR}/${EX1}${EXT}" "${OUT_DIR}/${EX2}${EXT}" "${OUT_DIR}/${RANDOM}${EXT}" 
