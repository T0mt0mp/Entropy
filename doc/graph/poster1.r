
pdf(file="poster1.pdf")

args <- commandArgs(trailingOnly = TRUE)
inputFolder <- args[1]

entropyTable <- read.table(file=paste(inputFolder, "/Poster1", sep=""), sep="\t", header=TRUE)

#dataTable <- data.frame(Entities=entropyTable$Entities, Entropy=entropyTable$EntropyPerEnt, Anax=anaxTable$AnaxPerEnt, Artemis=artemisTable$ArtemisPerEnt, EntityX=entityxTable$EntityXPerEnt)
dataTable <- data.frame(Entities=entropyTable$Change, Entropy=entropyTable$Entropy)

xRange <- range(dataTable$Entities)
yRange <- range(0, max(dataTable$Entropy))

par(bg="#e3e2e0")
#plot(dataTable$Entropy~dataTable$Entities, log="y", type="n", xlim=c(xRange[1], xRange[2]), ylim=c(100000, 10000000), ann=FALSE, axes=FALSE)
plot(dataTable$Entropy~dataTable$Entities, type="n", xlim=c(xRange[1], xRange[2]), ylim=c(70000000, 100000000), ann=FALSE, axes=FALSE)
rect(par("usr")[1], par("usr")[3], par("usr")[2], par("usr")[4], col ="#e3e2e0")
box("outer", lwd=2)
axis(1, at=seq(from=xRange[1], to=xRange[2], by=10), lwd=2)
#yTicks <- seq()
#axis(2, lwd=2)
yAt <- seq(from=70000000, to=100000000, by=5000000)
yVal1 <- transform(yAt, Result=round(yAt / 10 ^ floor(log10(yAt)), 1))$Result
yVal2 <- transform(yAt, Result=floor(log10(yAt)))$Result
yTicks <- parse(text=paste(yVal1, "%*%10^", yVal2, sep=""))
#yTicks[1][1] <- 0
axis(2, at=yAt, labels=yTicks, lwd=2)
#axis(2, at=seq(from=0, to=yRange[2], by=100), lwd=2)
#lines(dataTable$Entropy~dataTable$Entities, lwd=2, col="blue")
points(dataTable$Entropy~dataTable$Entities, type="o", lwd=2, col="blue", pch=19)

title(main="Entity change", xlab="Entities changed [%]", ylab="Time per iteration [ns]")

