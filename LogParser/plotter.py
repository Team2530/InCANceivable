import matplotlib.pyplot as plt
import numpy as np

voltages = [eval(f"({l})")
            for l in open("voltages.csv", 'r').readlines()]
plt.plot([v[0] for v in voltages], [v[1] for v in voltages])
plt.show()
