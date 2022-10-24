# -*- mode: Dockerfile -*-

FROM ghcr.io/feelpp/feelpp:jammy

RUN     echo "export FEELPP_REPOSITORY=/feel" >> ~feelpp/.bashrc;\
    echo "export OMPI_MCA_btl_vader_single_copy_mechanism=none" >> ~feelpp/.bashrc

COPY . /home/feelpp/
RUN ls -lrtR /home/feelpp

RUN dpkg -i /home/feelpp/*.deb 

USER feelpp

