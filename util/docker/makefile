
TAG := latest
GITHUB_REPOSITORY := vlsi-lab/x-heep-toolchain
PROJECT_ID := ghcr.io/$(GITHUB_REPOSITORY)
IMAGE_NAME := x-heep-toolchain

docker-build:
	docker build -t gcr.io/$(PROJECT_ID)/$(IMAGE_NAME):$(TAG) .

docker-push:
	docker push gcr.io/$(PROJECT_ID)/$(IMAGE_NAME):$(TAG)

docker-run:
	docker run -it --rm gcr.io/$(PROJECT_ID)/$(IMAGE_NAME):$(TAG)

docker-pull:
	docker pull gcr.io/$(PROJECT_ID)/$(IMAGE_NAME):$(TAG)
