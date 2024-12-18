#!/bin/bash

check_container_busy() {
    # Check if a container is busy by monitoring CPU usage
    container=$1
    usage=$(docker stats --no-stream --format "{{.CPUPerc}}" $container 2>/dev/null | sed 's/%//')
    if [ -z "$usage" ]; then
        echo "$container is not running"
        echo "idle"
        return
    fi
    usage=${usage%%.*}  # Convert to integer
    if [ "$usage" -gt 80 ]; then  # Assume >80% CPU usage is "busy"
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
    docker stop $container && docker rm $container
}

update_container() {
    container=$1
    echo "Updating $container"
    docker pull andrey509/worker:latest
    docker stop $container && docker rm $container
    docker run -d --cpuset-cpus="$(docker inspect --format '{{.HostConfig.CpusetCpus}}' $container)" --name "$container" andrey509/worker:latest
}

get_running_containers_count() {
    # Get the count of running containers srv1, srv2, srv3
    docker ps --filter "name=srv1" --filter "name=srv2" --filter "name=srv3" -q | wc -l
}

# Main logic
while true; do
    echo "Starting new iteration of the main loop"

    # Check srv1
    if docker ps --filter "name=srv1" | grep -q "srv1"; then
        echo "srv1 is running"
        if [ "$(check_container_busy srv1)" == "busy" ]; then
            echo "srv1 is busy"
            if ! docker ps --filter "name=srv2" | grep -q "srv2"; then
                echo "srv2 is not running, launching it"
                launch_container srv2 1
            fi
        fi
    else
        echo "srv1 is not running, launching it"
        launch_container srv1 0
    fi

    # Check srv2
    if docker ps --filter "name=srv2" | grep -q "srv2"; then
        echo "srv2 is running"
        if [ "$(check_container_busy srv2)" == "busy" ]; then
            echo "srv2 is busy"
            if ! docker ps --filter "name=srv3" | grep -q "srv3"; then
                echo "srv3 is not running, launching it"
                launch_container srv3 2
            fi
        elif [ "$(check_container_busy srv2)" == "idle" ]; then
            echo "srv2 is idle, terminating it"
            terminate_container srv2
        fi
    fi

    # Check srv3
    if docker ps --filter "name=srv3" | grep -q "srv3"; then
        echo "srv3 is running"
        if [ "$(check_container_busy srv3)" == "idle" ]; then
            echo "srv3 is idle, terminating it"
            terminate_container srv3
        fi
    fi

    # Check the number of running containers
    running_containers_count=$(get_running_containers_count)

    # Update containers if a new version is available and at least two containers are running
    if [ "$running_containers_count" -ge 2 ]; then
        echo "There are $running_containers_count containers running, proceeding to update"
        if docker pull andrey509/worker:latest | grep -q "Downloaded newer image"; then
            for container in srv1 srv2 srv3; do
                if docker ps --filter "name=$container" | grep -q "$container"; then
                    update_container $container
                fi
            done
        fi
    else
        echo "Only $running_containers_count container(s) are running, skipping update"
    fi

    sleep 120  # Check every 2 minutes
done
