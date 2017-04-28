#!/bin/bash

BUILD_PATH="../../build"
OUTPUT_DIR="comparisonOutput"
GRAPH_PATH="."

ENTROPY_EXEC="ComparisonEntropy"
EXECUTABLES=(
    "${ENTROPY_EXEC}"
    "ComparisonAnax"
    "ComparisonArtemis"
    "ComparisonEntityx"
)

COMP_PARAMS=(
#    CompNum, Start, delta and max entities
    "0 10000 20000 100000"
#    CompNum, Number of entities, iterations, start, delta and max usage %
    "1 10000 10 25 10 75"
)

NUM_COMP=${#COMP_PARAMS[@]}

echo "Running graph creation script: "
echo "  BUILD_PATH = ${BUILD_PATH}"
echo "  GRAPH_PATH = ${GRAPH_PATH}"

for exec in "${EXECUTABLES[@]}"; do
    if [ ! -f "${BUILD_PATH}/${exec}" ]; then
        echo "ERROR: ${BUILD_PATH}/${exec} has not been found!"
        exit 1
    fi
done

echo "All comparison executables are present."

if [ ! -d "${BUILD_PATH}/${OUTPUT_DIR}" ]; then
    echo "Creating output directory \"${BUILD_PATH/${OUTPUT_DIR}}\""
    mkdir "${BUILD_PATH}/${OUTPUT_DIR}"
    if [ ! -d "${BUILD_PATH}/${OUTPUT_DIR}" ]; then
        echo "Unable to create the output directory!"
        exit 1
    fi
fi

echo "${NUM_COMP} comparison parameter lists found."

for params in "${COMP_PARAMS[@]}"; do
    index=${params%% *}
    echo "Running comparison experiment number ${index}"
    for exec in "${EXECUTABLES[@]}"; do
        echo "Running comparison for \"${exec}\""
        ${BUILD_PATH}/${exec} ${params} \
            > ${BUILD_PATH}/${OUTPUT_DIR}/${exec}${index}
    done
done

echo "End of script."

