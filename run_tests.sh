#!/bin/bash

# Create results directory with timestamp
timestamp=$(date +"%Y%m%d_%H%M%S")
results_dir="result_tests/${timestamp}"
mkdir -p "$results_dir"

# Create a metadata file
metadata_file="$results_dir/test_metadata.txt"
echo "Bank System Tests - $(date)" > "$metadata_file"
echo "System Information:" >> "$metadata_file"
echo "OS: $(uname -a)" >> "$metadata_file"
echo "Compiler: $(gcc --version | head -n 1)" >> "$metadata_file"
echo "----------------------------------------" >> "$metadata_file"

# Compile the program
echo "Compiling program..."
gcc -o bank_program main.c bank.c client.c server.c worker.c request_queue.c -pthread
if [ $? -ne 0 ]; then
    echo "Compilation failed! Exiting..."
    exit 1
fi

# Function to run test and save results
run_test() {
    workers=$1
    clients=$2
    client_sleep=$3
    op_sleep=$4
    duration=$5
    test_name="test_w${workers}_c${clients}_d${duration}"
    output_file="$results_dir/${test_name}.log"
    
    # Log test configuration
    echo "Running test: $test_name"
    echo "Test Configuration:" > "$output_file"
    echo "- Workers: $workers" >> "$output_file"
    echo "- Clients: $clients" >> "$output_file"
    echo "- Client sleep: $client_sleep ms" >> "$output_file"
    echo "- Operation sleep: $op_sleep ms" >> "$output_file"
    echo "- Duration: $duration seconds" >> "$output_file"
    echo "----------------------------------------" >> "$output_file"
    
    # Run the test and capture output
    start_time=$(date +%s)
    ./bank_program $workers $clients $client_sleep $op_sleep $duration >> "$output_file" 2>&1
    end_time=$(date +%s)
    
    # Add execution time to log
    echo "----------------------------------------" >> "$output_file"
    echo "Test completed in $((end_time - start_time)) seconds" >> "$output_file"
    
    # Add test summary to metadata
    echo "Test: $test_name" >> "$metadata_file"
    echo "Duration: $((end_time - start_time)) seconds" >> "$metadata_file"
    echo "----------------------------------------" >> "$metadata_file"
    
    echo "Test completed. Results saved to ${output_file#$results_dir/}"
    echo "----------------------------------------"
}

# Create summary directory for processed results
summary_dir="$results_dir/summary"
mkdir -p "$summary_dir"

echo "Starting tests..."

# Basic tests
echo "Running basic tests..."
run_test 2 3 100 50 1
# run_test 4 6 100 50 10
# run_test 8 12 100 50 10

# # Load scaling tests
# echo "Running load scaling tests..."
# run_test 4 4 50 25 20
# run_test 4 8 50 25 20
# run_test 4 16 50 25 20

# # Worker scaling tests
# echo "Running worker scaling tests..."
# run_test 2 8 20 10 30
# run_test 4 8 20 10 30
# run_test 8 8 20 10 30

# # Stress tests
# echo "Running stress tests..."
# run_test 8 16 10 5 60
# run_test 16 32 10 5 60

# Analysis
echo "Generating analysis..."
analysis_file="$summary_dir/analysis.txt"
echo "Test Results Analysis - $(date)" > "$analysis_file"
echo "----------------------------------------" >> "$analysis_file"

for log_file in "$results_dir"/*.log; do
    if [ -f "$log_file" ]; then
        test_name=$(basename "$log_file" .log)
        echo "Analysis for $test_name:" >> "$analysis_file"
        echo "Total operations: $(grep "operações" "$log_file" | wc -l)" >> "$analysis_file"
        echo "Final balances:" >> "$analysis_file"
        tail -n 10 "$log_file" >> "$analysis_file"
        echo "----------------------------------------" >> "$analysis_file"
    fi
done

# Create a simple HTML report
html_report="$summary_dir/report.html"
cat > "$html_report" << HTML
<!DOCTYPE html>
<html>
<head>
    <title>Bank System Test Results - $timestamp</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        .test-result { margin: 20px 0; padding: 10px; border: 1px solid #ccc; }
        .metadata { background-color: #f5f5f5; padding: 10px; }
    </style>
</head>
<body>
    <h1>Bank System Test Results</h1>
    <div class="metadata">
        <h2>Test Information</h2>
        <pre>$(cat "$metadata_file")</pre>
    </div>
    <h2>Test Results</h2>
HTML

for log_file in "$results_dir"/*.log; do
    if [ -f "$log_file" ]; then
        test_name=$(basename "$log_file" .log)
        echo "<div class='test-result'>" >> "$html_report"
        echo "<h3>$test_name</h3>" >> "$html_report"
        echo "<pre>" >> "$html_report"
        cat "$log_file" >> "$html_report"
        echo "</pre>" >> "$html_report"
        echo "</div>" >> "$html_report"
    fi
done

echo "</body></html>" >> "$html_report"

echo "Testing completed!"
echo "Results directory: $results_dir"
echo "Summary directory: $summary_dir"
echo "View the HTML report at: $html_report"