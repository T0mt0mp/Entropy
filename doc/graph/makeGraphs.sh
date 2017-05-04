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
    "3 10000 100 1 1 12"

    "4 10000 100 1 1 12"

    "6 10000 10 2 25 10 75"
    "6 10000 10 4 25 10 75"

    "7 10000 10 2 1 1 10"
    "7 10000 10 4 1 1 10"
    "7 10000 10 2 10 5 50"
    "7 10000 10 4 10 5 50"
)

NUM_COMP_ENT=${#COMP_PARAMS_ENTROPY[@]}

: <<'END'
COMP_PARAMS_HOLDERS=(
    "5 10000 10000 100000 20 20 sm 0"
    "5 10000 10000 100000 20 20 mm 0"
    "5 10000 10000 100000 20 20 lm 0"

    "5 10000 10000 100000 20 20 sml 0"
    "5 10000 10000 100000 20 20 mml 0"
    "5 10000 10000 100000 20 20 lml 0"

    "5 10000 10000 100000 20 20 sl 0"
    "5 10000 10000 100000 20 20 ml 0"
    "5 10000 10000 100000 20 20 ll 0"

    "5 10000 10000 100000 20 20 sm 1"
    "5 10000 10000 100000 20 20 mm 1"
    "5 10000 10000 100000 20 20 lm 1"

    "5 10000 10000 100000 20 20 sml 1"
    "5 10000 10000 100000 20 20 mml 1"
    "5 10000 10000 100000 20 20 lml 1"

    "5 10000 10000 100000 20 20 sl 1"
    "5 10000 10000 100000 20 20 ml 1"
    "5 10000 10000 100000 20 20 ll 1"
)
END

NUM_COMP_HOLDERS=${#COMP_PARAMS_HOLDERS[@]}

GRAPH_SCRIPTS=(
    "entityCreation.r"
    "movementSystem.r"
    "advancedMovementSystemLow.r"
    "advancedMovementSystemHigh.r"
    "holders.r"
    "entityParallel.r"
    "entityChangeset.r"
    "holdersR.r"
)

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
        sudo nice -20 ${BUILD_PATH}/${exec} ${params} \
            > ${BUILD_PATH}/${OUTPUT_DIR}/${exec}${index}
    done
done

echo "${NUM_COMP_ENT} comparison parameter lists specific to Entropy found."

for (( index=0; index<${#COMP_PARAMS_ENTROPY[@]}; index++ )); do
    params="${COMP_PARAMS_ENTROPY[${index}]}"
    echo "Running Entropy comparison experiment number ${index}, parameters \"${params}\""
    sudo nice -20 ${BUILD_PATH}/${ENTROPY_EXEC} ${params} \
        > ${BUILD_PATH}/${OUTPUT_DIR}/${ENTROPY_EXEC}Spec${index}
done

echo "${NUM_COMP_HOLDERS} comparison parameter lists for Entropy holders found."

for (( index=0; index<${#COMP_PARAMS_HOLDERS[@]}; index++ )); do
    params="${COMP_PARAMS_HOLDERS[${index}]}"
    echo "Running Entropy holder comparison experiment number ${index}, parameters \"${params}\""
    sudo nice -20 ${BUILD_PATH}/${ENTROPY_EXEC} ${params} \
        > ${BUILD_PATH}/${OUTPUT_DIR}/${ENTROPY_EXEC}Holder${index}
done

echo "Running graph creation scripts!"

for script in "${GRAPH_SCRIPTS[@]}"; do
    echo "Running ${script} ..."
    Rscript ${script} ${BUILD_PATH}/${OUTPUT_DIR}/
done

echo "End of script."

