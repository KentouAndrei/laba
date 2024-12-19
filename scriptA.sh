#!/bin/bash

check_container_busy() {
    container=$1
    # Get CPU usage of the container
    usage=$(docker stats --no-stream --format "{{.CPUPerc}}" "$container" 2>/dev/null | sed 's/%//')
    if [ -z "$usage" ]; then
        echo "idle"  # Container is not active
        return
    fi
    usage=${usage%%.*}  # Convert usage to an integer
    if [ "$usage" -gt 80 ]; then
        echo "busy"
    else
        echo "idle"
    fi
}

launch_container() {
    container=$1
    core=$2
    echo "Launching $container on CPU core $core"
    docker run -d --cpuset-cpus="$core" --name "$container" andrey509/worker
}

terminate_container() {
    container=$1
    echo "Terminating $container"
    docker stop "$container" && docker rm "$container"
}

# Main loop
while true; do
    echo "Starting new iteration of the main loop"

    # Ensure srv1 is running
    if ! docker ps --filter "name=srv1" | grep -q "srv1"; then
        echo "srv1 is not running, launching it"
        launch_container srv1 0
    fi

    # Launch srv2 after 1 minute if srv1 is running
    if docker ps --filter "name=srv1" | grep -q "srv1"; then
        echo "srv1 is running, ensuring srv2 is started after 1 minute"
        if ! docker ps --filter "name=srv2" | grep -q "srv2"; then
            sleep 60  
            echo "Launching srv2"
            launch_container srv2 1
        fi
    fi

    # Launch srv3 after 1 minute if srv2 is running
    if docker ps --filter "name=srv2" | grep -q "srv2"; then
        echo "srv2 is running, ensuring srv3 is started after 1 minute"
        if ! docker ps --filter "name=srv3" | grep -q "srv3"; then
            sleep 60  
            echo "Launching srv3"
            launch_container srv3 2
        fi
    fi

    # Check and terminate srv2 if idle for 1 minute
    if docker ps --filter "name=srv2" | grep -q "srv2"; then
        echo "Checking if srv2 is idle"
        if [ "$(check_container_busy srv2)" == "idle" ]; then
            echo "srv2 is idle, waiting for 1 minute before terminating"
            sleep 60  
            # Re-check after 1 minute
            if [ "$(check_container_busy srv2)" == "idle" ]; then
                echo "srv2 is still idle after 1 minute, terminating"
                terminate_container srv2
            else
                echo "srv2 is no longer idle, skipping termination"
            fi
        fi
    else
        echo "srv2 is not running, skipping idle check"
    fi

    # Check and terminate srv3 if idle for 1 minute (only after srv2 is terminated)
    if ! docker ps --filter "name=srv2" | grep -q "srv2"; then
        if docker ps --filter "name=srv3" | grep -q "srv3"; then
            echo "Checking if srv3 is idle"
            if [ "$(check_container_busy srv3)" == "idle" ]; then
                echo "srv3 is idle, waiting for 1 minute before terminating"
                sleep 60  
                # Re-check after 1 minute
                if [ "$(check_container_busy srv3)" == "idle" ]; then
                    echo "srv3 is still idle after 1 minute, terminating"
                    terminate_container srv3
                else
                    echo "srv3 is no longer idle, skipping termination"
                fi
            fi
        else
            echo "srv3 is not running, skipping idle check"
        fi
    fi

    # Check for new version of image and update containers
    echo "Pulling latest image..."
    output=$(docker pull andrey509/worker:latest)
    echo "$output"  # Output the result for debugging

    # Check if a new image was downloaded
    if echo "$output" | grep -q "Downloaded newer image"; then
        echo "New image found, updating containers..."
        for container in srv1 srv2 srv3; do
            if docker ps --filter "name=$container" | grep -q "$container"; then
                update_container $container
            fi
        done
    else
        echo "No new image found, skipping update."
    fi

    sleep 60  
done
