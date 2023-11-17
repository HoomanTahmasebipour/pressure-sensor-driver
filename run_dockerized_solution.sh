# Set variables
IMAGE_TAG="pressure_sensor_driver"
CONTAINER_NAME="pressure_sensor_driver"

# Check if a container with the same name already exists
if [ "$(docker ps -a -q -f "name=${CONTAINER_NAME}")" ]; then
    # Stop and remove the existing container
    echo "Stopping and removing existing container with name: $CONTAINER_NAME"
    docker stop $CONTAINER_NAME
    docker rm $CONTAINER_NAME
fi

# Build the Docker image
echo "Building Docker image with tag: $IMAGE_TAG"
docker build -t $IMAGE_TAG .

# Run the Docker container and attach the terminal to the instance
echo "Running new container with name: $CONTAINER_NAME"
docker run -it --name $CONTAINER_NAME $IMAGE_TAG