#!/bin/bash

# Function to execute the program in the background
execute_program() {
    ./tracer execute -u "sleep 10"
}

# Launch 20 instances of the program in the background
for i in {1..20}; do
    execute_program &
done

# Wait for all background processes to finish
wait
