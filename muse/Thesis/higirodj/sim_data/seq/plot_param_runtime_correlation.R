
#' correlation matrix chart
#' 
#' Visualization of a Correlation Matrix. On top the (absolute) value of the
#' correlation plus the result of the cor.test as stars. On bottom, the
#' bivariate scatterplots, with a fitted line
#' 
#' 
#' @param R data for the x axis, can take matrix,vector, or timeseries
#' @param histogram TRUE/FALSE whether or not to display a histogram
#' @param method a character string indicating which correlation coefficient
#'           (or covariance) is to be computed.  One of "pearson"
#'           (default), "kendall", or "spearman", can be abbreviated.
#' @param \dots any other passthru parameters into \code{\link{pairs}}
#' @note based on plot at
#' \url{http://addictedtor.free.fr/graphiques/sources/source_137.R}
#' @author Peter Carl
#' @seealso \code{\link{table.Correlation}}
###keywords ts multivariate distribution models hplot
#' @examples
#' 
#' data(managers)
#' chart.Correlation(managers[,1:8], histogram=TRUE, pch="+")
#' 
#' @export

library("PerformanceAnalytics")

plot_correlation <-
function (R, histogram = TRUE, method=c("pearson", "kendall", "spearman"), ...)
{ # @author R Development Core Team
  # @author modified by Peter Carl
    # Visualization of a Correlation Matrix. On top the (absolute) value of the
    # correlation plus the result of the cor.test as stars. On botttom, the
    # bivariate scatterplots, with a fitted line

    x = checkData(R, method="matrix")
    
    if(missing(method)) method=method[1] #only use one

    # Published at http://addictedtor.free.fr/graphiques/sources/source_137.R
    panel.cor <- function(x, y, digits=2, prefix="", use="pairwise.complete.obs", method, cex.cor, ...)
    {
        usr <- par("usr"); on.exit(par(usr))
        par(usr = c(0, 1, 0, 1))
        r <- cor(x, y, use=use, method=method)
        txt <- format(c(r, 0.123456789), digits=digits)[1]
        txt <- paste(prefix, txt, sep="")
        if(missing(cex.cor)) cex <- 0.8/strwidth(txt)

        test <- cor.test(x,y, method=method)
        # borrowed from printCoefmat
        # Signif <- symnum(test$p.value, corr = FALSE, na = FALSE,
        #            cutpoints = c(0, 0.001, 0.01, 0.05, 0.1, 1),
        #            symbols = c("***", "**", "*", ".", " "))
        Signif <- sprintf("p = %.3f", test$p.value)
        # Add color for recantlge
        colfunc <- colorRampPalette(c("#ff5500", "#cccccc", "#55ff00"))
        rectCol = colfunc(40)[20 + r * 20]
        rect(par("usr")[1], par("usr")[3], par("usr")[2], par("usr")[4], col=rectCol)
        # MG: add abs here and also include a 30% buffer for small numbers
        # text(0.5, 0.5, txt, cex = cex * (abs(r) + .3) / 1.3)
        text(0.5, 0.6, txt, cex=0.8)
        # text(.8, .8, Signif, cex=cex, col=2)
        text(.5, 0.4, Signif, cex=0.65)
    }
    f <- function(t) {
        dnorm(t, mean=mean(x), sd=sd.xts(x) )
    }
    hist.panel = function (x, ...) {
        # par(new = TRUE, ps=28)
      opar=par(new = TRUE, ps=18, cex.axis=0.75)
        hist(x,
             col = '#b3d9ff',
             probability = FALSE,
             axes = FALSE,
             main = "",
             breaks = "FD", border="#80bfff",
             cex.axis=0.5, cex.lab=0.5, cex.main=0.5)
        # lines(density(x, na.rm=TRUE),
        #      col = "#3399ff",
        #      lwd = 2, col.main="red")
        # lines(f, col="blue", lwd=1, lty=1) how to add gaussian normal overlay?
        rug(x)
      }
    panel.smooth<-function (x, y, col = "blue", bg = NA, pch = 18,
                            cex = 1.25, col.smooth = "red", span = 2/3, iter = 3, ...)
      {
        points(x, y, pch = pch, col = col, bg = bg, cex = cex)
        ok <- is.finite(x) & is.finite(y)
        if (any(ok))
          lines(stats::lowess(x[ok], y[ok], f = span, iter = iter),
                lwd=2, col = col.smooth, ...)
    }
    # Draw the chart
    grayScale <- colorRampPalette(c("#333333", "#999999"))
    darkColors = c("#006600", "#996600", "#006699", "660066", "#666666")
    if(histogram)
      pairs(x, gap=0, lower.panel=panel.smooth, upper.panel=panel.cor, diag.panel=hist.panel, method=method, pch=19, col=grayScale(5), ...)
    else
        pairs(x, gap=0, lower.panel=panel.smooth, upper.panel=panel.cor, method=method, ...) 
}


saData = read.csv('runtime_heap2tQ_ladderQ_corr.csv', header=TRUE)
pdf("runtime_heap2tQ_ladderQ_corr.pdf", width=3.5,height=3.5,paper='special', pointsize=10)
plot_correlation(saData)
dev.off()
