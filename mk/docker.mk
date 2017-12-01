##################################################
# Docker related logic
##################################################

docker_image_prefix := gil0mendes/
docker_image_name := initium-loader-build
docker_image_full := $(docker_image_prefix)$(docker_image_name)
docker_run := PREFIX=$(docker_image_prefix) IMAGE_NAME=$(docker_image_name) bash docker/run.sh

# Build docker image
docker_build:
	cd docker; \
	docker build -t $(docker_image_full) -f Dockerfile .

# Execute commands inside container
docker_run:
	$(docker_run) --it

# TODO: implement rule to execute a command inside Docker
