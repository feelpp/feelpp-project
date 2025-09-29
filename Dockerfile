# -*- mode: Dockerfile -*-

FROM ghcr.io/feelpp/feelpp:v0.111.0-preview.12-noble

ARG APT_CHANNEL=testing
ARG APT_COMPONENT=feelpp-project

USER root

# Install dependencies for fetching the Feel++ APT repository
RUN apt-get update \
	&& apt-get install -y --no-install-recommends curl gnupg ca-certificates \
	&& rm -rf /var/lib/apt/lists/*

# Configure Feel++ APT repository (hosted on GitHub Pages)
RUN curl -fsSL https://feelpp.github.io/apt/feelpp.gpg \
	| gpg --dearmor --yes > /usr/share/keyrings/feelpp.gpg \
	&& chmod 644 /usr/share/keyrings/feelpp.gpg \
	&& printf "deb [signed-by=/usr/share/keyrings/feelpp.gpg] https://feelpp.github.io/apt/%s noble %s\n" "$APT_CHANNEL" "$APT_COMPONENT" \
		> /etc/apt/sources.list.d/feelpp.list

# Install the feelpp-project package from the freshly published APT repository
RUN apt-get update \
	&& apt-get install -y --no-install-recommends ${APT_COMPONENT} \
	&& rm -rf /var/lib/apt/lists/*

USER feelpp

