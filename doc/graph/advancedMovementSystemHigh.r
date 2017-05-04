
pdf(file="advancedMovementSystemHighGraph.pdf")

args <- commandArgs(trailingOnly = TRUE)
inputFolder <- args[1]

entropyTable <- read.table(file=paste(inputFolder, "/ComparisonEntropy3", sep=""), sep="\t", header=TRUE)
entropy2Table <- read.table(file=paste(inputFolder, "/ComparisonEntropySpec6", sep=""), sep="\t", header=TRUE)
entropy4Table <- read.table(file=paste(inputFolder, "/ComparisonEntropySpec7", sep=""), sep="\t", header=TRUE)
anaxTable <- read.table(file=paste(inputFolder, "/ComparisonAnax3", sep=""), sep="\t", header=TRUE)
artemisTable <- read.table(file=paste(inputFolder, "/ComparisonArtemis3", sep=""), sep="\t", header=TRUE)
entityxTable <- read.table(file=paste(inputFolder, "/ComparisonEntityx3", sep=""), sep="\t", header=TRUE)

#dataTable <- data.frame(Entities=entropyTable$Entities, Entropy=entropyTable$EntropyPerEnt, Anax=anaxTable$AnaxPerEnt, Artemis=artemisTable$ArtemisPerEnt, EntityX=entityxTable$EntityXPerEnt)
dataTable <- data.frame(Entities=entropyTable$Change, Entropy=entropyTable$Entropy, Entropy2=entropy2Table$Entropy, Entropy4=entropy4Table$Entropy, Anax=anaxTable$Anax, Artemis=artemisTable$Artemis, EntityX=entityxTable$EntityX)

xRange <- range(dataTable$Entities)
yRange <- range(0, max(dataTable$Entropy, dataTable$Anax, dataTable$Artemis, dataTable$EntityX))

plot(dataTable$Entropy~dataTable$Entities, type="n", xlim=c(xRange[1], xRange[2] + 1), ylim=c(0, 43000000), ann=FALSE, axes=FALSE)
axis(1, at=seq(from=xRange[1], to=xRange[2], by=5), lwd=2)
#yTicks <- seq()
#axis(2, lwd=2)
yAt <- seq(from=0, to=40000000, by=10000000)
yVal1 <- transform(yAt, Result=round(yAt / 10 ^ floor(log10(yAt)), 1))$Result
yVal2 <- transform(yAt, Result=floor(log10(yAt)))$Result
yTicks <- parse(text=paste(yVal1, "%*%10^", yVal2, sep=""))
yTicks[1][1] <- 0
axis(2, at=yAt, labels=yTicks, lwd=2)
#axis(2, at=seq(from=0, to=yRange[2], by=100), lwd=2)
#lines(dataTable$Entropy~dataTable$Entities, lwd=2, col="blue")
points(dataTable$Entropy~dataTable$Entities, type="o", lwd=2, col="blue", pch=19)
points(dataTable$Entropy2~dataTable$Entities, type="o", lwd=2, col="darkorchid1", pch=10)
points(dataTable$Entropy4~dataTable$Entities, type="o", lwd=2, col="darkred", pch=13)
#lines(dataTable$Anax~dataTable$Entities, lwd=2, col="green")
points(dataTable$Anax~dataTable$Entities, type="o", lwd=2, col="green", pch=15)
#lines(dataTable$Artemis~dataTable$Entities, lwd=2, col="orange")
points(dataTable$Artemis~dataTable$Entities, type="o", lwd=2, col="orange", pch=18)
#lines(dataTable$EntityX~dataTable$Entities, lwd=2, col="red")
points(dataTable$EntityX~dataTable$Entities, type="o", lwd=2, col="red", pch=17)

legend("topright", pch=c(19, 10, 13, 15, 18, 17), lty=c(1,1,1,1,1,1), legend=c("Entropy", "Entropy 2 threads", "Entropy 4 threads", "Anax", "ArtemisCpp", "EntityX"), col=c("blue","darkorchid1","darkred","green","orange","red"), pt.lwd=2, box.lwd=2)
title(main="Entity change", xlab="Entities changed [%]", ylab="Time per iteration [ns]")

