ROOT_DIR 			:= $(shell git rev-parse --show-toplevel)
TAG 				?= $(shell git describe --tags --abbrev=0)
GITHUB_REPOSITORY 	:= ghcr.io/esl-epfl/x-heep
IMAGE_NAME 			:= x-heep-toolchain
IMAGE_ID 			:= $(GITHUB_REPOSITORY)/$(IMAGE_NAME)

.PHONY: docker-build docker-push docker-run docker-pull

# User targets

# Try to download the image corresponding to the current revision, else default to 'latest'
docker-pull:
	docker pull $(IMAGE_ID):$(TAG) || docker pull $(IMAGE_ID):latest

docker-run:
	docker run -it --rm -v $(ROOT_DIR):/workspace/x-heep $(IMAGE_ID):$(TAG)

# Build the image locally
docker-build: toolchain.tar.gz
	docker build -t $(IMAGE_ID):$(TAG) -f ./dockerfile $(ROOT_DIR) --build-arg toolchain_path=util/docker/$<
	$(RM) $<

toolchain.tar.gz:
	wget $$(curl -s https://api.github.com/repos/esl-epfl/x-heep/releases/latest | grep -o 'https://[^"]*ubuntu2204.tar.gz' | head -n 1) -O $@
