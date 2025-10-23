ROOT_DIR 			:= $(shell git rev-parse --show-toplevel)
GIT_SHA				:= $(shell git rev-parse --short HEAD)
TAG 				?= latest
GITHUB_REPOSITORY 	:= esl-epfl
IMAGE_NAME 			:= x-heep-toolchain
IMAGE_ID 			:= $(GITHUB_REPOSITORY)/$(IMAGE_NAME)

.PHONY: docker-build docker-push docker-run docker-pull

# User targets
docker-pull:
	docker pull ghcr.io/$(IMAGE_ID):latest

docker-run:
	docker run -it --rm -v $(ROOT_DIR):/workspace/x-heep $(IMAGE_ID):$(TAG)

# Test targets
docker-build:
	docker build -t $(IMAGE_ID):$(TAG) -f ./dockerfile $(ROOT_DIR)

# To build and publish the container to the registry, launch the "Create X-HEEP
# Release" workflow from GitHub Action tab.