
args <- commandArgs(trailingOnly = TRUE)
inputFile <- args[1]

input <- read.csv(file=inputFile, sep="\t")

usage <- input$Usage
entropyVals <- input$Entropy

yRange <- range(0, entropyVals)
onePart <- yRange[2] / 10

#print(onePart * 0:yRange[2])

plot(entropyVals, type="o", x=usage, axes=FALSE, ann=FALSE)
axis(1, at=usage);
#axis(2, at=onePart * 0:yRange[2])

