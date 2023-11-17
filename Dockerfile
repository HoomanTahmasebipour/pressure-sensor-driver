# Use an official base image with a C compiler
FROM debian:buster
LABEL authors="Hooman Tahmasebipour"
LABEL name="pressure_sensor_driver"
LABEL email="hooman.tahmasebipour@mail.utoronto.ca"

# Set the working directory inside the container
WORKDIR /usr/src/myapp

# Install gcc
RUN apt-get update && \
    apt-get install -y gcc && \
    rm -rf /var/lib/apt/lists/*

# Copy the current directory contents into the container
COPY . .

# Specify the name of your executable
ARG EXECUTABLE=read_pressure_sensor

# Compile your project
RUN gcc -o ${EXECUTABLE} *.c

# Run the compiled program
CMD ["./read_pressure_sensor"]
