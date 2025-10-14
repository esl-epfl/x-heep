ROOT_DIR 			:= $(shell git rev-parse --show-toplevel)
TAG 				:= latest
GITHUB_REPOSITORY 	:= esl-epfl
IMAGE_NAME 			:= x-heep-toolchain
IMAGE_ID 			:= $(GITHUB_REPOSITORY)/$(IMAGE_NAME)

.PHONY: docker-build docker-push docker-run docker-pull

docker-build:
	docker build -t $(IMAGE_ID):$(TAG) -f ./dockerfile $(ROOT_DIR)

docker-push:
	docker push $(IMAGE_ID):$(TAG)

docker-run:
	docker run -it --rm -v $(ROOT_DIR):/workspace/x-heep $(IMAGE_ID):$(TAG)

docker-pull:
	docker pull ghcr.io/$(IMAGE_ID):$(TAG)
