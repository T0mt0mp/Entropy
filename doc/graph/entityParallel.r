
pdf(file="entityParallelGraph.pdf")

args <- commandArgs(trailingOnly = TRUE)
inputFolder <- args[1]

#entropyTable <- read.table(file=paste(inputFolder, "/ComparisonEntropy0", sep=""), sep="\t", header=TRUE)
#anaxTable <- read.table(file=paste(inputFolder, "/ComparisonAnax0", sep=""), sep="\t", header=TRUE)
#artemisTable <- read.table(file=paste(inputFolder, "/ComparisonArtemis0", sep=""), sep="\t", header=TRUE)
#entityxTable <- read.table(file=paste(inputFolder, "/ComparisonEntityx0", sep=""), sep="\t", header=TRUE)

dataTable <- read.table(file=paste(inputFolder, "/ComparisonEntropySpec9", sep=""), sep="\t", header=TRUE)

baseVal <- dataTable$Entropy[1]
#print(dataTable)
#dataTable <- transform(dataTable, Entropy=paste((baseVal / Entropy * 100), "%"))
dataTable <- transform(dataTable, Entropy=(baseVal / Entropy * 100))
#print(dataTable)

#dataTable <- data.frame(Entities=entropyTable$Entities, Entropy=entropyTable$EntropyPerEnt, Anax=anaxTable$AnaxPerEnt, Artemis=artemisTable$ArtemisPerEnt, EntityX=entityxTable$EntityXPerEnt)
#dataTable <- data.frame(Entities=entropyTable$Entities, Entropy=entropyTable$Entropy, Anax=anaxTable$Anax, Artemis=artemisTable$Artemis, EntityX=entityxTable$EntityX)

xRange <- range(dataTable$Threads)
yRange <- range(0, dataTable$Entropy)

plot(dataTable$Entropy~dataTable$Threads, type="n", xlim=c(xRange[1], xRange[2] + 1), ylim=c(100, 500), ann=FALSE, axes=FALSE)
axis(1, at=seq(from=xRange[1], to=xRange[2], by=1), lwd=2)
#yTicks <- seq()
#axis(2, lwd=2)
#axis(2, at=seq(from=0, to=yRange[2], by=100), lwd=2)
atTicks <- seq(from=100, to=500, by=50)
myTicks <- transform(atTicks, Entropy=paste(round(atTicks / 100, 1), "x", sep=""))$Entropy
#print(myTicks)
axis(2, at=atTicks, labels=myTicks)

points(dataTable$Entropy~dataTable$Threads, type="o", lwd=2, col="blue", pch=19)

legend("bottomright", pch=c(19), lty=c(1), legend=c("Entropy"), col=c("blue"), pt.lwd=2, box.lwd=2)
title(main="Processing entities in parallel", xlab="Number of threads", ylab="Relative performance")

