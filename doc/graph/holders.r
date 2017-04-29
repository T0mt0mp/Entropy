
args <- commandArgs(trailingOnly = TRUE)
inputFolder <- args[1]

smTable <- read.table(file=paste(inputFolder, "/ComparisonEntropySpec0", sep=""), sep="\t", header=TRUE)
mmTable <- read.table(file=paste(inputFolder, "/ComparisonEntropySpec1", sep=""), sep="\t", header=TRUE)
lmTable <- read.table(file=paste(inputFolder, "/ComparisonEntropySpec2", sep=""), sep="\t", header=TRUE)

smlTable <- read.table(file=paste(inputFolder, "/ComparisonEntropySpec3", sep=""), sep="\t", header=TRUE)
mmlTable <- read.table(file=paste(inputFolder, "/ComparisonEntropySpec4", sep=""), sep="\t", header=TRUE)
lmlTable <- read.table(file=paste(inputFolder, "/ComparisonEntropySpec5", sep=""), sep="\t", header=TRUE)

slTable <- read.table(file=paste(inputFolder, "/ComparisonEntropySpec6", sep=""), sep="\t", header=TRUE)
mlTable <- read.table(file=paste(inputFolder, "/ComparisonEntropySpec7", sep=""), sep="\t", header=TRUE)
llTable <- read.table(file=paste(inputFolder, "/ComparisonEntropySpec8", sep=""), sep="\t", header=TRUE)

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
yRange <- range(200, 400)

pdf(file="holdersS.pdf")

plot(dataTable$sm~dataTable$Entities, type="n", xlim=c(xRange[1], xRange[2] + 1), ylim=c(yRange[1], yRange[2]), ann=FALSE, axes=FALSE)
axis(1, at=seq(from=xRange[1], to=xRange[2], by=5000), lwd=2)
axis(2, lwd=2)
points(dataTable$sm~dataTable$Entities, type="o", lwd=2, col="blue", pch=19)
points(dataTable$sml~dataTable$Entities, type="o", lwd=2, col="green", pch=15)
points(dataTable$sl~dataTable$Entities, type="o", lwd=2, col="orange", pch=18)

legend("bottomright", pch=c(19, 15, 18), lty=c(1,1,1), legend=c("Map", "MapList", "List"), col=c("blue","green","orange"), pt.lwd=2, box.lwd=2)
title(main="Small component (16B)", xlab="Number of entities", ylab="Time per entity [ns]")

pdf(file="holdersM.pdf")

plot(dataTable$sm~dataTable$Entities, type="n", xlim=c(xRange[1], xRange[2] + 1), ylim=c(yRange[1], yRange[2]), ann=FALSE, axes=FALSE)
axis(1, at=seq(from=xRange[1], to=xRange[2], by=5000), lwd=2)
axis(2, lwd=2)
points(dataTable$mm~dataTable$Entities, type="o", lwd=2, col="blue", pch=19)
points(dataTable$mml~dataTable$Entities, type="o", lwd=2, col="green", pch=15)
points(dataTable$ml~dataTable$Entities, type="o", lwd=2, col="orange", pch=18)

legend("bottomright", pch=c(19, 15, 18), lty=c(1,1,1), legend=c("Map", "MapList", "List"), col=c("blue","green","orange"), pt.lwd=2, box.lwd=2)
title(main="Medium component (64B)", xlab="Number of entities", ylab="Time per entity [ns]")

pdf(file="holdersL.pdf")

plot(dataTable$sm~dataTable$Entities, type="n", xlim=c(xRange[1], xRange[2] + 1), ylim=c(yRange[1], yRange[2]), ann=FALSE, axes=FALSE)
axis(1, at=seq(from=xRange[1], to=xRange[2], by=5000), lwd=2)
axis(2, lwd=2)
points(dataTable$lm~dataTable$Entities, type="o", lwd=2, col="blue", pch=19)
points(dataTable$lml~dataTable$Entities, type="o", lwd=2, col="green", pch=15)
points(dataTable$ll~dataTable$Entities, type="o", lwd=2, col="orange", pch=18)

legend("bottomright", pch=c(19, 15, 18), lty=c(1,1,1), legend=c("Map", "MapList", "List"), col=c("blue","green","orange"), pt.lwd=2, box.lwd=2)
title(main="Large component (128B)", xlab="Number of entities", ylab="Time per entity [ns]")

