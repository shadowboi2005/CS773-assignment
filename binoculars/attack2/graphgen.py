import os

import matplotlib.pyplot as plt

def plot_graph_from_file(file_path, output_path="graph.png" , mode = 0):
    """
    Reads y-values from a text file, generates x-values as indices, and plots a graph.
    
    Args:
        file_path (str): Path to the text file containing y-values.
        output_path (str): Path to save the generated graph image.
    """
    if not os.path.exists(file_path):
        print(f"Error: File '{file_path}' does not exist.")
        return

    try:
        # Read y-values from the file
        if(mode == 0):
            with open(file_path, 'r') as file:
                y_values = [float(line.strip()) for line in file if line.strip()]
        else:
            with open(file_path, 'r') as file:
                lines = [float(line.strip()) for line in file if line.strip()]
                y_values = [lines[:10000] , lines[10000:20000]]    

        
        # Generate x-values as indices
        if(mode == 0):
            x_values = list(range(len(y_values)))
        else:
            x_values = list(range(len(y_values[0])))
        
        # Plot the graph
        plt.figure(figsize=(15, 10))
        if mode == 0:
            plt.plot(x_values, y_values, alpha=0.7, color='blue', label='Data Points')
        else:
            plt.scatter(x_values, y_values[0], s= 1, alpha=0.7, color='blue', label='Data Points')
            plt.scatter(x_values, y_values[1], s= 1, alpha=0.7, color='red', label='Data Points')
        plt.title("cycles per access time")
        plt.xlabel("iteration number")
        plt.ylabel("cycles")
        plt.ylim(0, 10000)
        plt.legend()
        plt.grid(True)
        
        # Save the graph as an image
        plt.savefig(output_path)
        print(f"Graph saved as '{output_path}'.")
        # plt.show()
    except Exception as e:
        print(f"Error: {e}")

# Example usage
# Replace 'data.txt' with the path to your text file containing y-values
plot_graph_from_file('cycles_only.txt' , 'cyc_only.png')
plot_graph_from_file('cycles_both_samecpu.txt' , 'cycles_both_samecpu.png')
plot_graph_from_file('cycles_both_diffcpu.txt' , 'cycles_both_diffcpu.png')
plot_graph_from_file('cycles_bus_samecpu.txt' , 'cycles_bus_samecpu.png')
plot_graph_from_file('cycles_bus_diffcpu.txt' , 'cycles_bus_diffcpu.png')

import plotly.graph_objects as go

def plot_combined_graph(file_paths, output_path="combined_graph.html" , mode=0 ):
    """
    Combines multiple plots into a single interactive graph using Plotly.
    
    Args:
        file_paths (list): List of file paths containing y-values.
        output_path (str): Path to save the combined graph as an HTML file.
    """
    fig = go.Figure()

    for file_path in file_paths:
        if not os.path.exists(file_path):
            print(f"Error: File '{file_path}' does not exist.")
            continue

        try:
            # Read y-values from the file
            with open(file_path, 'r') as file:
                y_values = [float(line.strip()) for line in file if line.strip()]
            
            # Generate x-values as indices
            x_values = list(range(len(y_values)))
            
            # Add a trace for the current file
            if mode == 1:
                fig.add_trace(go.Scatter(
                    x=x_values,
                    y=y_values[:10000],
                    mode='markers',
                    name=os.path.basename(file_path) + 'avg'
                ))
                fig.add_trace(go.Scatter(
                    x=x_values,
                    y=y_values[10000:20000],
                    mode='markers',
                    name=os.path.basename(file_path) + 'max'
                ))
            else:
                fig.add_trace(go.Scatter(
                    x=x_values,
                    y=y_values,
                    mode='markers',
                    name=os.path.basename(file_path)
                ))
        except Exception as e:
            print(f"Error processing file '{file_path}': {e}")

    # Update layout
    fig.update_layout(
        title="Combined Graph of Cycles per Access Time",
        xaxis_title="Iteration Number",
        yaxis_title="Cycles",
        yaxis=dict(range=[0, 10000]),
        legend_title="Data Source",
        template="plotly_white"
    )

    # Save the graph as an HTML file
    fig.write_html(output_path)
    print(f"Combined graph saved as '{output_path}'.")

# Example usage
file_paths = [
    'cycles_only.txt',
    'cycles_both_samecpu.txt',
    'cycles_both_diffcpu.txt',
    'cycles_bus_samecpu.txt',
    'cycles_bus_diffcpu.txt'
]
plot_combined_graph(file_paths, 'combined_graph.html')

plot_graph_from_file('pgwlk_wo_bus.txt' , 'pgwlk_wo_bus.png')
plot_graph_from_file('pgwlk_w_bus.txt' , 'pgwlk_w_bus.png')

plot_combined_graph(['pgwlk_wo_bus.txt', 'pgwlk_w_bus.txt'], 'pgwlk_combined_graph.html')

plot_graph_from_file('wo_bus.txt' , 'wo_bus.png')
plot_graph_from_file('w_bus.txt' , 'w_bus.png')
plot_combined_graph(['wo_bus.txt', 'w_bus.txt'], 'combined_graph.html')
