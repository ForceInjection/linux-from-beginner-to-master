#!/bin/bash

# Initialize a temporary file to store the results
temp_file=$(mktemp)

# Loop through all processes in the /proc directory
for pid in $(ls /proc | grep -E '^[0-9]+$'); do

    echo -ne "."

    # Check if the process has an fd directory
    fd_dir="/proc/$pid/fd"
    if [ -d "$fd_dir" ]; then
        inotify_instance_count=0
        inotify_watch_count=0
        
        # Get the UID of the process
        uid=$(awk '/^Uid:/ {print $2}' /proc/$pid/status)
        user=$(getent passwd "$uid" | cut -d: -f1)

        # If user is empty, set it to UID
        if [ -z "$user" ]; then
            user="$uid"
        fi

        # Loop through all file descriptors in the fd directory
        for fd in "$fd_dir"/*; do
            fdinfo="/proc/$pid/fdinfo/$(basename "$fd")"
            
            # Check if fdinfo file contains inotify instances
            if grep -q 'inotify' "$fdinfo" 2>/dev/null; then

                inotify_instance_count=$((inotify_instance_count + 1))
                
                # Count the number of inotify watches
                count=$(grep '^inotify' "$fdinfo" | wc -l)
                inotify_watch_count=$((inotify_watch_count + count))
            fi
        done

        # If inotify instances or watches are found, record the PID, counts, and command name (comm)
        if [ "$inotify_instance_count" -gt 0 ] || [ "$inotify_watch_count" -gt 0 ]; then
            comm=$(cat /proc/$pid/comm 2>/dev/null)
            echo "$user $pid $inotify_instance_count $inotify_watch_count $comm" >> "$temp_file"
        fi
    fi
done

echo -e "\n"

# Print the header
printf "%-15s %-10s %-20s %-20s %-s\n" "USER" "PID" "INOTIFY INSTANCES" "INOTIFY WATCHES" "COMMAND"

# Sort by inotify watch count in descending order and output the results
sort -k4 -n -r "$temp_file" | while read -r line; do
    user=$(echo "$line" | awk '{print $1}')
    pid=$(echo "$line" | awk '{print $2}')
    instance_count=$(echo "$line" | awk '{print $3}')
    watch_count=$(echo "$line" | awk '{print $4}')
    comm=$(echo "$line" | awk '{print $5}')
    printf "%-15s %-10s %-20s %-20s %-s\n" "$user" "$pid" "$instance_count" "$watch_count" "$comm"
done

# Remove the temporary file
rm -f "$temp_file"