# An R script to estimate MQ gas sensors correlation curve and compute Ro, min and max Rs/Ro
#
# Copyright (c) Davide Gironi, 2016
#
# Released under GPLv3.
# Please refer to LICENSE file for licensing information.

# How to use this script:
# 1) set limits as datasheet curve ("xlim" and "ylim")
#    ex.
#      xlim = c(10, 1000)
#      ylim = c(0.1, 10)
# 2) find out datasheet curve points, and write it out (to "pointsdata")
#    each line it's a point on cartesian coordinate system
#    the useful WebPlotDigitizer app can help you extract points from the graph
#    ex.
#      pointsdata = "
#        10.052112405371744, 2.283698378106183
#        20.171602728600178, 1.8052797165878915
#        30.099224396434586, 1.5715748803154423
#        50.09267987761949, 1.3195287228519417
#        80.38812026903305, 1.1281218760133969
#        90.12665922665023, 1.0815121769656304
#        100.52112405371739, 1.0430967861855598
#        199.62996638292853, 0.8000946404902397
#      "
# 3) optional for Ro estimation: measure the sensor resistance (set it to "mres" ohm value) at a know amount of gas
#    set it to 0 if you do not need the Ro estimation
#    ex.
#      mres = 26954
# 4) optional for Ro estimation: set the know amount of gas for the resistance measure of the previous step (to "mppm")
#    set it to 0 if you do not need the Ro estimation
#    ex.
#      mppm = 392
# 5) optional for min-max Rs/Ro estimation: set the minand max amount of gas the sensor will react to (as "minppm" and "maxppm")
#    set it to 0 if you do not need the min-max Rs/Ro estimation
#    ex.
#      minppm = 10
#      maxppm = 200

library(data.table)

# Remove old variables
rm(list=ls())

# Set input values
xlim <- c(0.1, 10)
ylim <- c(0.1, 10)
minppm <- 0
maxppm <- 0
mres <- 0
mppm <- 0

# Define your points data
pointsdata <- "
198.99754597957264, 1.7595138102004522
298.15162005803353, 1.5167067975538893
394.60853506402566, 1.3855604396994872
500.2594854576871, 1.2496512917783915
597.6326350373234, 1.194339770479718
695.0089567490493, 1.1269292998582316
791.0003471609897, 1.077122568315866
885.8197927685817, 1.0295403718944587
1008.1499165586224, 0.9903874801643513
1988.4640853097526, 0.769935941147508
2995.1615842002175, 0.6809786566394687
3964.2645230672047, 0.6141447331299836
4944.789183067113, 0.5683516447879128
5971.517832054689, 0.5328067561539367
6907.193792728543, 0.5027380619540954
7946.802645643756, 0.46830680288281074
8946.66381132412, 0.46522740204279434
9965.455842977808, 0.4418282743929923
"

# Load points using fread
points <- fread(text = pointsdata, sep = ",", header = FALSE)

# Set column names
setnames(points, c("x", "y"))

# Set named list of points, and swapped list of points
x <- as.vector(points$x)
y <- as.vector(points$y)
points <- list(x = x, y = y)
pointsrev <- list(x = y, y = x)

# Estimate fit curve initial values
xfirst <- head(points$x, n = 1)
xlast <- tail(points$x, n = 1)
yfirst <- head(points$y, n = 1)
ylast <- tail(points$y, n = 1)
bstart <- log(ylast/yfirst) / log(xlast/xfirst)
astart <- yfirst / (xfirst^bstart)

# Perform the fit
fit <- nls(y ~ a * x^b, start = list(a = astart, b = bstart), data = points)

# Estimate fitrev curve initial values
xfirstrev <- head(pointsrev$x, n = 1)
xlastrev <- tail(pointsrev$x, n = 1)
yfirstrev <- head(pointsrev$y, n = 1)
ylastrev <- tail(pointsrev$y, n = 1)
bstartrev <- log(ylastrev/yfirstrev) / log(xlastrev/xfirstrev)
astartrev <- yfirstrev / (xfirstrev^bstartrev)

fitrev <- nls(y ~ a * x^b, start = list(a = astartrev, b = bstartrev), data = pointsrev)

# Plot fit curve (log-log scale)
fiteq <- function(x) coef(fit)["a"] * x^(coef(fit)["b"])
plot(points, log = "xy", col = "blue", xlab = "ppm", ylab = "Rs/Ro", xlim = xlim, ylim = ylim, panel.first = grid(equilogs = FALSE))
curve(fiteq, col = "red", add = TRUE)

# Plot fitrev curve (log-log scale)
fiteqrev <- function(x) coef(fitrev)["a"] * x^(coef(fitrev)["b"])
plot(pointsrev, log = "xy", col = "blue", xlab = "Rs/Ro", ylab = "ppm", xlim = ylim, ylim = xlim, panel.first = grid(equilogs = FALSE))
curve(fiteqrev, col = "red", add = TRUE)

# Plot fit curve (linear scale)
plot(points, col = "blue", xlab = "ppm", ylab = "Rs/Ro", panel.first = grid(equilogs = FALSE))
curve(fiteq, col = "red", add = TRUE)

# Plot fitrev curve (linear scale)
plot(pointsrev, col = "blue", xlab = "Rs/Ro", ylab = "ppm", panel.first = grid(equilogs = FALSE))
curve(fiteqrev, col = "red", add = TRUE)

# Estimate correlation function coefficients
cat("\nCorrelation function coefficients")
cat("\nEstimated a\n")
cat("  ")
cat(coef(fitrev)["a"])
cat("\nEstimated b\n")
cat("  ")
cat(coef(fitrev)["b"])
cat("\n")

# Estimate min Rs/Ro
if (minppm != 0) {
  minRsRo <- (maxppm / coef(fitrev)["a"])^(1 / coef(fitrev)["b"])
  cat("\nEstimated min Rs/Ro\n")
  cat("  ")
  cat(minRsRo)
  cat("\n")
}

# Estimate max Rs/Ro
if (maxppm != 0) {
  maxRsRo <- (minppm / coef(fitrev)["a"])^(1 / coef(fitrev)["b"])
  cat("\nEstimated max Rs/Ro\n")
  cat("  ")
  cat(maxRsRo)
  cat("\n")
}

# Estimate Ro
if (mppm != 0 && mres != 0) {
  Ro <- mres * (coef(fitrev)["a"] / mppm)^(1 / coef(fitrev)["b"])
  cat("\nEstimated Ro\n")
  cat("  ")
  cat(Ro)
  cat("\n")
}
