def calculate_accuracy(file1_path, file2_path):
    try:
        with open(file1_path, 'r') as file1, open(file2_path, 'r') as file2:
            # Read and split the contents of both files
            data1 = file1.read().strip().split(", ")
            data2 = file2.read().strip().split(", ")

            # Ensure both files have the same length
            if len(data1) != len(data2):
                raise ValueError("Files have different lengths.")

            # Calculate accuracy
            total = len(data1)
            print(f"Total: {total}")
            matches = sum(1 for a, b in zip(data1, data2) if a == b)
            accuracy = (matches / total) * 100

            return accuracy
    except Exception as e:
        print(f"Error: {e}")
        return None


# Example usage
file1_path = "./input_arr.txt"
file2_path = "./output_covert"

accuracy = calculate_accuracy(file1_path, file2_path)
if accuracy is not None:
    print(f"Accuracy: {accuracy:.2f}%")