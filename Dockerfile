FROM ghcr.io/murachue/toolchaincdi
RUN apt update && apt install --no-install-recommends -y make ruby && apt clean
