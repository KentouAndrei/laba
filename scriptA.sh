#!/bin/bash

check_container_busy() {
    # Check if a container is busy by monitoring CPU usage
    container=$1
    usage=$(docker stats --no-stream --format "{{.CPUPerc}}" $container | sed 's/%//')
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

# Main logic
while true; do
    # Check srv1
    if docker ps --filter "name=srv1" | grep -q "srv1"; then
        if [ "$(check_container_busy srv1)" == "busy" ]; then
            if ! docker ps --filter "name=srv2" | grep -q "srv2"; then
                launch_container srv2 1
            fi
        fi
    else
        launch_container srv1 0
    fi

    # Check srv2
    if docker ps --filter "name=srv2" | grep -q "srv2"; then
        if [ "$(check_container_busy srv2)" == "busy" ]; then
            if ! docker ps --filter "name=srv3" | grep -q "srv3"; then
                launch_container srv3 2
            fi
        elif [ "$(check_container_busy srv2)" == "idle" ]; then
            terminate_container srv2
        fi
    fi

    # Check srv3
    if docker ps --filter "name=srv3" | grep -q "srv3"; then
        if [ "$(check_container_busy srv3)" == "idle" ]; then
            terminate_container srv3
        fi
    fi

    # Update containers if a new version is available
    if docker pull andrey509/worker:latest | grep -q "Downloaded newer image"; then
        for container in srv1 srv2 srv3; do
            if docker ps --filter "name=$container" | grep -q "$container"; then
                update_container $container
            fi
        done
    fi

    sleep 120  # Check every 2 minutes
done
