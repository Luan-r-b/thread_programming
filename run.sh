#!/bin/bash

# Create results directory with timestamp
timestamp=$(date +"%Y%m%d_%H%M%S")
results_dir="result_tests/${timestamp}"
mkdir -p "$results_dir"

# Create a metadata file
metadata_file="$results_dir/test_metadata.txt"
echo "Bank System Tests (~20 requests) - $(date)" > "$metadata_file"
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
    accounts=$2
    clients=$3
    client_sleep=$4
    op_sleep=$5
    duration=$6
    test_name="test_w${workers}_c${clients}_d${duration}"
    output_file="$results_dir/${test_name}.log"
    
    # Log test configuration
    echo "Running test: $test_name"
    echo "Test Configuration:" > "$output_file"
    echo "- Workers: $workers" >> "$output_file"
    echo "- Accounts: $accounts" >> "$output_file"
    echo "- Clients: $clients" >> "$output_file"
    echo "- Client sleep: $client_sleep ms" >> "$output_file"
    echo "- Operation sleep: $op_sleep ms" >> "$output_file"
    echo "- Duration: $duration seconds" >> "$output_file"
    echo "----------------------------------------" >> "$output_file"
    
    # Run the test and capture output
    start_time=$(date +%s)
    ./bank_program $workers $accounts $clients $client_sleep $op_sleep $duration >> "$output_file" 2>&1
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

# Tests designed to generate ~20 requests
# Formula: (duration * 1000) / client_sleep * num_clients ≈ 20 requests

# Test 1: 2 clients, slower rate
# 2 clients * (5 seconds * 1000ms) / 500ms sleep ≈ 20 requests
# echo "Running test 1 (2 clients, slower rate)..."
run_test 2 4 2 500 100 5

# Test 2: 4 clients, medium rate
# 4 clients * (2 seconds * 1000ms) / 400ms sleep ≈ 20 requests
# echo "Running test 2 (4 clients, medium rate)..."
run_test 2 8 4 400 100 2

# Test 3: 5 clients, faster rate
# 5 clients * (1 seconds * 1000ms) / 250ms sleep ≈ 20 requests
# echo "Running test 3 (5 clients, faster rate)..."
run_test 2 10 5 250 100 10

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