
args <- commandArgs(trailingOnly = TRUE)
inputFolder <- args[1]

smTable <- read.table(file=paste(inputFolder, "/ComparisonEntropyHolder0", sep=""), sep="\t", header=TRUE)
mmTable <- read.table(file=paste(inputFolder, "/ComparisonEntropyHolder1", sep=""), sep="\t", header=TRUE)
lmTable <- read.table(file=paste(inputFolder, "/ComparisonEntropyHolder2", sep=""), sep="\t", header=TRUE)

smlTable <- read.table(file=paste(inputFolder, "/ComparisonEntropyHolder3", sep=""), sep="\t", header=TRUE)
mmlTable <- read.table(file=paste(inputFolder, "/ComparisonEntropyHolder4", sep=""), sep="\t", header=TRUE)
lmlTable <- read.table(file=paste(inputFolder, "/ComparisonEntropyHolder5", sep=""), sep="\t", header=TRUE)

slTable <- read.table(file=paste(inputFolder, "/ComparisonEntropyHolder6", sep=""), sep="\t", header=TRUE)
mlTable <- read.table(file=paste(inputFolder, "/ComparisonEntropyHolder7", sep=""), sep="\t", header=TRUE)
llTable <- read.table(file=paste(inputFolder, "/ComparisonEntropyHolder8", sep=""), sep="\t", header=TRUE)

#dataTable <- data.frame(Entities=entropyTable$Entities, Entropy=entropyTable$EntropyPerEnt, Anax=anaxTable$AnaxPerEnt, Artemis=artemisTable$ArtemisPerEnt, EntityX=entityxTable$EntityXPerEnt)
dataTable <- data.frame(Entities=smTable$Entities, 
                        sm=smTable$EntropyPerEnt, 
                        mm=mmTable$EntropyPerEnt, 
                        lm=lmTable$EntropyPerEnt, 
                        sml=smlTable$EntropyPerEnt, 
                        mml=mmlTable$EntropyPerEnt, 
                        lml=lmlTable$EntropyPerEnt, 
                        sl=slTable$EntropyPerEnt, 
                        ml=mlTable$EntropyPerEnt, 
                        ll=llTable$EntropyPerEnt)

xRange <- range(dataTable$Entities)
yRange <- range(0, 800)

pdf(file="holdersS.pdf")

plot(dataTable$sm~dataTable$Entities, type="n", xlim=c(xRange[1], xRange[2] + 1), ylim=c(yRange[1], yRange[2]), ann=FALSE, axes=FALSE)
xAt <- seq(from=10000, to=100000, by=10000)
xVal1 <- transform(xAt, Result=round(xAt / 10 ^ floor(log10(xAt)), 1))$Result
xVal2 <- transform(xAt, Result=floor(log10(xAt)))$Result
xTicks <- parse(text=paste(xVal1, "%*%10^", xVal2, sep=""))
axis(1, at=xAt, labels=xTicks, lwd=2)
axis(2, lwd=2)
points(dataTable$sm~dataTable$Entities, type="o", lwd=2, col="blue", pch=19)
points(dataTable$sml~dataTable$Entities, type="o", lwd=2, col="green", pch=15)
points(dataTable$sl~dataTable$Entities, type="o", lwd=2, col="orange", pch=18)

legend("topright", pch=c(19, 15, 18), lty=c(1,1,1), legend=c("Map", "MapList", "List"), col=c("blue","green","orange"), pt.lwd=2, box.lwd=2, cex=1.5)
title(main="Small component (16B) - sequential", xlab="Number of entities", ylab="Time per entity [ns]")

pdf(file="holdersM.pdf")

plot(dataTable$sm~dataTable$Entities, type="n", xlim=c(xRange[1], xRange[2] + 1), ylim=c(yRange[1], yRange[2]), ann=FALSE, axes=FALSE)
xAt <- seq(from=10000, to=100000, by=10000)
xVal1 <- transform(xAt, Result=round(xAt / 10 ^ floor(log10(xAt)), 1))$Result
xVal2 <- transform(xAt, Result=floor(log10(xAt)))$Result
xTicks <- parse(text=paste(xVal1, "%*%10^", xVal2, sep=""))
axis(1, at=xAt, labels=xTicks, lwd=2)
axis(2, lwd=2)
points(dataTable$mm~dataTable$Entities, type="o", lwd=2, col="blue", pch=19)
points(dataTable$mml~dataTable$Entities, type="o", lwd=2, col="green", pch=15)
points(dataTable$ml~dataTable$Entities, type="o", lwd=2, col="orange", pch=18)

#legend("topright", pch=c(19, 15, 18), lty=c(1,1,1), legend=c("Map", "MapList", "List"), col=c("blue","green","orange"), pt.lwd=2, box.lwd=2)
title(main="Medium component (64B) - sequential", xlab="Number of entities", ylab="Time per entity [ns]")

pdf(file="holdersL.pdf")

plot(dataTable$sm~dataTable$Entities, type="n", xlim=c(xRange[1], xRange[2] + 1), ylim=c(yRange[1], yRange[2]), ann=FALSE, axes=FALSE)
xAt <- seq(from=10000, to=100000, by=10000)
xVal1 <- transform(xAt, Result=round(xAt / 10 ^ floor(log10(xAt)), 1))$Result
xVal2 <- transform(xAt, Result=floor(log10(xAt)))$Result
xTicks <- parse(text=paste(xVal1, "%*%10^", xVal2, sep=""))
axis(1, at=xAt, labels=xTicks, lwd=2)
axis(2, lwd=2)
points(dataTable$lm~dataTable$Entities, type="o", lwd=2, col="blue", pch=19)
points(dataTable$lml~dataTable$Entities, type="o", lwd=2, col="green", pch=15)
points(dataTable$ll~dataTable$Entities, type="o", lwd=2, col="orange", pch=18)

#legend("topright", pch=c(19, 15, 18), lty=c(1,1,1), legend=c("Map", "MapList", "List"), col=c("blue","green","orange"), pt.lwd=2, box.lwd=2)
title(main="Large component (128B) - sequential", xlab="Number of entities", ylab="Time per entity [ns]")

