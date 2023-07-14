#!/usr/bin/env python3

from sys import argv
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv(argv[1])

if "cores" in df.columns: # this is a "fixed matrices, increasing number of threads" result
	df.drop(columns="size", inplace=True)
	avgd = df.groupby("cores", as_index=False).mean()
	avgd.plot(x="cores", y="time")
	plt.show()
else: # this is a "fixed number of cores, increasing matrices size" result
	avgd = df.groupby("size", as_index=False).mean()
	avgd.plot(x="size", y="gflops")
	plt.show()

