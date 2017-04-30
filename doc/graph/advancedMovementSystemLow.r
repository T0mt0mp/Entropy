
pdf(file="advancedMovementSystemLowGraph.pdf")

args <- commandArgs(trailingOnly = TRUE)
inputFolder <- args[1]

entropyTable <- read.table(file=paste(inputFolder, "/ComparisonEntropy2", sep=""), sep="\t", header=TRUE)
anaxTable <- read.table(file=paste(inputFolder, "/ComparisonAnax2", sep=""), sep="\t", header=TRUE)
artemisTable <- read.table(file=paste(inputFolder, "/ComparisonArtemis2", sep=""), sep="\t", header=TRUE)
entityxTable <- read.table(file=paste(inputFolder, "/ComparisonEntityx2", sep=""), sep="\t", header=TRUE)

#dataTable <- data.frame(Entities=entropyTable$Entities, Entropy=entropyTable$EntropyPerEnt, Anax=anaxTable$AnaxPerEnt, Artemis=artemisTable$ArtemisPerEnt, EntityX=entityxTable$EntityXPerEnt)
dataTable <- data.frame(Entities=entropyTable$Change, Entropy=entropyTable$Entropy, Anax=anaxTable$Anax, Artemis=artemisTable$Artemis, EntityX=entityxTable$EntityX)

xRange <- range(dataTable$Entities)
yRange <- range(0, max(dataTable$Entropy, dataTable$Anax, dataTable$Artemis, dataTable$EntityX))

#plot(dataTable$Entropy~dataTable$Entities, log="y", type="n", xlim=c(xRange[1], xRange[2]), ylim=c(100000, 10000000), ann=FALSE, axes=FALSE)
plot(dataTable$Anax~dataTable$Entities, log="y", type="n", xlim=c(xRange[1], xRange[2]), ylim=c(100000, 10000000), ann=FALSE, axes=FALSE)
axis(1, at=seq(from=xRange[1], to=xRange[2], by=1), lwd=2)
#yTicks <- seq()
axis(2, lwd=2)
#axis(2, at=seq(from=0, to=yRange[2], by=100), lwd=2)
#lines(dataTable$Entropy~dataTable$Entities, lwd=2, col="blue")
points(dataTable$Entropy~dataTable$Entities, type="o", lwd=2, col="blue", pch=19)
#lines(dataTable$Anax~dataTable$Entities, lwd=2, col="green")
points(dataTable$Anax~dataTable$Entities, type="o", lwd=2, col="green", pch=15)
#lines(dataTable$Artemis~dataTable$Entities, lwd=2, col="orange")
points(dataTable$Artemis~dataTable$Entities, type="o", lwd=2, col="orange", pch=18)
#lines(dataTable$EntityX~dataTable$Entities, lwd=2, col="red")
points(dataTable$EntityX~dataTable$Entities, type="o", lwd=2, col="red", pch=17)

legend("bottomright", pch=c(19, 15, 18, 17), lty=c(1,1,1,1), legend=c("Entropy", "Anax", "ArtemisCpp", "EntityX"), col=c("blue","green","orange","red"), pt.lwd=2, box.lwd=2)
title(main="Entity change", xlab="Entities changed [%]", ylab="Time per iteration [ns]")

