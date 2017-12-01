#!/usr/bin/env bash
VOLUME=$PWD
INTERACTIVE=false

for i in "$@"
do
    case $i in
	--interactive|--it)
	    INTERACTIVE=true
	    shift
	    ;;
	-v=*|--volume=*)
            VOLUME="${i#*=}"
	    shift
	    ;;
	*)
	    ;;
    esac
done

IMAGE=$PREFIX$IMAGE_NAME
IMAGE_ID=$(docker inspect --format="{{.Id}}" "$IMAGE" | cut -c 8-19)

ARGS="--rm                                                                \
      --cap-add MKNOD                                                     \
      --cap-add SYS_ADMIN                                                 \
      -e LOCAL_UID=$(id -u)                                               \
      -e LOCAL_GID=$(id -g)                                               \
      -v initium-$(id -u)-$(id -g)-cargo:/usr/local/cargo                 \
      -v initium-$(id -u)-$(id -g)-rustup:/usr/local/rustup               \
      -v $VOLUME:$VOLUME                                                  \
      -w $VOLUME"

if [ "$INTERACTIVE" = true ] ; then
    ARGS="$ARGS -it"
    docker run $ARGS $IMAGE
else
    docker run $ARGS $IMAGE $@
fi
