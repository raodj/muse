library(dplyr)
library(ggplot2)

# Function to check for matching row elements in 2 data frames
matching <- function(x, data){
  for(i in 1:nrow(data)) {
    if(x[2] == data[i,]$X..delay &&
       x[3] == data[i,]$X..selfEvents.
       && x[4] == data[i,]$X..eventsPerAgent) { 
      return (data[i,]$runtime_mean)
    }
  }
}

#---------------------------------------(3tHeap ph3)--------------------------------------------#

#Read serial/parallel ph3 configuration (3tHeap data) 
ph3_3tHeap_serial <- read.csv("sim_data/seq/ph3/ph3_3tHeap_runtime_stats.csv")
ph3_3tHeap_parallel <- read.csv("sim_data/parallel/ph3_3tHeap_parallel_stats.csv")


#Filter out non-applicable rows and columns
#Certain configurations in serial were not performed in parallel
#Only care about runtime
ph3_3tHeap_serial <- ph3_3tHeap_serial[2:6]
ph3_3tHeap_parallel <-ph3_3tHeap_parallel[2:6]
ph3_3tHeap <-rbind(ph3_3tHeap_serial, ph3_3tHeap_parallel)
ph3_3tHeap <-ph3_3tHeap[!(ph3_3tHeap$X..delay %in% c(2, 5)), ] 
ph3_3tHeap <-ph3_3tHeap[!(ph3_3tHeap$X..eventsPerAgent %in% c(1, 15, 20)), ]
ph3_3tHeap <-ph3_3tHeap[order(ph3_3tHeap$X..delay,
                              ph3_3tHeap$X..selfEvents.,
                              ph3_3tHeap$X..eventsPerAgent), ]

#Rename column name of runtime mean
colnames(ph3_3tHeap)[5] <- "para_runtime_mean"

#Add mean serial runtime to parallel data
ph3_3tHeap$serial_runtime_mean <-apply(ph3_3tHeap, 1, function(X) matching(X, ph3_3tHeap_serial))

#Compute SpeedUp
ph3_3tHeap$SpeedUp <-apply(ph3_3tHeap, 1, function(x) x[6]/x[5])

#Compute Efficiency
ph3_3tHeap$Efficiency <-apply(ph3_3tHeap, 1, function(x) x[6]/(x[5]*x[1]))

#Compute Total overhead
ph3_3tHeap$TotOverhead <-apply(ph3_3tHeap, 1, function(x) (x[5]*x[1])-x[6])

#---------------------------------------(2tladderQ ph3)--------------------------------------------#

#Read serial/parallel ph3 configuration (3tHeap data) 
ph3_2tladderQ_serial <- read.csv("sim_data/seq/ph3/ph3_2tladderQ_runtime_stats.csv")
ph3_2tladderQ_parallel <- read.csv("sim_data/parallel/ph3_2tladderQ_parallel_stats.csv")

#Filter out non-applicable rows and columns
#Certain configurations in serial were not performed in parallel
#Only care about runtime
ph3_2tladderQ_serial <- ph3_2tladderQ_serial[2:6]
ph3_2tladderQ_parallel <-ph3_2tladderQ_parallel[2:6]
ph3_2tladderQ <-rbind(ph3_2tladderQ_serial, ph3_2tladderQ_parallel)
ph3_2tladderQ <-ph3_2tladderQ[!(ph3_2tladderQ$X..delay %in% c(2, 5)), ] 
ph3_2tladderQ <-ph3_2tladderQ[!(ph3_2tladderQ$X..eventsPerAgent %in% c(1, 15, 20)), ]
ph3_2tladderQ <-ph3_2tladderQ[order(ph3_2tladderQ$X..delay,
                              ph3_2tladderQ$X..selfEvents.,
                              ph3_2tladderQ$X..eventsPerAgent), ]

#Rename column name of runtime mean
colnames(ph3_2tladderQ)[5] <- "para_runtime_mean"

#Add mean serial runtime to parallel data
ph3_2tladderQ$serial_runtime_mean <-apply(ph3_2tladderQ, 1, function(X) matching(X, ph3_2tladderQ_serial))

#Compute SpeedUp
ph3_2tladderQ$SpeedUp <-apply(ph3_2tladderQ, 1, function(x) x[6]/x[5])

#Compute Efficiency
ph3_2tladderQ$Efficiency <-apply(ph3_2tladderQ, 1, function(x) x[6]/(x[5]*x[1]))

#Compute Total overhead
ph3_2tladderQ$TotOverhead <-apply(ph3_2tladderQ, 1, function(x) (x[5]*x[1])-x[6])

#---------------------------------------(ladderQ ph3)--------------------------------------------#

#Read serial/parallel ph3 configuration (3tHeap data) 
ph3_ladderQ_serial <- read.csv("sim_data/seq/ph3/ph3_ladderQ_runtime_stats.csv")
ph3_ladderQ_parallel <- read.csv("sim_data/parallel/ph3_ladderQ_parallel_stats.csv")


#Filter out non-applicable rows and columns
#Certain configurations in serial were not performed in parallel
#Only care about runtime
ph3_ladderQ_serial <- ph3_ladderQ_serial[2:6]
ph3_ladderQ_parallel <-ph3_ladderQ_parallel[2:6]
ph3_ladderQ <-rbind(ph3_ladderQ_serial, ph3_ladderQ_parallel)
ph3_ladderQ <-ph3_ladderQ[!(ph3_ladderQ$X..delay %in% c(2, 5)), ] 
ph3_ladderQ <-ph3_ladderQ[!(ph3_ladderQ$X..eventsPerAgent %in% c(1, 15, 20)), ]
ph3_ladderQ <-ph3_ladderQ[order(ph3_ladderQ$X..delay,
                              ph3_ladderQ$X..selfEvents.,
                              ph3_ladderQ$X..eventsPerAgent), ]

#Rename column name of runtime mean
colnames(ph3_ladderQ)[5] <- "para_runtime_mean"

#Add mean serial runtime to parallel data
ph3_ladderQ$serial_runtime_mean <-apply(ph3_ladderQ, 1, function(X) matching(X, ph3_ladderQ_serial))

#Compute SpeedUp
ph3_ladderQ$SpeedUp <-apply(ph3_ladderQ, 1, function(x) x[6]/x[5])

#Compute Efficiency
ph3_ladderQ$Efficiency <-apply(ph3_ladderQ, 1, function(x) x[6]/(x[5]*x[1]))

#Compute Total overhead
ph3_ladderQ$TotOverhead <-apply(ph3_ladderQ, 1, function(x) (x[5]*x[1])-x[6])

#---------------------------------------(3tHeap ph4)--------------------------------------------#

#Read serial/parallel ph3 configuration (3tHeap data) 
ph4_3tHeap_serial <- read.csv("sim_data/seq/ph4/ph4_3tHeap_runtime_stats.csv")
ph4_3tHeap_parallel <- read.csv("sim_data/parallel/ph4_3tHeap_parallel_stats.csv")


#Filter out non-applicable rows and columns
#Certain configurations in serial were not performed in parallel
#Only care about runtime
ph4_3tHeap_serial <- ph4_3tHeap_serial[2:6]
ph4_3tHeap_parallel <-ph4_3tHeap_parallel[2:6]
ph4_3tHeap <-rbind(ph4_3tHeap_serial, ph4_3tHeap_parallel)
ph4_3tHeap <-ph4_3tHeap[!(ph4_3tHeap$X..delay %in% c(2, 5)), ] 
ph4_3tHeap <-ph4_3tHeap[!(ph4_3tHeap$X..eventsPerAgent %in% c(1, 15, 20)), ]
ph4_3tHeap <-ph4_3tHeap[order(ph4_3tHeap$X..delay,
                              ph4_3tHeap$X..selfEvents.,
                              ph4_3tHeap$X..eventsPerAgent), ]

#Rename column name of runtime mean
colnames(ph4_3tHeap)[5] <- "para_runtime_mean"

#Add mean serial runtime to parallel data
ph4_3tHeap$serial_runtime_mean <-apply(ph4_3tHeap, 1, function(X) matching(X, ph4_3tHeap_serial))

#Compute SpeedUp
ph4_3tHeap$SpeedUp <-apply(ph4_3tHeap, 1, function(x) x[6]/x[5])

#Compute Efficiency
ph4_3tHeap$Efficiency <-apply(ph4_3tHeap, 1, function(x) x[6]/(x[5]*x[1]))

#Compute Total overhead
ph4_3tHeap$TotOverhead <-apply(ph4_3tHeap, 1, function(x) (x[5]*x[1])-x[6])

#---------------------------------------(2tladderQ ph4)--------------------------------------------#

#Read serial/parallel ph3 configuration (3tHeap data) 
ph4_2tladderQ_serial <- read.csv("sim_data/seq/ph4/ph4_2tladderQ_runtime_stats.csv")
ph4_2tladderQ_parallel <- read.csv("sim_data/parallel/ph4_2tladderQ_parallel_stats.csv")

#Filter out non-applicable rows and columns
#Certain configurations in serial were not performed in parallel
#Only care about runtime
ph4_2tladderQ_serial <- ph4_2tladderQ_serial[2:6]
ph4_2tladderQ_parallel <-ph4_2tladderQ_parallel[2:6]
ph4_2tladderQ <-rbind(ph4_2tladderQ_serial, ph4_2tladderQ_parallel)
ph4_2tladderQ <-ph4_2tladderQ[!(ph4_2tladderQ$X..delay %in% c(2, 5)), ] 
ph4_2tladderQ <-ph4_2tladderQ[!(ph4_2tladderQ$X..eventsPerAgent %in% c(1, 15, 20)), ]
ph4_2tladderQ <-ph4_2tladderQ[order(ph4_2tladderQ$X..delay,
                                    ph4_2tladderQ$X..selfEvents.,
                                    ph4_2tladderQ$X..eventsPerAgent), ]

#Rename column name of runtime mean
colnames(ph4_2tladderQ)[5] <- "para_runtime_mean"

#Add mean serial runtime to parallel data
ph4_2tladderQ$serial_runtime_mean <-apply(ph4_2tladderQ, 1, function(X) matching(X, ph4_2tladderQ_serial))

#Compute SpeedUp
ph4_2tladderQ$SpeedUp <-apply(ph4_2tladderQ, 1, function(x) x[6]/x[5])

#Compute Efficiency
ph4_2tladderQ$Efficiency <-apply(ph4_2tladderQ, 1, function(x) x[6]/(x[5]*x[1]))

#Compute Total overhead
ph4_2tladderQ$TotOverhead <-apply(ph4_2tladderQ, 1, function(x) (x[5]*x[1])-x[6])

#---------------------------------------(ladderQ ph4)--------------------------------------------#

#Read serial/parallel ph3 configuration (3tHeap data) 
ph4_ladderQ_serial <- read.csv("sim_data/seq/ph4/ph4_ladderQ_runtime_stats.csv")
ph4_ladderQ_parallel <- read.csv("sim_data/parallel/ph4_ladderQ_parallel_stats.csv")


#Filter out non-applicable rows and columns
#Certain configurations in serial were not performed in parallel
#Only care about runtime
ph4_ladderQ_serial <- ph4_ladderQ_serial[2:6]
ph4_ladderQ_parallel <-ph4_ladderQ_parallel[2:6]
ph4_ladderQ <-rbind(ph4_ladderQ_serial, ph4_ladderQ_parallel)
ph4_ladderQ <-ph4_ladderQ[!(ph4_ladderQ$X..delay %in% c(2, 5)), ] 
ph4_ladderQ <-ph4_ladderQ[!(ph4_ladderQ$X..eventsPerAgent %in% c(1, 15, 20)), ]
ph4_ladderQ <-ph4_ladderQ[order(ph4_ladderQ$X..delay,
                                ph4_ladderQ$X..selfEvents.,
                                ph4_ladderQ$X..eventsPerAgent), ]

#Rename column name of runtime mean
colnames(ph4_ladderQ)[5] <- "para_runtime_mean"

#Add mean serial runtime to parallel data
ph4_ladderQ$serial_runtime_mean <-apply(ph4_ladderQ, 1, function(X) matching(X, ph4_ladderQ_serial))

#Compute SpeedUp
ph4_ladderQ$SpeedUp <-apply(ph4_ladderQ, 1, function(x) x[6]/x[5])

#Compute Efficiency
ph4_ladderQ$Efficiency <-apply(ph4_ladderQ, 1, function(x) x[6]/(x[5]*x[1]))

#Compute Total overhead
ph4_ladderQ$TotOverhead <-apply(ph4_ladderQ, 1, function(x) (x[5]*x[1])-x[6])

#Write to csv files
write.csv(ph3_3tHeap, file="3tHeapMetricsPH3.csv", row.names = FALSE)
write.csv(ph4_3tHeap, file="3tHeapMetricsPH4.csv", row.names = FALSE)
write.csv(ph3_2tladderQ, file="2tladderQPH3.csv", row.names = FALSE)
write.csv(ph4_2tladderQ, file="2tladderQMetricsPH4.csv", row.names = FALSE)
write.csv(ph3_ladderQ, file="ladderQMetricsPH3.csv", row.names = FALSE)
write.csv(ph4_ladderQ, file="ladderQMetricsPH4.csv", row.names = FALSE)

#Plot speedup vs. # of processors for certain configs.
ggplot(data=ph3_3tHeap[1:7,], aes(x=procs, y=SpeedUp)) + geom_line() + geom_point()
ggplot(data=ph3_2tladderQ[1:7,], aes(x=procs, y=SpeedUp)) + geom_line() + geom_point()
ggplot(data=ph3_ladderQ[1:6,], aes(x=procs, y=SpeedUp)) + geom_line() + geom_point()
?geom_point()

#Plots of merged data (delay = 1, self-events = 0, eventsPerAgent = 2)
data1<-ph3_3tHeap[1:7,]
data2<-ph3_2tladderQ[1:6,]
data3<-ph3_ladderQ[1:6,]
data1$Type <-"3tHeap"
data2$Type <-"2tladderQ"
data3$Type <-"ladderQ"
z<-rbind(data1,data2,data3)
ggplot(data=z, aes(x=procs, y=SpeedUp, colour = Type)) + geom_line() + geom_point() +
  labs(x = "# Parallel processors", y = "Speedup")+ scale_fill_discrete(name = "New Legend Title") 

ggplot(data=z, aes(x=procs, y=Efficiency, colour = Type)) + geom_line() + geom_point() +
  labs(x = "# Parallel processors", y = "Efficiency")

ggplot(data=z, aes(x=procs, y=TotOverhead, colour = Type)) + geom_line() + geom_point() +
  labs(x = "# Parallel processors", y = "Overhead")

