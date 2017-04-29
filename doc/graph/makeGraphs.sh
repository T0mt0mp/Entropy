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
#    CompNum, Number of entities, iterations, start, delta and max usage %
    "2 10000 10 1 1 10"
    "2 10000 10 10 5 50"
)

NUM_COMP=${#COMP_PARAMS[@]}

COMP_PARAMS_ENTROPY=(
    "5 10000 20000 100000 20 5 sm"
    "5 10000 20000 100000 20 5 mm"
    "5 10000 20000 100000 20 5 lm"

    "5 10000 20000 100000 20 5 sml"
    "5 10000 20000 100000 20 5 mml"
    "5 10000 20000 100000 20 5 lml"

    "5 10000 20000 100000 20 5 sl"
    "5 10000 20000 100000 20 5 ml"
    "5 10000 20000 100000 20 5 ll"

    "3 10000 100 1 1 12"

    "4 10000 100 1 1 12"
)

GRAPH_SCRIPTS=(
    "entityCreation.r"
    "movementSystem.r"
    "advancedMovementSystemLow.r"
    "advancedMovementSystemHigh.r"
    "holders.r"
    "entityParallel.r"
    "entityChangeset.r"
)

NUM_COMP_ENT=${#COMP_PARAMS_ENTROPY[@]}

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

for (( index=0; index<${#COMP_PARAMS[@]}; index++ )); do
    params="${COMP_PARAMS[${index}]}"
    echo "Running comparison experiment number ${index}, parameters \"${params}\""
    for exec in "${EXECUTABLES[@]}"; do
        echo "Running comparison for \"${exec}\""
        sudo nice -n -20 ${BUILD_PATH}/${exec} ${params} \
            > ${BUILD_PATH}/${OUTPUT_DIR}/${exec}${index}
    done
done

echo "${NUM_COMP_ENT} comparison parameter lists specific to Entropy found."

for (( index=0; index<${#COMP_PARAMS_ENTROPY[@]}; index++ )); do
    params="${COMP_PARAMS_ENTROPY[${index}]}"
    echo "Running Entropy comparison experiment number ${index}, parameters \"${params}\""
    sudo nice -n -20 ${BUILD_PATH}/${ENTROPY_EXEC} ${params} \
        > ${BUILD_PATH}/${OUTPUT_DIR}/${ENTROPY_EXEC}Spec${index}
done

echo "Running graph creation scripts!"

for script in "${GRAPH_SCRIPTS[@]}"; do
    Rscript ${script} ${BUILD_PATH}/${OUTPUT_DIR}/
done

echo "End of script."

