import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import re

# Load the data
data = pd.read_csv('cache_aliasing_data.csv')

# Try to extract the conflict page from the console output
conflict_page = None
try:
    with open('cache_aliasing_output.txt', 'r') as f:
        output = f.read()
        match = re.search(r'conflict address at 4K offset: (\d+)', output)
        if match:
            conflict_page = int(match.group(1))
except:
    # If we can't find it, we'll try to infer it
    pass

# Identify the conflict address (marked with 0 in the data)
if conflict_page is None:
    conflict_entries = data[data['access_time_cycles'] == 0]
    if not conflict_entries.empty:
        conflict_kb = conflict_entries['offset_kb'].iloc[0]
        conflict_page = conflict_kb // 4
        print(f"Inferred conflict page: {conflict_page}")
    else:
        # Just assume a default if we can't find it
        conflict_page = 42
        print(f"Using default conflict page: {conflict_page}")

# Calculate expected 4K aliasing points
aliasing_points = []
for i in range(1, 65):  # Up to 64 potential aliases
    alias_kb = (conflict_page * 4) + (i * 4)
    if alias_kb <= max(data['offset_kb']):
        aliasing_points.append(alias_kb)

# Create the plot
plt.figure(figsize=(14, 8))

# Plot access times
plt.plot(data['offset_kb'], data['access_time_cycles'], 'b-', linewidth=1)
plt.scatter(data['offset_kb'], data['access_time_cycles'],
            s=10, c='blue', alpha=0.5)

# Calculate a noise threshold to identify significant spikes
threshold = np.percentile(data['access_time_cycles'], 85)
print(f"Threshold for significant spikes: {threshold:.2f} cycles")

# Find significant spikes
spikes = data[data['access_time_cycles'] > threshold]
print(f"Found {len(spikes)} significant spikes")

# Format the plot
plt.title('4K Aliasing Detection in L1D Cache', fontsize=16)
plt.xlabel('Memory Offset (KB)', fontsize=14)
plt.ylabel('Access Time (CPU Cycles)', fontsize=14)
plt.grid(True, alpha=0.3)
plt.xlim(0, min(1024, max(data['offset_kb'])))
plt.axhline(y=threshold, color='purple', linestyle=':', alpha=0.7,
            label=f'Threshold ({threshold:.2f} cycles)')

plt.legend(loc='upper right')
plt.tight_layout(rect=[0, 0.03, 1, 0.95])
plt.savefig('4k_aliasing_plot.png', dpi=300)
plt.show()

print(f"Plot generated and saved as '4k_aliasing_plot.png'")
print(f"Analysis: {alias_text}")
